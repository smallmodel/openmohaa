/*
===========================================================================
Copyright (C) 2011 su44

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
// cg_specialfx.c - MoHAA special effects

#include "cg_local.h"

// MoHAA footstep sounds (from ubersound.scr) :
// snd_step_paper, snd_step_glass, snd_step_wood, snd_step_metal, 
// snd_step_grill, snd_step_stone, snd_step_dirt, snd_step_grass,
// snd_step_mud, snd_step_puddle, snd_step_gravel, snd_step_sand,
// snd_step_foliage, snd_step_snow, snd_step_carpet
// BODY MOVEMENT
// snd_step_equipment - (equipment; for walking)
// snd_step_wade - (wade; for when wading through deeper water)

static void CG_FootstepMain(trace_t *trace, int iRunning, int iEquipment) {
	char *soundNameBase = "snd_step_";
	vec3_t v;
	if(CG_PointContents(trace->endpos,-1) & 0x38 ) {
		v[0] = trace->endpos[0];
		v[1] = trace->endpos[1];
		v[2] = trace->endpos[2] + 16.0;
		if(CG_PointContents(trace->endpos,-1) & 0x38 ) {
			// TODO
		}
	}
	// TODO
}

void CG_MeleeImpact(float *vStart, float *vEnd) {
	trace_t trace;
	float vMins[3] = { -4.0, -4.0, 0 };
	float vMaxs[3] = { 4.0, 4.0, 2.0 };
	CG_Trace(&trace,vStart,vMins,vMaxs,vEnd,1023,MASK_SHOT);
	if(trace.fraction != 1.f) {
		CG_FootstepMain(&trace,1,0);
	}
}


void CG_Footstep(char *szTagName, centity_t *ent, /*refEntity_t *pREnt,*/
	int iRunning, int iEquipment)
{
	int mask;
	int cylinder;
	int cliptoentities;
	trace_t trace;
	vec3_t mins, maxs;
	vec3_t vStart, vEnd, forward;


	CG_Printf("CG_Footstep: %s\n",szTagName);

	vStart[0] = ent->lerpOrigin[0];
	vStart[1] = ent->lerpOrigin[1];
	vStart[2] = ent->lerpOrigin[2] + 8.0;
	if ( szTagName ) {
		tiki_t *tiki;
		int boneName;
		int tagIndex;
		vec3_t a;

		tiki = cgs.gameTIKIs[ent->currentState.modelindex];

		if(tiki == 0) {
			CG_Printf("CG_Footstep: tiki is NULL\n");
			return;
		}	

		// find tag in tiki
		boneName = trap_TIKI_GetBoneNameIndex(szTagName); 
		for(tagIndex = 0; tagIndex < tiki->numBones; tagIndex++) {
			if(tiki->boneNames[tagIndex] == boneName) {
				break;
			}
		}
		if(tagIndex == tiki->numBones) {
			CG_Printf("CG_Footstep: Cant find bone %s in tiki %s f\n",szTagName,tiki->name);
			return;
		}
		if(ent->bones == 0) {
			CG_Printf("CG_Footstep: entity has null bones ptr\n");
			return;
		}	

		CG_CentBoneIndexLocal2World(tagIndex, ent, vStart, a);
		vStart[2] += 8;
	}
	if ( iRunning == -1 ) {
		AngleVectors(ent->lerpAngles, forward, 0, 0);
		vStart[0] = forward[0] * -16.0 + vStart[0];
		vStart[1] = forward[1] * -16.0 + vStart[1];
		vStart[2] = forward[2] * -16.0 + vStart[2];
		vEnd[0] = forward[0] * 64.0 + vStart[0];
		vEnd[1] = forward[1] * 64.0 + vStart[1];
		vEnd[2] = forward[2] * 64.0 + vStart[2];
		mins[0] = -2.0;
		mins[1] = -2.0;
		mins[2] = -8.0;
		maxs[0] = 2.0;
		maxs[1] = 2.0;
		maxs[2] = 8.0;
	} else {
		vEnd[0] = vStart[0];
		vEnd[1] = vStart[1];

		vStart[2] = vStart[2] + 16.0;
		vEnd[2] = vStart[2] - 64.0;

		mins[0] = -4.0;
		mins[1] = -4.0;
		mins[2] = 0.0;

		maxs[0] = 4.0;
		maxs[1] = 4.0;
		maxs[2] = 2.0;
	}
	if ( ent->currentState.eType == ET_PLAYER )	{
		cliptoentities = 1;
		cylinder = 1;
		mask = 1107372801;
	} else {
		cliptoentities = 0;
		cylinder = 0;
		mask = 1107437825;
	}
	CG_Trace(&trace, vStart, mins, maxs, vEnd,
		ent->currentState.number, mask/*, cylinder, cliptoentities*/);
	if ( trace.fraction == 1.0 ) {
		CG_FootstepMain(&trace, iRunning, iEquipment);
	} else {
		//if ( cg_debugFootsteps.integer )
		//	CG_Printf("Footstep: missed floor\n");
	}
}