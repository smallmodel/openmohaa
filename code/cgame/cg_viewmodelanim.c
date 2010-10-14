/*
===========================================================================
Copyright (C) 2010 su44

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

char itemNames[64][64];
void CG_RegisterItemName(int index, const char *str) {
	if(index >= 64 || index < 0) {
		CG_Error("CG_RegisterItemName: item index out of range 64 (%i, %s)\n",index,str);
		return;
	}
	strcpy(itemNames[index],str);
}

const char *CG_GetItemName(int index) {
	if(index >= 64 || index < 0) {
		CG_Error("CG_GetItemName: item index out of range 64 (%i)\n",index);
		return 0;
	}
	return itemNames[index];
}
//papers  colt45  p38 histandard  garand  kar98   kar98sniper springfield thompson    mp40    bar mp44    fraggrenade stielhandgranate    bazooka panzerschreck   shotgun unarmed
//Papers  Colt 45 Walther P38 Hi-Standard Silenced    M1 Garand   Mauser KAR 98K  KAR98 - Sniper  Springfield '03 Sniper  Thompson    MP40    BAR StG 44  Frag Grenade    Stielhandgranate    Bazooka Panzerschreck   Shotgun
const char *CG_GetVMAnimPrefixString(int index) {
	const char *wpn;
	wpn = CG_GetItemName(index);
	if(wpn) {
		if(!Q_stricmp(wpn,"Papers"))
			return "papers";
		if(!Q_stricmp(wpn,"Colt 45"))
			return "colt45";
		if(!Q_stricmp(wpn,"Walther P38"))
			return "p38";
		if(!Q_stricmp(wpn,"Hi-Standard Silenced"))
			return "histandard";
		if(!Q_stricmp(wpn,"M1 Garand"))
			return "garand";
		if(!Q_stricmp(wpn,"Mauser KAR 98K"))
			return "kar98";
		if(!Q_stricmp(wpn,"KAR98 - Sniper"))
			return "kar98sniper";
		if(!Q_stricmp(wpn,"Springfield '03 Sniper"))
			return "springfield";
		if(!Q_stricmp(wpn,"Thompson"))
			return "thompson";
		if(!Q_stricmp(wpn,"MP40"))
			return "mp40";
		if(!Q_stricmp(wpn,"BAR"))
			return "bar";
		if(!Q_stricmp(wpn,"StG 44"))
			return "mp44";
		if(!Q_stricmp(wpn,"Frag Grenade"))
			return "fraggrenade";
		if(!Q_stricmp(wpn,"Stielhandgranate"))
			return "stielhandgranate";
		if(!Q_stricmp(wpn,"Bazooka"))
			return "bazooka";
		if(!Q_stricmp(wpn,"Panzerschreck"))
			return "panzerschreck";
		if(!Q_stricmp(wpn,"Shotgun"))
			return "shotgun";
	}
	return "unarmed";
}

char	*viewModelAnimTypes[] = {
	"idle", // 0
	"charge", // 1
	"fire", // 2
	"fire_secondary", // 3
	"rechamber", // 4
	"reload", // 5
	"reload_single", // 6
	"reload_end", // 7
	"pullout", // 8
	"putaway", // 9
	"ladderstep" // 10
};
const char *CG_GetVMTypeString(int index) {
	if(index < 0 || index > VMA_NUMANIMATIONS) {
		return "idle";
	}
	return viewModelAnimTypes[index];
}
#define ITEM_WEAPON 1
void CG_ViewModelAnim() {
	char anim[128];
	char tmp[128];
	int i;
	char *ptr;
	tiki_t *fps, *tiki;
	refEntity_t ent;
	bone_t *bone;
	vec3_t v,a;

	// mp40_reload, mp40_fire
	if(cg.predictedPlayerState.activeItems[ITEM_WEAPON]!=-1)	{
		sprintf(anim,"%s_%s",CG_GetVMAnimPrefixString(cg.predictedPlayerState.activeItems[ITEM_WEAPON]),CG_GetVMTypeString(cg.predictedPlayerState.iViewModelAnim));
		//CG_Printf("VMA %s \n",anim);
	} else
		return;
#if 0
	for(i = 0; i < 8; i++) {
		if(cg.predictedPlayerState.activeItems[i]!= -1 && cg.predictedPlayerState.activeItems[i]) {
			CG_Printf("Item %i of 8, %s (indx %i)\n",i,itemNames[cg.predictedPlayerState.activeItems[i]],cg.predictedPlayerState.activeItems[i]);
		}
		/*
item 0 "pistol"
item 1 "Walther P38"
item 2 "Binoculars"

item 0 "mg"
item 1 "MP44"
item 2 "Binoculars"
		*/
	}
#endif
	tiki = cgs.gameTIKIs[cg.predictedPlayerEntity.currentState.modelindex]; // doesnt work... currentState.modelindex == 0
	if(!tiki) {
		tiki = trap_TIKI_RegisterModel("models/player/american_army.tik");
		if(!tiki)
			return;
	}
	strcpy(tmp,tiki->name);
	ptr = strchr(tmp,'.');
	if(!ptr)
		return;
	*ptr = 0;
	strcat(tmp,"_fps.tik");
	fps = trap_TIKI_RegisterModel(tmp);
	if(!fps) {
		CG_Printf("CG_ViewModelAnim: error, cannot register first person player model for %s\n",tiki->name);
		return;
	}
	for(i = 0; i < fps->numAnims; i++) {
//		if(!Q_stricmp(anim,fps->anims[i]->alias)) {
		if(!Q_stricmpn(anim,fps->anims[i]->alias,strlen(anim))) {
			break;
		}
	}
	if(fps->numAnims == i) {
		CG_Printf("WARNING: vma %s not found in %s\n",anim,fps->name);
		return;
	}
	cg.viewModelAnimTime += ((float)cg.frametime/1000);
	if(cg.lastViewModelAnim != i) {
		cg.viewModelAnimTime = 0;
		cg.lastViewModelAnim = i;
	}
	// su44: ok, we have chosen the proper viewmodelanim
	// but ent.origin and ent.axis still needs to be adjusted
	memset(&ent,0,sizeof(ent));

	ent.hModel = trap_R_RegisterModel(tmp);
	ent.bones = trap_TIKI_GetBones(fps->numBones);
	trap_TIKI_SetChannels(fps,i,cg.viewModelAnimTime,1,ent.bones);
	trap_TIKI_Animate(fps,ent.bones);

#if 0
	boneName = trap_TIKI_GetBoneNameIndex("eyes bone");
	for(i = 0; i < fps->numBones; i++) {
		if(fps->boneNames[i] == boneName) {
			bone = ent.bones+i;
			break;
		}
	}
	////CG_BoneLocal2World(bone,cg.refdef.vieworg,cg.refdefViewAngles,v,a);
#endif

	VectorCopy( cg.refdef.vieworg, ent.origin );

	VectorMA( ent.origin, cg_gun_x.value, cg.refdef.viewaxis[0], ent.origin );
	VectorMA( ent.origin, cg_gun_y.value, cg.refdef.viewaxis[1], ent.origin );
	VectorMA( ent.origin, cg_gun_z.value, cg.refdef.viewaxis[2], ent.origin );

	AnglesToAxis( cg.refdefViewAngles, ent.axis );

	trap_R_AddRefEntityToScene(&ent);
}