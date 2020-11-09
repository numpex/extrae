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

#ifndef __PLUGIN_MANAGER_H__
#define __PLUGIN_MANAGER_H__

#include <stdlib.h>
#include "timer_manager.h"

#define INIT_FUNC_NAME "Extrae_plugin_init"
#define READ_FUNC_NAME "Extrae_plugin_read"

typedef void (*func_ptr_t) (void);

struct callbacks_t
{
	void (*init_function) (char **);
	func_ptr_t read_function;
	func_ptr_t fini_function;
};

typedef struct callbacks_t callbacks_t;

/* call callback read at */
enum
{
	PLUGIN_NOREAD,
	PLUGIN_READ_AT_INI = 0x1,
	PLUGIN_READ_AT_FIN = 0x2,
	PLUGIN_READ_AT_FLUSH = 0x4
};

/* callback level */
enum
{
  PLUGIN_LVL_APP,
	PLUGIN_LVL_NODE,
	PLUGIN_LVL_TASK,
	PLUGIN_LVL_THREAD
};

/* callback type */
enum
{
	CALLBACK_INIT,
	CALLBACK_READ,
	CALLBACK_FINI
};

enum
{
  ERROR = -1,
  SUCCESS
};

typedef struct 
{
	void * dl_handle;
	int initialized;
	int read_at;
	int period;
	xtr_timer_t * timer_handle;
	callbacks_t callbacks;
	int level;
} xtr_plugin_info_t;

typedef struct xtr_plugin_t xtr_plugin_t;

struct xtr_plugin_t
{
	xtr_plugin_info_t info;
	struct xtr_plugin_t * next;
	struct xtr_plugin_t * previous;
};

#define GET_READ_FN(xtr_plugin) (xtr_plugin->info.callbacks.read_function)
#define GET_INIT_FN(xtr_plugin) (xtr_plugin->info.callbacks.init_function)

xtr_plugin_t * xtr_plugin_load(char * so_name, int read_at, UINT64 period, int level);

void xtr_plugins_exe_callbacks(int read_at);

void xtr_plugin_disable(xtr_plugin_t * plugin);

void xtr_plugin_enable(xtr_plugin_t * plugin);

#endif /* __PLUGIN_MANAGER_H__ */