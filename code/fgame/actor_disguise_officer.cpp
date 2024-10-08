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

// actor_disguise_officier.cpp

#include "actor.h"

void Actor::InitDisguiseOfficer(GlobalFuncs_t *func)
{
    func->ThinkState                 = &Actor::Think_DisguiseOfficer;
    func->BeginState                 = &Actor::Begin_DisguiseOfficer;
    func->EndState                   = &Actor::End_DisguiseOfficer;
    func->ResumeState                = &Actor::Resume_DisguiseOfficer;
    func->SuspendState               = &Actor::Suspend_DisguiseOfficer;
    func->PassesTransitionConditions = &Actor::PassesTransitionConditions_Disguise;
    func->IsState                    = &Actor::IsDisguiseState;
}

void Actor::Begin_DisguiseOfficer(void)
{
    m_csMood = STRING_BORED;
    assert(m_Enemy);

    if (!m_Enemy) {
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
        return;
    }

    if ((EnemyIsDisguised() || (m_Enemy->flags & FL_NOTARGET)) && level.m_bAlarm != qtrue) {
        SetDesiredYawDest(m_Enemy->origin);
        SetDesiredLookDir(m_Enemy->origin - origin);

        DesiredAnimation(ANIM_MODE_NORMAL, STRING_ANIM_DISGUISE_PAPERS_SCR);
        m_iEnemyShowPapersTime = m_Enemy->m_ShowPapersTime;

        TransitionState(ACTOR_STATE_DISGUISE_PAPERS);
    } else {
        SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_IDLE);
    }
}

void Actor::End_DisguiseOfficer(void)
{
    m_iNextDisguiseTime = level.inttime + (m_State ? m_iDisguisePeriod : 500);
}

void Actor::Resume_DisguiseOfficer(void)
{
    Begin_DisguiseOfficer();
}

void Actor::Suspend_DisguiseOfficer(void)
{
    End_DisguiseOfficer();
}

void Actor::Think_DisguiseOfficer(void)
{
    if (!RequireThink()) {
        return;
    }

    UpdateEyeOrigin();
    NoPoint();
    ContinueAnimation();
    UpdateEnemy(1500);

    assert(m_Enemy);

    if (!m_Enemy) {
        SetThinkState(THINKSTATE_IDLE, THINKLEVEL_IDLE);
        return;
    }

    if (!EnemyIsDisguised() && !(m_Enemy->flags & FL_NOTARGET) && m_State != ACTOR_STATE_DISGUISE_ENEMY) {
        TransitionState(ACTOR_STATE_DISGUISE_ENEMY, 0);
    }

    if (level.m_bAlarm == qtrue) {
        SetThinkState(THINKSTATE_ATTACK, THINKLEVEL_IDLE);
        return;
    }

    SetDesiredYawDest(m_Enemy->origin);
    SetDesiredLookDir(m_Enemy->origin - origin);

    switch (m_State) {
    case ACTOR_STATE_DISGUISE_ENEMY:
        m_pszDebugState = "enemy";
        State_Disguise_Enemy();
        break;
    case ACTOR_STATE_DISGUISE_HALT:
        m_pszDebugState = "halt";
        State_Disguise_Halt();
        break;
    case ACTOR_STATE_DISGUISE_PAPERS:
        m_pszDebugState = "papers";
        State_Disguise_Fake_Papers();
        break;
    default:
        {
            Com_Printf("Actor::Think_DisguiseOfficer: invalid think state %i\n", m_State);
            char assertStr[16317] = {0};
            Q_strncpyz(assertStr, "\"invalid think state\"\n\tMessage: ", sizeof(assertStr));
            Q_strcat(assertStr, sizeof(assertStr), DumpCallTrace("thinkstate = %i", m_State));
            assert(!assertStr);
        }
        break;
    }

    if (!CheckForTransition(THINKSTATE_GRENADE, THINKLEVEL_IDLE)) {
        CheckForTransition(THINKSTATE_GRENADE, THINKLEVEL_IDLE);
    }

    PostThink(true);
}
