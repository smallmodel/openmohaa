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

// DESCRIPTION:
// cg_servercmds.c -- text commands sent by the server

#include "cg_local.h"

static const char* IsWeaponAllowed(int dmFlags, int flags) {
    return (dmFlags & flags) ? "0" : "1";
}

static qboolean QueryLandminesAllowed2(const char* mapname, int dmflags) {
    if (dmflags & DF_WEAPON_NO_LANDMINE) {
        return qfalse;
    }

    if (dmflags & DF_WEAPON_LANDMINE_ALWAYS) {
        return qtrue;
    }

	if (!Q_stricmpn(mapname, "obj/obj_", 8u))
		return qfalse;
	if (!Q_stricmpn(mapname, "dm/mohdm", 8u))
		return qfalse;
	if (!Q_stricmp(mapname, "DM/MP_Bahnhof_DM"))
		return qfalse;
	if (!Q_stricmp(mapname, "obj/MP_Ardennes_TOW"))
		return qfalse;
	if (!Q_stricmp(mapname, "DM/MP_Bazaar_DM"))
		return qfalse;
	if (!Q_stricmp(mapname, "obj/MP_Berlin_TOW"))
		return qfalse;
	if (!Q_stricmp(mapname, "DM/MP_Brest_DM"))
		return qfalse;
	if (!Q_stricmp(mapname, "obj/MP_Druckkammern_TOW"))
		return qfalse;
	if (!Q_stricmp(mapname, "DM/MP_Gewitter_DM"))
		return qfalse;
	if (!Q_stricmp(mapname, "obj/MP_Flughafen_TOW"))
		return qfalse;
	if (!Q_stricmp(mapname, "DM/MP_Holland_DM"))
		return qfalse;
	if (!Q_stricmp(mapname, "DM/MP_Malta_DM"))
		return qfalse;
	if (!Q_stricmp(mapname, "DM/MP_Stadt_DM"))
		return qfalse;
	if (!Q_stricmp(mapname, "DM/MP_Unterseite_DM"))
		return qfalse;
    if (!Q_stricmp(mapname, "DM/MP_Verschneit_DM"))
        return qfalse;
    if (!Q_stricmp(mapname, "lib/mp_ship_lib"))
        return qfalse;
    if (!Q_stricmp(mapname, "DM/MP_Verschneit_DM"))
        return qfalse;
    if (!Q_stricmp(mapname, "lib/mp_ship_lib"))
        return qfalse;
	return qtrue;
}

