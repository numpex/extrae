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

# define __USE_GNU
# include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "xalloc.h"
#include "plugin_manager.h"
#include "utils.h"
#include "events.h"
#include "taskid.h"
#include "threadid.h"
#include "timer_manager.h"
#include "wrapper.h"


xtr_plugin_t * enabled_plugins = NULL;
xtr_plugin_t * disabled_plugins = NULL;

static inline void _xtr_plugin_add_node (xtr_plugin_t * node, xtr_plugin_t ** list_head)
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

static inline void _xtr_plugin_remove_node (xtr_plugin_t * node, xtr_plugin_t * list_head)
{
	if (node == list_head)
		enabled_plugins = node->next;

	if(node->previous != NULL)
	{
		(node->previous)->next = node->next;
	}
	if (node->next != NULL)
	{
		(node->next)->previous = node->previous;
	}
}

static inline void _xtr_plugin_configure(xtr_plugin_t * plugin, int read_at, func_ptr_t read_fn, UINT64 period, int level)
{
	int timer_level = CALLBACK_LVL_NODE;
  plugin->info.read_at = read_at;
	if ( period > 0 && read_fn != NULL )
	{
		plugin->info.period = period;
		if ( timer_level == PLUGIN_LVL_TASK ) timer_level = CALLBACK_LVL_TASK;
		plugin->info.timer_handle = xtrTimerManager_SetTimer(read_fn, timer_level, period);
	}
	plugin->info.level = level;
}

static int _xtr_plugin_set_callbacks(xtr_plugin_t * plugin, char * so_name)
{
		plugin->info.dl_handle = dlopen(so_name, RTLD_LAZY);
		char so_path[1024];

		if (plugin->info.dl_handle == NULL )
    {// tracehome from xml
			sprintf(so_path, "%s/plugins/lib%s.so", trace_home, so_name); 
		  plugin->info.dl_handle = dlopen(so_path, RTLD_LAZY);
		}
		if (plugin->info.dl_handle == NULL )
		{
			sprintf(so_path, "%s/plugins/%s", trace_home, so_name);
			plugin->info.dl_handle = dlopen(so_path, RTLD_LAZY);
		}

		if (plugin->info.dl_handle != NULL )
    {
      plugin->info.callbacks.init_function = dlsym (plugin->info.dl_handle, INIT_FUNC_NAME);
      plugin->info.callbacks.read_function = dlsym (plugin->info.dl_handle, READ_FUNC_NAME);

			plugin->info.initialized = (plugin->info.callbacks.init_function == NULL);
    }
  
    //cannot make a read here, find why
    if ( plugin->info.dl_handle == NULL || ( plugin->info.callbacks.read_function == NULL && plugin->info.callbacks.init_function == NULL ) )
		{
			printf("plugin %s could not be loaded\n", so_name);
      return ERROR;
		}
    else
		{
			printf("plugin %s loaded correctly\n", so_name);
      return SUCCESS;
		}
}

static inline int _xtr_assert_level(xtr_plugin_t * plugin)
{
	int execute_callback = 0;

	printf("switcash level%d plugin %p \n", plugin->info.level, plugin);
	if(plugin != NULL)
	{
		switch (plugin->info.level)
		{
		case PLUGIN_LVL_TASK:
			execute_callback = (THREADID == 0) ? 1 : 0;
			break;
		case PLUGIN_LVL_APP:
			execute_callback = (THREADID == 0 && TASKID == 0) ? 1 : 0;
			break;
		default:
			break;
		}
	}
	return execute_callback;
}

void execute_init(xtr_plugin_t * plugin)
{
  char *extraeMetadata;
  if (plugin->info.dl_handle != NULL && plugin->info.callbacks.init_function != NULL)
  {
    plugin->info.callbacks.init_function(&extraeMetadata);
    Extrae_AddStringEntryToGlobalSYM ('M', extraeMetadata);
  }
}

void _xtr_plugin_execute_callback(xtr_plugin_t * plugin, int type)
{
	int execute_callback = _xtr_assert_level(plugin);

	printf("Executing init tid %d callback %d \n", TASKID, execute_callback);
	if (execute_callback)
	{
		switch (type)
		{
			case CALLBACK_INIT:
        execute_init(plugin);
				break;
			case CALLBACK_READ:
        if (plugin->info.dl_handle != NULL && plugin->info.callbacks.read_function != NULL)
        {
				  plugin->info.callbacks.read_function ();
        }
				break;
			default:
				break;
		}
	}
}

//The init callbacks cannot be executed at this point, the loading may be done from xml parsing
xtr_plugin_t * xtr_plugin_load(char * so_name , int read_at, UINT64 period, int level)
{
	xtr_plugin_t * new_plugin = NULL;
	new_plugin = (xtr_plugin_t *) xmalloc(sizeof(xtr_plugin_t));
	if (new_plugin != NULL && so_name != NULL)
	{
		if (_xtr_plugin_set_callbacks(new_plugin, so_name) == ERROR )
    {
      xfree(new_plugin);
      new_plugin = NULL;
    }
    else
    {
		  _xtr_plugin_configure(new_plugin, read_at, GET_READ_FN(new_plugin), period, level);
		  _xtr_plugin_add_node(new_plugin, &enabled_plugins);
    }
	}
	return new_plugin;
}

void xtr_plugins_exe_callbacks(int read_at)
{
	xtr_plugin_t * plugin = enabled_plugins;
	if (read_at == PLUGIN_READ_AT_INI )
	{
		while ( plugin != NULL )
		{
			if( plugin->info.initialized == 0 )
			{
				printf("taskid %d enabled_plugins \n", TASKID);
				_xtr_plugin_execute_callback(plugin, CALLBACK_INIT);
				plugin->info.initialized = 1;
			}
			plugin = plugin->next;
		}
		plugin = enabled_plugins;
	}

	while ( plugin != NULL && ( read_at & plugin->info.read_at ) )
	{
    if( plugin->info.initialized == 0 )
		{
	  	_xtr_plugin_execute_callback(plugin, CALLBACK_INIT);
			plugin->info.initialized = 1;
    }
		_xtr_plugin_execute_callback(plugin, CALLBACK_READ);
		plugin = plugin->next;
	}
}

void xtr_plugin_disable(xtr_plugin_t * plugin)
{
	if (plugin != NULL)
	{
		_xtr_plugin_remove_node(plugin, enabled_plugins);
		_xtr_plugin_add_node(plugin, &disabled_plugins);
	}
}

void xtr_plugin_enable(xtr_plugin_t * plugin)
{
	if (plugin != NULL)
	{
		void * read_fn = plugin->info.callbacks.read_function;
		if (plugin->info.period > 0 && read_fn != NULL)
			plugin->info.timer_handle = xtrTimerManager_SetTimer(read_fn, plugin->info.period, plugin->info.level);

		_xtr_plugin_remove_node(plugin, disabled_plugins);
		_xtr_plugin_add_node(plugin, &enabled_plugins);
	}
}