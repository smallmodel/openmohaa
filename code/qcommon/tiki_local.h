/*
===========================================================================
Copyright (C) 2008 Leszek Godlewski

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

#ifndef TIKI_LOCAL_H
#define TIKI_LOCAL_H

#include "tiki_public.h"
#include "qcommon.h"
#include "qfiles.h"

#define TIKI_MAX_RECURSION_LEVEL	32
#define TIKI_MAX_DEFINES			32
#define TIKI_MAX_INCLUDES			32

typedef struct {
	char			macro[MAX_QPATH];
	int				macroLen;
	char			expansion[MAX_STRING_CHARS];
	int				expLen;

	char			*file;	// the file the define was introduced in
	int				line;	// the line the define was introduced at; will be ignored if this->file != fname
} tikiDefine_t;

typedef struct {
	char			file[MAX_QPATH];
	int				fileLen;
	char			*data;
	int				dataLen;
} tikiInclude_t;

typedef struct tikiCommand_s {
	int				frame;	// -1 == at entry, -2 == at exit
	char			*text;
} tikiCommand_t;

typedef struct tikiAnim_s {
	char			name[MAX_QPATH];
	unsigned int	flags;
	float			crossblend;
	skcHeader_t		*motion;
	int				numClientCmds;
	tikiCommand_t	**clientCmds;
	int				numServerCmds;
	tikiCommand_t	**serverCmds;
} tikiAnim_t;

typedef struct tiki_s {
	char			name[MAX_QPATH];

	char			*init;	// command block executed on entity initialization

	float			scale;

	int				numSurfaces;
	skdSurface_t	*surfs;
	char			*(surfShaders[MAX_QPATH]);
	int				numBones;
	skdBone_t		*bones;
	int				numHitBoxes;
	skdHitbox_t		*hitBoxes;
	int				numMorphTargets;
	skdMorph_t		*morphTargets;

	int				numAnims;
	tikiAnim_t		**anims;

	int				lodIndex[8];

	struct tiki_s	*prev, *next;
} tiki_t;

#endif // TIKI_LOCAL_H
