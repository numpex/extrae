/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                   Extrae                                  *
 *              Instrumentation package for parallel applications            *
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\
 | @file: $HeadURL$
 | @last_commit: $Date$
 | @version:     $Revision$
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
#include "common.h"

static char UNUSED rcsid[] = "$Id$";

#include "communication_queues.h"

#ifndef HAVE_MPI_H
# define MPI_ANY_TAG (-1)
#else
# include <mpi.h>
#endif

/**************************************************************************
*** SEND PART
**************************************************************************/

typedef struct
{
	event_t *send_begin;
	event_t *send_end;
	off_t send_position;
	long long key;
	unsigned thread;
	unsigned vthread;
} 
SendData_t;

typedef struct
{
	long long key;
	int tag;
	int target;
}
SendDataReference_t;

void CommunicationQueues_QueueSend (NewQueue_t *qsend, event_t *send_begin,
	event_t *send_end, off_t send_position, unsigned thread, unsigned vthread,
	long long key)
{
	SendData_t tmp;

	tmp.send_begin = send_begin;
	tmp.send_end = send_end;
	tmp.send_position = send_position;
	tmp.thread = thread;
	tmp.vthread = vthread;
	tmp.key = key;

	NewQueue_add (qsend, &tmp);
}

static int CompareSend_cbk (void *reference, void *data)
{
	SendData_t *d = (SendData_t*) data;
	SendDataReference_t *ref = (SendDataReference_t*) reference;

	/* Return OK if the TAG, TARGET and KEY are the same */
	/*   we look for senders, check whether the receiver is any tag */
	return (ref->tag == Get_EvTag(d->send_begin) || ref->tag == MPI_ANY_TAG) &&
	  ref->target == Get_EvTarget(d->send_begin) &&
		ref->key == d->key;
}

void CommunicationQueues_ExtractSend (NewQueue_t *qsend, int receiver,
	int tag, event_t **send_begin, event_t **send_end,
	off_t *send_position, unsigned *thread, unsigned *vthread, long long key)
{
	SendData_t *res;
	SendDataReference_t reference;
	reference.tag = tag;
	reference.target = receiver;
	reference.key = key;

	res = (SendData_t*) NewQueue_search (qsend, &reference, CompareSend_cbk);

	if (NULL != res)
	{
		*send_begin = res->send_begin;
		*send_end = res->send_end;
		*send_position = res->send_position;
		*thread = res->thread;
		*vthread = res->vthread;
		NewQueue_delete (qsend, res);
	}
	else
	{
		*send_begin = NULL;
		*send_end = NULL;
		*send_position = 0;
	}
}

/**************************************************************************
*** RECEIVE PART
**************************************************************************/

typedef struct
{
	event_t *recv_begin;
	event_t *recv_end;
	long long key;
	unsigned thread;
	unsigned vthread;
} 
RecvData_t;

typedef struct
{
	long long key;
	int tag;
	int target;
}
RecvDataReference_t;

void CommunicationQueues_QueueRecv (NewQueue_t *qreceive, event_t *recv_begin,
	event_t *recv_end, unsigned thread, unsigned vthread, long long key)
{
	RecvData_t tmp;

	tmp.recv_begin = recv_begin;
	tmp.recv_end = recv_end;
	tmp.thread = thread;
	tmp.vthread = vthread;
	tmp.key = key;

	NewQueue_add (qreceive, &tmp);
}

static int CompareRecv_cbk (void *reference, void *data)
{
	RecvData_t *d = (RecvData_t*) data;
	RecvDataReference_t *ref = (RecvDataReference_t*) reference;

	/* Return OK if the TAG, TARGET and KEY are the same */
	/*   we look for recvs, check whether the receiver is any tag */
	return (ref->tag == Get_EvTag(d->recv_end) || MPI_ANY_TAG == Get_EvTag(d->recv_end)) && 
	  ref->target == Get_EvTarget(d->recv_end) &&
		ref->key == d->key;
}

void CommunicationQueues_ExtractRecv (NewQueue_t *qreceive, int sender,
	int tag, event_t **recv_begin, event_t **recv_end, unsigned *thread,
	unsigned *vthread, long long key)
{
	RecvData_t *res;
	RecvDataReference_t reference;
	reference.tag = tag;
	reference.target = sender;
	reference.key = key;

	res = (RecvData_t*) NewQueue_search (qreceive, &reference, CompareRecv_cbk);

	if (NULL != res)
	{
		*recv_begin = res->recv_begin;
		*recv_end = res->recv_end;
		*thread = res->thread;
		*vthread = res->vthread;
		NewQueue_delete (qreceive, res);
	}
	else
	{
		*recv_begin = NULL;
		*recv_end = NULL;
	}
}

/**********************************************************************
*** INITIALIZATION 
**********************************************************************/

void CommunicationQueues_Init (NewQueue_t **send, NewQueue_t **receive)
{
	/* Initialize queues. Allocate 1024 entries by default. */
	*send = NewQueue_create (sizeof(SendData_t), 1024);
	*receive = NewQueue_create (sizeof(RecvData_t), 1024);
}
