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
#include "../../qcommon/qfiles.h" // su44: I need this for boneType enum

cvar_t *cg_skel_show;
cvar_t *cg_skel_printBoneDirty;
cvar_t *cg_skel_printBonePos;
cvar_t *cg_skel_printHoseParms;
cvar_t *cg_skel_drawBones;
cvar_t *cg_skel_su44;
cvar_t *cheats;

void CG_InitSkeletorCvarsAndCmds() {
	cg_skel_show = cgi.Cvar_Get("cg_skel_show","0",0);
	cg_skel_printBoneDirty = cgi.Cvar_Get("cg_skel_printBoneDirty","0",0);
	cg_skel_printBonePos = cgi.Cvar_Get("cg_skel_printBonePos","0",0);
	cg_skel_drawBones = cgi.Cvar_Get("cg_skel_drawBones","0",0);
	cg_skel_printHoseParms = cgi.Cvar_Get("cg_skel_printHoseParms","0",0);
	cg_skel_su44 = cgi.Cvar_Get("cg_skel_su44","0",0);
	// hack, force some cvars to their usefull values
	cheats = cgi.Cvar_Get("cheats","0",0);
	cheats->integer = 1;
	cheats = cgi.Cvar_Get("sv_cheats","0",0);
	cheats->integer = 1;
	cheats = cgi.Cvar_Get("thereisnomonkey","0",0);
	cheats->integer = 1;
}

typedef struct {
	int type;
	char *name;
} boneNameAndType_t;

// su44: known boneType / boneName pairs
// (from human/coxswain.tik)
boneNameAndType_t knownBoneTypes [] = {
	{ JT_ROTATION, "Bip01 Spine" }, // boneType 0
	{ JT_POSROT_SKC, "Bip01 Pelvis" }, // boneType 1
	{ JT_SHOULDER, "Bip01 L Thigh" }, // boneType 2
	{ JT_ELBOW, "Bip01 L Calf" }, // boneType 3
	{ JT_WRIST, "Bip01 L Foot" }, // boneType 4
	// boneType 5, hoseType 2, bendMax 180, bendRatio 1, spinRatio 0.6
	{ JT_HOSEROTBOTH, "helper Rankle" }, 
	// boneType 5, hoseType 1, bendMax 180, bendRatio 0.5, spinRatio 0.9
	{ JT_HOSEROTPARENT, "helper Lhip" }, 
	{ JT_AVROT, "helper Lelbow" }, // boneType 6
};

static int numKnownBoneTypes = sizeof(knownBoneTypes) / sizeof(knownBoneTypes[0]);

qboolean vPtrsExtracted = qfalse;
void *boneVPtrs[16];

// su44: extract vTable pointer for each bone class
// (this is totally offset-independent  
// and should run with any build of MoH)
void CG_Skel_ExtractVPTRs(skeletor_c *skel) {
	int i, j;

	if(vPtrsExtracted)
		return;

	for(i = 0; i < skel->m_Tiki->m_boneList.m_numChannels; i++) {
		skelBone_Base_c *b;
		const char *name;
		boneNameAndType_t *bt;

		b = skel->m_bone[i];
		name = cgi.Tag_NameForNum(skel->m_Tiki,i);
		bt = knownBoneTypes;
		for(j = 0; j < numKnownBoneTypes; j++,bt++) {
			if(!stricmp(bt->name,name)) {
				// match found, so safe the vtable pointer
				if(boneVPtrs[bt->type] == 0) {
					boneVPtrs[bt->type] = (void*)b->vptr;
				} else {
					if(boneVPtrs[bt->type] != (void*)b->vptr) {
						cgi.Error(0,"CG_Skel_ExtractVPTRs: this should never happen; ask su44 why\n");
					}
				}

			}
		}
	}	

	if(boneVPtrs[JT_POSROT_SKC] && boneVPtrs[JT_ROTATION] && boneVPtrs[JT_AVROT]
		&& boneVPtrs[JT_ELBOW] && boneVPtrs[JT_SHOULDER] && boneVPtrs[JT_WRIST]
		&& boneVPtrs[JT_HOSEROTBOTH] && boneVPtrs[JT_HOSEROTPARENT]) {
		vPtrsExtracted = qtrue;
		Com_Printf("CG_Skel_ExtractVPTRs: extraced all vPtrs! \n");
	}
}

// su44: some util functions
qboolean CG_Bone_IsHoseRot(skelBone_Base_c *b) {
	if(boneVPtrs[JT_HOSEROTBOTH] == b->vptr
		|| boneVPtrs[JT_HOSEROTPARENT] == b->vptr)
		return qtrue;

	return qfalse;
}

qboolean CG_Bone_IsHoseRotBoth(skelBone_Base_c *b) {
	if(boneVPtrs[JT_HOSEROTBOTH] == b->vptr)
		return qtrue;

	return qfalse;
}
qboolean CG_Bone_IsHoseRotParent(skelBone_Base_c *b) {
	if(boneVPtrs[JT_HOSEROTPARENT] == b->vptr)
		return qtrue;

	return qfalse;
}
qboolean CG_Bone_IsAVRot(skelBone_Base_c *b) {
	if(boneVPtrs[JT_AVROT] == b->vptr)
		return qtrue;

	return qfalse;
}
qboolean CG_Bone_IsWrist(skelBone_Base_c *b) {
	if(boneVPtrs[JT_WRIST] == b->vptr)
		return qtrue;

	return qfalse;
}
qboolean CG_Bone_IsElbow(skelBone_Base_c *b) {
	if(boneVPtrs[JT_ELBOW] == b->vptr)
		return qtrue;

	return qfalse;
}
qboolean CG_Bone_IsShoulder(skelBone_Base_c *b) {
	if(boneVPtrs[JT_SHOULDER] == b->vptr)
		return qtrue;

	return qfalse;
}

// returns skdJointType_t or -1 if unknown (0 is used for JT_ROTATION)
int CG_Bone_GetType(skelBone_Base_c *b) {
	int i;

	for(i = 0; i < 16; i++) {
		if(b->vptr == boneVPtrs[i])
			return i;
	}
	return -1;
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

	CG_Skel_ExtractVPTRs(skel);

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
	if(cg_skel_printHoseParms->integer && vPtrsExtracted) {
		for(i = 0; i < skel->m_Tiki->m_boneList.m_numChannels; i++) {
			skelBone_Base_c *b = skel->m_bone[i];

			if(CG_Bone_IsHoseRot(b)) {
				skelBone_HoseRot_c *h;

				h = (skelBone_HoseRot_c*)b;

				cgi.Printf("Bone %i of %i is a hoserot, name %s, model %s bendMax %f bendRatio %f spinRatio %f\n",
				i,skel->m_Tiki->m_boneList.m_numChannels,cgi.Tag_NameForNum(skel->m_Tiki,i),
				ent->tiki->name,
				h->m_bendMax,h->m_bendRatio,h->m_spinRatio
				);
			}
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


