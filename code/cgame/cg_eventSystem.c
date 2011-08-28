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
// cg_eventSystem.c

#include "cg_local.h"

typedef struct event_s {
	centity_t *ent;
	char buffer[1024];
	int execTime; // event should be executed and fried if cg.time >= ev->execTime
	struct event_s *next,*prev; // linked list
} event_t;

#define MAX_PENDING_EVENTS 512

static event_t events[MAX_PENDING_EVENTS];
static event_t cg_pendingEvents;	// double linked list
static event_t *cg_freeEvents;		// single linked list

void CG_InitEventSystem() {
	int		i;

	memset( events, 0, sizeof( events ) );
	cg_pendingEvents.next = &cg_pendingEvents;
	cg_pendingEvents.prev = &cg_pendingEvents;
	cg_freeEvents = events;
	for ( i = 0 ; i < MAX_PENDING_EVENTS - 1 ; i++ ) {
		events[i].next = &events[i+1];
	}
}

void CG_ProcessEventText(centity_t *ent, const char *eventText) {
	char *text;
	char *token;
	float f;

	// if(cg_printEvents.integer)
		CG_Printf("CG_ProcessEventText: event %s\n",eventText);
	text = eventText;

	// get event name
	token = COM_ParseExt( &text, qtrue );
	Q_strlwr(token);
	if(!strcmp(token,"sound")) {
	// syntax: sound( String soundName,
	// [ String channelName ], [ Float volume ],
	// [ Float min_distance ], [ Float pitch ],
	// [ Float randompitch ], [ String randomvolume ] )
		token = COM_ParseExt( &text, qtrue );
		CG_Printf("sound event with sound %s\n",token);
		token = COM_ParseExt( &text, qtrue );
		if(token[0]) {

		}

		// TODO

	} else if(!strcmp(token,"stopsound")) {
		// syntax: stopsound( String channelName )
		// TODO
	} else if(!strcmp(token,"loopsound")) {
		// syntax: loopsound( String soundName, [ Float volume ],
		// [ Float min_distance ], Float pitch )
		// TODO
	} else if(!strcmp(token,"stoploopsound")) {
		// TODO
	} else if(!strcmp(token,"stopaliaschannel")) {
		// TODO
	} else if(!strcmp(token,"viewkick")) {
	// viewkick( Float pitchmin, Float pitchmax,
	// Float yawmin, Float yawmax, Float recenterspeed,
	// String patters, Float pitchmax, Float yawmax )
		// TODO
	} else if(!strcmp(token,"cache")) {
		token = COM_ParseExt( &text, qtrue );
		CG_Printf("Caching %s...\n",token);
		trap_R_RegisterModel(token);
	} else if(!strcmp(token,"commanddelay")) {
		token = COM_ParseExt( &text, qtrue );
		f = atof(token);
		CG_PostEvent(ent,text,f*1000);
	}

	

	
}

void CG_FreeEvent(event_t *ev) {
	if ( !ev->prev ) {
		CG_Error( "CG_FreeEvent: not active" );
	}

	// remove from the doubly linked active list
	ev->prev->next = ev->next;
	ev->next->prev = ev->prev;

	// the free list is only singly linked
	ev->next = cg_freeEvents;
	cg_freeEvents = ev;
}
event_t	*CG_AllocEvent() {
	event_t	*ev;

	if ( !cg_freeEvents ) {
		CG_Printf("CG_AllocEvent: no free events\n");
		return 0;
	}

	ev = cg_freeEvents;
	cg_freeEvents = cg_freeEvents->next;

	memset( ev, 0, sizeof( *ev ) );

	// link into the active list
	ev->next = cg_pendingEvents.next;
	ev->prev = &cg_pendingEvents;
	cg_pendingEvents.next->prev = ev;
	cg_pendingEvents.next = ev;
	return ev;
}

void CG_PostEvent(centity_t *ent, const char *eventText, int eventDelay) {
	event_t	*ev;
	ev = CG_AllocEvent();
	if(!ev)
		return; // failed to alloc an event
	ev->ent = ent;
	strcpy(ev->buffer,eventText);
	ev->execTime = cg.time + eventDelay;
}

void CG_ProcessEvent(event_t *ev) {
	CG_ProcessEventText(ev->ent,ev->buffer);
	CG_FreeEvent(ev);
}

void CG_ProcessPendingEvents() {
	event_t	*ev, *next;
	ev = cg_pendingEvents.next;
	for ( ; ev != &cg_pendingEvents ; ev = next ) {
		// grab next now, so if the event is freed we
		// still have it
		next = ev->next;
		if ( cg.time >= ev->execTime ) {
			// execute it
			CG_ProcessEventText(ev->ent, ev->buffer);
			// and free
			CG_FreeEvent(ev);
			continue;
		}
	}

}



