/*
===========================================================================
Copyright (C) 2008 Leszek Godlewski
Copyright (C) 2010-2011 su44

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

#include "tiki_local.h"
//#include "../renderer/tr_local.h"

#define TIKI_FILE_HASH_SIZE		1024
static	tiki_t					*hashTable[TIKI_FILE_HASH_SIZE];


char *TIKI_GetBoneNameFromIndex(int globalBoneName);
qhandle_t RE_RegisterShader( const char *name );
// copied over from renderer/tr_shader.c
#ifdef __GNUCC__
  #warning TODO: check if long is ok here
#endif
static long generateHashValue(const char *fname, const int size) {
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (fname[i] != '\0') {
		letter = tolower(fname[i]);
		if (letter =='.') break;				// don't include extension
		if (letter =='\\') letter = '/';		// damn path names
		if (letter == PATH_SEP) letter = '/';		// damn path names
		hash+=(long)(letter)*(i+119);
		i++;
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	hash &= (size-1);
	return hash;
}

// bones... called every frame from cgame, animated by CG_ModelAnim, then passed to renderer with refEntities
#define MAX_GLOBAL_BONES 4096
static bone_t globalBones[MAX_GLOBAL_BONES];
static bone_t *currBone = globalBones;
bone_t *TIKI_GetBones(int numBones) {
	int i;
	bone_t *tmp;
	if(currBone+numBones >= globalBones+MAX_GLOBAL_BONES) {
		Com_Error(ERR_DROP,"TIKI_GetBones: overflowed. Increase MAX_GLOBAL_BONES (%i) and recompile!\n",MAX_GLOBAL_BONES);
		return 0;
	}
	tmp = currBone;
	currBone+=numBones;
	for(i = 0; i < numBones; i++) {
		VectorSet(tmp[i].p,0,0,0);
		VectorSet(tmp[i].q,0,0,0);
		tmp[i].q[3] = 1;
	}
	return tmp;
}
void TIKI_ResetBones() {//called every frame before CG_AddPackEntities
	currBone = globalBones;
}
/*
===============================
ANIMATION CODE
===============================
*/
int TIKI_FindPosChannel(tikiAnim_t *anim, int globalBoneName) {
	int i;
	for(i = 0; i < anim->numPosChannels; i++) {
		if(anim->posChannelNames[i] == globalBoneName)
			return i;
	}
	return -1;
}
int TIKI_FindRotChannel(tikiAnim_t *anim, int globalBoneName) {
	int i;
	for(i = 0; i < anim->numRotChannels; i++) {
		if(anim->rotChannelNames[i] == globalBoneName)
			return i;
	}
	return -1;
}
qboolean TIKI_FindAndCopyPosChannel(tikiAnim_t *anim, int globalBoneName, tikiFrame_t *frame, vec3_t out) {
	int i;
	for(i = 0; i < anim->numPosChannels; i++) {
		if(anim->posChannelNames[i] == globalBoneName) {
			VectorCopy(frame->posChannels[i],out);
			return qtrue;
		}
	}
	return qfalse;
}
qboolean TIKI_FindAndCopyRotChannel(tikiAnim_t *anim, int globalBoneName, tikiFrame_t *frame, quat_t out) {
	int i;
	for(i = 0; i < anim->numRotChannels; i++) {
		if(anim->rotChannelNames[i] == globalBoneName) {
			VectorCopy(frame->rotChannels[i],out);
			out[3] = frame->rotChannels[i][3];
			return qtrue;
		}
	}
	return qfalse;
}
qboolean TIKI_FindAndCopyRotFKChannel(tikiAnim_t *anim, int globalBoneName, tikiFrame_t *frame, quat_t out) {
	int i;
	for(i = 0; i < anim->numRotFKChannels; i++) {
		if(anim->rotFKChannelNames[i] == globalBoneName) {
			VectorCopy(frame->rotFKChannels[i],out);
			out[3] = frame->rotFKChannels[i][3];
			return qtrue;
		}
	}
	return qfalse;
}
void TIKI_SetChannels_internal(tiki_t *tiki, bone_t *bones, tikiAnim_t *anim, tikiFrame_t *frame) {
	int i;
	int **ptr = (int**)tiki->bones;
	for(i = 0; i < tiki->numBones; i++)	{
		switch(**ptr) {
			case JT_ROTATION:
				TIKI_FindAndCopyRotChannel(anim,tiki->boneNames[i],frame,bones->q);
			break;
			case JT_POSROT_SKC:
				//rot = TIKI_FindRotChannel(anim,tiki->boneNames[i]);
				//pos = TIKI_FindRotChannel(anim,tiki->boneNames[i]);
				TIKI_FindAndCopyPosChannel(anim,tiki->boneNames[i],frame,bones->p);
				TIKI_FindAndCopyRotChannel(anim,tiki->boneNames[i],frame,bones->q);
			break;
			case JT_SHOULDER:		
				TIKI_FindAndCopyRotFKChannel(anim,tiki->boneNames[i],frame,bones->q);
			break;
			case JT_ELBOW:		
				TIKI_FindAndCopyRotFKChannel(anim,tiki->boneNames[i],frame,bones->q);
			break;
			case JT_WRIST:
				TIKI_FindAndCopyPosChannel(anim,tiki->boneNames[i],frame,bones->p);
				TIKI_FindAndCopyRotFKChannel(anim,tiki->boneNames[i],frame,bones->q);
				//TIKI_FindAndCopyRotChannel(anim,tiki->boneNames[i],frame,bones->q);
				break;
			case JT_HOSEROT:
			case JT_AVROT:
				break;
			default:
				Com_Error(ERR_DROP,"Unknwon wbonetye %i\n",**ptr);
				break;
		}
		ptr++;
		bones++;
	}
}

