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

#include "tiki_local.h"

#define TIKI_FILE_HASH_SIZE		1024
static	tiki_t					*hashTable[TIKI_FILE_HASH_SIZE];

static int						numDefines = 0;
static	tikiDefine_t			defines[TIKI_MAX_DEFINES];

static int						numIncludes = 0;
static	tikiInclude_t			includes[TIKI_MAX_INCLUDES];

/*
===============================
SK* FILES LOADING
===============================
*/

void TIKI_AppendSKD(tiki_t *out, char *fname) {

}

/*
===============================
TIKI FILES LOADING AND PARSING
===============================
*/

/*
NOTE: There is a lot of room for optimization of this code. For instance,
caching and reusing the already preprocessed files would give elaborate model
trees (such as humans - both players and actors) a major speed boost in terms
of loading times. Also, some string comparisions could be replaced with
hashing.
*/

/*
===================
TIKI_Preprocess_r

A simple, C-like preprocessor that supports macros ($define name expansion) and
includes ($include path/to/file), although with certain limitations (e.g.
nested or multi-line macros are not supported). Processing is done in 2 passes.
The first one analyses the file and computes the amount of memory required to
accomodate the final, processed file, the second one performs the actual
copying and substitution.
===================
*/
static char *TIKI_Preprocess_r(const char *parentFname, const char *fname) {
	static int		recursionLvl = 0;
	char			*buf, *output = NULL, *out, *p, *q, *r;
	int				quotes = 0, line = 0;
	tikiDefine_t	*def;
	tikiInclude_t	*inc;
	int				len, i;
	char			temp[MAX_QPATH];
	qboolean		comment = qfalse;

	recursionLvl++;
	if (recursionLvl > TIKI_MAX_RECURSION_LEVEL) {
		Com_Printf("WARNING: TIKI_Preprocess: TIKI_MAX_RECURSION_LEVEL (%d) exceeded in \"%s\"\n",
			TIKI_MAX_RECURSION_LEVEL, parentFname);
		goto finished;
	}

	len = FS_ReadFile(fname, (void **)&buf);
	if (!buf) {
		Com_Printf("WARNING: TIKI_Preprocess: Failed to load \"%s\" included from \"%s\"\n", fname, parentFname);
		goto finished;
	}

	// first pass - analysis
	for (p = buf; *p; p++) {
		if (comment) {
			if (*p == '\n' || *p == 0)
				comment = qfalse;
			continue;
		} else if (*p == '/' && *(p + 1) == '/') {
			comment = qtrue;
			continue;
		}
		if (*p == '\"' && p > buf && *(p - 1) != '\\')	// allow for string escaping
			quotes++;
		else if (*p == '$' && !(quotes & 1)) {
			q = p;
			p++;
			if (!Q_stricmpn(p, "define", 6)) {
				if (numDefines >= TIKI_MAX_DEFINES - 1) {
					Com_Printf("WARNING: TIKI_Preprocess: TIKI_MAX_DEFINES (%d) exceeded at line %d in \"%s\"\n",
						TIKI_MAX_DEFINES, line, fname);
					output = NULL;
					goto finished;
				}

				p += 6;
				// skip whitespaces
				while (*p <= ' ') {
					if (*p == '\n') {
						Com_Printf("WARNING: TIKI_Preprocess: Error in macro definition: Unexpected end of line at line %d in \"%s\"\n",
							line, fname);
						output = NULL;
						goto finished;
					}
					p++;
				}

				def = &defines[numDefines];
				numDefines++;
				def->file = (char *)fname;
				def->line = line;
				out = def->macro;

				// copy macro name
				while (*p && *p > ' ' && out < def->macro + sizeof(def->macro))
					*out++ = *p++;
				if (out >= def->macro + sizeof(def->macro))
					out = def->macro + sizeof(def->macro) - 1;
				*out = 0;	// ensure null termination
				def->macroLen = strlen(def->macro);
				if (*p == '\n') {
					Com_Printf("WARNING: TIKI_Preprocess: Error in macro definition: Unexpected end of line at line %d in \"%s\"\n",
						line, fname);
					output = NULL;
					goto finished;
				}

				// skip whitespaces
				while (*p <= ' ') {
					if (*p == '\n') {
						Com_Printf("WARNING: TIKI_Preprocess: Error in macro definition: Unexpected end of line at line %d in \"%s\"\n",
							line, fname);
						output = NULL;
						goto finished;
					}
					p++;
				}

				// copy macro expansion
				out = def->expansion;
				while (*p && *p != '\n' && out < def->expansion + sizeof(def->expansion))
					*out++ = *p++;
				def->expLen = strlen(def->expansion);
				// subtract the definition from the target file size
				len -= (p - q);
				continue;	// return to the loop
			}
			if (!Q_stricmpn(p, "include", 7)) {
				p += 7;
				// skip whitespaces
				while (*p <= ' ') {
					if (*p == '\n') {
						Com_Printf("WARNING: TIKI_Preprocess: Error in macro definition: Unexpected end of line at line %d in \"%s\"\n",
							line, fname);
						output = NULL;
						goto finished;
					}
					p++;
				}

				out = temp;
				while (*p && *p > ' ' && out < temp + sizeof(temp))
					*out++ = *p++;
				if (out >= temp + sizeof(temp))
					out = temp + sizeof(temp) - 1;
				*out = 0;
				for (i = 0; i < numIncludes; i++) {
					if (!Q_stricmp(includes[i].file, temp))
						goto skip;	// this file is already being included somewhere, no need to use another slot
				}

				// this file hasn't been included in the current tree yet, use a slot
				if (numIncludes >= TIKI_MAX_INCLUDES - 1) {
					Com_Printf("WARNING: TIKI_Preprocess: TIKI_MAX_INCLUDES (%d) exceeded at line %d in \"%s\"\n",
						TIKI_MAX_INCLUDES, line, fname);
					output = NULL;
					goto finished;
				}

				inc = &includes[numIncludes];
				numIncludes++;
				Q_strncpyz(inc->file, temp, sizeof(inc->file));
				inc->fileLen = strlen(inc->file);
				inc->data = TIKI_Preprocess_r(fname, temp);
				if (!inc->data) {
					Com_Printf("WARNING: TIKI_Preprocess: Failed to include file \"%s\" at line %d in \"%s\"\n",
						temp, line, fname);
					output = NULL;
					goto finished;
				}
				inc->dataLen = strlen(inc->data);
				// subtract the inclusion from the target file size
				len -= (p - q);
				// add the included file's length to the target file size
				len += inc->dataLen;
skip:
				continue;	// return to the loop
			}
			// neither a define nor an include, so it may be an instance of a macro
			for (i = 0; i < numDefines; i++) {
				if (!Q_strncmp(defines[i].macro, p, defines[i].macroLen) && p[defines[i].macroLen] == '$') {	// all macro instances must end with a $
					p += defines[i].macroLen + 1;
					len += (defines[i].expLen - defines[i].macroLen);
					break;
				}
			}
		}
		if (*p == '\n')
			line++;
	}

	// allocate memory for the processed file
	out = output = Z_Malloc(len + 1);

	line = 0;
	// second pass - actual processing
	// no error checking here - if there were any, they'd be detected in the first pass anyway
	comment = qfalse;
	for (p = buf; *p;) {
		if (comment) {
			if (*p == '\n' || *p == 0)
				comment = qfalse;
			else
				p++;
			continue;
		} else if (*p == '/' && *(p + 1) == '/') {
			comment = qtrue;
			p++;
			continue;
		}
		if (*p == '\"' && p > buf && *(p - 1) != '\\')	// allow for string escaping
			quotes++;
		else if (*p == '$' && !(quotes & 1)) {
			q = p;
			p++;
			if (!Q_stricmpn(p, "define", 6)) {	// just skip it
				p += 6;
				// skip whitespaces
				while (*p <= ' ')
					p++;
				// skip macro name
				while (*p && *p > ' ')
					p++;
				// skip whitespaces
				while (*p <= ' ')
					p++;
				// skip macro expansion
				while (*p && *p != '\n')
					p++;
				continue;	// return to the loop
			}
			if (!Q_stricmpn(p, "include", 7)) {
				p += 7;
				// skip whitespaces
				while (*p <= ' ')
					p++;

				// read the filename, choose the include and copy it
				r = temp;
				while (*p && *p > ' ' && r < temp + sizeof(temp))
					*r++ = *p++;
				if (r >= temp + sizeof(temp))
					r = temp + sizeof(temp) - 1;
				*r = 0;
				for (i = 0; i < numIncludes; i++) {
					if (!Q_stricmp(includes[i].file, temp)) {
						q = includes[i].data;
						while (*q)
							*out++ = *q++;
						break;
					}
				}
				continue;	// return to the loop
			}
			// neither a define nor an include, so it may be an instance of
			// a macro
			// this also makes sure that macros are only substituted if the
			// instance appears after the definition; the check is irrelevant
			// if the current file is different than the one the macro was
			// defined in (i.e. we're further down the tree and inherited the
			// define by inclusion)
			for (i = 0; i < numDefines; i++) {
				if (p[defines[i].macroLen] == '$' // all macro instances must end with a $
					&& (fname != defines[i].file || line > defines[i].line)
					&& !Q_strncmp(defines[i].macro, p, defines[i].macroLen)) {
					// skip the macro instance in the input
					p += defines[i].macroLen + 1;
					q = defines[i].expansion;
					while (*q)
						*out++ = *q++;
					break;
				}
			}
		}
		if (*p == '\n')
			line++;
		*out++ = *p++;
	}
	*out = 0;

	// we no longer need the original
	FS_FreeFile(buf);

finished:
	recursionLvl--;
	if (recursionLvl == 0) {
		//memset(&defines[0], 0, sizeof(defines));
		numDefines = 0;
		for (i = 0; i < numIncludes; i++)
			Z_Free(includes[i].data);
		//memset(&includes[0], 0, sizeof(includes));
		numIncludes = 0;
	}
	return output;
}

