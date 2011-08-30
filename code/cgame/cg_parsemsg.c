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
// cg_parsemsg.c

#include "cg_local.h"

typedef struct hdelement_s {
	qhandle_t hShader;
	char shaderName[64];
	int iX;
	int iY;
	int iWidth;
	int iHeight;
	float vColor[4];
	int iHorizontalAlign;
	int iVerticalAlign;
	qboolean bVirtualScreen;
	char string[2048];
	char fontName[64];
	fontInfo_t pFont;
} hdelement_t;

#define MAX_HDELEMENTS 64

hdelement_t hdelements[MAX_HDELEMENTS];

typedef struct {
	float i_vBarrel[3];
	float i_vStart[3];
	float (*i_vEnd)[3];
	int i_iNumBullets;
	qboolean iLarge;
	int iTracerVisible;
	qboolean bIgnoreEntities;
} bullet_tracer_t;

#define MAX_IMPACTS 64
#define MAX_BULLER_TRACERS 32
#define MAX_BULLET_TRACE_BULLETS 1024

// bullet tracers
static bullet_tracer_t bullet_tracers[MAX_BULLER_TRACERS];
static int bullet_tracers_count;
static float bullet_tracer_bullets[MAX_BULLET_TRACE_BULLETS][3];
static int bullet_tracer_bullets_count;

// wall impacts
static vec3_t wall_impact_pos[MAX_IMPACTS];
static vec3_t wall_impact_norm[MAX_IMPACTS];
static int wall_impact_large[MAX_IMPACTS];
static int wall_impact_type[MAX_IMPACTS]; // 0,2,3,4,5
static int wall_impact_count;
// flesh impacts
static vec3_t flesh_impact_pos[MAX_IMPACTS];
static vec3_t flesh_impact_norm[MAX_IMPACTS];
static int flesh_impact_large[MAX_IMPACTS];
static int flesh_impact_type[MAX_IMPACTS];
static int flesh_impact_count;

static int current_entity_number;

void CG_MakeBulletHole(float *i_vPos, float *i_vNorm, int iLarge,
	trace_t *pPreTrace, qboolean bMakeSound) {
	char *s;


	s = "bhole_";

	///////CG_ImpactMark(0,i_vPos,i_vNorm,0,1,1,1,1,qfalse,8,qfalse);
}

void CG_AddBulletImpacts() {
	int i;
	char *s;
	int type;

	for(i = 0; i < wall_impact_count; i++) {
		CG_MakeBulletHole(wall_impact_pos[i],wall_impact_norm[i],
			wall_impact_large[i],0,qtrue);

		type = wall_impact_type[i];
		if(type) {
			if ( wall_impact_type[i] < 2 || wall_impact_type[i] > 3 )
				s = "snd_bh_metal";
			else
				s = "snd_bh_wood";
			//CG_PlaySound(s,wall_impact_pos[i],-1, 
		}
	}
	for(i = 0; i < flesh_impact_count; i++) {

	}
}

static void CG_MakeBubbleTrail(float *i_vStart, float *i_vEnd, int iLarge) {
	// TODO
}

static void CG_MakeBulletTracerInternal(float *i_vBarrel, float *i_vStart,
	float (*i_vEnd)[3], int i_iNumBullets, qboolean iLarge,
	int iTracerVisible, qboolean bIgnoreEntities) {
	trace_t tr;
	int i;


	////for(i = 0; i < i_iNumBullets; i++) {
	////	CG_Trace(&tr,i_vStart,i_vEnd[i],vec3_origin,vec3_origin,-1,MASK_SHOT);
	////	if(tr.fraction != 1.f) {
	////		CG_MakeBulletHole(tr.endpos,tr.plane.normal,iLarge,&tr,qfalse);
	////	}
	////}

}

