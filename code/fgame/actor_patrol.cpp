/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

This file is part of OpenMoHAA source code.

OpenMoHAA source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMoHAA source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMoHAA source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// actor_patrol.cpp

#include "actor.h"

void Actor::InitPatrol(GlobalFuncs_t *func)
{
    func->ThinkState                 = &Actor::Think_Patrol;
    func->BeginState                 = &Actor::Begin_Patrol;
    func->EndState                   = &Actor::End_Patrol;
    func->ResumeState                = &Actor::Resume_Patrol;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Idle;
    func->ShowInfo                   = &Actor::ShowInfo_Patrol;
    func->IsState                    = &Actor::IsIdleState;
}

void Actor::Begin_Patrol(void)
{
    m_csMood = STRING_BORED;
    StopTurning();
    ClearPath();
}

void Actor::End_Patrol(void)
{
    parm.movefail = true;
}

void Actor::Resume_Patrol(void) {}

void Actor::Think_Patrol(void)
{
    bool bMoveInRadius;

    if (!RequireThink()) {
        return;
    }

    parm.movefail = false;

    UpdateEyeOrigin();
    NoPoint();

    m_pszDebugState       = "";
    m_csPatrolCurrentAnim = STRING_ANIM_PATROL_SCR;
    bMoveInRadius         = MoveToPatrolCurrentNode();

    if (m_fLookAroundFov > 1.0) {
        LookAround(m_fLookAroundFov);
    }

    CheckForThinkStateTransition();

    if (!m_patrolCurrentNode) {
        SetThinkIdle(THINK_IDLE);
        m_bScriptGoalValid = false;

        parm.movedone = true;
        Unregister(STRING_MOVEDONE);
    } else if (bMoveInRadius) {
        ClearPatrolCurrentNode();
        SetThinkIdle(THINK_IDLE);
        // Added in OPM
        //  Not sure why it was not cleared originally.
        //  Once you reach your goal, there is no point
        //  in leaving it set to true
        m_bScriptGoalValid = false;

        parm.movedone = true;
        Unregister(STRING_MOVEDONE);
    }

    PostThink(true);
}

void Actor::ShowInfo_Patrol(void)
{
    ShowInfo_PatrolCurrentNode();
}
