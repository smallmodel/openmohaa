#pragma once

#include "ida_cgame.h"
#include "ida_common_tt.h"

typedef struct clientInfo_s
{
	teamType_t team;
	char name[64];

} clientInfo_t;

typedef struct cgs_s
{
	gameState_t gameState;
	glconfig_t glConfig;
	float screenXScale;
	float screenYScale;
	float screenXBias;
	int serverCommandSequence;
	int processedSnapshotNum;
	qboolean localServer;
	int levelStartTime;
	int matchEndTime;
	int serverLagTime;
	int gameType;
	int dmFlags;
	int teamFlags;
	int fragLimit;
	int timeLimit;
	int maxClients;
	int cinematic;
	int mapChecksum;
	char mapName[MAX_QPATH];
	int voteTime;
	int numVotesYes;
	int numVotesNo;
	int numUndecidedVotes;
	qboolean voteRefreshed;
	char voteString[1024];
	int modelDraw[MAX_MODELS];
	int unk1;
	int soundPrecache[MAX_SOUNDS];
	int numInlineModels;
	int inlineDrawModel[MAX_MODELS];
	vec3_t inlineModelMidpoints[MAX_MODELS];
	media_t media;

} cgs_t;

typedef struct cg_s
{
	int clientFrame;
	int clientNum;
	int demoPlayback;
	int levelShot;
	int latestSnapshotNum;
	int latestSnapshotTime;
	snapshot_t* snap;
	snapshot_t* nextSnap;
	snapshot_t activeSnapshots[MAX_ACTIVESNAPSHOTS];
	float frameInterpolation;
	int thisFrameTeleport;
	int nextFrameTeleport;
	int nextFrameCameraCut;
	int frametime;
	int time;
	int physicsTime;
	int renderingThirdPerson;
	int hyperSpace;
	playerState_t predicted_player_state;
	int validPPS;
	int predictedErrorTime;
	vec3_t predictedError;
    int         iWeaponCommand;
    int         iWeaponCommandSend;
	vec3_t autoAngles;
	vec3_t autoAxis[3];
	vec3_t autoAnglesSlow;
	vec3_t autoAxisSlow[3];
	vec3_t autoAnglesFast;
	vec3_t autoAxisFast[3];
	refDef_t refdef;
	vec3_t playerHeadPos;
	vec3_t refdefViewAngles;
	vec3_t currentViewPos;
	vec3_t currentViewAngles;
    float fCurrentViewHeight;
    float fCurrentViewBobPhase;
    float fCurrentViewBobAmp;
	int unk1[10];
    dtiki_t*    pLastPlayerWorldModel;
    dtiki_t*    pPlayerFPSModel;
    qhandle_t   hPlayerFPSModelHandle;
    qboolean    bFPSModelLastFrame;
    qboolean    bFPSOnGround;
    dtiki_t*    pAlliedPlayerModel;
    qhandle_t   hAlliedPlayerModelHandle;
    dtiki_t*    pAxisPlayerModel;
    qhandle_t   hAxisPlayerModelHandle;
    vec3_t      vOffsetViewAngles;
    vec3_t      vLastHeadAngles;
    vec3_t      vLastViewAngles;
    vec3_t      vEyeOffsetMax;
    float       fEyeOffsetFrac;
    float       SoundOrg[3];
    vec3_t      SoundAxis[3];
    vec3_t      camera_origin;
    vec3_t      camera_angles;
    float       camera_fov;
	float zoomSensitivity;
	int intermissionDisplay;
	int scoresRequestTime;
	int showScores;
	char scoresMenuName[256];
    int         iInstaMessageMenu;
	int centerPrintTime;
	int centerPrintCharWidth;
	int centerPrintY;
	char centerPrint[1024];
	int centerPrintLines;
	int matchStartTime;
	refEntity_t testModelEntity;
	char testModelName[MAX_QPATH];
	int testGun;
	float unk12;
    float farplane_distance;
	float farplane_bias;
    vec3_t farplane_color;
    qboolean farplane_cull;
	int skyboxFarplane;
	qboolean renderTerrain;
	float farclipOverride;
	vec3_t farplaneColorOverride;
    qboolean sky_portal;
    float sky_alpha;
    vec3_t sky_origin;
    vec3_t sky_axis[3];
	int skyboxSpeed;
	vec2_t viewKick;
	float viewKickRecenter;
	float viewKickMinDecay;
	float viewKickMaxDecay;
    cobjective_t  Objectives[20];
    float         ObjectivesAlphaTime;
    float         ObjectivesBaseAlpha;
    float         ObjectivesDesiredAlpha;
    float         ObjectivesCurrentAlpha;
    crain_t rain;
	int unk14[18];
	clientInfo_t clientinfo[MAX_CLIENTS];

} cg_t;