static void CG_MakeBulletTracer(float *i_vBarrel, float *i_vStart,
	float (*i_vEnd)[3], int i_iNumBullets, qboolean iLarge,
	int iTracerVisible, qboolean bIgnoreEntities) {
	bullet_tracer_t *bt;
	int i;
	if(bullet_tracers_count < MAX_BULLER_TRACERS) {
		if(i_iNumBullets + bullet_tracer_bullets_count < MAX_BULLET_TRACE_BULLETS) {
			bt = bullet_tracers + bullet_tracers_count;
			VectorCopy(i_vBarrel,bt->i_vBarrel);
			VectorCopy(i_vStart,bt->i_vStart);
			bt->i_vEnd = &bullet_tracer_bullets[bullet_tracer_bullets_count];
			bt->i_iNumBullets = i_iNumBullets;
			bt->iLarge = iLarge;
			bt->iTracerVisible = iTracerVisible;
			bt->bIgnoreEntities = bIgnoreEntities;
			for(i = 0; i < i_iNumBullets; i++) {
				VectorCopy(i_vEnd[i],bullet_tracer_bullets[bullet_tracer_bullets_count]);
				bullet_tracer_bullets_count++;
			}
		} else {
			CG_Printf("CG_MakeBulletTracerInternal: MAX_BULLET_TRACE_BULLETS exceeded\n");
		}
	} else {
		CG_Printf("CG_MakeBulletTracer: MAX_BULLER_TRACERS exceeded\n");
	}
}

// su44: this should be called after CG_AddPacketEntities and before CG_DrawActive
void CG_AddBulletTracers() {
	int i;
	bullet_tracer_t *bt;

	bt = bullet_tracers;
	for(i = 0; i < bullet_tracers_count; i++,bt++) {
		CG_MakeBulletTracerInternal(bt->i_vBarrel,bt->i_vStart,bt->i_vEnd,bt->i_iNumBullets,
			bt->iLarge,bt->iTracerVisible,bt->bIgnoreEntities);
	}
	bullet_tracers_count = 0;
	bullet_tracer_bullets_count = 0;
}
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
static void CG_MeleeImpact(float *vStart, float *vEnd) {
	trace_t trace;
	float vMins[3] = { -4.0, -4.0, 0 };
	float vMaxs[3] = { 4.0, 4.0, 2.0 };
	CG_Trace(&trace,vStart,vMins,vMaxs,vEnd,1023,MASK_SHOT);
	if(trace.fraction != 1.f) {
		CG_FootstepMain(&trace,1,0);
	}
}

static void CG_MakeExplosionEffect(float *vPos, int iType) {
	// TODO
}

static void CG_MakeEffect_Normal(int iEffect, vec3_t vPos, vec3_t vNormal) {
	// TODO
}

static void CG_SpawnEffectModel(char *model, vec3_t origin) {
	// TODO
}

static void CG_HudDrawShader (int iInfo) {
	hdelement_t *hdi = hdelements + iInfo;
	if(hdi->shaderName[0]) {
		hdi->hShader = trap_R_RegisterShaderNoMip(hdi->shaderName);
	} else {
		hdi->hShader = 0;
	}
}

static void CG_HudDrawFont (int iInfo) {
	hdelement_t *hdi = hdelements + iInfo;
	if(hdi->fontName[0]) {
		trap_R_RegisterFont(hdi->fontName,0,&hdi->pFont);
	} else {
		memset(&hdi->pFont,0,sizeof(hdi->pFont));
	}
}

static void CG_PlaySound(char *sound_name, float *origin, int channel,
	float volume, float min_distance, float pitch, int argstype) {
	ubersound_t* sound;

	sound = CG_GetUbersound(sound_name);


	trap_S_StartSound( origin, -1, channel, sound->sfxHandle );


}