static tiki_t *TIKI_Load(const char *fname) {
	char			*token, *text;
	char			path[MAX_QPATH] = {0};
	char			temp[MAX_QPATH];
	int				i, level = 0;
	enum {
		SECTION_ROOT,
		SECTION_SETUP,
		SECTION_INIT,
		SECTION_ANIMATIONS
	}				section = SECTION_ROOT;
	tiki_t			*out = NULL;
	char			*file = TIKI_Preprocess_r(fname, fname);
	tikiAnim_t		**anims;

#if 0
	FILE *f = fopen("preprocessed.tik", "w");
	fwrite(file, strlen(file), 1, f);
	fclose(f);
#endif

	if (*(int *)(file) != TIKI_IDENT) {
		Com_Printf("WARNING: TIKI_Load: invalid ident in file %s\n", fname);
		Z_Free(file);
		return NULL;
	}

	out = Hunk_Alloc(sizeof(*out), h_high);
	memset(out, 0, sizeof(*out));
	Q_strncpyz(out->name, fname, sizeof(out->name));

	COM_BeginParseSession(fname);
	text = file;
	while (1) {
		token = COM_ParseExt(&text, qfalse);

		if (!text)
			break;

startagain:
		switch (section) {
			case SECTION_ROOT:
				if (!Q_stricmp(token, "setup"))
					section = SECTION_SETUP;
				else if (!Q_stricmp(token, "init"))
					section = SECTION_INIT;
				else if (!Q_stricmp(token, "animations"))
					section = SECTION_ANIMATIONS;
				else {
					COM_ParseError("ERROR: unknown token \"%s\"", token);
					Z_Free(file);
					return NULL;
				}
				continue;
			case SECTION_SETUP:	// this section contains information about the modules that make up the model
				if (token[0] == '{') {
					level++;
					token++;
					goto startagain;
				}
				if (token[0] == '}') {
					level--;
					if (level == 0)
						section = SECTION_ROOT;
					token++;
					goto startagain;
				}
				if (level == 1) {
					if (!Q_stricmp(token, "scale")) {
						token = COM_ParseExt(&text, qfalse);
						out->scale = atof(token);
						continue;
					} else if (!Q_stricmp(token, "path")) {
						token = COM_ParseExt(&text, qfalse);
						Q_strncpyz(path, token, sizeof(path));
						continue;
					}
					// implementation of the ones below is questionable, they're never used in MoHAA
					/*else if (!Q_stricmp(token, "origin")) {
						token = COM_ParseExt(&text, qfalse);
					} else if (!Q_stricmp(token, "lightoffset")) {
						token = COM_ParseExt(&text, qfalse);
					}*/ else if (!Q_stricmp(token, "surface")) {
						token = COM_ParseExt(&text, qfalse);
						for (i = 0; i < out->numSurfs; i++) {
							if (!Q_stricmp(token, out->surfs[i].name))
								break;
						}
						if (i >= out->numSurfs) {
							COM_ParseError("WARNING: no surface named \"%s\"", token);
							continue;
						}
						token = COM_ParseExt(&text, qfalse);
						if (!Q_stricmp(token, "shader")) {
							token = COM_ParseExt(&text, qfalse);
							Q_strncpyz(out->surfShaders[i], token, sizeof(out->surfShaders[i]));
							continue;
						}
						// again, questionable, never used in MoHAA
						/*else if (!Q_stricmp(token, "flags")) {
							continue;
						} else if (!Q_stricmp(token, "damage")) {
							continue;
						}*/ else {
							COM_ParseError("WARNING: unknown surface token \"%s\" in surface %s", token, out->surfs[i].name);
							continue;
						}
					} else if (!Q_stricmp(token, "skelmodel")) {
						token = COM_ParseExt(&text, qfalse);
						Com_sprintf(temp, sizeof(temp), "%s/%s", path, token);
						TIKI_AppendSKD(out, temp);
						continue;
					} else {
						COM_ParseError("ERROR: unknown token \"%s\"", token);
						Z_Free(file);
						return NULL;
					}
				} else {
					COM_ParseError("ERROR: unknown token \"%s\"", token);
					Z_Free(file);
					return NULL;
				}
				continue;
			case SECTION_INIT:	// this section contains script commands to be executed by the game scripting engine on entity initialization
				if (level == 0 && token[0] == '{') {
					level++;
					token++;
				} else {
					COM_ParseError("ERROR: unknown token \"%s\"", token);
					Z_Free(file);
					return NULL;
				}
				text -= strlen(token);
				token = text;
				while (level > 0) {
					if (*token == '{') {
						level++;
						token++;
						continue;
					}
					if (*token == '}') {
						level--;
						out->init = Hunk_Alloc(token - text + 1, h_high);
						Q_strncpyz(out->init, text, token - text + 1);
						text = token + 1;
						if (level == 0) {
							section = SECTION_ROOT;
							goto startagain;
						}
						continue;
					}
					if (!*token) {
						COM_ParseError("ERROR: unexpected end of file");
						Z_Free(file);
						return NULL;
					}
					token++;
				}
				continue;
			case SECTION_ANIMATIONS:
				if (token[0] == '{') {
					level++;
					token++;
					goto startagain;
				}
				if (token[0] == '}') {
					level--;
					if (level == 0)
						section = SECTION_ROOT;
					token++;
					goto startagain;
				}
				switch (level) {
					case 1:
						token = COM_ParseExt(&text, qfalse);
						break;
					case 2:
						break;
				} else {
					COM_ParseError("ERROR: unknown token \"%s\"", token);
					Z_Free(file);
					return NULL;
				}
				continue;
		}
	}

	Z_Free(file);
	return out;
}

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

qhandle_t TIKI_RegisterModel(const char *fname) {
	tiki_t	*tiki;
	int		hash = generateHashValue(fname, TIKI_FILE_HASH_SIZE);

	// see if the TIKI is already loaded
	for (tiki = hashTable[hash]; tiki; tiki = tiki->next) {
		if (!Q_stricmp(tiki->name, fname))
			// match found
			return (qhandle_t)tiki;
	}
	tiki = TIKI_Load(fname);
	if (!tiki) {
		Com_Printf("WARNING: TIKI_Load: Failed to load \"%s\"\n", fname);
		return -1;
	}
	if (hashTable[hash]) {
		hashTable[hash]->prev = tiki;
		tiki->next = hashTable[hash];
	}
	hashTable[hash] = tiki;
	return (qhandle_t)tiki;
}

/*
===============================
DATA QUERYING API
===============================
*/
