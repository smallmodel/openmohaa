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

#include "Tow_Entities.h"
#include "player.h"
#include "scriptthread.h"

TOWObjectiveMan g_TOWObjectiveMan;

TOWObjectiveMan::TOWObjectiveMan()
{
    Reset();
}

void TOWObjectiveMan::Reset()
{
    m_towObjectives.ClearObjectList();
    m_iNumAxisObjectives   = 0;
    m_iNumAlliesObjectives = 0;
}

void TOWObjectiveMan::AddObjective(TOWObjective *objective)
{
    if (!objective) {
        return;
    }

    switch (objective->GetController()) {
    case CONTROLLER_ALLIES:
        m_iNumAlliesObjectives++;
        break;
    case CONTROLLER_AXIS:
        m_iNumAxisObjectives++;
        break;
    }

    m_towObjectives.AddObject(objective);
}

void TOWObjectiveMan::TakeOverObjective(eController controller, eController controllingTeam)
{
    int i;

    if (controllingTeam == CONTROLLER_DRAW) {
        return;
    }

    switch (controllingTeam) {
    case CONTROLLER_ALLIES:
        m_iNumAlliesObjectives++;
        if (m_iNumAxisObjectives) {
            m_iNumAxisObjectives--;
        }
        dmManager.CheckEndMatch();
        break;
    case CONTROLLER_AXIS:
        m_iNumAxisObjectives++;
        if (m_iNumAlliesObjectives) {
            m_iNumAlliesObjectives--;
        }
        dmManager.CheckEndMatch();
        break;
    }

    for (i = 0; i < game.maxclients; i++) {
        gentity_t *ent = &g_entities[i];
        Player    *player;
        bool       bSameTeam;

        if (!ent->inuse || !ent->entity) {
            continue;
        }

        player = static_cast<Player *>(ent->entity);

        switch (controllingTeam) {
        case CONTROLLER_ALLIES:
            bSameTeam = player->GetTeam() == TEAM_ALLIES;
            break;
        case CONTROLLER_AXIS:
            bSameTeam = player->GetTeam() == TEAM_AXIS;
            break;
        }

        // play a different sound for the losing team
        if (bSameTeam) {
            player->PlayInstantMessageSound("success");
        } else {
            player->PlayInstantMessageSound("failure");
        }
    }
}

Event EV_SetController("ControlledBy", EV_DEFAULT, "i", "ControlledBy", "Sets the team controlling the objective");
Event EV_TakeOver(
    "TakeOver", EV_DEFAULT, "i", "TeamNum", "Sets the team controlling the objective. 0 = Axis, 1 = Allies"
);
Event EV_SetAxisObjNum("AxisObjNum", EV_DEFAULT, "i", "AxisObjNum", "Sets the objective number for the axis team");
Event EV_SetAlliesObjNum(
    "AlliesObjNum", EV_DEFAULT, "i", "AlliesObjNum", "Sets the objective number for the allies team"
);
static Event EV_Initialize("initialize", EV_DEFAULT, 0, 0, "Initialize object");
static Event EV_SetCurrent(
    "SetCurrent", EV_DEFAULT, "i", "TeamNum", "Set this objective as the current objective for the specified team."
);
Event EV_GetController("ControlledBy", EV_DEFAULT, 0, 0, "Objective controller", EV_GETTER);

CLASS_DECLARATION(Objective, TOWObjective, "func_towobjective") {
    {&EV_SetController,   &TOWObjective::SetController  },
    {&EV_TakeOver,        &TOWObjective::TakeOver       },
    {&EV_SetAxisObjNum,   &TOWObjective::SetAxisObjNum  },
    {&EV_SetAlliesObjNum, &TOWObjective::SetAlliesObjNum},
    {&EV_Initialize,      &TOWObjective::OnInitialize   },
    {&EV_SetCurrent,      &TOWObjective::SetCurrent     },
    {&EV_GetController,   &TOWObjective::GetController  },
    {NULL,                NULL                          }
};