/*
=====================
CG_ParseCGMessage
=====================
*/
void CG_ParseCGMessage() {
	int msgtype;
	vec3_t vecStart, vecTmp, vecEnd, vecArray[64];
	int iCount, iLarge;
	int i, iTemp;
	char *s;
	hdelement_t *hde;

	// thats a hack, it should be done somewhere else [?]
	wall_impact_count = 0;
	flesh_impact_count = 0;

	do {
		msgtype = trap_MSG_ReadBits( 6 );
//Com_Printf( "CG_ParseCGMessage: command type %i\n", msgtype ); 
		switch ( msgtype ) {
			case 1: // BulletTracer (visible?)
				vecTmp[0] = trap_MSG_ReadCoord();
				vecTmp[1] = trap_MSG_ReadCoord();
				vecTmp[2] = trap_MSG_ReadCoord();
			case 2: // BulletTracer (invisible?)
			case 5: // BubbleTrail
				vecStart[0] = trap_MSG_ReadCoord();
				vecStart[1] = trap_MSG_ReadCoord();
				vecStart[2] = trap_MSG_ReadCoord();
				if ( msgtype != 1 ) {
					vecTmp[0] = vecStart[0];
					vecTmp[1] = vecStart[1];
					vecTmp[2] = vecStart[2];
				}
				vecArray[0][0] = trap_MSG_ReadCoord();
				vecArray[0][1] = trap_MSG_ReadCoord();
				vecArray[0][2] = trap_MSG_ReadCoord();

				iLarge = trap_MSG_ReadBits( 1 );
				if(msgtype==5) {
					CG_MakeBubbleTrail(vecStart,vecArray[0],iLarge);
				} else if(msgtype==1) {
					// su44: I am not sure about the last parameters...
					CG_MakeBulletTracer(vecTmp, vecStart, vecArray, 1, iLarge, 1, 1);
				} else { //msgtype == 2
					CG_MakeBulletTracer(vecTmp, vecStart, vecArray, 1, iLarge, 0, 1);
				}
				break;
			case 3: // BulletTracer multiple times
				vecTmp[0] = trap_MSG_ReadCoord();
				vecTmp[1] = trap_MSG_ReadCoord();
				vecTmp[2] = trap_MSG_ReadCoord();
				iTemp = trap_MSG_ReadBits( 6 );
			case 4: // BulletTracer multiple times
				if ( msgtype == 4 )
					iTemp = 0;
				vecStart[0] = trap_MSG_ReadCoord();
				vecStart[1] = trap_MSG_ReadCoord();
				vecStart[2] = trap_MSG_ReadCoord();

				iLarge = trap_MSG_ReadBits( 1 );
				iCount = trap_MSG_ReadBits( 6 );

				// this check is missing in MOHAA code, so this has buffer overflow risk in AA
				if ( iCount > 64 )
					Com_Error( ERR_DROP, "CG message type 4 sent too many data.\n" );
				for (i=0;i<iCount;i++) {
					vecArray[i][0] = trap_MSG_ReadCoord();
					vecArray[i][1] = trap_MSG_ReadCoord();
					vecArray[i][2] = trap_MSG_ReadCoord();
				}
				CG_MakeBulletTracer(vecTmp, vecStart, vecArray, iCount, iLarge, iTemp, 1);
				break;
			case 6: // wall impact
			case 7: // flesh impact
			case 8: // flesh impact (?)
			case 9: // wall impact (?)
			case 10: // wall impact (?)
				vecStart[0] = trap_MSG_ReadCoord();
				vecStart[1] = trap_MSG_ReadCoord();
				vecStart[2] = trap_MSG_ReadCoord();

				trap_MSG_ReadDir( vecEnd );
				iLarge = trap_MSG_ReadBits( 1 );
				switch (msgtype) {
					case 6:
						if(wall_impact_count < MAX_IMPACTS) {
							VectorCopy(vecStart,wall_impact_pos[wall_impact_count]);
							VectorCopy(vecEnd,wall_impact_norm[wall_impact_count]);
							wall_impact_large[wall_impact_count] = iLarge;
							wall_impact_type[wall_impact_count] = 0;
							wall_impact_count++;
						}
						break;
					case 7:
						if(flesh_impact_count < MAX_IMPACTS) {
							// inverse hit normal, I dont know why,
							// but that's done by MoHAA
							VectorInverse(vecEnd);
							VectorCopy(vecStart,flesh_impact_pos[flesh_impact_count]);
							VectorCopy(vecEnd,flesh_impact_norm[flesh_impact_count]);
							flesh_impact_large[flesh_impact_count] = iLarge;
							flesh_impact_count++;
						}
						break;
					case 8:
						if(flesh_impact_count < MAX_IMPACTS) {
							// same here?
							VectorInverse(vecEnd);
							VectorCopy(vecStart,flesh_impact_pos[flesh_impact_count]);
							VectorCopy(vecEnd,flesh_impact_norm[flesh_impact_count]);
							flesh_impact_large[flesh_impact_count] = iLarge;
							flesh_impact_count++;
						}
						break;
					case 9:
						if(wall_impact_count < MAX_IMPACTS) {
							VectorCopy(vecStart,wall_impact_pos[wall_impact_count]);
							VectorCopy(vecEnd,wall_impact_norm[wall_impact_count]);
							wall_impact_large[wall_impact_count] = iLarge;
							if(iLarge) {
								wall_impact_type[wall_impact_count] = 3;
							} else {
								wall_impact_type[wall_impact_count] = 2;
							}
							wall_impact_count++;
						}
						break;
					case 10:
						if(wall_impact_count < MAX_IMPACTS) {
							VectorCopy(vecStart,wall_impact_pos[wall_impact_count]);
							VectorCopy(vecEnd,wall_impact_norm[wall_impact_count]);
							wall_impact_large[wall_impact_count] = iLarge;
							if(iLarge) {
								wall_impact_type[wall_impact_count] = 5;
							} else {
								wall_impact_type[wall_impact_count] = 4;
							}
							wall_impact_count++;
						}
						break;
					default:
						break;
				}
				break;
			case 11:
				vecStart[0] = trap_MSG_ReadCoord();
				vecStart[1] = trap_MSG_ReadCoord();
				vecStart[2] = trap_MSG_ReadCoord();
				vecEnd[0] = trap_MSG_ReadCoord();
				vecEnd[1] = trap_MSG_ReadCoord();
				vecEnd[2] = trap_MSG_ReadCoord();
				CG_MeleeImpact(vecStart,vecEnd);
				break;
			case 12:
			case 13:
				vecStart[0] = trap_MSG_ReadCoord();
				vecStart[1] = trap_MSG_ReadCoord();
				vecStart[2] = trap_MSG_ReadCoord();
				CG_MakeExplosionEffect(vecStart,msgtype);
				break;
			default: //unknown message
				Com_Error(ERR_DROP, "CG_ParseCGMessage: Unknown CG Message %i", msgtype);
				break;
			case 14:
			case 15: // MakeEffect
			case 16:
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
			case 22:
				vecStart[0] = trap_MSG_ReadCoord();
				vecStart[1] = trap_MSG_ReadCoord();
				vecStart[2] = trap_MSG_ReadCoord();
				trap_MSG_ReadDir( vecEnd );
				CG_MakeEffect_Normal(msgtype/* + 67*/,vecStart,vecEnd);
				break;
			case 23:
			case 24:
				vecStart[0] = trap_MSG_ReadCoord();
				vecStart[1] = trap_MSG_ReadCoord();
				vecStart[2] = trap_MSG_ReadCoord();
				i = trap_MSG_ReadByte();
				if(msgtype == 23) {
					s = va("models/fx/crates/debris_%i.tik",i);
				} else {
					s = va("models/fx/windows/debris_%i.tik",i);
				}
				CG_SpawnEffectModel(s, vecStart);
				break;
			case 25: // Bullet tracer
				vecTmp[0] = trap_MSG_ReadCoord();
				vecTmp[1] = trap_MSG_ReadCoord();
				vecTmp[2] = trap_MSG_ReadCoord();
				vecStart[0] = trap_MSG_ReadCoord();
				vecStart[1] = trap_MSG_ReadCoord();
				vecStart[2] = trap_MSG_ReadCoord();

				vecArray[0][0] = trap_MSG_ReadCoord();
				vecArray[0][1] = trap_MSG_ReadCoord();
				vecArray[0][2] = trap_MSG_ReadCoord();

				iLarge = trap_MSG_ReadBits( 1 );

				CG_MakeBulletTracer(vecTmp,vecStart,vecArray,1,iLarge,0,1);				
				break;
			case 26: // Bullet tracer
				vecTmp[0] = 0;
				vecTmp[1] = 0;
				vecTmp[2] = 0;
				vecStart[0] = trap_MSG_ReadCoord();
				vecStart[1] = trap_MSG_ReadCoord();
				vecStart[2] = trap_MSG_ReadCoord();

				vecArray[0][0] = trap_MSG_ReadCoord();
				vecArray[0][1] = trap_MSG_ReadCoord();
				vecArray[0][2] = trap_MSG_ReadCoord();

				iLarge = trap_MSG_ReadBits( 1 );
				CG_MakeBulletTracer(vecTmp,vecStart,vecArray,1,iLarge,0,1);
				break;
			case 27: // HUD drawing...
				i = trap_MSG_ReadByte();
				hde = hdelements + i;
				Q_strncpyz( hde->shaderName, trap_MSG_ReadString(), sizeof(hde->shaderName) );
				hde->string[0] = 0;
				memset(&hde->pFont,0,sizeof(hde->pFont));
				hde->fontName[0] = 0;	
				
				CG_HudDrawShader(i);
				break;
			case 28: // HUD drawing...
				i = trap_MSG_ReadByte();
				hde = hdelements + i;
				hde->iHorizontalAlign = trap_MSG_ReadBits( 2 );
				hde->iHorizontalAlign = trap_MSG_ReadBits( 2 );
				break;
			case 29:
				i = trap_MSG_ReadByte();
				hde = hdelements + i;
				hde->iX = trap_MSG_ReadShort();
				hde->iY = trap_MSG_ReadShort();
				hde->iWidth = trap_MSG_ReadShort();
				hde->iHeight = trap_MSG_ReadShort();
				break;
			case 30:
				i = trap_MSG_ReadByte();
				hde = hdelements + i;
				hde->bVirtualScreen = trap_MSG_ReadBits( 1 );
				break;
			case 31: // huddraw_color
				i = trap_MSG_ReadByte();
				hde = hdelements + i;
				hde->vColor[0] = trap_MSG_ReadByte()*0.003921568859368563;
				hde->vColor[1] = trap_MSG_ReadByte()*0.003921568859368563;
				hde->vColor[2] = trap_MSG_ReadByte()*0.003921568859368563;
				break;
			case 32: // huddraw_alpha
				i = trap_MSG_ReadByte();
				hde = hdelements + i;
				hde->vColor[3] = trap_MSG_ReadByte()*0.003921568859368563;
				break;
			case 33: // huddraw_string
				i = trap_MSG_ReadByte();
				hde = hdelements + i;
				hde->hShader = 0;
				Q_strncpyz( hde->string, trap_MSG_ReadString(), sizeof(hde->string) );
				break;
			case 34: // huddraw_font
				i = trap_MSG_ReadByte();
				hde = hdelements + i;
				Q_strncpyz( hde->fontName, trap_MSG_ReadString(), sizeof(hde->fontName) );
				hde->hShader = 0;
				hde->shaderName[0] = 0;
				CG_HudDrawFont(i);
				break;
			case 35:
			case 36:
				// TODO - play sound (?)
				//CG_PlaySound(s, 0, 5, 2.0, -1.0, vecStart, 1);
				break;
			case 37: // taunt message?
				vecStart[0] = trap_MSG_ReadCoord();
				vecStart[1] = trap_MSG_ReadCoord();
				vecStart[2] = trap_MSG_ReadCoord();

				iTemp = trap_MSG_ReadBits( 1 );
				i = trap_MSG_ReadBits( 6 );
				s = trap_MSG_ReadString();

				if(iTemp) {
					current_entity_number = i;

				}
				//CG_Printf("Case 37: iTemp %i, i %i, s %s\n",iTemp,i,s);
				CG_PlaySound(s,vecStart,5,-1.0,-1.0,-1.0,0);
				break;
		}
	} while ( trap_MSG_ReadBits(1) );
}
