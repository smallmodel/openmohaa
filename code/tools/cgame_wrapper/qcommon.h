typedef int qboolean;

typedef int qhandle_t;
typedef int sfxHandle_t;
typedef int fileHandle_t;
typedef int clipHandle_t;

typedef struct cvar_s { /* size 44 id 30 */
  char *name; /* bitsize 32, bitpos 0 */
  char *string; /* bitsize 32, bitpos 32 */
  char *resetString; /* bitsize 32, bitpos 64 */
  char *latchedString; /* bitsize 32, bitpos 96 */
  int flags; /* bitsize 32, bitpos 128 */
  qboolean modified; /* bitsize 32, bitpos 160 */
  int modificationCount; /* bitsize 32, bitpos 192 */
  float value; /* bitsize 32, bitpos 224 */
  int integer; /* bitsize 32, bitpos 256 */
  struct cvar_s /* id 30 */ *next; /* bitsize 32, bitpos 288 */
  struct cvar_s /* id 30 */ *hashNext; /* bitsize 32, bitpos 320 */
} cvar_t;

typedef struct letterloc_s { /* size 16 */
  float pos[2]; /* bitsize 64, bitpos 0 */
  float size[2]; /* bitsize 64, bitpos 64 */
} letterloc_t;

typedef struct fontheader_s { /* size 5200 */
  int indirection[256]; /* bitsize 8192, bitpos 0 */
  letterloc_t locations[256]; /* bitsize 32768, bitpos 8192 */
  char name[64]; /* bitsize 512, bitpos 40960 */
  float height; /* bitsize 32, bitpos 41472 */
  float aspectRatio; /* bitsize 32, bitpos 41504 */
  void *shader; /* bitsize 32, bitpos 41536 */
  int trhandle; /* bitsize 32, bitpos 41568 */
} fontheader_t;

typedef struct hdelement_s { /* size 2228 id 44 */
  qhandle_t hShader; /* bitsize 32, bitpos 0 */
  char shaderName[64]; /* bitsize 512, bitpos 32 */
  int iX; /* bitsize 32, bitpos 544 */
  int iY; /* bitsize 32, bitpos 576 */
  int iWidth; /* bitsize 32, bitpos 608 */
  int iHeight; /* bitsize 32, bitpos 640 */
  float vColor[4]; /* bitsize 128, bitpos 672 */
  int iHorizontalAlign; /* bitsize 32, bitpos 800 */
  int iVerticalAlign; /* bitsize 32, bitpos 832 */
  qboolean bVirtualScreen; /* bitsize 32, bitpos 864 */
  char string[2048]; /* bitsize 16384, bitpos 896 */
  char fontName[64]; /* bitsize 512, bitpos 17280 */
  fontheader_t *pFont; /* bitsize 32, bitpos 17792 */
} hdelement_t;

typedef struct frameInfo_s { /* size 12 id 21 */
  int index; /* bitsize 32, bitpos 0 */
  float time; /* bitsize 32, bitpos 32 */
  float weight; /* bitsize 32, bitpos 64 */
} frameInfo_t;

typedef struct clientAnim_s { /* size 312 */
  frameInfo_t g_VMFrameInfo[16]; /* bitsize 1536, bitpos 0 */
  int g_iLastVMAnim; /* bitsize 32, bitpos 1536 */
  int g_iLastVMAnimChanged; /* bitsize 32, bitpos 1568 */
  int g_iCurrentVMAnimSlot; /* bitsize 32, bitpos 1600 */
  int g_iCurrentVMDuration; /* bitsize 32, bitpos 1632 */
  qboolean g_bCrossblending; /* bitsize 32, bitpos 1664 */
  int g_iLastEquippedWeaponStat; /* bitsize 32, bitpos 1696 */
  char g_szLastActiveItem[80]; /* bitsize 640, bitpos 1728 */
  int g_iLastAnimPrefixIndex; /* bitsize 32, bitpos 2368 */
  float g_vCurrentVMPosOffset[3]; /* bitsize 96, bitpos 2400 */
} clientAnim_t;

