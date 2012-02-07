/*
===========================================================================
Copyright (C) 2012 Michael Rieder

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
#include "qcommon.h"

struct dtiki_s *R_Model_GetHandle ( qhandle_t handle ) {
	struct dtiki_s *ret;
	int i = sizeof(dtiki_t);

	ret = cgi.R_Model_GetHandle( handle );
	if ( ret )
		return ret;
	else
		return ret;
}

struct dtiki_s *TIKI_FindTiki ( char *path ) {
	return cgi.TIKI_FindTiki(path);
}