typedef struct clientGameImport_s
{
	int apiVersion;

	void (*Printf)(char* fmt, ...);
	void (*DPrintf)(char* fmt, ...);
	void (*DebugPrintf)(char* fmt, ...);
	void* (*Malloc)(int size);
	void (*Free)(void* ptr);
	void (*Error)(errorParm_t code, char* fmt, ...);
	int (*Milliseconds)();
	char* (*LV_ConvertString)(char* string);
	cvar_t* (*Cvar_Get)(const char* varName, const char* varValue, int varFlags);
	void (*Cvar_Set)(const char* varName, const char* varValue);
	int (*Argc)();
	char* (*Argv)(int arg);
	char* (*Args)();
	void (*AddCommand)(char* cmdName, xcommand_t cmdFunction);
	void (*Cmd_Stuff)(const char* text);
	void (*Cmd_Execute)(cbufExec_t execWhen, char* text);
	void (*Cmd_TokenizeString)(char* textIn);
	int (*FS_ReadFile)(char* qpath, void** buffer);
	void (*FS_FreeFile)(void* buffer);
	int (*FS_WriteFile)(char* qpath, void* buffer, int size);
	void (*FS_WriteTextFile)(char* qpath, void* buffer, int size);
	void (*FS_Unk1)();
	void (*FS_Unk2)();
	void (*SendConsoleCommand)(const char* text);
	int (*MSG_ReadBits)(int bits);
	int (*MSG_ReadChar)();
	int (*MSG_ReadByte)();
	int (*MSG_ReadSVC)();
	int (*MSG_ReadShort)();
	int (*MSG_ReadLong)();
	float (*MSG_ReadFloat)();
	char* (*MSG_ReadString)();
	char* (*MSG_ReadStringLine)();
	float (*MSG_ReadAngle8)();
	float (*MSG_ReadAngle16)();
	void (*MSG_ReadData)(void* data, int len);
	float (*MSG_ReadCoord)();
	void (*MSG_ReadDir)(vec3_t dir);
	void (*SendClientCommand)(const char* text);
	void (*CM_LoadMap)(char* name);
	clipHandle_t(*CM_InlineModel)(int index);
	int (*CM_NumInlineModels)();
	int (*CM_PointContents)(vec3_t point, clipHandle_t model);
	int (*CM_TransformedPointContents)(vec3_t point, clipHandle_t model, vec3_t origin, vec3_t angles);
	void (*CM_BoxTrace)(trace_t* results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, int model, int brushMask, int cylinder);
	void (*CM_TransformedBoxTrace)(trace_t* results, vec3_t start, vec3_t end, vec3_t mins, vec3_t maxs, int model, int brushMask, vec3_t origin, vec3_t angles, int cylinder);
	clipHandle_t(*CM_TempBoxModel)(vec3_t mins, vec3_t maxs, int contents);
	void (*CM_PrintBSPFileSizes)();
	qboolean(*CM_LeafInPVS)(int leaf1, int leaf2);
	int (*CM_PointLeafnum)(vec3_t p);
	int (*R_MarkFragments)(int numPoints, vec3_t* points, vec3_t projection, int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t* fragmentBuffer, float fRadiusSquared);
	int (*R_MarkFragmentsForInlineModel)(clipHandle_t bmodel, vec3_t vAngles, vec3_t vOrigin, int numPoints, vec3_t* points, vec3_t projection, int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t* fragmentBuffer, float fRadiusSquared);
	void (*R_GetInlineModelBounds)(int index, vec3_t mins, vec3_t maxs);
	void (*R_GetLightingForDecal)(vec3_t light, vec3_t facing, vec3_t origin);
	void (*R_GetLightingForSmoke)(vec3_t light, vec3_t origin);
	int (*R_GatherLightSources)(vec3_t pos, vec3_t* lightPos, vec3_t* lightIntensity, int maxLights);
	void (*S_StartSound)(vec3_t origin, int entNum, int entChannel, sfxHandle_t sfxHandle, float volume, float minDist, float pitch, float maxDist, int streamed);
	void (*S_StartLocalSound)(char* soundName, qboolean forceLoad);
	void (*S_StopSound)(int entNum, int channel);
	void (*S_ClearLoopingSounds)();
	void (*S_AddLoopingSound)(vec3_t origin, vec3_t velocity, sfxHandle_t sfxHandle, float volume, float minDist, float maxDist, float pitch, int flags);
	void (*S_Respatialize)(int entityNum, vec3_t head, vec3_t axis[3]);
	void (*S_BeginRegistration)();
	sfxHandle_t(*S_RegisterSound)(char* name, int streamed, qboolean forceLoad);
	void (*S_EndRegistration)();
	void (*S_UpdateEntity)(int entityNum, vec3_t origin, vec3_t velocity, qboolean useListener);
	void (*S_SetReverb)(int reverbType, float reverbLevel);
	void (*S_SetGlobalAmbientVolumeLevel)(float volume);
	float (*S_GetSoundTime)(sfxHandle_t handle);
	int (*S_ChannelNameToNum)(char* name);
	char* (*S_ChannelNumToName)(int channel);
	int (*S_IsSoundPlaying)(int channelNumber, char* name);
	void (*MUSIC_NewSoundtrack)(char* name);
	void (*MUSIC_UpdateMood)(musicMood_t current, musicMood_t fallback);
	void (*MUSIC_UpdateVolume)(float volume, float fadeTime);
	float* (*get_camera_offset)(qboolean* lookActive, qboolean* resetView);
	void (*R_ClearScene)();
	void (*R_RenderScene)(refdef_t* fd);
	void (*R_LoadWorldMap)(char* name);
	void (*R_PrintBSPFileSizes)();
	int (*MapVersion)();
	int (*R_MapVersion)();
	qhandle_t(*R_RegisterModel)(char* name);
	qhandle_t(*R_SpawnEffectModel)(char* name, vec3_t pos, vec3_t axis[3]);
	qhandle_t(*R_RegisterServerModel)(char* name);
	void (*R_Unk1)();
	void (*R_UnregisterServerModel)(qhandle_t hModel);
	qhandle_t(*R_RegisterShader)(char* name);
	qhandle_t(*R_RegisterShaderNoMip)(char* name);
	void (*R_Unk2)();
	void (*R_AddRefEntityToScene)(refEntity_t* ent, int parentEntityNumber);
	void (*R_AddRefSpriteToScene)(refEntity_t* ent);
	void (*R_AddLightToScene)(vec3_t org, float intensity, float r, float g, float b, dlighttype_t type);
	qboolean(*R_AddPolyToScene)(qhandle_t hShader, int numVerts, polyVert_t* verts, int renderFx);
	void (*R_AddTerrainMarkToScene)(int terrainIndex, qhandle_t hShader, int numVerts, polyVert_t* verts, int renderFx);
	void (*R_SetColor)(vec4_t rgba);
	void (*R_DrawStretchPic)(float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader);
	fontheader_t* (*R_LoadFont)(const char* name);
	void (*R_DrawString)(fontheader_t* font, const char* text, float x, float y, int maxLen, qboolean virtualScreen);
	refEntity_t* (*R_GetRenderEntity)(int entityNumber);
	void (*R_ModelBounds)(clipHandle_t model, vec3_t mins, vec3_t maxs);
	float (*R_ModelRadius)(qhandle_t handle);
	float (*R_Noise)(float x, float y, float z, float t);
	void (*R_DebugLine)(const vec3_t start, const vec3_t end, float r, float g, float b, float alpha);
	baseShader_t* (*GetShader)(int shaderNum);
	void (*R_SwipeBegin)(float thistime, float life, qhandle_t shader);
	void (*R_SwipePoint)(vec3_t point1, vec3_t point2, float time);
	void (*R_SwipeEnd)();
	int (*R_GetShaderWidth)(qhandle_t hShader);
	int (*R_GetShaderHeight)(qhandle_t hShader);
	void (*R_DrawBox)(float x, float y, float w, float h);
	void (*GetGameState)(gameState_t* gameState);
	int (*GetSnapshot)(int snapshotNumber, snapshot_t* snapshot);
	int (*GetServerStartTime)();
	void (*SetTime)(float time);
	void (*GetCurrentSnapshotNumber)(int* snapshotNumber, int* serverTime);
	void (*GetGlconfig)(glconfig_t* glConfig);
	qboolean(*GetParseEntityState)(int parseEntityNumber, entityState_t* state);
	int (*GetCurrentCmdNumber)();
	qboolean(*GetUserCmd)(int cmdNumber, userCmd_t* userCmd);
	qboolean(*GetServerCommand)(int serverCommandNumber, qboolean differentServer);
	qboolean(*Alias_Add)(char* alias, char* name, char* parameters);
	qboolean(*Alias_ListAdd)(AliasList_t* list, char* alias, char* name, char* parameters);
	char* (*Alias_FindRandom)(char* alias, AliasListNode_t** ret);
	char* (*Alias_ListFindRandom)(AliasList_t* list, char* alias, AliasListNode_t** ret);
	void (*Alias_Dump)();
	void (*Alias_Clear)();
	AliasList_t* (*AliasList_New)(char* name);
	void (*Alias_ListFindRandomRange)(AliasList_t* list, char* alias, int* minIndex, int* maxIndex, float* totalWeight);
	AliasList_t* (*Alias_GetGlobalList)();
	void (*UI_ShowMenu)(char* name, qboolean bForce);
	void (*UI_HideMenu)(char* name, qboolean bForce);
	int (*UI_FontStringWidth)(fontheader_t* font, char* string, int maxLen);
	void (*Key_Unk1)();
	int (*Key_StringToKeynum)(char* str);
	char* (*Key_KeynumToBindString)(int keyNum);
	void (*Key_GetKeysForCommand)(char* command, int* key1, int* key2);
	float (*GetFontHeight)(fontheader_t* font);
	int (*TIKI_NumAnims)(dtiki_t* pmdl);
	void (*TIKI_CalculateBounds)(dtiki_t* pmdl, float scale, vec3_t mins, vec3_t maxs);
	char* (*TIKI_Name)(dtiki_t* tiki);
	void* (*TIKI_GetSkeletor)(dtiki_t* tiki, int entNum);
	void (*TIKI_SetEyeTargetPos)(dtiki_t* tiki, int entNum, vec3_t pos);
	char* (*Anim_NameForNum)(dtiki_t* tiki, int animNum);
	int (*Anim_NumForName)(dtiki_t* tiki, char* name);
	int (*Anim_Random)(dtiki_t* tiki, char* name);
	int (*Anim_NumFrames)(dtiki_t* tiki, int animNum);
	float (*Anim_Time)(dtiki_t* tiki, int animNum);
	float (*Anim_Frametime)(dtiki_t* tiki, int animNum);
	void (*Anim_Delta)(dtiki_t* tiki, int animNum, vec3_t delta);
	int (*Anim_Flags)(dtiki_t* tiki, int animNum);
	int (*Anim_FlagsSkel)(dtiki_t* tiki, int animNum);
	float (*Anim_CrossblendTime)(dtiki_t* tiki, int animNum);
	qboolean(*Anim_HasCommands)(dtiki_t* tiki, int animNum);
	qboolean(*Frame_Commands)(dtiki_t* tiki, int animNum, int frameNum, dtikicmd_t* tikiCmds);
	qboolean(*Frame_CommandsTime)(dtiki_t* pmdl, int animNum, float start, float end, dtikicmd_t* tikiCmd);
	int (*Surface_NameToNum)(dtiki_t* pmdl, const char* name);
	int (*Tag_NumForName)(dtiki_t* pmdl, const char* name);
	char* (*Tag_NameForNum)(dtiki_t* pmdl, int animNum);
	void (*ForceUpdatePose)(refEntity_t* model);
	orientation_t* (*TIKI_Orientation)(orientation_t* orientation, refEntity_t* model, int tagNum);
	qboolean (*TIKI_IsOnGround)(refEntity_t* model, int tagNum, float threshold);
	void (*UI_ShowScoreBoard)(const char* menuName);
	void (*UI_HideScoreBoard)();
	void (*UI_SetScoreBoardItem)(int itemNumber, char* data1, char* data2, char* data3, char* data4, char* data5, char* data6, char* data7, char* data8, float* textColor, float* backColor, qboolean isHeader);
	void (*UI_DeleteScoreBoardItems)(int maxIndex);
	void (*UI_ToggleDMMessageConsole)(int consoleMode);
    void (*unknown1_0)();
	dtiki_t* (*TIKI_FindTiki)(char* path);
	void (*LoadResource)(char* name);
	void (*FS_CanonicalFilename)(char* name);
	void (*CL_RestoreSavedCgameState)();
	void (*CL_ClearSavedCgameState)();

	cvar_t* fsDebug;
	hdelement_t* hudDrawElements;
	clientAnim_t* anim;
	stopWatch_t* stopWatch;

} clientGameImport_t;

