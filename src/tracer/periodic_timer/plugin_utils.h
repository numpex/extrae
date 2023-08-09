#pragma once
#include "taskid.h"
#include "threadid.h"

/* callback level */
enum
{
  PLUGIN_LVL_APP,
	PLUGIN_LVL_NODE,
	PLUGIN_LVL_TASK,
	PLUGIN_LVL_THREAD,
	PLUGIN_NUM_LVLS
};

#define PLUGIN_CURRENT_LEVEL ( THREADID != 0 ? PLUGIN_LVL_THREAD : TASKID != 0 ? PLUGIN_LVL_TASK : PLUGIN_LVL_APP )