TOWObjective::TOWObjective()
{
    m_eController          = CONTROLLER_DRAW;
    m_eLeadingController   = CONTROLLER_DRAW;
    m_bNoRespawnForLeading = false;
    m_iAxisObjNum          = 1;
    m_iAlliesObjNum        = 1;
    m_sAxisObjName         = "default";
    m_sAlliesObjName       = "default";

    if (spawnflags & TOWOBJECTIVE_SPAWNFLAG_NO_RESPAWN) {
        m_bNoRespawnForLeading = true;
    }

    PostEvent(EV_Initialize, 0.001f);
}

void TOWObjective::SetController(Event *ev)
{
    m_eController = static_cast<eController>(ev->GetInteger(1));

    if (m_bNoRespawnForLeading) {
        m_eLeadingController = m_eController;
    }

    if (m_eController != CONTROLLER_ALLIES && m_eController != CONTROLLER_AXIS) {
        m_eController = CONTROLLER_DRAW;
    }
}

void TOWObjective::GetController(Event *ev)
{
    ev->AddInteger(m_eController);
}

void TOWObjective::TakeOver(Event *ev)
{
    eController controller = static_cast<eController>(ev->GetInteger(1));
    if (controller == m_eController || controller == CONTROLLER_DRAW) {
        return;
    }

    switch (controller) {
    case CONTROLLER_ALLIES:
        gi.Cvar_Set(m_sAxisObjName.c_str(), "1");
        gi.Cvar_Set(m_sAlliesObjName.c_str(), "0");
        break;
    case CONTROLLER_AXIS:
        gi.Cvar_Set(m_sAxisObjName.c_str(), "0");
        gi.Cvar_Set(m_sAlliesObjName.c_str(), "1");
        break;
    }

    g_TOWObjectiveMan.TakeOverObjective(m_eController, controller);

    if (m_bNoRespawnForLeading) {
        //
        // Disable team respawn for the leading team
        //
        dmManager.StopTeamRespawn(m_eLeadingController);
    }
}

void TOWObjective::SetAlliesObjNum(Event *ev)
{
    m_iAlliesObjNum = ev->GetInteger(1);
}

void TOWObjective::SetAxisObjNum(Event *ev)
{
    m_iAxisObjNum = ev->GetInteger(1);
}

void TOWObjective::OnInitialize(Event *ev)
{
    m_sAlliesObjName = "tow_allied_obj" + m_iAlliesObjNum;
    m_sAxisObjName   = "tow_allied_obj" + m_iAxisObjNum;

    switch (m_eController) {
    case CONTROLLER_ALLIES:
        gi.Cvar_Set(m_sAxisObjName.c_str(), "1");
        gi.Cvar_Set(m_sAlliesObjName.c_str(), "0");
        break;
    case CONTROLLER_AXIS:
        gi.Cvar_Set(m_sAxisObjName.c_str(), "0");
        gi.Cvar_Set(m_sAlliesObjName.c_str(), "1");
        break;
    case CONTROLLER_DRAW:
        gi.Cvar_Set(m_sAxisObjName.c_str(), "0");
        gi.Cvar_Set(m_sAlliesObjName.c_str(), "0");
        break;
    }

    if (!LoadingSavegame) {
        g_TOWObjectiveMan.AddObjective(this);
    }
}

void TOWObjective::SetCurrent(Event *ev)
{
    eController controller = static_cast<eController>(ev->GetInteger(1));
    switch (controller) {
    case CONTROLLER_ALLIES:
        ScriptThread::SetCurrentObjective(GetObjectiveIndex(), TEAM_ALLIES);
        break;
    case CONTROLLER_AXIS:
        ScriptThread::SetCurrentObjective(GetObjectiveIndex(), TEAM_AXIS);
        break;
    default:
        SetCurrent(ev);
        break;
    }
}

int TOWObjective::GetController() const
{
    return m_eController;
}

void TOWObjective::Archive(Archiver& arc)
{
    arc.ArchiveInteger(&m_iAxisObjNum);
    arc.ArchiveInteger(&m_iAlliesObjNum);
    ArchiveEnum(m_eController, eController);
    ArchiveEnum(m_eLeadingController, eController);
    arc.ArchiveBool(&m_bNoRespawnForLeading);
    arc.ArchiveString(&m_sAxisObjName);
    arc.ArchiveString(&m_sAlliesObjName);
}