/*
================
CG_ParseServerinfo

This is called explicitly when the gamestate is first received,
and whenever the server updates any serverinfo flagged cvars
================
*/
void CG_ParseServerinfo(void)
{
    const char* info;
    const char* mapname;
    char map[MAX_QPATH];
    char* spawnpos;
    const char* version;

    info = CG_ConfigString(CS_SERVERINFO);
    cgs.gametype = atoi(Info_ValueForKey(info, "g_gametype"));
    cgs.dmflags = atoi(Info_ValueForKey(info, "dmflags"));
    cgs.teamflags = atoi(Info_ValueForKey(info, "teamflags"));
    cgs.fraglimit = atoi(Info_ValueForKey(info, "fraglimit"));
    cgs.timelimit = atoi(Info_ValueForKey(info, "timelimit"));
    cgs.maxclients = atoi(Info_ValueForKey(info, "sv_maxclients"));

    version = Info_ValueForKey(info, "version");
    if (strstr(version, "Spearhead")) {
        cgi.Cvar_Set("g_servertype", "1");
    } else {
		cgi.Cvar_Set("g_servertype", "2");
    }

    cgi.Cvar_Set("cg_gametype", Info_ValueForKey(info, "g_gametype"));
    cgi.Cvar_Set("cg_fraglimit", Info_ValueForKey(info, "fraglimit"));
    cgi.Cvar_Set("cg_timelimit", Info_ValueForKey(info, "timelimit"));
    cgi.Cvar_Set("cg_maxclients", Info_ValueForKey(info, "sv_gametype"));
    cgi.Cvar_Set("cg_obj_alliedtext1", Info_ValueForKey(info, "g_obj_alliedtext1"));
    cgi.Cvar_Set("cg_obj_alliedtext2", Info_ValueForKey(info, "g_obj_alliedtext2"));
    cgi.Cvar_Set("cg_obj_alliedtext3", Info_ValueForKey(info, "g_obj_alliedtext3"));
    cgi.Cvar_Set("cg_obj_axistext1", Info_ValueForKey(info, "cg_obj_axistext1"));
    cgi.Cvar_Set("cg_obj_axistext2", Info_ValueForKey(info, "cg_obj_axistext2"));
    cgi.Cvar_Set("cg_obj_axistext3", Info_ValueForKey(info, "cg_obj_axistext3"));
    cgi.Cvar_Set("cg_scoreboardpic", Info_ValueForKey(info, "g_scoreboardpic"));
    cgi.Cvar_Set("cg_scoreboardpicover", Info_ValueForKey(info, "g_scoreboardpicover"));
    cgs.mapChecksum = atoi(Info_ValueForKey(info, "sv_mapChecksum"));

    mapname = Info_ValueForKey(info, "mapname");

    cgi.Cvar_Set("cg_weapon_rifle", IsWeaponAllowed(cgs.dmflags, DF_WEAPON_NO_RIFLE));
    cgi.Cvar_Set("cg_weapon_sniper", IsWeaponAllowed(cgs.dmflags, DF_WEAPON_NO_SNIPER));
    cgi.Cvar_Set("cg_weapon_mg", IsWeaponAllowed(cgs.dmflags, DF_WEAPON_NO_MG));
    cgi.Cvar_Set("cg_weapon_smg", IsWeaponAllowed(cgs.dmflags, DF_WEAPON_NO_SMG));
    cgi.Cvar_Set("cg_weapon_rocket", IsWeaponAllowed(cgs.dmflags, DF_WEAPON_NO_ROCKET));
    cgi.Cvar_Set("cg_weapon_shotgun", IsWeaponAllowed(cgs.dmflags, DF_WEAPON_NO_SHOTGUN));
    cgi.Cvar_Set("cg_weapon_landmine", QueryLandminesAllowed2(mapname, cgs.dmflags) ? "1" : "0");

    spawnpos = strchr(mapname, '$');
    if (spawnpos) {
        Q_strncpyz(map, mapname, spawnpos - mapname + 1);
    } else {
        strcpy(map, mapname);
    }

    Com_sprintf(cgs.mapname, sizeof(cgs.mapname), "maps/%s.bsp", map);

    // hide/show huds
    if (cgs.gametype)
    {
        cgi.Cmd_Execute(EXEC_NOW, "ui_addhud hud_timelimit\n");
        if (cgs.fraglimit)
        {
            cgi.Cmd_Execute(EXEC_NOW, "ui_addhud hud_fraglimit\n");
            cgi.Cmd_Execute(EXEC_NOW, "ui_removehud hud_score\n");
        }
        else
        {
            cgi.Cmd_Execute(EXEC_NOW, "ui_addhud hud_score\n");
            cgi.Cmd_Execute(EXEC_NOW, "ui_removehud hud_fraglimit\n");
        }
    }
    else
    {
        cgi.Cmd_Execute(EXEC_NOW, "ui_removehud hud_timelimit\n");
        cgi.Cmd_Execute(EXEC_NOW, "ui_removehud hud_fraglimit\n");
        cgi.Cmd_Execute(EXEC_NOW, "ui_removehud hud_score\n");
    }
}

/*
================
CG_ConfigStringModified

================
*/
static void CG_ConfigStringModified(void)
{
    int num;

    num = atoi(cgi.Argv(1));

    // get the gamestate from the client system, which will have the
    // new configstring already integrated
    cgi.GetGameState(&cgs.gameState);

    CG_ProcessConfigString(num);
}

/*
================
CG_ParseStats

================
*/
static void CG_ParseStats()
{
    cgi.Cvar_Set("ui_NumObjectives", cgi.Argv(1));
    cgi.Cvar_Set("ui_NumComplete", cgi.Argv(2));
    cgi.Cvar_Set("ui_NumShotsFired", cgi.Argv(3));
    cgi.Cvar_Set("ui_NumHits", cgi.Argv(4));
    cgi.Cvar_Set("ui_Accuracy", cgi.Argv(5));
    cgi.Cvar_Set("ui_PreferredWeapon", cgi.Argv(6));
    cgi.Cvar_Set("ui_NumHitsTaken", cgi.Argv(7));
    cgi.Cvar_Set("ui_NumObjectsDestroyed", cgi.Argv(8));
    cgi.Cvar_Set("ui_NumEnemysKilled", cgi.Argv(9));
    cgi.Cvar_Set("ui_HeadShots", cgi.Argv(10));
    cgi.Cvar_Set("ui_TorsoShots", cgi.Argv(11));
    cgi.Cvar_Set("ui_LeftLegShots", cgi.Argv(12));
    cgi.Cvar_Set("ui_RightLegShots", cgi.Argv(13));
    cgi.Cvar_Set("ui_GroinShots", cgi.Argv(14));
    cgi.Cvar_Set("ui_LeftArmShots", cgi.Argv(15));
    cgi.Cvar_Set("ui_RightArmShots", cgi.Argv(16));
    cgi.Cvar_Set("ui_GunneryEvaluation", cgi.Argv(17));
    cgi.Cvar_Set("ui_gotmedal", cgi.Argv(18));
    cgi.Cvar_Set("ui_success", cgi.Argv(19));
    cgi.Cvar_Set("ui_failed", cgi.Argv(20));
}

