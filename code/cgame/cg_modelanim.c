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
//translate bone_s b to pos/rot coordinates space. (treat pos/rot as parent bone orientation)
void CG_BoneLocal2World(bone_t *b, vec3_t pos, vec3_t rot, vec3_t outPos, vec3_t outRot) {
	quat_t q,qpos,res,tmp;
	quat_t outQuat;
#if 0
	QuatFromAngles(q,rot[0],rot[1],rot[2]);
#else
	matrix_t m;
	MatrixFromAngles(m,rot[0],rot[1],rot[2]);
	QuatFromMatrix(q,m);
#endif
	VectorCopy(b->p,qpos);
	qpos[3] = 0;


	QuaternionMultiply(tmp,qpos,q);

	QuatInverse(q);
	QuaternionMultiply(res,q,tmp);
	QuatInverse(q);

	outPos[0] = res[0] + pos[0];
	outPos[1] = res[1] + pos[1];
	outPos[2] = res[2] + pos[2];

	QuaternionMultiply(outQuat,b->q,q);
	QuatNormalize(outQuat);
#if 0
	QuatToAngles(outQuat, outRot);
#else
	MatrixFromQuat(m,outQuat);
	MatrixToAngles(m,outRot);
#endif
}
void CG_AttachEntity(refEntity_t *e, centity_t *parent, int boneIndex) {
	bone_t *b;
	vec3_t a;
	if(parent->bones==0)
		return;
	b = &parent->bones[boneIndex];
//	CG_BoneLocal2World(b,parent->lerpOrigin,parent->lerpAngles,e->origin,e->axis);
	CG_BoneLocal2World(b,parent->currentState.origin,parent->currentState.angles,e->origin,a);
	AnglesToAxis(a,e->axis);
}
void CG_ModelAnim( centity_t *cent ) {
	refEntity_t ent;
	entityState_t *s1;
	tiki_t *tiki;
	int i;
	cent->bones = 0;
	s1 = &cent->currentState;

	memset(&ent,0,sizeof(ent));

	if(s1->tag_num != -1 && s1->parent != 1023) {
		CG_AttachEntity(&ent,&cg_entities[s1->parent],s1->tag_num);
	} else {
	//	AnglesToAxis(cent->lerpAngles,ent.axis);
	//	VectorCopy(cent->lerpOrigin,ent.origin);
		AnglesToAxis(cent->currentState.angles,ent.axis);
		VectorCopy(cent->currentState.origin,ent.origin);
	}

	ent.hModel = cgs.gameModels[s1->modelindex];
	
	tiki = cgs.gameTIKIs[s1->modelindex];
	if(tiki && tiki->numAnims) {
		int idleIndex = 0;
		ent.bones = trap_TIKI_GetBones(tiki->numBones);
		cent->bones = ent.bones;
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
	}
	// player model
	if ( cent->currentState.number == cg.snap->ps.clientNum) {
#if 1 //calculate eye pos/rot for usereyes_t
		if(tiki) {
			vec3_t eyePos,eyeRot;
#if 1
			VectorSet(eyePos,0,0,cg.predictedPlayerState.viewheight);

			VectorCopy(cg.predictedPlayerState.viewangles,eyeRot);
			trap_SetEyeInfo(eyePos,eyeRot);
#else
			int eyeBoneName;
			eyeBoneName = trap_TIKI_GetBoneNameIndex("eyes bone");//("tag_weapon_right");
			for(i = 0; i < tiki->numBones; i++) {
				if(tiki->boneNames[i] == eyeBoneName) {
					CG_BoneLocal2World(ent.bones + i,ent.origin,cent->currentState.angles,eyePos,eyeRot);

					VectorCopy(cg.predictedPlayerState.viewangles,eyeRot);
					VectorSubtract(eyePos,cent->currentState.origin,eyePos);
					trap_SetEyeInfo(eyePos,eyeRot);
				}
			}
#endif
		}
#endif
		if (!cg.renderingThirdPerson) {
			ent.renderfx = RF_THIRD_PERSON;			// only draw in mirrors
		} else {
			if (cg_cameraMode.integer) {
				return;
			}
		}
	}
	trap_R_AddRefEntityToScene(&ent);
}

