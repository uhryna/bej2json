#include "io.h"
#include <stdlib.h>
#include <stdio.h>

bej_status io_read_all( char *path, uint8_t **out_data, size_t *out_len){
  
  if(!path || !out_data || !out_len) return E_RANGE;
  *out_data=NULL; *out_len=0;

  FILE *f=fopen(path,"rb");
  if(!f) return E_IO;

  if(fseek(f,0,SEEK_END)!=0){ fclose(f); return E_IO; }
  long sz=ftell(f);
  if(sz<0){ fclose(f); return E_IO; }
  if(fseek(f,0,SEEK_SET)!=0){ fclose(f); return E_IO; }

  uint8_t *buf=(uint8_t*)malloc((size_t)sz);
  if(!buf){ fclose(f); return E_IO; }
  size_t rd=fread(buf,1,(size_t)sz,f);
  fclose(f);
  if(rd!=(size_t)sz){ free(buf); return E_IO; }

  *out_data=buf; *out_len=(size_t)sz;
  return E_OK;
}

void io_free(uint8_t *p){ if(p) free(p); }