/*
================
CG_Stopwatch_f

================
*/
static void CG_Stopwatch_f()
{
    if (cgi.Argc() < 3) {
        Com_Error(ERR_DROP, "stopwatch didn't have 2 parameters");
    }

    if (cgi.protocol >= PROTOCOL_MOHTA_MIN) {
        cgi.stopWatch->iStartTime = atoi(cgi.Argv(1));
    } else {
        // The base game has it wrong
        cgi.stopWatch->iStartTime = 1000 * atoi(cgi.Argv(1));
    }

    cgi.stopWatch->iEndTime = cgi.stopWatch->iStartTime + 1000 * atoi(cgi.Argv(2));
}

/*
================
CG_ServerLag_f

================
*/
static void CG_ServerLag_f()
{
    cgs.serverLagTime = cg.time;
}

/*
=================
CG_ServerCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
static void CG_ServerCommand(void)
{
    const char* cmd;

    cmd = cgi.Argv(0);

    if (!cmd[0]) {
        // server claimed the command
        return;
    }

    if (!strcmp(cmd, "cs")) {
        CG_ConfigStringModified();
        return;
    }

    if (!strcmp(cmd, "print") || !strcmp(cmd, "hudprint")) {
        cgi.Printf("%s", cgi.Argv(1));
        if (!strcmp(cmd, "hudprint")) {
            CG_HudPrint_f();
        }
        return;
    }
    else if (!strcmp(cmd, "printdeathmsg")) {
        const char* s1, * s2, * s3, * s4, * s5;
        const char* result1, * result2;
        int hudColor;

		result1 = NULL;
		result2 = NULL;
		s1 = cgi.Argv(1);
		s2 = cgi.Argv(2);
		s3 = cgi.Argv(3);
		s4 = cgi.Argv(4);
		s5 = cgi.Argv(5);

        if (*s5 == tolower(*s5)) {
            hudColor = 4;
        } else {
			hudColor = 5;
        }

		if (*s1 != 'x')
			result1 = cgi.LV_ConvertString(s1);
		if (*s2 != 'x')
			result2 = cgi.LV_ConvertString(s2);
		if (tolower(*s5) == 's')
		{
			cgi.Printf("%c%s %s\n", hudColor, s4, result1);
		}
		else if (tolower(*s5) == 'p')
		{
			if (*s2 == 'x')
				cgi.Printf("%c%s %s %s\n", hudColor, s4, result1, s3);
			else
				cgi.Printf("%c%s %s %s%s\n", hudColor, s4, result1, s3, result2);
		}
		else if (tolower(*s5) == 'w')
		{
			cgi.Printf("%c%s %s\n", hudColor, s4, result1);
		}
		else
		{
			cgi.Printf("%s", cgi.Argv(1));
		}
        return;
    }
    
    if (!strcmp(cmd, "stufftext")) {
        cgi.Cmd_Stuff(cgi.Argv(1));
        cgi.Cmd_Stuff("\n");
        return;
    }

    if (!strcmp(cmd, "scores")) {
        CG_ParseScores();
        return;
    }

    if (!strcmp(cmd, "stats")) {
        CG_ParseStats();
        return;
    }

    if (!strcmp(cmd, "stopwatch")) {
        CG_Stopwatch_f();
        return;
    }

    if (!strcmp(cmd, "svlag")) {
        CG_ServerLag_f();
        return;
    }

    cgi.Printf("Unknown client game command: %s\n", cmd);
}

/*
====================
CG_ExecuteNewServerCommands

Execute all of the server commands that were received along
with this this snapshot.
====================
*/
void CG_ExecuteNewServerCommands(int latestSequence, qboolean differentServer)
{
    while (cgs.serverCommandSequence < latestSequence) {
        if (cgi.GetServerCommand(++cgs.serverCommandSequence, differentServer)) {
            CG_ServerCommand();
        }
    }
}
