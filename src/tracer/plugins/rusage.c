#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "extrae_user_events.h"
#include "extrae_plugins.h"

#define RUSAGE_BASE 450

extrae_plugin_ev rusage_events[] = 
{
    {"ru_utime",    "User time used"},
    {"ru_stime",    "System time used"},
    {"ru_maxrss",   "Maximum resident set size (in kilobytes)"},
    {"ru_ixrss",    "Text segment memory shared with other processes (kilobyte-seconds)"},
    {"ru_idrss",    "Data segment memory used (kilobyte-seconds)"},
    {"ru_isrss",    "Stack memory used (kilobyte-seconds)"},
    {"ru_minflt",   "Soft page faults"},
    {"ru_majflt",   "Hard page faults"},
    {"ru_nswap",    "Times a process was swapped out of physical memory"},
    {"ru_inblock",  "Input operations via the file system"},
    {"ru_oublock",  "Output operations via the file system"},
    {"ru_msgsnd",   "IPC messages sent"},
    {"ru_msgrcv",   "IPC messages received"},
    {"ru_nsignals", "Signals delivered"},
    {"ru_nvcsw",    "Voluntary context switches"},
    {"ru_nivcsw",   "Involuntary context switches"}
};

static struct rusage last_usage;

extrae_plugin_ev *active_events;
extrae_value_t *values;
extrae_type_t *types;
int numevents=0;
int total_num_events = 0;

int rusageGetEvent(const char *name_in, extrae_plugin_ev *event_out, extrae_type_t *type_out)
{
    for(int i=0; i<total_num_events; i++)
    {
        if( strcmp(rusage_events[i].name, name_in) == 0 )
        {
            event_out->name = rusage_events[i].name;
            event_out->description = rusage_events[i].description;
            *type_out = RUSAGE_BASE+i;

            return 1;
        }
    }
    return 0;
}

void Extrae_plugin_init( char ** ExtraeMetadata )
{
    int err;
    char *params;
    char *name;

    total_num_events = sizeof(rusage_events) / sizeof(rusage_events[0]);
    active_events = malloc(total_num_events * sizeof(extrae_plugin_ev));
    types = malloc(total_num_events * sizeof(extrae_type_t));

    //read envar
    params = getenv ("EXTRAE_PLUGIN_RUSAGE_PARAM");
    if ( params ==   NULL )
    {
        params = strdup ("ru_utime,ru_stime,ru_minflt,ru_majflt,ru_nvcsw,ru_nivcsw");
    }

    //generate types and labels
    name = strtok(params, ",");
    printf("name %s \n", name);
    while (name != NULL)
    {
        int found = rusageGetEvent(name, &active_events[numevents], &types[numevents]);
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

    //first read needed to calculate deltas
    getrusage(RUSAGE_SELF, &last_usage);
}

// //This call should only be executed from the main thread
void Extrae_plugin_read (void)
{
	int err;
    struct rusage current_usage;
    struct rusage delta_usage;

    /* gathering information */
    err = getrusage(RUSAGE_SELF, &current_usage);

    delta_usage.ru_utime.tv_sec = current_usage.ru_utime.tv_sec - last_usage.ru_utime.tv_sec;
    delta_usage.ru_utime.tv_usec = current_usage.ru_utime.tv_usec - last_usage.ru_utime.tv_usec;
    delta_usage.ru_stime.tv_sec = current_usage.ru_stime.tv_sec - last_usage.ru_stime.tv_sec;
    delta_usage.ru_stime.tv_usec = current_usage.ru_stime.tv_usec - last_usage.ru_stime.tv_usec;
    delta_usage.ru_minflt = current_usage.ru_minflt - last_usage.ru_minflt;
    delta_usage.ru_majflt = current_usage.ru_majflt - last_usage.ru_majflt;
    delta_usage.ru_nvcsw = current_usage.ru_nvcsw - last_usage.ru_nvcsw;
    delta_usage.ru_nivcsw = current_usage.ru_nivcsw - last_usage.ru_nivcsw;

    last_usage = current_usage;

    /* emmit extrae events */
    if (!err) 
    {
        extrae_value_t values[] = 
        {
            delta_usage.ru_utime.tv_sec * 1000000 + delta_usage.ru_utime.tv_usec, 
            delta_usage.ru_stime.tv_sec * 1000000 + delta_usage.ru_stime.tv_usec,
            delta_usage.ru_minflt,
            delta_usage.ru_majflt,
            delta_usage.ru_nvcsw,
            delta_usage.ru_nivcsw
        };

        Extrae_nevent (numevents, types, values);
    }
}
