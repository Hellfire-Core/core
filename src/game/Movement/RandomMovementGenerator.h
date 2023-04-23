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

#ifndef _RANDOMMOVEMENTGENERATOR_H
#define _RANDOMMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

template<class T>
class RandomMovementGenerator : public MovementGeneratorMedium< T, RandomMovementGenerator<T> >
{
    public:
        explicit RandomMovementGenerator(const Unit &) : i_nextMoveTime(0), i_wanderDistance(5.0f), i_wanderSteps(0) {}
        explicit RandomMovementGenerator(float dist) : i_nextMoveTime(0), i_wanderDistance(dist), i_wanderSteps(0) {}

        void _setRandomLocation(T &);
        void Initialize(T &);
        void Finalize(T &);
        void Interrupt(T &);
        void Reset(T &);
        bool Update(T &, const uint32 &);

        const char* Name() const { return "<Random>"; }
        MovementGeneratorType GetMovementGeneratorType() const { return RANDOM_MOTION_TYPE; }

        bool GetResetPosition(T&, float& x, float& y, float& z);

    private:
        TimeTrackerSmall i_nextMoveTime;
        uint32 i_nextMove;
        float i_wanderDistance;
        uint8 i_wanderSteps;
};

#endif
