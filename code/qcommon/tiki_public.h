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

#include "q_shared.h"

// animation flags
#define TAF_DELTADRIVEN				0x1
#define TAF_AUTOSTEPS_RUN			0x2
#define TAF_AUTOSTEPS_WALK			0x4

typedef struct tiki_s tiki_t;
tiki_t*	TIKI_RegisterModel(const char *fname);
void TIKI_SetChannels(tiki_t *tiki, int animIndex, float animTime, float animWeight, bone_t *bones);
void TIKI_Animate(tiki_t *tiki, bone_t *bones);
int	TIKI_RegisterBoneName(const char *boneName);
