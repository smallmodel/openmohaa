/*
===========================================================================
Copyright (C) 2012 su44

This file is part of OpenMohaa source code.

OpenMohaa source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMohaa source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMohaa source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// cg_skeletor.c -- MoHAA model skeletor related functions

#include "qcommon.h"

cvar_t *cg_skel_show;
cvar_t *cg_skel_printBoneDirty;

void CG_InitSkeletorCvarsAndCmds() {
	cg_skel_show = cgi.Cvar_Get("cg_skel_show","0",0);
	cg_skel_printBoneDirty = cgi.Cvar_Get("cg_skel_printBoneDirty","0",0);

}

// su44: this is called for *every* entity with valid TIKI and skel pointers
void CG_TIKISkeletor(refEntity_t *ent, skeletor_c *skel) {
	vec3_t end;
	int i;


	if(cg_skel_show->integer) {
		VectorCopy( ent->origin, end );
		end[2] += 50;
		cgi.R_DebugLine( ent->origin, end, 1, 1, 1, 1 );
	}

	if(cg_skel_printBoneDirty->integer) {
		// FIXME! "m_headBoneIndex" obviously shouldnt be used here :)
		for(i = 0; i <= skel->m_headBoneIndex; i++) {
			skelBone_Base_c *b = skel->m_bone[i];

			cgi.Printf("Bone %i of %s dirty %i\n",i,ent->tiki->name,b->m_isDirty);
		}	
	}


}

// su44: we need to access skeletor_c class somehow
void R_AddRefEntityToScene(refEntity_t *ent) {
	skeletor_c *skel;
	
	if(ent->tiki) {
		// su44: to get entity skeletor pointer, we need to know
		// its TIKI pointer and entityNumber (the one send 
		// through entityState_t). Bad things happen if TIKI
		// pointer passed to TIKI_GetSkeletor is NULL.
		skel = cgi.TIKI_GetSkeletor(ent->tiki,ent->entityNumber);
		// got it
		CG_TIKISkeletor(ent,skel);
	} else {
		skel = 0;
	}

	cgi.R_AddRefEntityToScene(ent);
}


