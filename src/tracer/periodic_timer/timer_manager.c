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

#include "common.h"
#include "xalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include "timer_manager.h"
#include "utils.h"
#include "wrapper.h"

int timersEnabled = 0;

int *in_timer_trigger = NULL;

xtr_TimerNode_t * enabled_timers[CALLBACK_NUM_LVLS] = { NULL };

// return 1 if found, else 0, node_position will hold the position of the node with the matching period if found
// otherwise the previous node 
static inline int _xtr_timers_find_position (int level, UINT64 period, xtr_TimerNode_t ** node_position)
{
	xtr_TimerNode_t * node = enabled_timers[level];
	xtr_TimerNode_t * last_node = NULL;

	while(node != NULL)
	{
		UINT64 node_period = xtrPeriodicEvTimer_GetPeriod(node->timer);
		if(node_period == period)
		{
			*node_position = node;
			return TRUE;
		}
		else if(node_period > period)
		{
			*node_position = last_node;
			return FALSE;
		}

		last_node = node;
		node = node->next;
	}
	
	node_position = last_node;
	return FALSE;
}

static inline void _xtr_timers_insert_node (xtr_TimerNode_t * node, xtr_TimerNode_t * previous_node, int level)
{
	node->previous = previous_node;
	node->next = (previous_node == NULL)? NULL : previous_node->next;

	if(previous_node != NULL)
	{
		previous_node->next = node;

		if(node->next != NULL)
			node->next->previous = node;
	}
	else
	{
		if (enabled_timers[level] != NULL)
		{
			enabled_timers[level]->previous = node;
			node->next = enabled_timers[level];
		}
		enabled_timers[level] = node;
	}
}


void xtrTimerManager_DisableTimers(void)
{
	timersEnabled = FALSE;
}

void xtrTimerManager_EnableTimers(void)
{
	timersEnabled = TRUE;
  if (in_timer_trigger == NULL)
    in_timer_trigger = xmalloc_and_zero(Extrae_get_num_threads() * sizeof(int));
}

/**
 * adds the callback to an existing timer with the same period,
 * otherwise creates one.
 * @param callback_name Name of the function
 *                      Do not pass the function adress.
 *                      These are the function names, they must be translated by xtrTranslateToCallbackAddr
 * @param period timer expiration period value (nanoseconds)
 */
xtr_timer_t * xtrTimerManager_SetTimer(func_ptr_t callback_addr, int level, UINT64 period)
{
  xtrTimerManager_EnableTimers();
	xtr_timer_t * retHandle = NULL;

	// find if there is a timer with the same period
	// otherwise allocate one
	xtr_TimerNode_t * node = NULL;
	int found = _xtr_timers_find_position(level, period, &node);
	if (!found)
	{
		xtr_TimerNode_t * new_node = NULL;
		new_node = (xtr_TimerNode_t *) xmalloc(sizeof(xtr_TimerNode_t));
		new_node->timer = xtrPeriodicEvTimer_NewTimer(period);
		retHandle = (xtr_timer_t *) xtrPeriodicEvTimer_AddSingleCallback(new_node->timer, callback_addr);
		
		_xtr_timers_insert_node(new_node, node, level);
		return new_node;
	}
	else
	{
		retHandle = (xtr_timer_t *) xtrPeriodicEvTimer_AddSingleCallback(node->timer, callback_addr);
		return node;
	}
	return retHandle;
}

int _xtr_timer_calc_lvl(int threadID, int TaskID)
{
  int level = CALLBACK_LVL_THREAD;
  if( TASKID == 0 && THREADID == 0 )
    level = CALLBACK_LVL_NODE;
  else if( THREADID == 0 )
    level = CALLBACK_LVL_TASK;

  return level;
}

void xtrTimerManager_Trigger(void)
{
  if (timersEnabled && in_timer_trigger[THREADID] == FALSE)
  {
    in_timer_trigger[THREADID] = TRUE;
    int level = _xtr_timer_calc_lvl(THREADID, TASKID);
    for (int i=0; i<= level; ++i)
    {
      xtr_TimerNode_t * t_node = enabled_timers[i]; // el orden de los niveles importa thread 0-> task 1-> node 2
      int expired = TRUE;
      while (t_node != NULL && expired && timersEnabled)
      {
        expired = xtrPeriodicEvTimer_TreatEvTimers(t_node->timer);
        t_node = t_node->next;
      }
    }
    in_timer_trigger[THREADID] = FALSE;
  }
}

//MODIFICAR 
void xtrTimerManager_RemoveTimer(xtr_timer_t timer)
{
	xtrPeriodicEvTimer_removeCallback((xtr_cbk_node_t *)timer, NULL);
}