void TIKI_SetChannels_interpolate(tiki_t *tiki, bone_t *bones, tikiAnim_t *anim, tikiFrame_t *frame, tikiFrame_t *nextFrame, float f) {
	int i;
	quat_t q1,q2;
	vec3_t v1, v2;
	int **ptr;

	ptr = (int**)tiki->bones;
	for(i = 0; i < tiki->numBones; i++)	{
		switch(**ptr) {
			case JT_ROTATION:
				if(TIKI_FindAndCopyRotChannel(anim,tiki->boneNames[i],frame,q1)) {
					TIKI_FindAndCopyRotChannel(anim,tiki->boneNames[i],nextFrame,q2);
					QuatSlerp(q1,q2,f,bones->q);
				}
			break;
			case JT_POSROT_SKC:
				if(TIKI_FindAndCopyPosChannel(anim,tiki->boneNames[i],frame,v1)) {
					TIKI_FindAndCopyPosChannel(anim,tiki->boneNames[i],nextFrame,v2);
					VectorLerp(v1,v2,f,bones->p);
				}
				if(TIKI_FindAndCopyRotChannel(anim,tiki->boneNames[i],frame,q1)) {
					TIKI_FindAndCopyRotChannel(anim,tiki->boneNames[i],nextFrame,q2);
					QuatSlerp(q1,q2,f,bones->q);
				}
			break;
			case JT_SHOULDER:		
				if(TIKI_FindAndCopyRotFKChannel(anim,tiki->boneNames[i],frame,q1)) {
					TIKI_FindAndCopyRotFKChannel(anim,tiki->boneNames[i],nextFrame,q2);
					QuatSlerp(q1,q2,f,bones->q);	
				}
				break;
			case JT_ELBOW:		
				if(TIKI_FindAndCopyRotFKChannel(anim,tiki->boneNames[i],frame,q1)) {
					TIKI_FindAndCopyRotFKChannel(anim,tiki->boneNames[i],nextFrame,q2);
					QuatSlerp(q1,q2,f,bones->q);
				}
			break;
			case JT_WRIST:
				if(TIKI_FindAndCopyPosChannel(anim,tiki->boneNames[i],frame,v1)) {
					TIKI_FindAndCopyPosChannel(anim,tiki->boneNames[i],nextFrame,v2);
					VectorLerp(v1,v2,f,bones->p);
				}

				if(TIKI_FindAndCopyRotFKChannel(anim,tiki->boneNames[i],frame,q1)) {
					TIKI_FindAndCopyRotFKChannel(anim,tiki->boneNames[i],nextFrame,q2);
					QuatSlerp(q1,q2,f,bones->q);
				}
				//TIKI_FindAndCopyRotChannel(anim,tiki->boneNames[i],frame,bones->q);
				break;
			case JT_HOSEROT:
			case JT_AVROT:
				break;
			default:
				Com_Error(ERR_DROP,"Unknwon wbonetye %i\n",**ptr);
				break;
		}
		ptr++;
		bones++;
	}
}
static void TIKI_AppendFrameBoundsAndRadiusSingle( tikiFrame_t *frame, float *outRadius, vec3_t outBounds[2]) {
	BoundsAdd(outBounds[0],outBounds[1],frame->bounds[0],frame->bounds[1]);
	if(frame->radius > *outRadius) {
		*outRadius = frame->radius;
	}
}
void TIKI_AppendFrameBoundsAndRadius( struct tiki_s *tiki, int animIndex, float animTime, float *outRadius, vec3_t outBounds[2] ) {
	tikiAnim_t *anim;
	tikiFrame_t *frame;
	int i;

	if(tiki->numAnims <= animIndex) {
		Com_Printf("TIKI_AppendFrameBoundsAndRadius: animIndex %i out of range %i\n",animIndex,tiki->numAnims);
		return;
	}
	anim = tiki->anims[animIndex];
	if(anim->numFrames == 1) {
		TIKI_AppendFrameBoundsAndRadiusSingle(anim->frames,outRadius,outBounds);
		return;
	}
	i = 1;
	frame = anim->frames;
	while(animTime > anim->frameTime) {
		animTime-=anim->frameTime;
		frame++;
		i++;
		if(i == anim->numFrames) {
			TIKI_AppendFrameBoundsAndRadiusSingle(frame,outRadius,outBounds);
			return;
		}
	}
	TIKI_AppendFrameBoundsAndRadiusSingle(frame,outRadius,outBounds);
	TIKI_AppendFrameBoundsAndRadiusSingle(frame+1,outRadius,outBounds);
}
void TIKI_SetChannels(tiki_t *tiki, int animIndex, float animTime, float animWeight, bone_t *bones) {
	int i;
	skdJointType_t **ptr;
	tikiAnim_t *anim;
	tikiFrame_t *frame;

	if(tiki->numAnims <= animIndex) {
		Com_Printf("TIKI_SetChannels: animIndex %i out of range %i\n",animIndex,tiki->numAnims);
		return;
	}
	ptr = (skdJointType_t **)tiki->bones;
	anim = tiki->anims[animIndex];
	frame = anim->frames;
	if(anim->numFrames == 1) {
		//do not interpolate
		TIKI_SetChannels_internal(tiki,bones,anim,frame);
		return;
	}
	else {
		i = 1;
		while(animTime > anim->frameTime) {
			animTime-=anim->frameTime;
			frame++;
			i++;
			if(i == anim->numFrames) {
				TIKI_SetChannels_internal(tiki,bones,anim,frame); // last frame, do not interpolate
				return;
			}
		}
	}
	//interpolate between two frames
	TIKI_SetChannels_interpolate(tiki,bones,anim,frame,frame+1,animTime/anim->frameTime);
}

void TIKI_Animate(tiki_t *tiki, bone_t *bones)
{
	int i;
	tikiBonePosRot_t	*posrot, *bone;
	tikiBoneRotation_t	*rot;
	tikiBoneShoulder_t	*should;
	tikiBoneElbow_t		*elbow;
	tikiBoneWrist_t		*wrist;
	tikiBoneHoseRot_t	*hose;
	tikiBoneAVRot_t		*av;
	for(i = 0; i < tiki->numBones;i++) {
		bone = (tikiBonePosRot_t*) tiki->bones[i];
	
		if(bone->parentIndex==-1) {
			QuatInverse(bones[i].q);
		}
		else {

			switch(*(int*)tiki->bones[i])
			{	
				case JT_ROTATION:
				{
					quat_t	pos,res,temp;
					rot = (tikiBoneRotation_t*)tiki->bones[i];
					//const_ofs instead of channel p
					VectorCopy(rot->const_offset,pos);
					pos[3]=0;
					QuaternionMultiply(temp,pos,bones[rot->parentIndex].q);
					QuatInverse(bones[rot->parentIndex].q);
					QuaternionMultiply(res,bones[rot->parentIndex].q,temp);
					QuatInverse(bones[rot->parentIndex].q);
					bones[i].p[0] = res[0] + bones[rot->parentIndex].p[0];
					bones[i].p[1] = res[1] + bones[rot->parentIndex].p[1];
					bones[i].p[2] = res[2] + bones[rot->parentIndex].p[2];
					QuatCopy(bones[i].q,temp);
					QuatInverse(temp);
					QuaternionMultiply(bones[i].q,temp,bones[rot->parentIndex].q);
					QuatNormalize(bones[i].q);

				}
				break;
				case JT_POSROT_SKC:
				{
					quat_t	pos,res,temp;
					posrot = (tikiBonePosRot_t*)tiki->bones[i];
					VectorCopy(bones[i].p,pos);
					pos[3]=0;
					QuaternionMultiply(temp,pos,bones[posrot->parentIndex].q);
					QuatInverse(bones[posrot->parentIndex].q);
					QuaternionMultiply(res,bones[posrot->parentIndex].q,temp);
					QuatInverse(bones[posrot->parentIndex].q);
					bones[i].p[0] = res[0] +bones[posrot->parentIndex].p[0];
					bones[i].p[1] = res[1] +bones[posrot->parentIndex].p[1];
					bones[i].p[2] = res[2] +bones[posrot->parentIndex].p[2];
					QuatCopy(bones[i].q,temp);
					QuatInverse(temp);
					QuaternionMultiply(bones[i].q,temp,bones[posrot->parentIndex].q);
					QuatNormalize(bones[i].q);

				}
				break;	
				case JT_SHOULDER:
				{
					//Com_Printf("should %i",tiki->name);
					quat_t	pos,res,temp;
					should = (tikiBoneShoulder_t*)tiki->bones[i];
					//const_ofs instead of channel p
					VectorCopy(should->const_offset,pos);

					//we're using its rotfk channel instead of rot
					pos[3]=0;
					QuaternionMultiply(temp,pos,bones[should->parentIndex].q);
					QuatInverse(bones[should->parentIndex].q);
					QuaternionMultiply(res,bones[should->parentIndex].q,temp);
					QuatInverse(bones[should->parentIndex].q);
					bones[i].p[0] = res[0] +bones[should->parentIndex].p[0];
					bones[i].p[1] = res[1] +bones[should->parentIndex].p[1];
					bones[i].p[2] = res[2] +bones[should->parentIndex].p[2];
					QuatInverse(bones[i].q);
					QuatCopy(bones[i].q,temp);
					QuatInverse(bones[i].q);
					QuaternionMultiply(bones[i].q,temp,bones[should->parentIndex].q);
					QuatNormalize(bones[i].q);

				}
				break;
				case JT_ELBOW:
				{
					quat_t	pos,res,temp;


					elbow = (tikiBoneElbow_t*)tiki->bones[i];
					//Com_Printf("elb %i",tiki->name);

					//const_ofs instead of channel p
					VectorCopy(elbow->const_offset,pos);

					//like in ikshoulder,
					//we're using rotfk channel instead of rot ???
					pos[3]=0;
					QuaternionMultiply(temp,pos,bones[elbow->parentIndex].q);
					QuatInverse(bones[elbow->parentIndex].q);
					QuaternionMultiply(res,bones[elbow->parentIndex].q,temp);
					QuatInverse(bones[elbow->parentIndex].q);
					bones[i].p[0] = res[0] +bones[elbow->parentIndex].p[0];
					bones[i].p[1] = res[1] +bones[elbow->parentIndex].p[1];
					bones[i].p[2] = res[2] +bones[elbow->parentIndex].p[2];
					QuatInverse(bones[i].q);
					QuatCopy(bones[i].q,temp);
					QuatInverse(bones[i].q);
					QuaternionMultiply(bones[i].q,temp,bones[elbow->parentIndex].q);
					QuatNormalize(bones[i].q);
				}
				break;
				case JT_WRIST:
				{
					quat_t	pos,res,temp;
					wrist = (tikiBoneWrist_t*) tiki->bones[i];

					//const_ofs instead of channel p
					VectorCopy(wrist->const_offset,pos);
					//Com_Printf("wrist  %i",tiki->name);
					//like in ikshoulder, nad elbow
					//we're using its rotfk channel instead of rot
					pos[3]=0;
					QuaternionMultiply(temp,pos,bones[wrist->parentIndex].q);
					QuatInverse(bones[wrist->parentIndex].q);
					QuaternionMultiply(res,bones[wrist->parentIndex].q,temp);
					QuatInverse(bones[wrist->parentIndex].q);
					bones[i].p[0] = res[0] +bones[wrist->parentIndex].p[0];
					bones[i].p[1] = res[1] +bones[wrist->parentIndex].p[1];
					bones[i].p[2] = res[2] +bones[wrist->parentIndex].p[2];
					QuatInverse(bones[i].q);
					QuatCopy(bones[i].q,temp);
					QuatInverse(bones[i].q);
					QuaternionMultiply(bones[i].q,temp,bones[wrist->parentIndex].q);
					QuatNormalize(bones[i].q);
				}
				break;
				case JT_HOSEROT:
				{
					hose = (tikiBoneHoseRot_t*)tiki->bones[i];
			//		Com_Printf("hoserot const ofs %f %f %f for ",hose->const_offset[0],hose->const_offset[1],hose->const_offset[2]);

					QuatCopy(bones[hose->targetIndex].q,bones[i].q);
					VectorCopy(bones[hose->targetIndex].p,bones[i].p); 
				}
				break;
				case JT_AVROT:
				{
					quat_t	pos,res,temp;
					av = (tikiBoneAVRot_t*) tiki->bones[i];

					QuatSlerp(bones[av->m_reference1].q,bones[av->m_reference2].q,av->m_bone2weight,bones[i].q);
					/*
					bones[i].p[0] = bones[av->m_reference1].p[0] + av->m_bone2weight*(bones[av->m_reference2].p[0] - bones[av->m_reference1].p[0]);
					bones[i].p[1] = bones[av->m_reference1].p[1] + av->m_bone2weight*(bones[av->m_reference2].p[1] - bones[av->m_reference1].p[1]);
					bones[i].p[2] = bones[av->m_reference1].p[2] + av->m_bone2weight*(bones[av->m_reference2].p[2] - bones[av->m_reference1].p[2]);
					*/
					VectorCopy(av->const_offset,pos);
					pos[3]=0;
					QuaternionMultiply(temp,pos,bones[av->parentIndex].q);
					QuatInverse(bones[av->parentIndex].q);
					QuaternionMultiply(res,bones[av->parentIndex].q,temp);
					QuatInverse(bones[av->parentIndex].q);
					bones[i].p[0] = res[0] +bones[av->parentIndex].p[0];
					bones[i].p[1] = res[1] +bones[av->parentIndex].p[1];
					bones[i].p[2] = res[2] +bones[av->parentIndex].p[2];
				}
				break;
				default:
					Com_Error(ERR_DROP,"TIKI_Animate: unsupported bone type %i",*((int**)tiki->bones)[i]);
				break;
			}
		}
	}
#if 0
	if(tiki->numAnims>456) {
		for(i = 0; i < tiki->numBones; i++)	{
			Com_DPrintf("bone %i of %i (%s) - %f %f %f\n",i,tiki->numBones,TIKI_GetBoneNameFromIndex(tiki->boneNames[i]),bones[i].p[0],bones[i].p[1],bones[i].p[2]);
		}
	}
#endif
}

