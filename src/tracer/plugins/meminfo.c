#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "extrae_user_events.h"
#include "extrae_plugins.h"

#define MEMINFO_BASE 470

extrae_plugin_ev meminfo_events[] = 
{
  { "MemTotal"          , "Total usable RAM kB" },
  { "MemFree"           , "The sum of LowFree+HighFree kB" },
  { "MemAvailable"      , "An estimate of how much memory is available for starting new applications kB" },
  { "Buffers"           , "Relatively temporary storage for raw disk blocks (20MB) kB" },
  { "Cached"            , "In-memory cache for files read from the disk kB" },
  { "SwapCached"        , "Memory that once was swapped out and still in the swap file kB" },
  { "Active"            , "Memory that has been used more recently" },
  { "Inactive"          , "Memory which has been less recently used (more eligible to be reclaimed)" },
  { "Active(anon)"      , "Active(anon)" },
  { "Inactive(anon)"    , "Inactive(anon)" },
  { "Active(file)"      , "Active(file)" },
  { "Inactive(file)"    , "Inactive(file)" },
  { "Unevictable"       , "Unevictable" },
  { "Mlocked"           , "Mlocked" },
  { "SwapTotal"         , "Total amount of swap space available" },
  { "SwapFree"          , "Amount of swap space that is currently unused" },
  { "Dirty"             , "Memory which is waiting to get written back to the disk" },
  { "Writeback"         , "Memory which is actively being written back to the disk" },
  { "AnonPages"         , "Non-file backed pages mapped into user-space page tables" },
  { "Mapped"            , "Files which have been mapped into memory" },
  { "Shmem"             , "Amount of memory consumed in filesystems" },
  { "KReclaimable"      , "Kernel allocations that the kernel will attempt to reclaim under memory pressure. Includes SReclaimable" },
  { "Slab"              , "In-kernel data structures cache" },
  { "SReclaimable"      , "Part of Slab, that might be reclaimed" },
  { "SUnreclaim"        , "Part of Slab, that cannot be reclaimed" },
  { "KernelStack"       , "Amount of memory allocated to kernel stacks" },
  { "PageTables"        , "Amount of memory dedicated to the lowest level of page tables" },
  { "NFS_Unstable"      , "NFS pages sent to the server, but not yet committed to stable storage" },
  { "Bounce"            , "Memory used for block device" },
  { "WritebackTmp"      , "Memory used by FUSE for temporary writeback buffers" },
  { "CommitLimit"       , "Total amount of memory currently available to be allocated on the system" },
  { "Committed_AS"      , "The  amount  of  memory presently allocated on the system" },
  { "VmallocTotal"      , "Total size of vmalloc memory area" },
  { "VmallocUsed"       , "Amount of vmalloc area which is used" },
  { "VmallocChunk"      , "Largest contiguous block of vmalloc area which is free" },
  { "Percpu"            , "Percpu" },
  { "HardwareCorrupted" , "HardwareCorrupted" },
  { "AnonHugePages"     , "Non-file backed huge pages mapped into user-space page tables" },
  { "ShmemHugePages"    , "Memory used by shared memory and tmpfs allocated with huge pages" },
  { "ShmemPmdMapped"    , " Shared memory mapped into user space with huge pages" },
  { "FileHugePages"     , "FileHugePages" },
  { "FilePmdMapped"     , "FilePmdMapped" },
  { "Hugepagesize"      , "The size of huge pages" },
  { "Hugetlb"           , "Hugetlb" },
  { "DirectMap4k"       , "Number of bytes of RAM linearly mapped by kernel in 4 kB pages" },
  { "DirectMap2M"       , "Number of bytes of RAM linearly mapped by kernel in 2 MB pages" },
  { "DirectMap1G"       , "DirectMap1G" }
};

extrae_plugin_ev *active_events;
extrae_value_t *values;
extrae_type_t *types;
int numevents=0;
int total_num_meminfo_events=0;

int meminfoGetEvent(const char *name_in, extrae_plugin_ev *event_out, extrae_type_t *type_out)
{
  for(int i=0; i<total_num_meminfo_events; i++)
  {
    if( strcmp(meminfo_events[i].name, name_in) == 0 )
    {
      event_out->name = meminfo_events[i].name;
      event_out->description = meminfo_events[i].description;
      *type_out = MEMINFO_BASE+i;

      return 1;
    }
  }
  return 0;
}

void Extrae_plugin_init ( char ** ExtraeMetadata )
{
  char *params;
  char *name;
	char* match;

  total_num_meminfo_events = sizeof(meminfo_events) / sizeof(meminfo_events[0]);
  active_events = malloc(total_num_meminfo_events * sizeof(extrae_plugin_ev));
  types = malloc(total_num_meminfo_events * sizeof(extrae_type_t));

  //read envar
  params = getenv ("EXTRAE_PLUGIN_MEMINFO_PARAM");
  if ( params == NULL )
  {
    params = strdup ("MemTotal,MemFree");
  }

  //generate types and labels
  name = strtok(params, ",");
  while (name != NULL)
  {
    int found = meminfoGetEvent(name, &active_events[numevents], &types[numevents]);
    if( found )
    {
      numevents++;
    }
    name = strtok(NULL, ",");
  }

  active_events = realloc(active_events, numevents * sizeof(extrae_plugin_ev));
  types = realloc(types, numevents * sizeof(extrae_type_t));

  /* registering extrae event labels */
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
  /* gathering information */
  FILE *fp;
	char buffer[ 32768 ];
	size_t bytes_read;
	char* match;
	int res;

  fp = fopen( "/proc/meminfo", "r" );
  bytes_read = fread( buffer, 1, sizeof(buffer) - sizeof(char), fp );
  fclose( fp );

  if (bytes_read == 0)
    return;

  buffer[bytes_read] = '\0';
  unsigned i = 0;
  while (i < numevents)
  {
    match = strstr( buffer, active_events[i].name);
    if(match != NULL)
    {
      res = sscanf (match, "%*s %llu kB", &values[i]);
    }
    ++i;
  }

  /* launching extrae events */
  Extrae_nevent (numevents, types, values);
}