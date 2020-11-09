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

#ifndef _TIMER_MANAGER_H_INCLUDED_
#define _TIMER_MANAGER_H_INCLUDED_

#include "event_timer.h"
#include "clock.h"
#include "common.h"

typedef void (*func_ptr_t) (void);

typedef struct xtr_TimerNode_t xtr_TimerNode_t;

struct xtr_TimerNode_t
{
	ExtraeTimer_t * timer;
	struct xtr_TimerNode_t * next;
	struct xtr_TimerNode_t * previous;
};

enum
{
	CALLBACK_LVL_THREAD = 0,
	CALLBACK_LVL_TASK,
	CALLBACK_LVL_NODE,
	CALLBACK_NUM_LVLS
};

typedef void * xtr_timer_t;

void xtrTimerManager_Trigger(void);

xtr_timer_t * xtrTimerManager_SetTimer(func_ptr_t callback_addr, int level, UINT64 period);

void xtrTimerManager_SetTimerFromList(int num_callbacks, char *** callback_list, UINT64 period);

void xtrTimerManager_RemoveTimer(xtr_timer_t timer);

void xtrTimerManager_EnableTimers(void);

void xtrTimerManager_DisableTimers(void);


#endif