/*
===============================
SK* FILES LOADING
===============================
*/
typedef struct boneName_s
{
	char *name;
	struct boneName_s *next;
} boneName_t;
#define TIKI_BONE_HASH_SIZE 128
boneName_t globalBoneNames[4096];
boneName_t *freeBoneName = globalBoneNames;
boneName_t *boneHashTable[TIKI_BONE_HASH_SIZE];
int	TIKI_RegisterBoneName(const char *boneName) {
	boneName_t	*bone;
	int		hash = generateHashValue(boneName, TIKI_BONE_HASH_SIZE);

	for (bone = boneHashTable[hash]; bone; bone = bone->next) {
		if (!Q_stricmp(bone->name, boneName))
			return bone - globalBoneNames;
	}
	freeBoneName++;
	bone = freeBoneName;
	if (boneHashTable[hash]) {
		bone->next = boneHashTable[hash];
	}
	boneHashTable[hash] = bone;
	bone->name = Hunk_Alloc(strlen(boneName)+1,h_low);
	strcpy(bone->name,boneName);
	return bone - globalBoneNames;
}
char *TIKI_GetBoneNameFromIndex(int globalBoneName) {
	return globalBoneNames[globalBoneName].name;
}
int TIKI_GetLocalBoneIndex2(tiki_t *out, int globalBoneName) {
	int b;
	for(b = 0; b < out->numBones; b++) {
		if(out->boneNames[b]==globalBoneName)
			return b;
	}
	Com_Error(ERR_DROP,"TIKI_GetLocalBoneIndex failed for tiki %s, boneName %i \n",out->name,globalBoneName);
	return -1;
}
int TIKI_GetLocalBoneIndex(tiki_t *out, char *boneName) {
	int globalIndex;
	if(!strcmp(boneName,"worldbone")) {
		return -1;
	}
	globalIndex = TIKI_RegisterBoneName(boneName);
	return TIKI_GetLocalBoneIndex2(out,globalIndex);
}
qboolean TIKI_HasBone(tiki_t *out, int globalBoneName) {
	int i;
	for(i = 0; i < out->numBones; i++) {
		if(out->boneNames[i] == globalBoneName) {
			return qtrue;
		}
	}
	return qfalse;
}
// what a mess!
// but at least now it works...
void TIKI_MergeSKD(tiki_t *out, char *fname) {
	int				filesize;
	char			*buf;
	skdHeader_t		*header;
	skdBone_t		*bone;
	skdSurface_t	*surf, *outSurfs;
	float			*ptr;
	char			*bytePtr;
	int				i,j,k;
	skdVertex_t		*vert;
	skdWeight_t		*weight;
	tikiBonePosRot_t*posrot;
	tikiBoneRotation_t*rot;
	tikiBoneShoulder_t	*should;
	tikiBoneElbow_t		*elbow;
	tikiBoneWrist_t		*wrist;
	tikiBoneHoseRot_t	*hose;
	tikiBoneAVRot_t		*av;
	int					outSurfacesSize;
	int					numAddBones;
	int					addBonesIndexes[128];
	byte				notInBoneList[128];
	int					headerBoneNames[128];
	int *newBoneNames;
	int	*newBoneList;

	//Com_Printf("TIKI_MergeSKD for %s, skd name  %s\n", out->name, fname);
	filesize = FS_ReadFile(fname, (void **)&buf);
	if(!buf) {
		Com_Printf ("TIKI_MergeSKD: couldn't load %s\n", fname);
		return;
	}
	header = (skdHeader_t *)buf;
	if (header->version != SKD_VERSION) {
		Com_Printf( "TIKI_MergeSKD: %s has wrong version (%i should be %i)\n",
				 fname, header->version, SKD_VERSION);
		return;
	}
	if (header->ident != SKD_IDENT) {
		Com_Printf( "TIKI_MergeSKD: %s has wrong ident (%s should be %s)\n",
				 fname, header->ident, SKD_IDENT);
		return;
	}

	numAddBones = 0;
	//first check do we have to realloc the bones?
	bone = (skdBone_t *) ( (byte *)header + header->ofsBones );
	for ( i = 0 ; i < header->numBones ; i++) {
		headerBoneNames[i] = TIKI_RegisterBoneName(bone->name);
		notInBoneList[i] = TIKI_HasBone(out,headerBoneNames[i]);
		if(notInBoneList[i]==qfalse) {
			addBonesIndexes[numAddBones] = headerBoneNames[i];
			numAddBones++;
		}
		bone = (skdBone_t *)( (byte *)bone + bone->ofsEnd );
	}
	if(numAddBones) {
		newBoneNames = Z_Malloc((numAddBones + out->numBones)*4);
		memcpy(newBoneNames,out->boneNames,out->numBones*4);
		memcpy(&newBoneNames[out->numBones],addBonesIndexes,numAddBones*4);
		Z_Free(out->boneNames);
		out->boneNames = newBoneNames;

		newBoneList = Z_Malloc((out->numBones + numAddBones)*4);
		memcpy(newBoneList,out->bones,out->numBones*4);
		Z_Free(out->bones);
		out->bones = (void**)newBoneList;
		newBoneList+=out->numBones;
		out->numBones+=numAddBones;
		
		bone = (skdBone_t *) ( (byte *)header + header->ofsBones );
		for ( i = 0 ; i < header->numBones ; i++) {
			if(notInBoneList[i]==qfalse) { //bone i is not present in out->bones array, we have to load it from this skd file
				switch(bone->jointType)	{
					case JT_ROTATION:
					{
						*newBoneList = (int*)Hunk_Alloc(sizeof(tikiBoneRotation_t),h_low);
						rot = (tikiBoneRotation_t*)*newBoneList;
						rot->type=bone->jointType;
						rot->parentIndex = TIKI_GetLocalBoneIndex(out,bone->parent);
						ptr = (float *) ( (byte *)bone + sizeof(skdBone_t) );
						VectorScale(ptr,out->scale,rot->const_offset);
					}
					break;
					case JT_POSROT_SKC:
					{
						*newBoneList = (int*)Hunk_Alloc(sizeof(tikiBonePosRot_t),h_low);
						posrot = (tikiBonePosRot_t*)*newBoneList;
						posrot->type=bone->jointType;
						posrot->parentIndex = TIKI_GetLocalBoneIndex(out,bone->parent);
					}
					break;
					case JT_SHOULDER:
					{
						*newBoneList = Hunk_Alloc(sizeof(tikiBoneShoulder_t),h_low);
						should = (tikiBoneShoulder_t*)*newBoneList;
						should->type=bone->jointType;
						should->parentIndex = TIKI_GetLocalBoneIndex(out,bone->parent);
						ptr = (float *) ( (byte *)bone + sizeof(skdBone_t) );
						ptr+=(4);
						VectorScale(ptr,out->scale,should->const_offset);
					}
					break;
					case JT_ELBOW:
					{
						*newBoneList = Hunk_Alloc(sizeof(tikiBoneElbow_t),h_low);
						elbow = (tikiBoneElbow_t*)*newBoneList;
						elbow->type=bone->jointType;
						elbow->parentIndex = TIKI_GetLocalBoneIndex(out,bone->parent);
						ptr = (float *) ( (byte *)bone + sizeof(skdBone_t) );
						VectorScale(ptr,out->scale,ptr);
						VectorCopy(ptr,elbow->const_offset);
					}
					break;
					case JT_WRIST:
					{
						*newBoneList = Hunk_Alloc(sizeof(tikiBoneWrist_t),h_low);
						wrist = (tikiBoneWrist_t*)*newBoneList;
						wrist->type=bone->jointType;
						wrist->parentIndex = TIKI_GetLocalBoneIndex(out,bone->parent);
						ptr = (float *) ( (byte *)bone + sizeof(skdBone_t) );
						VectorScale(ptr,out->scale,wrist->const_offset);
					}
					break;
					case JT_HOSEROT:
					{
						char*txt;
						*newBoneList = Hunk_Alloc(sizeof(tikiBoneHoseRot_t),h_low);
						hose = (tikiBoneHoseRot_t*)*newBoneList;
						hose->type=bone->jointType;
						hose->parentIndex = TIKI_GetLocalBoneIndex(out,bone->parent);
						ptr = (float *) ( (byte *)bone + sizeof(skdBone_t) );
						Com_Printf("HOSEROT bendRatio %f \n",*ptr);
						hose->bendRatio = *ptr;
						ptr++;
						Com_Printf("HOSEROT bendMax %f \n",*ptr);
						hose->bendMax = *ptr;
						ptr++;
						Com_Printf("HOSEROT spinRatio %f \n",*ptr);
						hose->spinRatio = *ptr;
						ptr++;
						VectorScale(ptr,out->scale,hose->basePos);
						ptr+=3;
						//Com_Printf("%f %f %f \n",ptr[0],ptr[1],ptr[2]);
						ptr+=3; // skip 1.f 1.f 1.f
						Com_Printf("HOSEROT hoseRotType %i \n",*(int*)ptr);
						hose->hoseRotType = *(hoseRotType_t*)ptr;
						ptr++;
						txt = (char*)ptr;
						//Com_Printf("\n HOSEref <%s>\n",txt);
						hose->targetIndex = TIKI_GetLocalBoneIndex(out,txt);
					}
					break;
					case JT_AVROT: //
					{
						int size;
						*newBoneList = (int*)Hunk_Alloc(sizeof(tikiBoneAVRot_t),h_low);
						av = (tikiBoneAVRot_t*)*newBoneList;
						av->type=bone->jointType;
						av->parentIndex = TIKI_GetLocalBoneIndex(out,bone->parent);
						ptr = (float *) ( (byte *)bone + sizeof(skdBone_t) );
		//				Com_Printf("AVROT data weight %f offs %f %f %f unknown %f %f %f %f", ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7],ptr[8]);
						av->m_bone2weight = *ptr;
						ptr++;
						VectorScale(ptr,out->scale,ptr);
						VectorCopy(ptr,av->const_offset);
						ptr+=3;
						ptr+=3; //skip 1 1 1
						size = (int)bone + (int)bone->ofsEnd - (int)ptr;
						//Com_Printf("reference 1 %s ",(char*)ptr);
						bytePtr = (char*)ptr;
						av->m_reference1 = TIKI_GetLocalBoneIndex(out,bytePtr); 
						while(*bytePtr != 0)
							bytePtr++;
						bytePtr++;
						//Com_Printf("reference 2 %s\n",(char*)ptr);
						av->m_reference2 = TIKI_GetLocalBoneIndex(out,bytePtr); 
					}
					break;
					default:
						Com_Error(1,"unkwno nbone type %i",bone->jointType);
					break;
				}
				newBoneList++;
			}
			bone = (skdBone_t *)( (byte *)bone + bone->ofsEnd );
		}
	}
	outSurfacesSize = 0; // size of out->surfaces
	surf = out->surfs;
	for ( i = 0; i < out->numSurfaces; i++) {
		outSurfacesSize+=surf->ofsEnd;
		surf = (skdSurface_t *)( (byte *)surf + surf->ofsEnd );
	}
	
	outSurfs = Z_Malloc(outSurfacesSize+(header->ofsBoxes-header->ofsSurfaces));
	memcpy(outSurfs,out->surfs,outSurfacesSize);
	out->numSurfaces+=header->numSurfaces;
	if(out->numSurfaces >= MAX_TIKI_SURFACES) {
		Com_Error(ERR_DROP,"TIKI_MergeSKD: MAX_TIKI_SURFACES (%i) exceed while merging skelmodel %s with tiki %s\n",
			MAX_TIKI_SURFACES,fname,out->name);
	}
	Z_Free(out->surfs);
	out->surfs = outSurfs;

	outSurfs = (skdSurface_t*)(((byte*)outSurfs)+outSurfacesSize);
	memcpy(outSurfs,( ((byte *)header) + header->ofsSurfaces ),header->ofsBoxes-header->ofsSurfaces);
	surf = outSurfs;
	for ( i = 0 ; i < header->numSurfaces ; i++) {
		surf->ident = 12; //SF_SKD;
		vert = (skdVertex_t *) ( (byte *)surf + surf->ofsVerts );
		for ( j = 0 ; j < surf->numVerts ; j++) {
			weight = (skdWeight_t *) ( (byte *)vert + sizeof(skdVertex_t)+(sizeof(skdMorph_t)*vert->numMorphs));
			for ( k = 0 ; k < vert->numWeights ; k++) {
				VectorScale(weight->offset,out->scale,weight->offset);
				weight->boneIndex = TIKI_GetLocalBoneIndex2(out,headerBoneNames[weight->boneIndex]);
				weight = (skdWeight_t *) ( (byte *)weight + sizeof(skdWeight_t));
			}
			vert = (skdVertex_t *) ( (byte *)vert + sizeof(skdVertex_t) + vert->numWeights*sizeof(skdWeight_t) + vert->numMorphs*sizeof(skdMorph_t) );
		}
		surf = (skdSurface_t *)( (byte *)surf + surf->ofsEnd );
	}
	FS_FreeFile(buf);
#if 0
	surf = out->surfs;
	for ( i = 0; i < out->numSurfaces ; i++) {
		Com_Printf("Surf %i of %i - %s \n",i,out->numSurfaces,surf->name);
		surf = (skdSurface_t *)( (byte *)surf + surf->ofsEnd );
	}
#endif
}
void TIKI_AppendSKD(tiki_t *out, char *fname) {
	int				filesize;
	char			*buf;
	skdHeader_t		*header;
	skdBone_t		*bone;
	skdSurface_t	*surf;
	float			*ptr;
	char			*bytePtr;
	int				i,j,k;
	skdVertex_t		*vert;
	skdWeight_t		*weight;
	tikiBonePosRot_t*posrot;
	tikiBoneRotation_t*rot;
	tikiBoneShoulder_t	*should;
	tikiBoneElbow_t		*elbow;
	tikiBoneWrist_t		*wrist;
	tikiBoneHoseRot_t	*hose;
	tikiBoneAVRot_t		*av;

	//Com_Printf("TIKI_AppendSKD for %s, skd name  %s\n", out->name, fname);
	filesize = FS_ReadFile(fname, (void **)&buf);
	if(!buf) {
		Com_Printf ("IKI_AppendSkd: couldn't load %s\n", fname);
		return;
	}
	header = (skdHeader_t *)buf;
	if (header->version != SKD_VERSION) {
		Com_Printf( "TIKI_AppendSKD: %s has wrong version (%i should be %i)\n",
				 fname, header->version, SKD_VERSION);
		return;
	}
	if (header->ident != SKD_IDENT) {
		Com_Printf( "TIKI_AppendSKD: %s has wrong ident (%s should be %s)\n",
				 fname, header->ident, SKD_IDENT);
		return;
	}
	//Com_Printf("SKD file %s data, ident %i, v %i, nae %s, nbones %i, surfs %i ",fname, header->ident,header->version,header->name,header->numBones, header->numSurfaces);
	//Com_Printf("ofs bones %i , surfaces %i, boxes %i, end %i, lods %i, morpgh %i",header->ofsBones, header->ofsSurfaces, header->ofsBoxes,header->ofsEnd,header->ofsLODs, header->ofsMorphTargets);

	//names array
	out->boneNames = Z_Malloc(header->numBones*4);//Hunk_Alloc((header->numBones*4),h_high);
	bone = (skdBone_t *) ( (byte *)header + header->ofsBones );
	for ( i = 0 ; i < header->numBones ; i++) {
		out->boneNames[i] = TIKI_RegisterBoneName(bone->name);
//		Com_Printf("TIKI_RegisterBoneName returned %i for bone name %s\n",out->boneNames[i],bone->name);
		bone = (skdBone_t *)( (byte *)bone + bone->ofsEnd );
	}

	out->numBones = header->numBones;
	out->bones = Z_Malloc(header->numBones*sizeof(int));//Hunk_Alloc(header->numBones*sizeof(int),h_high);
	bone = (skdBone_t *) ( (byte *)header + header->ofsBones );
	for ( i = 0 ; i < header->numBones ; i++) {
		switch(bone->jointType)	{
			case JT_ROTATION:
			{
				out->bones[i] = Hunk_Alloc(sizeof(tikiBoneRotation_t),h_low);
				rot = (tikiBoneRotation_t*)out->bones[i];
				rot->type=bone->jointType;
				rot->parentIndex = TIKI_GetLocalBoneIndex(out,bone->parent);
				ptr = (float *) ( (byte *)bone + sizeof(skdBone_t) );
				VectorScale(ptr,out->scale,rot->const_offset);
			}
			break;
			case JT_POSROT_SKC:
			{
				out->bones[i] = Hunk_Alloc(sizeof(tikiBonePosRot_t),h_low);
				posrot = (tikiBonePosRot_t*)out->bones[i];
				posrot->type=bone->jointType;
				posrot->parentIndex = TIKI_GetLocalBoneIndex(out,bone->parent);
			}
			break;
			case JT_SHOULDER:
			{
				out->bones[i] = Hunk_Alloc(sizeof(tikiBoneShoulder_t),h_low);
				should = (tikiBoneShoulder_t*)out->bones[i];
				should->type=bone->jointType;
				should->parentIndex = TIKI_GetLocalBoneIndex(out,bone->parent);
				ptr = (float *) ( (byte *)bone + sizeof(skdBone_t) );
				ptr+=(4);
				VectorScale(ptr,out->scale,should->const_offset);
			}
			break;
			case JT_ELBOW:
			{
				out->bones[i] = Hunk_Alloc(sizeof(tikiBoneElbow_t),h_low);
				elbow = (tikiBoneElbow_t*)out->bones[i];
				elbow->type=bone->jointType;
				elbow->parentIndex = TIKI_GetLocalBoneIndex(out,bone->parent);
				ptr = (float *) ( (byte *)bone + sizeof(skdBone_t) );
				VectorScale(ptr,out->scale,elbow->const_offset);
			}
			break;
			case JT_WRIST:
			{
				out->bones[i] = Hunk_Alloc(sizeof(tikiBoneWrist_t),h_low);
				wrist = (tikiBoneWrist_t*)out->bones[i];
				wrist->type=bone->jointType;
				wrist->parentIndex = TIKI_GetLocalBoneIndex(out,bone->parent);
				ptr = (float *) ( (byte *)bone + sizeof(skdBone_t) );
				VectorScale(ptr,out->scale,wrist->const_offset);	
			}
			break;
			case JT_HOSEROT:
			{
				char*txt;
				out->bones[i] = Hunk_Alloc(sizeof(tikiBoneHoseRot_t),h_low);
				hose = (tikiBoneHoseRot_t*)out->bones[i];
				hose->type=bone->jointType;
				hose->parentIndex = TIKI_GetLocalBoneIndex(out,bone->parent);
				ptr = (float *) ( (byte *)bone + sizeof(skdBone_t) );
				Com_Printf("HOSEROT bendRatio %f \n",*ptr);
				hose->bendRatio = *ptr;
				ptr++;
				Com_Printf("HOSEROT bendMax %f \n",*ptr);
				hose->bendMax = *ptr;
				ptr++;
				Com_Printf("HOSEROT spinRatio %f \n",*ptr);
				hose->spinRatio = *ptr;
				ptr++;
				VectorScale(ptr,out->scale,hose->basePos);
				ptr+=3;
				//Com_Printf("%f %f %f \n",ptr[0],ptr[1],ptr[2]);
				ptr+=3; // skip 1.f 1.f 1.f
				Com_Printf("HOSEROT hoseRotType %i \n",*(int*)ptr);
				hose->hoseRotType = *(hoseRotType_t*)ptr;
				ptr++;
				txt = (char*)ptr;
				//Com_Printf("\n HOSEref <%s>\n",txt);
				hose->targetIndex = TIKI_GetLocalBoneIndex(out,txt);
			}
			break;
			case JT_AVROT: //
			{
				int size;
				out->bones[i] = Hunk_Alloc(sizeof(tikiBoneAVRot_t),h_low);
				av = (tikiBoneAVRot_t*)out->bones[i];
				av->type=bone->jointType;
				av->parentIndex = TIKI_GetLocalBoneIndex(out,bone->parent);
				ptr = (float *) ( (byte *)bone + sizeof(skdBone_t) );
//				Com_Printf("AVROT data weight %f offs %f %f %f unknown %f %f %f %f", ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7],ptr[8]);
				av->m_bone2weight = *ptr;
				ptr++;
				VectorScale(ptr,out->scale,av->const_offset);
				ptr+=3;
				ptr+=3; //skip 1 1 1
				size = (int)bone + (int)bone->ofsEnd - (int)ptr;
	//			Com_Printf("reference 1 %s ",(char*)ptr);
				bytePtr = (char*)ptr;
				av->m_reference1 = TIKI_GetLocalBoneIndex(out,bytePtr); 
				while(*bytePtr != 0)
					bytePtr++;
				bytePtr++;
	//			Com_Printf("reference 2 %s\n",(char*)ptr);
				av->m_reference2 = TIKI_GetLocalBoneIndex(out,bytePtr); 
			}
			break;
			default:
				Com_Error(1,"unkwno nbone type %i",bone->jointType);
			break;
		}
		bone = (skdBone_t *)( (byte *)bone + bone->ofsEnd );
	}

	out->numSurfaces = header->numSurfaces;
	out->surfs = Z_Malloc( header->ofsBoxes-header->ofsSurfaces);
	memcpy(out->surfs,( ((byte *)header) + header->ofsSurfaces ),header->ofsBoxes-header->ofsSurfaces);
	surf = out->surfs;
	for ( i = 0 ; i < header->numSurfaces ; i++) {
		surf->ident = 12; //SF_SKD;
		vert = (skdVertex_t *) ( (byte *)surf + surf->ofsVerts );
		for ( j = 0 ; j < surf->numVerts ; j++) {
			weight = (skdWeight_t *) ( (byte *)vert + sizeof(skdVertex_t)+(sizeof(skdMorph_t)*vert->numMorphs));
			for ( k = 0 ; k < vert->numWeights ; k++) {
				VectorScale(weight->offset,out->scale,weight->offset);
				if(weight->boneWeight<0 || weight->boneWeight>1) {
					Com_Error(ERR_DROP, "invalid bone weight %f in %s \n",weight->boneWeight,out->name);
				}
				weight = (skdWeight_t *) ( (byte *)weight + sizeof(skdWeight_t));
			}
			vert = (skdVertex_t *) ( (byte *)vert + sizeof(skdVertex_t) + vert->numWeights*sizeof(skdWeight_t) + vert->numMorphs*sizeof(skdMorph_t) );
		}
		surf = (skdSurface_t *)( (byte *)surf + surf->ofsEnd );
	}
	FS_FreeFile(buf);
}