typedef struct baseshader_s { /* size 72 */
  char shader[64]; /* bitsize 512, bitpos 0 */
  int surfaceFlags; /* bitsize 32, bitpos 512 */
  int contentFlags; /* bitsize 32, bitpos 544 */
} baseshader_t;

typedef struct stopwatch_s { /* size 8 id 112 */
  int iStartTime; /* bitsize 32, bitpos 0 */
  int iEndTime; /* bitsize 32, bitpos 32 */
} stopwatch_t;

typedef enum {
	RT_MODEL,
	RT_SPRITE,
	RT_BEAM,
	RT_PORTALSURFACE,
	RT_MAX_REF_ENTITY_TYPE
} refEntityType_t;

typedef struct refEntity_s { /* size 408 */
  refEntityType_t reType; /* bitsize 32, bitpos 0 */
  int renderfx; /* bitsize 32, bitpos 32 */
  qhandle_t hModel; /* bitsize 32, bitpos 64 */
  qhandle_t hOldModel; /* bitsize 32, bitpos 96 */
  float lightingOrigin[3]; /* bitsize 96, bitpos 128 */
  int parentEntity; /* bitsize 32, bitpos 224 */
  float axis[3][3]; /* bitsize 288, bitpos 256 */
  qboolean nonNormalizedAxes; /* bitsize 32, bitpos 544 */
  float origin[3]; /* bitsize 96, bitpos 576 */
  frameInfo_t frameInfo[16]; /* bitsize 1536, bitpos 672 */
  float actionWeight; /* bitsize 32, bitpos 2208 */
  short int wasframe; /* bitsize 16, bitpos 2240 */
  float scale; /* bitsize 32, bitpos 2272 */
  float oldorigin[3]; /* bitsize 96, bitpos 2304 */
  int skinNum; /* bitsize 32, bitpos 2400 */
  qhandle_t customShader; /* bitsize 32, bitpos 2432 */
  unsigned char shaderRGBA[4]; /* bitsize 32, bitpos 2464 */
  float shaderTexCoord[2]; /* bitsize 64, bitpos 2496 */
  float shaderTime; /* bitsize 32, bitpos 2560 */
  int entityNumber; /* bitsize 32, bitpos 2592 */
  unsigned char surfaces[32]; /* bitsize 256, bitpos 2624 */
  float shader_data[2]; /* bitsize 64, bitpos 2880 */
  int *bone_tag; /* bitsize 32, bitpos 2944 */
  float (*bone_quat)[4]; /* bitsize 32, bitpos 2976 */
  struct tikiFrame_s /* id 22 */ *of; /* bitsize 32, bitpos 3008 */
  struct tikiFrame_s /* id 22 */ *nf; /* bitsize 32, bitpos 3040 */
  struct dtiki_s /* id 19 */ *tiki; /* bitsize 32, bitpos 3072 */
  int bonestart; /* bitsize 32, bitpos 3104 */
  int morphstart; /* bitsize 32, bitpos 3136 */
  qboolean hasMorph; /* bitsize 32, bitpos 3168 */
  float radius; /* bitsize 32, bitpos 3200 */
  float rotation; /* bitsize 32, bitpos 3232 */
} refEntity_t;

typedef struct orientation_s { /* size 48 id 34 */
  float origin[3]; /* bitsize 96, bitpos 0 */
  float axis[3][3]; /* bitsize 288, bitpos 96 */
} orientation_t;

