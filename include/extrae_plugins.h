#pragma once
#include "extrae_types.h"

struct extrae_plugin_ev
{
   char* name;
   char* description;
};

typedef struct extrae_plugin_ev extrae_plugin_ev;

void Extrae_plugin_init ( char ** ExtraeMetadata );

void Extrae_plugin_read (void);