tikiAnim_t *TIKI_CacheAnim(char *fname, float scale) {
	int				filesize;
	char			*buf,*c;
	skcHeader_t		*header;
	skcFrame_t		*frame;
	int				i,j;
	tikiAnim_t		*anim;
	char			channelTypes[256];
	int				nameIndexes[256];
	int				*channelNames;

	//Com_Printf("TIKI_CacheAnim for %s, \n",  fname);
	filesize = FS_ReadFile(fname, (void **)&buf);
	if(!buf) {
		Com_Printf ("TIKI_CacheAnim: couldn't load %s\n", fname);
		return 0;
	}
	header = (skcHeader_t *)buf;
	if (header->version != SKC_VERSION) {
		Com_Printf( "TIKI_CacheAnim: %s has wrong version (%i should be %i)\n",
				 fname, header->version, SKC_VERSION);
		return 0;
	}
	if (header->ident != SKC_IDENT) {
		Com_Printf( "TIKI_CacheAnim: %s has wrong ident (%i should be %i)\n",
				 fname, header->ident, SKC_IDENT);
		return 0;
	}

	anim = Hunk_Alloc( sizeof(*anim),h_high);
	strcpy(anim->fname,fname);
	anim->frameTime = header->frameTime;
	anim->numFrames = header->numFrames;

	c = ( (byte *)header + header->ofsChannels );
	for(i = 0; i < header->numChannels; i++) {
		int len = strlen(c);
		if(!memcmp((c+(len-3)),"pos",3)) {
			channelTypes[i] = 'p';
			anim->numPosChannels++;
			c[len-4]=0;
			nameIndexes[i] = TIKI_RegisterBoneName(c);
		}
		else if(!memcmp((c+(len-2)),"FK",2)) {
			channelTypes[i] = 'f';
			anim->numRotFKChannels++;
			c[len-6]=0;
			nameIndexes[i] = TIKI_RegisterBoneName(c);
		}
		else if(!memcmp((c+(len-3)),"rot",3)) {
			channelTypes[i] = 'r';
			anim->numRotChannels++;
			c[len-4]=0;
			nameIndexes[i] = TIKI_RegisterBoneName(c);
		}
		c+=32;
	}
	channelNames = Hunk_Alloc(header->numChannels*4,h_high);
	anim->posChannelNames = channelNames;
	anim->rotChannelNames = channelNames + anim->numPosChannels;
	anim->rotFKChannelNames = anim->rotChannelNames + anim->numRotChannels;

	anim->numPosChannels = 0;
	anim->numRotChannels = 0;
	anim->numRotFKChannels = 0;
	for(i = 0; i < header->numChannels; i++) {
		if(channelTypes[i] == 'p') {
			anim->posChannelNames[anim->numPosChannels] = nameIndexes[i];
			anim->numPosChannels++;
		}
		else if(channelTypes[i] == 'r')	{
			anim->rotChannelNames[anim->numRotChannels] = nameIndexes[i];
			anim->numRotChannels++;
		}
		else if(channelTypes[i] == 'f')	{
			anim->rotFKChannelNames[anim->numRotFKChannels] = nameIndexes[i];
			anim->numRotFKChannels++;
		}
	}

	anim->frames = Hunk_Alloc(header->numFrames*sizeof(tikiFrame_t),h_low );
	for(i = 0; i < header->numFrames; i++) {
		anim->frames[i].posChannels = Hunk_Alloc(sizeof(vec3_t)*anim->numPosChannels,h_high);
		anim->frames[i].rotChannels = Hunk_Alloc(sizeof(quat_t)*anim->numRotChannels,h_high);
		if(anim->numRotFKChannels)
			anim->frames[i].rotFKChannels = Hunk_Alloc(sizeof(quat_t)*anim->numRotFKChannels,h_high);
	}
	frame = (skcFrame_t *)( (byte *)header + sizeof(*header) );
	for(i = 0; i < header->numFrames; i++) {
		float *values = (float*)((byte *)header+frame->ofsValues);
		anim->numPosChannels = 0;
		anim->numRotChannels = 0;
		anim->numRotFKChannels = 0;
		for(j = 0; j < header->numChannels; j++) {
			if(channelTypes[j] == 'p')	{
				VectorScale(values,scale,anim->frames[i].posChannels[anim->numPosChannels]);
				anim->numPosChannels++;
			}
			else if(channelTypes[j] == 'r')	{
				VectorCopy(values,anim->frames[i].rotChannels[anim->numRotChannels]);
				anim->frames[i].rotChannels[anim->numRotChannels][3] = values[3]; 
				anim->numRotChannels++;
			}
			else if(channelTypes[j] == 'f')	{
				VectorCopy(values,anim->frames[i].rotFKChannels[anim->numRotFKChannels]);
				anim->frames[i].rotFKChannels[anim->numRotFKChannels][3] = values[3];
				anim->numRotFKChannels++;
			}
			values += 4;
		}
		anim->frames[i].radius = frame->radius;
		VectorCopy(frame->bounds[0],anim->frames[i].bounds[0]);
		VectorCopy(frame->bounds[1],anim->frames[i].bounds[1]);
		frame++;
	}
	FS_FreeFile(buf);
	return anim;
}
/*
===============================
TIKI FILES LOADING AND PARSING
===============================
*/
int TIKI_CompareAnimName(const void *v1, const void *v2) {
	tikiAnim_t *anim1;
	tikiAnim_t *anim2;
	anim1 = *(tikiAnim_t**)v1;
	anim2 = *(tikiAnim_t**)v2;
	return strcmp((anim1)->alias,(anim2)->alias);
}
char	*sections[] = {
	"SECTION_ROOT",
	"SECTION_SETUP",
	"SECTION_INIT",
	"SECTION_ANIMATIONS"
};

