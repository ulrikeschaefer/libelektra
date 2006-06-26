/***************************************************************************
                   kdbd.c  -  The server for the daemon backend
                             -------------------
    begin                : Mon Dec 26 2004
    copyright            : (C) 2005 by Yannick Lecaillez
    email                : yl@itioweb.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License (revised).                      *
 *                                                                         *
 ***************************************************************************/


/* Subversion stuff

$Id: kdbd.c 788 2006-05-29 16:30:00Z aviram $

*/

#include "kdbbackend.h"

#include "protocol.h"
#include "message.h"
#include "kdb_wrapper.h"

#define	HANDLE_PREALLOC	5

static Message *processRequest(Message *request);

static Message *processRequest(Message *request)
{
	int	msgType, procedure;
	int	ret;

	msgType = messageGetType(request);
	if ( msgType != MESSAGE_REQUEST ) {
		fprintf(stderr, "processRequest(): Received a non-request message %d.\n", procedure);
		return NULL;
	}
	
	procedure = messageGetProcedure(request);
	switch(procedure) {
		case KDB_BE_OPEN:
			fprintf(stderr, "kdbOpen()\n");
			return wrapper_kdbOpen(request);
        	case KDB_BE_CLOSE:	
			fprintf(stderr, "kdbClose()\n");
			return wrapper_kdbClose(request);
        	case KDB_BE_STATKEY:
/*			ret = wrapper_kdbStatKey(request, t);
			break;  */
        	case KDB_BE_GETKEY:	
			fprintf(stderr, "kdbGetKey()\n");
			return wrapper_kdbGetKey(request);
/*        	case KDB_BE_SETKEY:
			ret = wrapper_kdbSetKey(msg->nbArgs, msg->args, reply);
			break;
        	case KDB_BE_SETKEYS:
			ret = wrapper_kdbSetKeys(msg->nbArgs, msg->args, reply);
			break;
        	case KDB_BE_RENAME:
			ret = wrapper_kdbRename(msg->nbArgs, msg->args, reply);
			break;
        	case KDB_BE_REMOVEKEY:
			ret = wrapper_kdbRemoveKey(msg->nbArgs, msg->args, reply);
			break; */
        	case KDB_BE_GETCHILD:	return wrapper_kdbGetChild(request);
/*        	case KDB_BE_MONITORKEY:
			ret = wrapper_kdbMonitorKey(msg->nbArgs, msg->args, reply);
			break;
        	case KDB_BE_MONITORKEYS:
			ret = wrapper_kdbMonitorKeys(msg->nbArgs, msg->args, reply);
			break; */
			
		default:
			return NULL;
	}

	return NULL;
}

int kdbd(int t)
{
	Message	*request, *reply;
	uid_t	remoteeuid;
	gid_t	remoteegid;
	int	closed;
	
	if ( ipc_eid(t, &remoteeuid, &remoteegid) == -1 ) {
		fprintf(stderr, "Can't get eUID & eGID\n");
		return 1;
	}

	closed = 0;
	while ( !closed ) {
		request = protocolReadMessage(t);
		closed = (messageGetProcedure(request) == KDB_BE_CLOSE);
		fprintf(stderr, "Closed = %d\n");
		
		reply = processRequest(request);
		messageDel(request);
		
		protocolSendMessage(t, reply);
		messageDel(reply);
	}

	fprintf(stderr, "Child exited\n");
	
	return 0;
}

