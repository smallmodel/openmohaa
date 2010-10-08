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
	
	s1 = &cent->currentState;

	memset(&ent,0,sizeof(ent));
//	AnglesToAxis(cent->lerpAngles,ent.axis);
//	VectorCopy(cent->lerpOrigin,ent.origin);
	AnglesToAxis(cent->currentState.angles,ent.axis);
	VectorCopy(cent->currentState.origin,ent.origin);

	ent.hModel = cgs.gameModels[s1->modelindex];
	
	tiki = cgs.gameTIKIs[s1->modelindex];
	if(tiki && tiki->numAnims) {
		ent.bones = trap_TIKI_GetBones(tiki->numBones);
		trap_TIKI_SetChannels(tiki,s1->frameInfo[0].index,s1->frameInfo[0].time,s1->frameInfo[0].weight,ent.bones);
		trap_TIKI_Animate(tiki,ent.bones);
	}

	trap_R_AddRefEntityToScene(&ent);
}