static tiki_t *TIKI_Load(const char *fname) {
	tiki_t			*out;
	char			*buf,*text, *token;
	char			tmp[128], tmp2[128];
	char			path[128];
	skdSurface_t	*sf;
	int				len;
	int				i, j, level = 0;
	tikiAnim_t		*anim;
	tikiAnim_t		*anims[1024];
	int				numCmds;
	tikiCommand_t	cmds[512];
	char			*cmdTxts[512];
	char			cmdText[2048];
	tiki_t			*includes[TIKI_MAX_INCLUDES];
	struct
	{
		char surface[64];
		char shader[64];
	} surfShaders[32];
	int numSurfShaders;
	enum {
		SECTION_ROOT,
		SECTION_SETUP,
		SECTION_INIT,
		SECTION_ANIMATIONS
	}				section = SECTION_ROOT;
	len = FS_ReadFile(fname, (void **)&buf);
	if (!buf) {
		Com_Printf("WARNING: TIKI_Load: Failed to open \"%s\"\n", fname);
		return 0;
	}
	numSurfShaders = 0;

	out = Hunk_Alloc(sizeof(*out), h_high);
	out->scale = 0.52f; // set default scale to 16/30.5 since world 
				// is in 16 units per foot and model is in cm's
	strcpy(out->name,fname);
	text = buf;
	COM_BeginParseSession(fname);
	token = COM_ParseExt(&text, qtrue);
	if(Q_stricmp(token,"TIKI")) {
	//	Com_Printf("WARNING: TIKI_Load: tiki file %s has wrong ident %s, should be TIKI\n",fname,token);
		goto again;
	}

	while (*token) {
		token = COM_ParseExt(&text, qtrue);
again:
		if(*token==0)
			token = COM_ParseExt(&text, qtrue);
		if (!Q_stricmp(token, "path") || !Q_stricmp(token, "$path")) {
			token = COM_ParseExt(&text, qtrue);
			strcpy(path,token);
		}
		else if(token[0]=='}') {
			level--;
			if(level == 0) {
				section = SECTION_ROOT;
			}
			token++;
			goto again;
		}
		else {
			switch (section) {
				case SECTION_ROOT: 
					if (!Q_stricmp(token, "setup"))	{
						token = COM_ParseExt(&text, qtrue);
						if(token[0]!='{')
							Com_Error(1,"expected { to follow setup, found %s",token);
						token++;
						level++;
						section = SECTION_SETUP;
						goto again;
					}
					else if (!Q_stricmp(token, "init")) {
						token = COM_ParseExt(&text, qtrue);
						if(token[0]!='{')
							Com_Error(1,"expected { to follow init, found %s",token);
						token++;
						level++;
						section = SECTION_INIT;
						goto again;
					}
					else if (!Q_stricmp(token, "animations")) {
						token = COM_ParseExt(&text, qtrue);
						if(token[0]!='{')
							Com_Error(1,"expected { to follow animations, found %s",token);
						token++;
						level++;
						section = SECTION_ANIMATIONS;
						goto again;
					}
					else if (!Q_stricmp(token, "$include")) {
						token = COM_ParseExt(&text, qtrue);
						strcpy(tmp,token);
						includes[out->numIncludes] = TIKI_RegisterModel(tmp);
						out->numIncludes++;
						goto again;
					}
					else {
						Com_Printf("Unknown token %s in section %s of file %s\n",token,sections[section],fname);
					}
				break;
				case SECTION_SETUP:
					if (!Q_stricmp(token, "skelmodel")) {
						token = COM_ParseExt(&text, qtrue);
						strcpy(tmp,path);
						i = strlen(tmp);
						if(tmp[i-1] != '/')
							strcat(tmp,"/");
						strcat(tmp,token);
						if(out->surfs == 0)
							TIKI_AppendSKD(out,tmp);
#if 1
						else
							TIKI_MergeSKD(out,tmp);
#endif
					}
					else if (!Q_stricmp(token, "surface")) {
						token = COM_ParseExt(&text, qtrue);
						strcpy(tmp,token);
						token = COM_ParseExt(&text, qtrue);
						if (!Q_stricmp(token, "shader")) {
							strcpy(surfShaders[numSurfShaders].surface,tmp);
							token = COM_ParseExt(&text, qtrue);
							strcpy(surfShaders[numSurfShaders].shader,token);

							numSurfShaders++;
						}
					}
					else if (!Q_stricmp(token, "scale")) {
						token = COM_ParseExt(&text, qtrue);
						out->scale = atof(token);
					}
					else {
						Com_Printf("Unknown token %s in section %s of file %s\n",token,sections[section],fname);
					}
				break;
				case SECTION_INIT:
					out->init = Hunk_Alloc(sizeof(tikiInit_t),h_low);
					if (!Q_stricmp(token, "server")) {
						token = COM_ParseExt(&text, qtrue);
						if(token[0]!='{')
							Com_Printf("expected { to follow server, found %s\n",token);

						while(1) {
							token = COM_ParseExt(&text, qtrue);
							if(token[0] == '}') 
								break;
						}
					} else if (!Q_stricmp(token, "client")) {
						char **cmd = cmdTxts;
						numCmds = 0;
						token = COM_ParseExt(&text, qtrue);
						if(token[0]!='{')
							Com_Printf("expected { to follow server, found %s\n",token);

						while(1) {
							cmdText[0] = 0;
							while(1) {
								token = COM_ParseExt(&text, qfalse);
								if(token[0]==0 || token[0] == '}') {
									break;
								}
								if(cmdText[0]!=0) {
									strcat(cmdText," ");
								}
								strcat(cmdText,token);
							}
							if(cmdText[0]) {
								*cmd = Hunk_Alloc(strlen(cmdText)+1,h_low);
								strcpy(*cmd,cmdText);
								//Com_Printf("Event text: %s\n",*cmd);
								// next command
								cmd++;
								numCmds++;
							}
							if(token[0] == '}') 
								break;
						}
						if(numCmds) {
							out->init->numClientCmds = numCmds;
							out->init->clCmds = Hunk_Alloc(sizeof(char*)*numCmds,h_low);
							memcpy(out->init->clCmds,cmdTxts,sizeof(char*)*numCmds);
						}
					} else if(token[0]) {
						Com_Printf("Unknown token %s in section %s of file %s\n",token,sections[section],fname);
					}
				break;
				case SECTION_ANIMATIONS:
					strcpy(tmp,token);
					token = COM_ParseExt(&text, qtrue);
					strcpy(tmp2,path);
					i = strlen(tmp2);
					if(tmp2[i-1] != '/')
						strcat(tmp2,"/");
					strcat(tmp2,token);
					anim = TIKI_CacheAnim(tmp2,out->scale);
					if(anim) {
						strcpy(anim->alias,tmp);
						anims[out->numAnims] = anim;
						out->numAnims++;
					}
					while(1) {
						token = COM_ParseExt(&text, qfalse);
						if(!*token)
							break;
					}
					token = COM_ParseExt(&text, qtrue);
					if(token[0] == '{')	{
						while(1) {
							token = COM_ParseExt(&text, qtrue);
							if(token[0] == '}')
								break;
							if (!Q_stricmp(token, "server")) {
								token = COM_ParseExt(&text, qtrue);
								if(token[0]!='{')
									Com_Printf("expected { to follow server, found %s\n",token);

								while(1) {
									token = COM_ParseExt(&text, qtrue);
									if(token[0] == '}') 
										break;
								}
							} else if (!Q_stricmp(token, "client")) {
								tikiCommand_t *cmd = cmds;
								numCmds = 0;
								token = COM_ParseExt(&text, qtrue);
								if(token[0]!='{')
									Com_Printf("expected { to follow server, found %s\n",token);

								while(1) {
									cmdText[0] = 0;
									token = COM_ParseExt(&text, qtrue);
									if(token[0] == '}')
										break;
									// parse frame index
									if(!Q_stricmp(token,"entry") || !Q_stricmp(token,"enter")) {
										cmd->frame = TIKI_FRAME_ENTRY;
									} else if(!Q_stricmp(token,"exit")) {
										cmd->frame = TIKI_FRAME_EXIT;
									} else if(!Q_stricmp(token,"first")) {
										cmd->frame = 0;
									} else if(!Q_stricmp(token,"last")) {
										cmd->frame = anim->numFrames-1;
									} else {
										cmd->frame = atoi(token);
									}
									// parse event
									while(1) {
										token = COM_ParseExt(&text, qfalse);
										if(token[0]==0 || token[0] == '}') {
											break;
										}

#if 1
										// HACK - MoHAA allows linebreaks in tagspawn commands
										if(!Q_stricmp(token,"tagspawn")) {
											if(cmdText[0]!=0) {
												strcat(cmdText," ");
											}
											strcat(cmdText,token);
											do {
												token = COM_ParseExt(&text, qtrue);
												if(token[0]==0) {
													Com_Printf("Unexpected end of file found in tagspawn command block in tiki file %s\n",fname);
													break;
												}
												if(cmdText[0]!=0) {
													strcat(cmdText," ");
												}
												strcat(cmdText,token);
											} while(token[0] != ')');
										}
#endif
										if(cmdText[0]!=0) {
											strcat(cmdText," ");
										}
										strcat(cmdText,token);
									}
									if(cmdText[0]) {
										cmd->text = Hunk_Alloc(strlen(cmdText)+1,h_low);
										strcpy(cmd->text,cmdText);
										//Com_Printf("Event text: %s\n",cmd->text);
										// next command
										cmd++;
										numCmds++;
									}
									if(token[0] == '}') 
										break;
								}
								if(numCmds) {
									anim->numClientCmds = numCmds;
									anim->clientCmds = Hunk_Alloc(sizeof(tikiCommand_t)*numCmds,h_low);
									memcpy(anim->clientCmds,cmds,sizeof(tikiCommand_t)*numCmds);
								}
							} else {
								Com_Printf("Unknown token %s in section %s of file %s\n",token,sections[section],fname);
							}	
						}
					}
					else {
						if(*token==0)
							break;
						goto again;
					}

				break;
			}
		}
	}

	out->includes = Hunk_Alloc(sizeof(tiki_t*)*out->numIncludes,h_high);
	memcpy(out->includes,includes,sizeof(tiki_t*)*out->numIncludes);

	// merge surfaces of all includes
	for(i = 0; i < out->numIncludes; i++) {
		if(out->includes[i]->surfs) {
#if 1 //FIXME!
			if(!out->surfs)	{
				out->surfs = out->includes[i]->surfs;
				out->numSurfaces = out->includes[i]->numSurfaces;
				out->surfShaders = out->includes[i]->surfShaders;
				out->boneNames =  out->includes[i]->boneNames;
				out->bones =  out->includes[i]->bones;
				out->numBones =  out->includes[i]->numBones;
			}
#endif
		}
	}

	// build anim list
	for(i = 0; i < out->numIncludes; i++) {
		tikiAnim_t **freeAnimPtr = &anims[out->numAnims];
		out->numAnims+=out->includes[i]->numAnims;
		memcpy(freeAnimPtr,out->includes[i]->anims,out->includes[i]->numAnims*sizeof(tikiAnim_t*));
	}
	qsort(anims,out->numAnims,sizeof(tikiAnim_t*),TIKI_CompareAnimName);
#if 0
	{
		char str[512];
		FILE *f = fopen("anims_sorted.txt","w");
		sprintf(str,"Sorted anims for tiki file %s (total anims %i)\n",out->name,out->numAnims);
		fwrite(str,strlen(str),1,f);
		for(i = 0; i < out->numAnims; i++) {
			sprintf(str,"%i: %s (%s)\n",i,anims[i]->alias,anims[i]->fname);
			fwrite(str,strlen(str),1,f);		
		}
		fclose(f);
	}
#endif
	out->anims = Hunk_Alloc(sizeof(tikiAnim_t*)*out->numAnims,h_high);
	memcpy(out->anims,anims,sizeof(tikiAnim_t*)*out->numAnims);

	if(!out->surfShaders) {
		out->surfShaders = Hunk_Alloc(sizeof(qhandle_t)*out->numSurfaces,h_high);
		sf = out->surfs;
		for(i = 0; i < out->numSurfaces; i++) {
			for(j = 0; j < numSurfShaders; j++) {
				// "all" keyword is used in eg. models/ammo/thompson_clip.tik
				if(!Q_stricmp(surfShaders[j].surface,"all")) {
					out->surfShaders[i] = RE_RegisterShader(surfShaders[j].shader);
				} else {
					int testLen;
					char *star;
					// '*' is used in eg. models/weapons/mp40.tik
					star = strchr(surfShaders[j].surface,'*');
					if(star) {
						testLen = star - surfShaders[j].surface;
					} else {
						testLen = strlen(surfShaders[j].surface)+1;
					}
					if(!Q_stricmpn(sf->name,surfShaders[j].surface,testLen)) {
						out->surfShaders[i] = RE_RegisterShader(surfShaders[j].shader);
					}
				}
			}
			sf = (skdSurface_t*)(((byte*)sf)+sf->ofsEnd);
		}
	}
	FS_FreeFile(buf);
	return out;
}

