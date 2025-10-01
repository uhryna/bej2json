#include <string.h>
#include <stdio.h>
#include "types.h"
#include "io.h"
#include "dict.h"
#include "decoder.h"
#include "writer.h"


int main(int argc, char *argv[]){
  char *dict_path = NULL;
  char *bej_path = NULL;
  char *out_path = NULL;
  int debug = 0;

  for(int i = 1; i < argc; i++){
    if(!strcmp(argv[i], "-d") || !strcmp(argv[i], "--dict")){
      if(++i < argc) dict_path = argv[i]; 
      else { 
        return 2; 
      } 
    } else if(!strcmp(argv[i], "-o") || !strcmp(argv[i], "--out")){
      if(++i < argc) out_path = argv[i]; 
      else { 
        return 2; 
      }
    } else if(argv[i][0] != '-'){
      bej_path = argv[i];
    } else {
      return 2;
    }
  }

  if(!dict_path || !bej_path) {
    return 2;
  }

  uint8_t *dict_buf = NULL;
  uint8_t *bej_buf = NULL;
  size_t dict_length = 0;
  size_t bej_length = 0;
  
  bej_status e = io_read_all(dict_path, &dict_buf, &dict_length);
  if(e){ 
    return 3;
  }
  
  e = io_read_all(bej_path, &bej_buf, &bej_length);
  if(e) {
    io_free(dict_buf); 
    return 3;
  }


  dict_t dict;
  e = dict_init_from_bytes(dict_buf, dict_length, &dict);
  if(e) {
    io_free(dict_buf); 
    io_free(bej_buf); 
    return 4;
  }


  FILE *out = stdout;
  if(out_path){
    out = fopen(out_path, "wb");
    if(!out){ 
      io_free(dict_buf); 
      io_free(bej_buf); 
      return 5;
    }
  }

  json_writer_t jw;
  jw_init(&jw, out);
  
  decoder_t decoder;
  decoder_init(&decoder, bej_buf, bej_length, &dict);
  
  e = decoder_run(&decoder, &jw);

  if(out_path) fclose(out);
  io_free(dict_buf); 
  io_free(bej_buf);
  return e ? 1 : 0;
}