typedef struct clientGameImport_s { /* size 684 */
  int apiversion; /* bitsize 32, bitpos 0 */
  void (*Printf) ( char *fmt );
  void (*DPrintf) (/* unknown */); /* bitsize 32, bitpos 64 */
  void (*DebugPrintf) (/* unknown */); /* bitsize 32, bitpos 96 */
  void *(*Malloc) (/* unknown */); /* bitsize 32, bitpos 128 */
  void (*Free) (/* unknown */); /* bitsize 32, bitpos 160 */
  void (*Error) (/* unknown */); /* bitsize 32, bitpos 192 */
  int (*Milliseconds) (/* unknown */); /* bitsize 32, bitpos 224 */
  char *(*LV_ConvertString) (/* unknown */); /* bitsize 32, bitpos 256 */
  cvar_t *(*Cvar_Get) (/* unknown */); /* bitsize 32, bitpos 288 */
  void (*Cvar_Set) (/* unknown */); /* bitsize 32, bitpos 320 */
  int (*Argc) (/* unknown */); /* bitsize 32, bitpos 352 */
  char *(*Argv) (/* unknown */); /* bitsize 32, bitpos 384 */
  char *(*Args) (/* unknown */); /* bitsize 32, bitpos 416 */
  void (*AddCommand) (/* unknown */); /* bitsize 32, bitpos 448 */
  void (*Cmd_Stuff) (/* unknown */); /* bitsize 32, bitpos 480 */
  void (*Cmd_Execute) (/* unknown */); /* bitsize 32, bitpos 512 */
  void (*Cmd_TokenizeString) (/* unknown */); /* bitsize 32, bitpos 544 */
  int (*FS_ReadFile) (/* unknown */); /* bitsize 32, bitpos 576 */
  void (*FS_FreeFile) (/* unknown */); /* bitsize 32, bitpos 608 */
  int (*FS_WriteFile) (/* unknown */); /* bitsize 32, bitpos 640 */
  void (*FS_WriteTextFile) (/* unknown */); /* bitsize 32, bitpos 672 */
  void (*SendConsoleCommand) (/* unknown */); /* bitsize 32, bitpos 704 */
  int (*MSG_ReadBits) (/* unknown */); /* bitsize 32, bitpos 736 */
  int (*MSG_ReadChar) (/* unknown */); /* bitsize 32, bitpos 768 */
  int (*MSG_ReadByte) (/* unknown */); /* bitsize 32, bitpos 800 */
  int (*MSG_ReadSVC) (/* unknown */); /* bitsize 32, bitpos 832 */
  int (*MSG_ReadShort) (/* unknown */); /* bitsize 32, bitpos 864 */
  int (*MSG_ReadLong) (/* unknown */); /* bitsize 32, bitpos 896 */
  float (*MSG_ReadFloat) (/* unknown */); /* bitsize 32, bitpos 928 */
  char *(*MSG_ReadString) (/* unknown */); /* bitsize 32, bitpos 960 */
  char *(*MSG_ReadStringLine) (/* unknown */); /* bitsize 32, bitpos 992 */
  float (*MSG_ReadAngle8) (/* unknown */); /* bitsize 32, bitpos 1024 */
  float (*MSG_ReadAngle16) (/* unknown */); /* bitsize 32, bitpos 1056 */
  void (*MSG_ReadData) (/* unknown */); /* bitsize 32, bitpos 1088 */
  float (*MSG_ReadCoord) (/* unknown */); /* bitsize 32, bitpos 1120 */
  void (*MSG_ReadDir) (/* unknown */); /* bitsize 32, bitpos 1152 */
  void (*SendClientCommand) (/* unknown */); /* bitsize 32, bitpos 1184 */
  void (*CM_LoadMap) (/* unknown */); /* bitsize 32, bitpos 1216 */
  clipHandle_t (*CM_InlineModel) (/* unknown */); /* bitsize 32, bitpos 1248 */
  int (*CM_NumInlineModels) (/* unknown */); /* bitsize 32, bitpos 1280 */
  int (*CM_PointContents) (/* unknown */); /* bitsize 32, bitpos 1312 */
  int (*CM_TransformedPointContents) (/* unknown */); /* bitsize 32, bitpos 1344 */
  void (*CM_BoxTrace) (/* unknown */); /* bitsize 32, bitpos 1376 */
  void (*CM_TransformedBoxTrace) (/* unknown */); /* bitsize 32, bitpos 1408 */
  clipHandle_t (*CM_TempBoxModel) (/* unknown */); /* bitsize 32, bitpos 1440 */
  void (*CM_PrintBSPFileSizes) (/* unknown */); /* bitsize 32, bitpos 1472 */
  qboolean (*CM_LeafInPVS) (/* unknown */); /* bitsize 32, bitpos 1504 */
  int (*CM_PointLeafnum) (/* unknown */); /* bitsize 32, bitpos 1536 */
  int (*R_MarkFragments) (/* unknown */); /* bitsize 32, bitpos 1568 */
  int (*R_MarkFragmentsForInlineModel) (/* unknown */); /* bitsize 32, bitpos 1600 */
  void (*R_GetInlineModelBounds) (/* unknown */); /* bitsize 32, bitpos 1632 */
  void (*R_GetLightingForDecal) (/* unknown */); /* bitsize 32, bitpos 1664 */
  void (*R_GetLightingForSmoke) (/* unknown */); /* bitsize 32, bitpos 1696 */
  int (*R_GatherLightSources) (/* unknown */); /* bitsize 32, bitpos 1728 */
  void (*S_StartSound) (/* unknown */); /* bitsize 32, bitpos 1760 */
  void (*S_StartLocalSound) (/* unknown */); /* bitsize 32, bitpos 1792 */
  void (*S_StopSound) (/* unknown */); /* bitsize 32, bitpos 1824 */
  void (*S_ClearLoopingSounds) (/* unknown */); /* bitsize 32, bitpos 1856 */
  void (*S_AddLoopingSound) (/* unknown */); /* bitsize 32, bitpos 1888 */
  void (*S_Respatialize) (/* unknown */); /* bitsize 32, bitpos 1920 */
  void (*S_BeginRegistration) (/* unknown */); /* bitsize 32, bitpos 1952 */
  sfxHandle_t (*S_RegisterSound) (/* unknown */); /* bitsize 32, bitpos 1984 */
  void (*S_EndRegistration) (/* unknown */); /* bitsize 32, bitpos 2016 */
  void (*S_UpdateEntity) (/* unknown */); /* bitsize 32, bitpos 2048 */
  void (*S_SetReverb) (/* unknown */); /* bitsize 32, bitpos 2080 */
  void (*S_SetGlobalAmbientVolumeLevel) (/* unknown */); /* bitsize 32, bitpos 2112 */
  float (*S_GetSoundTime) (/* unknown */); /* bitsize 32, bitpos 2144 */
  int (*S_ChannelNameToNum) (/* unknown */); /* bitsize 32, bitpos 2176 */
  char *(*S_ChannelNumToName) (/* unknown */); /* bitsize 32, bitpos 2208 */
  int (*S_IsSoundPlaying) (/* unknown */); /* bitsize 32, bitpos 2240 */
  void (*MUSIC_NewSoundtrack) (/* unknown */); /* bitsize 32, bitpos 2272 */
  void (*MUSIC_UpdateMood) (/* unknown */); /* bitsize 32, bitpos 2304 */
  void (*MUSIC_UpdateVolume) (/* unknown */); /* bitsize 32, bitpos 2336 */
  float *(*get_camera_offset) (/* unknown */); /* bitsize 32, bitpos 2368 */
  void (*R_ClearScene) (/* unknown */); /* bitsize 32, bitpos 2400 */
  void (*R_RenderScene) (/* unknown */); /* bitsize 32, bitpos 2432 */
  void (*R_LoadWorldMap) (/* unknown */); /* bitsize 32, bitpos 2464 */
  void (*R_PrintBSPFileSizes) (/* unknown */); /* bitsize 32, bitpos 2496 */
  int (*MapVersion) (/* unknown */); /* bitsize 32, bitpos 2528 */
  int (*R_MapVersion) (/* unknown */); /* bitsize 32, bitpos 2560 */
  qhandle_t (*R_RegisterModel) (/* unknown */); /* bitsize 32, bitpos 2592 */
  qhandle_t (*R_SpawnEffectModel) (/* unknown */); /* bitsize 32, bitpos 2624 */
  qhandle_t (*R_RegisterServerModel) (/* unknown */); /* bitsize 32, bitpos 2656 */
  void (*R_UnregisterServerModel) (/* unknown */); /* bitsize 32, bitpos 2688 */
  qhandle_t (*R_RegisterSkin) (/* unknown */); /* bitsize 32, bitpos 2720 */
  qhandle_t (*R_RegisterShader) (/* unknown */); /* bitsize 32, bitpos 2752 */
  qhandle_t (*R_RegisterShaderNoMip) (/* unknown */); /* bitsize 32, bitpos 2784 */
  void (*R_AddRefEntityToScene) (/* unknown */); /* bitsize 32, bitpos 2816 */
  void (*R_AddRefSpriteToScene) (/* unknown */); /* bitsize 32, bitpos 2848 */
  void (*R_AddLightToScene) (/* unknown */); /* bitsize 32, bitpos 2880 */
  qboolean (*R_AddPolyToScene) (/* unknown */); /* bitsize 32, bitpos 2912 */
  void (*R_AddTerrainMarkToScene) (/* unknown */); /* bitsize 32, bitpos 2944 */
  void (*R_SetColor) (/* unknown */); /* bitsize 32, bitpos 2976 */
  void (*R_DrawStretchPic) (/* unknown */); /* bitsize 32, bitpos 3008 */
  fontheader_t *(*R_LoadFont) (/* unknown */); /* bitsize 32, bitpos 3040 */
  void (*R_DrawString) (/* unknown */); /* bitsize 32, bitpos 3072 */
  refEntity_t *(*R_GetRenderEntity) (/* unknown */); /* bitsize 32, bitpos 3104 */
  void (*R_ModelBounds) (/* unknown */); /* bitsize 32, bitpos 3136 */
  float (*R_ModelRadius) (/* unknown */); /* bitsize 32, bitpos 3168 */
  float (*R_Noise) (/* unknown */); /* bitsize 32, bitpos 3200 */
  void (*R_DebugLine) (/* unknown */); /* bitsize 32, bitpos 3232 */
  baseshader_t *(*GetShader) (/* unknown */); /* bitsize 32, bitpos 3264 */
  void (*R_SwipeBegin) (/* unknown */); /* bitsize 32, bitpos 3296 */
  void (*R_SwipePoint) (/* unknown */); /* bitsize 32, bitpos 3328 */
  void (*R_SwipeEnd) (/* unknown */); /* bitsize 32, bitpos 3360 */
  int (*R_GetShaderWidth) (/* unknown */); /* bitsize 32, bitpos 3392 */
  int (*R_GetShaderHeight) (/* unknown */); /* bitsize 32, bitpos 3424 */
  void (*R_DrawBox) (/* unknown */); /* bitsize 32, bitpos 3456 */
  void (*GetGameState) (/* unknown */); /* bitsize 32, bitpos 3488 */
  int (*GetSnapshot) (/* unknown */); /* bitsize 32, bitpos 3520 */
  int (*GetServerStartTime) (/* unknown */); /* bitsize 32, bitpos 3552 */
  void (*SetTime) (/* unknown */); /* bitsize 32, bitpos 3584 */
  void (*GetCurrentSnapshotNumber) (/* unknown */); /* bitsize 32, bitpos 3616 */
  void (*GetGlconfig) (/* unknown */); /* bitsize 32, bitpos 3648 */
  qboolean (*GetParseEntityState) (/* unknown */); /* bitsize 32, bitpos 3680 */
  int (*GetCurrentCmdNumber) (/* unknown */); /* bitsize 32, bitpos 3712 */
  qboolean (*GetUserCmd) (/* unknown */); /* bitsize 32, bitpos 3744 */
  qboolean (*GetServerCommand) (/* unknown */); /* bitsize 32, bitpos 3776 */
  qboolean (*Alias_Add) (/* unknown */); /* bitsize 32, bitpos 3808 */
  qboolean (*Alias_ListAdd) (/* unknown */); /* bitsize 32, bitpos 3840 */
  char *(*Alias_FindRandom) (/* unknown */); /* bitsize 32, bitpos 3872 */
  char *(*Alias_ListFindRandom) (/* unknown */); /* bitsize 32, bitpos 3904 */
  void (*Alias_Dump) (/* unknown */); /* bitsize 32, bitpos 3936 */
  void (*Alias_Clear) (/* unknown */); /* bitsize 32, bitpos 3968 */
  struct AliasList_s /* id 114 */ *(*AliasList_New) (/* unknown */); /* bitsize 32, bitpos 4000 */
  void (*Alias_ListFindRandomRange) (/* unknown */); /* bitsize 32, bitpos 4032 */
  struct AliasList_s /* id 114 */ *(*Alias_GetGlobalList) (/* unknown */); /* bitsize 32, bitpos 4064 */
  void (*UI_ShowMenu) (/* unknown */); /* bitsize 32, bitpos 4096 */
  void (*UI_HideMenu) (/* unknown */); /* bitsize 32, bitpos 4128 */
  int (*UI_FontStringWidth) (/* unknown */); /* bitsize 32, bitpos 4160 */
  int (*Key_StringToKeynum) (/* unknown */); /* bitsize 32, bitpos 4192 */
  char *(*Key_KeynumToBindString) (/* unknown */); /* bitsize 32, bitpos 4224 */
  void (*Key_GetKeysForCommand) (/* unknown */); /* bitsize 32, bitpos 4256 */
  struct dtiki_s /* id 19 */ *(*R_Model_GetHandle) (/* unknown */); /* bitsize 32, bitpos 4288 */
  int (*TIKI_NumAnims) (/* unknown */); /* bitsize 32, bitpos 4320 */
  void (*TIKI_CalculateBounds) (/* unknown */); /* bitsize 32, bitpos 4352 */
  char *(*TIKI_Name) (/* unknown */); /* bitsize 32, bitpos 4384 */
  void *(*TIKI_GetSkeletor) (/* unknown */); /* bitsize 32, bitpos 4416 */
  void (*TIKI_SetEyeTargetPos) (/* unknown */); /* bitsize 32, bitpos 4448 */
  char *(*Anim_NameForNum) (/* unknown */); /* bitsize 32, bitpos 4480 */
  int (*Anim_NumForName) (/* unknown */); /* bitsize 32, bitpos 4512 */
  int (*Anim_Random) (/* unknown */); /* bitsize 32, bitpos 4544 */
  int (*Anim_NumFrames) (/* unknown */); /* bitsize 32, bitpos 4576 */
  float (*Anim_Time) (/* unknown */); /* bitsize 32, bitpos 4608 */
  float (*Anim_Frametime) (/* unknown */); /* bitsize 32, bitpos 4640 */
  void (*Anim_Delta) (/* unknown */); /* bitsize 32, bitpos 4672 */
  int (*Anim_Flags) (/* unknown */); /* bitsize 32, bitpos 4704 */
  int (*Anim_FlagsSkel) (/* unknown */); /* bitsize 32, bitpos 4736 */
  float (*Anim_CrossblendTime) (/* unknown */); /* bitsize 32, bitpos 4768 */
  qboolean (*Anim_HasCommands) (/* unknown */); /* bitsize 32, bitpos 4800 */
  qboolean (*Frame_Commands) (/* unknown */); /* bitsize 32, bitpos 4832 */
  qboolean (*Frame_CommandsTime) (/* unknown */); /* bitsize 32, bitpos 4864 */
  int (*Surface_NameToNum) (/* unknown */); /* bitsize 32, bitpos 4896 */
  int (*Tag_NumForName) (/* unknown */); /* bitsize 32, bitpos 4928 */
  char *(*Tag_NameForNum) (/* unknown */); /* bitsize 32, bitpos 4960 */
  void (*ForceUpdatePose) (/* unknown */); /* bitsize 32, bitpos 4992 */
  orientation_t (*TIKI_Orientation) (/* unknown */); /* bitsize 32, bitpos 5024 */
  qboolean (*TIKI_IsOnGround) (/* unknown */); /* bitsize 32, bitpos 5056 */
  void (*UI_ShowScoreBoard) (/* unknown */); /* bitsize 32, bitpos 5088 */
  void (*UI_HideScoreBoard) (/* unknown */); /* bitsize 32, bitpos 5120 */
  void (*UI_SetScoreBoardItem) (/* unknown */); /* bitsize 32, bitpos 5152 */
  void (*UI_DeleteScoreBoardItems) (/* unknown */); /* bitsize 32, bitpos 5184 */
  void (*UI_ToggleDMMessageConsole) (/* unknown */); /* bitsize 32, bitpos 5216 */
  struct dtiki_s /* id 19 */ *(*TIKI_FindTiki) (/* unknown */); /* bitsize 32, bitpos 5248 */
  void (*LoadResource) (/* unknown */); /* bitsize 32, bitpos 5280 */
  void (*FS_CanonicalFilename) (/* unknown */); /* bitsize 32, bitpos 5312 */
  cvar_t *fs_debug; /* bitsize 32, bitpos 5344 */
  hdelement_t *HudDrawElements; /* bitsize 32, bitpos 5376 */
  clientAnim_t *anim; /* bitsize 32, bitpos 5408 */
  stopwatch_t *stopwatch; /* bitsize 32, bitpos 5440 */
} clientGameImport_t;

