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
#include "clock.h"
#include "common.h"
#include "event_timer.h"
#include "xalloc.h"

#define DEFAULT_N_ELEMS 4

static void _xtr_add_newFreeNodes(xtr_cbk_node_t * allocatedRegion, int oldSize, int newSize, xtr_cbk_node_t** freeNodes)
{
	if(newSize - oldSize > 1)
	{
		allocatedRegion[oldSize].callback = NULL;
		allocatedRegion[oldSize].previous = NULL;
		allocatedRegion[oldSize].next = &allocatedRegion[oldSize+1];

		for (int i=oldSize+1; i < newSize-1; ++i)
		{
			allocatedRegion[i].callback = NULL;
			allocatedRegion[i].previous = &allocatedRegion[i-1];
			allocatedRegion[i].next = &allocatedRegion[i+1];
		}

		allocatedRegion[newSize-1].callback = NULL;
		allocatedRegion[newSize-1].next = NULL;
		allocatedRegion[newSize-1].previous = &allocatedRegion[newSize-2];

		*freeNodes = &allocatedRegion[oldSize];
	}
}

/**
 * * allocates timer and sets initial values
 * @param period timer expiration period value (nanoseconds)
 * @return Extrae Periodic Event Timer
 * ! nanoseconds
 */
ExtraeTimer_t * xtrPeriodicEvTimer_NewTimer(UINT64 period)
{
	ExtraeTimer_t * newTimer = NULL;
	newTimer =xmalloc( sizeof(ExtraeTimer_t) );
	newTimer->lastEmissionTime = 0;
	newTimer->timerPeriod = period;
	
	newTimer->allocatedRegion = xmalloc( DEFAULT_N_ELEMS * sizeof(xtr_cbk_node_t) );
	newTimer->allocatedSize = DEFAULT_N_ELEMS;

	newTimer->callbacksList = NULL;
	newTimer->freeNodes = NULL;
	_xtr_add_newFreeNodes(newTimer->allocatedRegion, 0, newTimer->allocatedSize, &newTimer->freeNodes);

	return newTimer;
}

/**
 * * frees all asociated buffers and deletes the timer
 * @param Etimer Event Timer
 */
void xtrPeriodicEvTimer_DeleteTimer(ExtraeTimer_t * Etimer)
{
	printf("hola %p \n", Etimer);
	xfree(Etimer->allocatedRegion);
	xfree(Etimer);
	Etimer = NULL;
}

/**
 * * Adds a callback function to a previously allocated Event Timer
 * @param Etimer Event Timer
 * @param num_callbacks number of callback functions to be added
 * @param function_list list of callback function to be added
 * ! these functions are called with no parameter
 */
void xtrEventTimer_AddCallbackList(ExtraeTimer_t * Etimer, int num_callbacks, func_ptr_t * function_list)
{
	for (int i=0; i< num_callbacks; ++i)
	{
		xtrPeriodicEvTimer_AddSingleCallback(Etimer, function_list[i]);
	}
}

static inline void _xtr_add_node (xtr_cbk_node_t * node, xtr_cbk_node_t ** list_head)
{
	node->previous = NULL;

	if (*list_head == NULL)
	{
		node->next = NULL;
	}
	else
	{
		node->next = *list_head;
		(*list_head)->previous = node;
	}

	*list_head = node;
}

static inline void _xtr_remove_node (xtr_cbk_node_t * node, xtr_cbk_node_t ** list_head)
{
	if(*list_head == node)
	{
		*list_head = node->next;
	}

	if(node->previous != NULL)
	{
		node->previous->next = node->next;
	}

	if(node->next != NULL)
	{
		node->next->previous = node->previous;
	}

	node->next = NULL;
	node->previous = NULL;
}

static inline xtr_cbk_node_t * _xtr_getFreeNode(xtr_cbk_node_t ** freeNodes)
{

	xtr_cbk_node_t * node = *freeNodes;
	*freeNodes = (*freeNodes)->next;
	if (node->next != NULL)
	{
		node->next->previous = NULL;
		node->next = NULL;
	}
	
	return node;
}

xtr_cbk_node_t * xtrPeriodicEvTimer_AddSingleCallback(ExtraeTimer_t * Etimer, func_ptr_t callback)
{

	// resize allocatedRegion if we have reached the limit
	if (Etimer->freeNodes == NULL)
	{
		Etimer->allocatedRegion = xrealloc( Etimer->allocatedRegion, 2 * Etimer->allocatedSize );
		_xtr_add_newFreeNodes(Etimer->allocatedRegion, Etimer->allocatedSize, 2 * Etimer->allocatedSize, Etimer->freeNodes);
		Etimer->allocatedSize *= 2;
	}
	
	xtr_cbk_node_t * node = NULL;
	// add callback if not null
	if (callback != NULL)
	{
		node = _xtr_getFreeNode(&Etimer->freeNodes);
		node->callback = callback;

		_xtr_add_node(node, &Etimer->callbacksList);

		++Etimer->numCallbaks;
	}

	return node;
}

/**
 * * calls the function asociated with the Event Timer
 * @param Etimer Event Timer
 * ! these functions are called with no parameter
 */
void xtrPeriodicEvTimer_CallFuncs(ExtraeTimer_t * Etimer)
{
	xtr_cbk_node_t * node = Etimer->callbacksList;
	while (node != NULL)
	{
		// no need to check if not null
		node->callback();

		node = node->next;
	}
}

/**
 * * checks if timer has expired and works acordingly
 * @param Etimer Event Timer
 */
int xtrPeriodicEvTimer_TreatEvTimers(ExtraeTimer_t * Etimer)
{
	UINT64 current_time = LAST_READ_TIME;
	UINT64 delta = current_time - Etimer->lastEmissionTime;
	if(Etimer->timerPeriod <= delta)
	{
		xtrPeriodicEvTimer_CallFuncs(Etimer);
		Etimer->lastEmissionTime = current_time;
		return TRUE;
	}
	return FALSE;
}

void xtrPeriodicEvTimer_TriggerCallbacks(ExtraeTimer_t * Etimer)
{
	xtrPeriodicEvTimer_CallFuncs(Etimer);
	Etimer->lastEmissionTime = LAST_READ_TIME;
}

UINT64 xtrPeriodicEvTimer_GetPeriod(ExtraeTimer_t * Etimer)
{
	return Etimer->timerPeriod;
}

xtr_cbk_node_t * xtrPeriodicEvTimer_GetCallbackList (ExtraeTimer_t * Etimer)
{
	return Etimer->callbacksList;
}

void xtrPeriodicEvTimer_removeCallback(xtr_cbk_node_t * cbk_node, ExtraeTimer_t * Etimer)
{
	cbk_node->callback = NULL;
	_xtr_remove_node(cbk_node, &Etimer->callbacksList);
	_xtr_add_node(cbk_node, &Etimer->freeNodes);
}
