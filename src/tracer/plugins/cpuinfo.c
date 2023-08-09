#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "extrae_plugins.h"

void Extrae_plugin_init ( char ** ExtraeMetadata )
{
  int num_chars = 200;
  char filepath[1024];
  char buffer[ 32768 ];

  *ExtraeMetadata = malloc ( num_chars * sizeof ( char ) );

  sprintf(filepath, "/proc/cpuinfo");
  FILE *fp = fopen( filepath, "r" );
  if (fp == NULL) {
    printf("Couldn't open file %s\n", filepath);
    return;
  }
  else{
    size_t bytes_read = fread( *ExtraeMetadata, sizeof(char),num_chars, fp );
    printf("metadada:  %s\n", *ExtraeMetadata);
    if(bytes_read <= 0)
      sprintf(*ExtraeMetadata, "error reading %s\n", filepath);
    close( (int)fp );
  }
}