#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "extrae_user_events.h"
#include "extrae_plugins.h"

#define MALLINFO_BASE 550

extrae_plugin_ev mallinfo_events[] = 
{
  {"arena", "Total bytes allocated with brk/sbrk" },
  {"hblkhd", "Total bytes allocated with mmap" },
  {"uordblks", "Total sbrk memory in use" },
  {"fordblks", "Total sbrk memory free" },
  {"inuse", "Total memory in use" }
};

extrae_plugin_ev *active_events;
extrae_value_t *values;
extrae_type_t *types;

int numevents=0;
int total_num_events = 0;


int mallinfoGetEvent(const char *name_in, extrae_plugin_ev *event_out, extrae_type_t *type_out)
{
    for(int i=0; i<total_num_events; i++)
    {
        if( strcmp(mallinfo_events[i].name, name_in) == 0 )
        {
            event_out->name = mallinfo_events[i].name;
            event_out->description = mallinfo_events[i].description;
            *type_out = MALLINFO_BASE+i;

            return 1;
        }
    }
    return 0;
}

void Extrae_plugin_init( char ** ExtraeMetadata )
{
  char *params;
  char *name;

  total_num_events = sizeof(mallinfo_events) / sizeof(mallinfo_events[0]);
  active_events = malloc(total_num_events * sizeof(extrae_plugin_ev));  
  types = malloc(total_num_events * sizeof(extrae_type_t));

  //read envar
  params = getenv ("EXTRAE_PLUGIN_MALLINFO_PARAM");
  if ( params ==   NULL )
  {
      params = strdup ("arena,hblkhd,uordblks,fordblks,inuse");
  }

  //generate types and labels
  name = strtok(params, ",");
  while (name != NULL)
  {
      int found = mallinfoGetEvent(name, &active_events[numevents], &types[numevents]);
      if( found )
      {
      numevents++;
      }
      name = strtok(NULL, ",");
  }

  active_events = realloc(active_events, numevents * sizeof(extrae_plugin_ev));
  types = realloc(types, numevents * sizeof(extrae_type_t));

  //register labels for these type values
  unsigned zero = 0;
  for(int i=0; i< numevents; ++i)
  {
    Extrae_define_event_type (&types[i], active_events[i].description, &zero ,NULL, NULL);
  }

  //emmit events with value 0 for these counters
    values = calloc(numevents, sizeof(extrae_value_t));
    Extrae_nevent (numevents, types, values);

}

void Extrae_plugin_read (void)
{
  /* read information */
  struct mallinfo current_mi = mallinfo();

  int inuse = current_mi.arena + current_mi.hblkhd - current_mi.fordblks;

  /* emmit extrae events */
  if (inuse > 0)
  {
    extrae_value_t values[] = 
    { 
      current_mi.arena, 
      current_mi.hblkhd, 
      current_mi.uordblks, 
      current_mi.fordblks, 
      inuse
    };

    Extrae_nevent( numevents, types, values );
  }
}
