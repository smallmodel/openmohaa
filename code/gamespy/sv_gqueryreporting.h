/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

#include <common/gsPlatformSocket.h>

typedef void (*qr_querycallback_t) (char* outbuf, int maxlen, void* userdata);
typedef void (*qr_custom_handler_t) (const char* query, struct sockaddr* sender);

typedef enum query_e {
	qtunknown,
	qtbasic,
	qtinfo,
	qtrules,
	qtplayers,
	qtstatus,
	qtpackets,
	qtecho,
	qtsecure
} query_t;

typedef struct qr_implementation_s {
	void* querysock;
	void* hbsock;
	char gamename[64];
	char secret_key[128];
	qr_querycallback_t qr_basic_callback;
	qr_querycallback_t qr_info_callback;
	qr_querycallback_t qr_rules_callback;
	qr_querycallback_t qr_players_callback;
	long unsigned int lastheartbeat;
	int queryid;
	int packetnumber;
	int qport;
	char no_query;
	struct sockaddr_in master_saddr;
	int unk2;
	int unk3;
	int unk4;
	qr_custom_handler_t qr_custom_handler;
	void* udata;
} qr_implementation_t, *qr_t;
