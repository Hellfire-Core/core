/*
 * Copyright (C) 2005-2008 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2017 MaNGOSOne <https://github.com/mangos/one>
 * Copyright (C) 2017 Hellfire <https://hellfire-core.github.io/>
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

#include "ConfusedMovementGenerator.h"

#include "Creature.h"
#include "Player.h"

#include "MoveSplineInit.h"
#include "MoveSpline.h"

template <>
void ConfusedMovementGenerator<Player>::Initialize(Player &unit)
{
    _swimOrFly = (unit.IsInWater() || unit.CanFly());
    _generateMovement(unit);

    unit.InterruptNonMeleeSpells(false);

    unit.StopMoving();
    unit.addUnitState(UNIT_STAT_CONFUSED);
}

template <>
void ConfusedMovementGenerator<Creature>::Initialize(Creature &unit)
{
    _swimOrFly = (unit.CanSwim() || unit.CanFly());
    _generateMovement(unit);

    unit.InterruptNonMeleeSpells(false);

    unit.StopMoving();
    unit.addUnitState(UNIT_STAT_CONFUSED);
}

template<class UNIT>
void ConfusedMovementGenerator<UNIT>::Reset(UNIT &u)
{
    Initialize(u);
}

template<class UNIT>
void ConfusedMovementGenerator<UNIT>::Interrupt(UNIT &unit)
{
    unit.StopMoving();
    unit.ClearUnitState(UNIT_STAT_CONFUSED);
}

template<class UNIT>
void ConfusedMovementGenerator<UNIT>::_generateMovement(UNIT &unit)
{
    if (_swimOrFly)
    {
        for (uint8 idx = 0; idx < MAX_RANDOM_POINTS; ++idx)
        {
            unit.GetPosition(&_randomPosition[idx]);
            float angle = frand(0, 2 * M_PI);
            _randomPosition[idx].m_positionX += WANDER_DISTANCE * cos(angle);
            _randomPosition[idx].m_positionY += WANDER_DISTANCE * sin(angle);
            unit.UpdateAllowedPositionZ(_randomPosition[idx].m_positionX, _randomPosition[idx].m_positionY, _randomPosition[idx].m_positionZ);
        }
        return;
    }
    for (uint8 idx = 0; idx < MAX_RANDOM_POINTS; ++idx)
        unit.GetValidPointInAngle(_randomPosition[idx], WANDER_DISTANCE, frand(0, 2*M_PI), true);
}

template<class UNIT>
bool ConfusedMovementGenerator<UNIT>::Update(UNIT &unit, const uint32 &diff)
{
    unit.SetSelection(0);

    if (_nextMoveTime.Expired(diff) || static_cast<MovementGenerator*>(this)->_recalculateTravel)
    {
        uint32 nextMove = urand(0, MAX_RANDOM_POINTS-1);

        Movement::MoveSplineInit init(unit);
        if (_swimOrFly)
            init.MoveTo(_randomPosition[nextMove].m_positionX, _randomPosition[nextMove].m_positionY, _randomPosition[nextMove].m_positionZ);
        else
        {
            PathFinder path(&unit);
            path.setPathLengthLimit(30.0f);
            bool result = path.calculate(_randomPosition[nextMove].m_positionX, _randomPosition[nextMove].m_positionY, _randomPosition[nextMove].m_positionZ);
            if (!result || path.getPathType() & PATHFIND_NOPATH)
                init.MoveTo(_randomPosition[nextMove].m_positionX, _randomPosition[nextMove].m_positionY, _randomPosition[nextMove].m_positionZ);
            else
                init.MovebyPath(path.getPath());
        }
        init.SetWalk(true);
        init.Launch();

        static_cast<MovementGenerator*>(this)->_recalculateTravel = false;
        _nextMoveTime.Reset(urand(0, 2000));
    }
    return true;
}

template<class UNIT>
void ConfusedMovementGenerator<UNIT>::Finalize(UNIT &unit)
{
    unit.StopMoving();

    unit.ClearUnitState(UNIT_STAT_CONFUSED);
    unit.AddEvent(new AttackResumeEvent(unit), ATTACK_DISPLAY_DELAY);
}

template void ConfusedMovementGenerator<Player>::Interrupt(Player &);
template void ConfusedMovementGenerator<Creature>::Interrupt(Creature &);
template void ConfusedMovementGenerator<Player>::Reset(Player &);
template void ConfusedMovementGenerator<Creature>::Reset(Creature &);
template bool ConfusedMovementGenerator<Player>::Update(Player &, const uint32 &);
template bool ConfusedMovementGenerator<Creature>::Update(Creature &, const uint32 &);
template void ConfusedMovementGenerator<Player>::Finalize(Player &);
template void ConfusedMovementGenerator<Creature>::Finalize(Creature &);