typedef struct clientGameExport_s { /* size 120 */
  void (*CG_Init) ( clientGameImport_t *imported, int serverMessageNum, int serverCommandSequence, int clientNum );
  void (*CG_Shutdown) (/* unknown */); /* bitsize 32, bitpos 32 */
  void (*CG_DrawActiveFrame) (/* unknown */); /* bitsize 32, bitpos 64 */
  qboolean (*CG_ConsoleCommand) (/* unknown */); /* bitsize 32, bitpos 96 */
  void (*CG_GetRendererConfig) (/* unknown */); /* bitsize 32, bitpos 128 */
  void (*CG_Draw2D) (/* unknown */); /* bitsize 32, bitpos 160 */
  void (*CG_EyePosition) (/* unknown */); /* bitsize 32, bitpos 192 */
  void (*CG_EyeOffset) (/* unknown */); /* bitsize 32, bitpos 224 */
  void (*CG_EyeAngles) (/* unknown */); /* bitsize 32, bitpos 256 */
  float (*CG_SensitivityScale) (/* unknown */); /* bitsize 32, bitpos 288 */
  void (*CG_ParseCGMessage) (/* unknown */); /* bitsize 32, bitpos 320 */
  void (*CG_RefreshHudDrawElements) (/* unknown */); /* bitsize 32, bitpos 352 */
  void (*CG_HudDrawShader) (/* unknown */); /* bitsize 32, bitpos 384 */
  void (*CG_HudDrawFont) (/* unknown */); /* bitsize 32, bitpos 416 */
  int (*CG_GetParent) (/* unknown */); /* bitsize 32, bitpos 448 */
  float (*CG_GetObjectiveAlpha) (/* unknown */); /* bitsize 32, bitpos 480 */
  int (*CG_PermanentMark) (/* unknown */); /* bitsize 32, bitpos 512 */
  int (*CG_PermanentTreadMarkDecal) (/* unknown */); /* bitsize 32, bitpos 544 */
  int (*CG_PermanentUpdateTreadMark) (/* unknown */); /* bitsize 32, bitpos 576 */
  void (*CG_ProcessInitCommands) (/* unknown */); /* bitsize 32, bitpos 608 */
  void (*CG_EndTiki) (/* unknown */); /* bitsize 32, bitpos 640 */
  char *(*CG_GetColumnName) (/* unknown */); /* bitsize 32, bitpos 672 */
  void (*CG_GetScoreBoardColor) (/* unknown */); /* bitsize 32, bitpos 704 */
  void (*CG_GetScoreBoardFontColor) (/* unknown */); /* bitsize 32, bitpos 736 */
  int (*CG_GetScoreBoardDrawHeader) (/* unknown */); /* bitsize 32, bitpos 768 */
  void (*CG_GetScoreBoardPosition) (/* unknown */); /* bitsize 32, bitpos 800 */
  int (*CG_WeaponCommandButtonBits) (/* unknown */); /* bitsize 32, bitpos 832 */
  int (*CG_CheckCaptureKey) (/* unknown */); /* bitsize 32, bitpos 864 */
  struct prof_cgame_s /* id 116 */ *prof_struct; /* bitsize 32, bitpos 896 */
  qboolean (*CG_Command_ProcessFile) (/* unknown */); /* bitsize 32, bitpos 928 */
} clientGameExport_t;