typedef struct clientGameExport_s
{
	void (*CG_Init)(struct clientGameImport_s* imported, int serverMessageNum, int serverCommandSequence, int clientNum);
	void (*CG_Shutdown)();
	void (*CG_DrawActiveFrame)(int serverTime, int frameTime, stereoFrame_t stereoView, qboolean demoPlayback);
	qboolean (*CG_ConsoleCommand)();
	void (*CG_GetRendererConfig)();
	void (*CG_Draw2D)();
	void (*CG_EyePosition)(vec3_t eyePos);
	void (*CG_EyeOffset)(vec3_t eyeOffset);
	void (*CG_EyeAngles)(vec3_t eyeAngles);
	float (*CG_SensitivityScale)();
	void (*CG_ParseCGMessage)();
	void (*CG_RefreshHudDrawElements)();
	void (*CG_HudDrawShader)(int info);
	void (*CG_HudDrawFont)(int info);
	int (*CG_GetParent)(int entNum);
	float (*CG_GetObjectiveAlpha)();
	int (*CG_PermanentMark)(vec3_t origin, vec3_t dir, float orientation, float sScale, float tScale, float red, float green, float blue, float alpha, qboolean doLighting, float sCenter, float tCenter, markFragment_t* markFragments, void* polyVerts);
	int (*CG_PermanentTreadMarkDecal)(treadMark_t* treadMark, qboolean startSegment, qboolean doLighting, markFragment_t* markFragments, void* polyVerts);
	int (*CG_PermanentUpdateTreadMark)(treadMark_t* treadMark, float alpha, float minSegment, float maxSegment, float maxOffset, float texScale);
	void (*CG_ProcessInitCommands)(dtiki_t* tiki, refEntity_t* ent);
	void (*CG_EndTiki)(dtiki_t* tiki);
	char* (*CG_GetColumnName)(int columnNum, int* columnWidth);
	void (*CG_GetScoreBoardColor)(float* red, float* green, float* blue, float* alpha);
	void (*CG_GetScoreBoardFontColor)(float* red, float* green, float* blue, float* alpha);
	int (*CG_GetScoreBoardDrawHeader)();
	void (*CG_GetScoreBoardPosition)(float* x, float* y, float* width, float* height);
	int (*CG_WeaponCommandButtonBits)();
	int (*CG_CheckCaptureKey)(keyNum_t key, qboolean down, unsigned int time);

	profCGame_t* profStruct;

	qboolean(*CG_Command_ProcessFile)(char* name, qboolean quiet, dtiki_t* curTiki);
	void(*unk1)();
	void(*unk2)();
	void(*unk3)();
	void(*unk4)();
	void(*unk5)();

} clientGameExport_t;