// called from Hunk_Clear
void TIKI_ClearUp() {
	memset(hashTable,0,TIKI_FILE_HASH_SIZE*4);
	memset(globalBoneNames,0,sizeof(globalBoneNames));
	freeBoneName = globalBoneNames;
	memset(boneHashTable,0,sizeof(boneHashTable));
}
tiki_t	*TIKI_RegisterModel(const char *fname) {
	tiki_t	*tiki;
	int		hash;
	if(!*fname)
		return 0;
#if 0
	if(!strcmp("models/weapons/steilhandgranate.tik",fname))
		__asm int 3
#endif
	hash = generateHashValue(fname, TIKI_FILE_HASH_SIZE);
	// see if the TIKI is already loaded
	for (tiki = hashTable[hash]; tiki; tiki = tiki->next) {
		if (!Q_stricmp(tiki->name, fname))
			// match found
			return tiki;
	}
	tiki = TIKI_Load(fname);
	if (!tiki) {
		Com_Printf("WARNING: TIKI_Load: Failed to load \"%s\"\n", fname);
		return 0;
	}
	if (hashTable[hash]) {
		tiki->next = hashTable[hash];
	}
	hashTable[hash] = tiki;
	return tiki;
}

/*
===============================
DATA QUERYING API
===============================
*/
