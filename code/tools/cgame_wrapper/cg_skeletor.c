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
cvar_t *cg_skel_printBonePos;
cvar_t *cg_skel_drawBones;
cvar_t *cg_skel_su44;
cvar_t *cheats;

void CG_InitSkeletorCvarsAndCmds() {
	cg_skel_show = cgi.Cvar_Get("cg_skel_show","0",0);
	cg_skel_printBoneDirty = cgi.Cvar_Get("cg_skel_printBoneDirty","0",0);
	cg_skel_printBonePos = cgi.Cvar_Get("cg_skel_printBonePos","0",0);
	cg_skel_drawBones = cgi.Cvar_Get("cg_skel_drawBones","0",0);
	cg_skel_su44 = cgi.Cvar_Get("cg_skel_su44","0",0);
	// hack, force some cvars to their usefull values
	cheats = cgi.Cvar_Get("cheats","0",0);
	cheats->integer = 1;
	cheats = cgi.Cvar_Get("sv_cheats","0",0);
	cheats->integer = 1;
	cheats = cgi.Cvar_Get("thereisnomonkey","0",0);
	cheats->integer = 1;
}

typedef SkelMat4_t *(__stdcall *GetTransformFunc)(skelBone_Base_c *bone, skelAnimStoreFrameList_c *channels);

int CG_Skel_BoneIndexForPointer(skelBone_Base_c *bone, skeletor_c *skel) {
	int i;

	for(i = 0; i < skel->m_Tiki->m_boneList.m_numChannels; i++) {
		skelBone_Base_c *b;

		b = skel->m_bone[i];

		if(b == bone)
			return i;
	}
	return -1;
}
// su44: this is called for *every* entity with valid TIKI and skel pointers
qboolean CG_TIKISkeletor(refEntity_t *ent, skeletor_c *skel) {
	vec3_t end;
	int i, j;


	if(cg_skel_show->integer) {
		CG_MakeCross( ent->origin );
	}

	if(cg_skel_printBoneDirty->integer) {
		for(i = 0; i < skel->m_Tiki->m_boneList.m_numChannels; i++) {
			skelBone_Base_c *b = skel->m_bone[i];

			cgi.Printf("Bone %i of %i, name %s, model %s dirty %i\n",
				i,skel->m_Tiki->m_boneList.m_numChannels,cgi.Tag_NameForNum(skel->m_Tiki,i),
				ent->tiki->name,b->m_isDirty);
		}	
	}

	if(cg_skel_printBonePos->integer) {
		for(i = 0; i < skel->m_Tiki->m_boneList.m_numChannels; i++) {
			skelBone_Base_c *b = skel->m_bone[i];

			cgi.Printf("Bone %i of %i, name %s, model %s pos %f %f %f\n",
				i,skel->m_Tiki->m_boneList.m_numChannels,cgi.Tag_NameForNum(skel->m_Tiki,i),
				ent->tiki->name,b->m_cachedValue.val[3][0],b->m_cachedValue.val[3][1],b->m_cachedValue.val[3][2]);
		}	
	}
	if(cg_skel_drawBones->integer) {
		matrix_t m;
		matrix_t mb[128];
		MatrixSetupTransformFromVectorsFLU(m,ent->axis[0],ent->axis[1],ent->axis[2],ent->origin);
		for(i = 0; i < skel->m_Tiki->m_boneList.m_numChannels; i++) {
			skelBone_Base_c *b;
			matrix_t bm;

			b = skel->m_bone[i];
			MatrixSetupTransformFromVectorsFLU(bm,b->m_cachedValue.val[0],
				b->m_cachedValue.val[1],b->m_cachedValue.val[2],b->m_cachedValue.val[3]);
			
			Matrix4x4Multiply(m,bm,mb[i]);
		}	
		for(i = 0; i < skel->m_Tiki->m_boneList.m_numChannels; i++) {
			vec3_t p;
			vec3_t axis[3];
			skelBone_Base_c *b;
			int parent;

			b = skel->m_bone[i];
			
			MatrixToVectorsFLU(mb[i],axis[0],axis[1],axis[2]);
			CG_DrawAxisAtPoint(&mb[i][12],axis);

			if(b->m_parent) {


				parent = CG_Skel_BoneIndexForPointer(b->m_parent,skel);

				cgi.R_DebugLine( &mb[i][12], &mb[parent][12], 1, 1, 1, 1 );

				
			}
		//	
		}	
		return qfalse;
	}
	if(cg_skel_su44->integer) {
		for(i = 0; i <= skel->m_headBoneIndex; i++) {
			GetTransformFunc GetDirtyTransform;
			skelBone_Base_c *b;
			int **v;
			SkelMat4_t *m;
			skelAnimStoreFrameList_c dummy;

			memset(&dummy,0,sizeof(dummy));

			// access bone virtual function table
			b = skel->m_bone[i];
			v = b->vptr;
			
			for(j = 0; j < 10; j++) {
				cgi.Printf("vptrBase %i index %i ofs %i\n",b->vptr,j,v[j]);
			}

			j = 2;
			GetDirtyTransform = v[j];
		//	m = GetDirtyTransform(b,&skel->m_frameList);

			cgi.Printf("t.");
		}	
	}
	return qtrue;

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
		if(CG_TIKISkeletor(ent,skel) == qfalse)
			return; // dont add it to scene
	} else {
		skel = 0;
	}

	cgi.R_AddRefEntityToScene(ent);
}


