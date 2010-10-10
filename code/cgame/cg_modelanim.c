/*
===========================================================================
Copyright (C) 2009-2010 su44

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

#include "cg_local.h"

void CG_ModelAnim( centity_t *cent ) {
	refEntity_t ent;
	entityState_t *s1;
	tiki_t *tiki;
	int i;
	
	s1 = &cent->currentState;

	memset(&ent,0,sizeof(ent));

	// player model
	if ( cent->currentState.number == cg.snap->ps.clientNum) {
		if (!cg.renderingThirdPerson) {
			ent.renderfx = RF_THIRD_PERSON;			// only draw in mirrors
		} else {
			if (cg_cameraMode.integer) {
				return;
			}
		}
	}


//	AnglesToAxis(cent->lerpAngles,ent.axis);
//	VectorCopy(cent->lerpOrigin,ent.origin);
	AnglesToAxis(cent->currentState.angles,ent.axis);
	VectorCopy(cent->currentState.origin,ent.origin);

	ent.hModel = cgs.gameModels[s1->modelindex];
	
	tiki = cgs.gameTIKIs[s1->modelindex];
	if(tiki && tiki->numAnims) {
		int idleIndex = 0;
		ent.bones = trap_TIKI_GetBones(tiki->numBones);

#if 0
		for(i = 0; i < tiki->numAnims; i++) {
			if(!Q_stricmp(tiki->anims[i]->alias,"unarmed_stand_idle")) {
			idleIndex = i;
				break;
			}
		}
#endif
		if(idleIndex!=0) {
			trap_TIKI_SetChannels(tiki,idleIndex,0,0,ent.bones);
		}
		else {
			frameInfo_t *fi = s1->frameInfo;
			for(i = 0; i < 16; i++)	{
				if(fi->weight!=0)
					trap_TIKI_SetChannels(tiki,fi->index,fi->time,fi->weight,ent.bones);
				fi++;
			}
		}
		trap_TIKI_Animate(tiki,ent.bones);
#if 0
		if(idleIndex!=0) {
			for(i = 0; i < tiki->numBones; i++)	{
				Com_Printf("bone %i of %i - %f %f %f\n",i,tiki->numBones,ent.bones[i].p[0],ent.bones[i].p[1],ent.bones[i].p[2]);
			}
		}
#endif
	}

	trap_R_AddRefEntityToScene(&ent);
}

