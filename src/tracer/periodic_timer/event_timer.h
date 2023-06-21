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

#ifndef _EVENT_TIMER_H_INCLUDED_
#define _EVENT_TIMER_H_INCLUDED_

typedef void (*func_ptr_t) (void);

typedef struct xtr_cbk_node_t xtr_cbk_node_t;

struct xtr_cbk_node_t
{
	func_ptr_t callback;
	struct xtr_cbk_node_t * next;
	struct xtr_cbk_node_t * previous;
};

struct ExtraeTimer
{
  UINT64 lastEmissionTime;
  UINT64 timerPeriod;
  int numCallbaks;
  int allocatedSize;
  xtr_cbk_node_t * callbacksList;
  xtr_cbk_node_t * freeNodes;
  xtr_cbk_node_t * allocatedRegion;
};

typedef struct ExtraeTimer ExtraeTimer_t;

ExtraeTimer_t * xtrPeriodicEvTimer_NewTimer(UINT64 period);
xtr_cbk_node_t * xtrPeriodicEvTimer_addCallback(ExtraeTimer_t * Etimer, func_ptr_t callback);
int xtrPeriodicEvTimer_TreatEvTimers(ExtraeTimer_t * Etimer);

UINT64 xtrPeriodicEvTimer_GetPeriod(ExtraeTimer_t * Etimer);

void xtrPeriodicEvTimer_DeleteTimer(ExtraeTimer_t * Etimer);
void xtrPeriodicEvTimer_removeCallback(xtr_cbk_node_t * cbk_node, ExtraeTimer_t * Etimer);

#endif


