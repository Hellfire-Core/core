/*
 * Copyright (C) 2005-2008 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2008-2015 Hellground <http://hellground.net/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef HELLGROUND_TIMER_H
#define HELLGROUND_TIMER_H

#include "Common.h"
#include "Log.h"
#include <ace/OS_NS_sys_time.h>

class WorldTimer
{
    public:
        // Get current server time
        static uint32 getMSTime();

        // Get time difference between two timestamps
        static inline uint32 getMSTimeDiff(const uint32& oldMSTime, const uint32& newMSTime)
        {
            if (oldMSTime > newMSTime)
            {
                const uint32 diff_1 = (uint32(0xFFFFFFFF) - oldMSTime) + newMSTime;
                const uint32 diff_2 = oldMSTime - newMSTime;

                return std::min(diff_1, diff_2);
            }

            return newMSTime - oldMSTime;
        }

        // Get time difference between oldMSTime and current server time
        static inline uint32 getMSTimeDiffToNow(const uint32& oldMSTime)
        {
            return getMSTimeDiff(oldMSTime, WorldTimer::getMSTime());
        }

        // Get last world tick time
        static uint32 tickTime();
        // Get previous world tick time
        static uint32 tickPrevTime();
        // Tick world timer
        static uint32 tick();
        // renew tickTime after sleep
        static void tickTimeRenew();

    private:
        WorldTimer();
        WorldTimer(const WorldTimer& );

        // Analogue to WorldTimer::getMSTime() but it persists m_SystemTickTime
        static uint32 getMSTime_internal();

        static uint32 m_iTime;
        static uint32 m_iPrevTime;
};

class IntervalTimer
{
    public:
        IntervalTimer() : _interval(0), _current(0) {}

        void Update(time_t diff)
        {
            _current += diff;
            if (_current < 0)
                _current = 0;
        }

        bool Passed() { return _current >= _interval; }
        void Reset()
        {
            if (_current >= _interval)
                _current -= _interval;
        }

        void SetCurrent(time_t current) { _current = current; }
        void SetInterval(time_t interval) { _interval = interval; }
        time_t GetInterval() const { return _interval; }
        time_t GetCurrent() const { return _current; }

    private:
        time_t _interval;
        time_t _current;
};

struct ShortIntervalTimer
{
    uint32 _interval;
    uint32 _current;

    ShortIntervalTimer() : _interval(0), _current(0) {}
    ShortIntervalTimer(const uint32 interval) : _interval(interval), _current(0) {}

    void Delay(const uint32 delay)
    {
        _interval += delay;
    }

    bool Expired(const uint32 diff)
    {
        Update(diff);
        return Passed();
    }

    bool Passed() const
    {
        return _interval != 0 ? _current >= _interval : false;
    }

    void Reset(const uint32 interval)
    {
        SetCurrent(0);
        SetInterval(interval);
    }

    void Reschedule(const uint32 interval)
    {
        if (interval == 0)
            _current = 0;
        else if (_current >= _interval)
            _current -= _interval;

        SetInterval(interval);
    }

    void Update(const uint32 diff)
    {
        if (_interval != 0)
            _current += diff;
    }

    uint32 GetInterval() const { return _interval; }
    uint32 GetCurrent() const { return _current; }
    uint32 GetTimeLeft() const { return _interval > _current ? _interval - _current : 0; }

    void SetCurrent(const uint32 current) { _current = current; }
    void SetInterval(const uint32 interval) { _interval = interval; }

    const uint32 operator=(const uint32 interval)
    {
        Reschedule(interval);
        return interval;
    }
};

struct TimeTracker
{
    public:
        TimeTracker(time_t expiry) : i_expiryTime(expiry) {}
        void Update(time_t diff) { i_expiryTime -= diff; }
        bool Passed(void) const { return (i_expiryTime <= 0); }
        void Reset(time_t interval) { i_expiryTime = interval; }
        time_t GetExpiry(void) const { return i_expiryTime; }

    private:
        time_t i_expiryTime;
};

struct TimeTrackerSmall
{
    public:
        TimeTrackerSmall(uint32 expiry =0) : i_expiryTime(expiry) {}
        void Update(uint32 diff) { diff < i_expiryTime ? i_expiryTime -= diff : i_expiryTime = 0; }
        bool Passed(void) const { return (i_expiryTime <= 0); }
        void Reset(uint32 interval) { i_expiryTime = interval; }
        bool Expired(uint32 diff) { Update(diff); return Passed(); }
        int32 GetExpiry(void) const { return i_expiryTime; }

    private:
        uint32 i_expiryTime;
};

// for periodic events
typedef ShortIntervalTimer Timer;
// for single events
typedef TimeTrackerSmall Countdown;
typedef TimeTrackerSmall ShortTimeTracker;

struct PeriodicTimer
{
    public:
        PeriodicTimer(int32 period, int32 start_time) : i_expireTime(start_time), i_period(period) {}

        bool Update(const uint32 &diff)
        {
            if ((i_expireTime -= diff) > 0)
                return false;

            i_expireTime += i_period > diff ? i_period : diff;
            return true;
        }

        void SetPeriodic(int32 period, int32 start_time)
        {
            i_expireTime = start_time;
            i_period = period;
        }

        // Tracker interface
        void TUpdate(int32 diff) { i_expireTime -= diff; }
        bool TPassed() const { return i_expireTime <= 0; }
        void TReset(int32 diff, int32 period)  { i_expireTime += period > diff ? period : diff; }

    private:
        int32 i_period;
        int32 i_expireTime;
};

class WorldUpdateCounter
{
    public:
        WorldUpdateCounter() : m_tmStart(0) {}

        time_t timeElapsed()
        {
            if (!m_tmStart)
                m_tmStart = WorldTimer::tickPrevTime();

            return WorldTimer::getMSTimeDiff(m_tmStart, WorldTimer::tickTime());
        }

        void Reset() { m_tmStart = WorldTimer::tickTime(); }

    private:
        uint32 m_tmStart;
};

class DiffRecorder
{
    public:
        DiffRecorder(uint32 treshold = 0)
        {
            _diffTresholdForFile = treshold;

            _prevTime = WorldTimer::getMSTime();
        }

        inline bool RecordTimeFor(char const* str, uint32 shorter = 0)
        {
            if (!shorter) shorter = _diffTresholdForFile;
            if (!shorter) return false;
            uint32 diffTime = WorldTimer::getMSTimeDiffToNow(_prevTime);

            _prevTime = WorldTimer::getMSTime();

            if (diffTime > shorter)
            {
                sLog.outLog(LOG_DIFF, "%s [diff: %u].", str, diffTime);
                return true;
            }
            return false;
        }

        inline void reset()
        {
            _prevTime = WorldTimer::getMSTime();
        }

    private:
        uint32 _prevTime;
        uint32 _diffTresholdForFile;
};

#endif
