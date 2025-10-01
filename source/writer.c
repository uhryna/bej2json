#include <string.h>
#include "writer.h"

static void out(json_writer_t *jw, const char *s){ fputs(s, jw->f); }
static void outc(json_writer_t *jw, char c){ fputc(c, jw->f); }

static void newline(json_writer_t *jw){
  out(jw, "\n");
  for(int i=0;i<jw->indent;i++) out(jw,"  ");
}

void jw_init(json_writer_t *jw, FILE *f){
  jw->f=f; jw->indent=0; jw->sp=0; jw->expect_value=0;
  memset(jw->need_comma_stack,0,sizeof(jw->need_comma_stack));
}

static void coma_if_needed(json_writer_t *jw){
  if(jw->sp>0 && jw->need_comma_stack[jw->sp-1]){ outc(jw,','); newline(jw); }
  else if(jw->sp>0){ newline(jw); }
  if(jw->sp>0) jw->need_comma_stack[jw->sp-1]=1;
}

static void print_escaped(json_writer_t *jw, const char *s, size_t n){
  outc(jw,'"');
  for(size_t i=0;i<n;i++){
    unsigned char c=(unsigned char)s[i];
    switch(c){
      case '\"': out(jw,"\\\""); break;
      case '\\': out(jw,"\\\\"); break;
      case '\b': out(jw,"\\b");  break;
      case '\f': out(jw,"\\f");  break;
      case '\n': out(jw,"\\n");  break;
      case '\r': out(jw,"\\r");  break;
      case '\t': out(jw,"\\t");  break;
      default:
        if(c<0x20) fprintf(jw->f,"\\u%04X",(unsigned)c);
        else fputc(c,jw->f);
    }
  }
  outc(jw,'"');
}

void jw_begin_object(json_writer_t *jw){
  coma_if_needed(jw);
  outc(jw,'{'); jw->indent++; jw->sp++; jw->need_comma_stack[jw->sp-1]=0; jw->expect_value=0;
}

void jw_end_object(json_writer_t *jw){
  outc(jw,'\n'); jw->indent--;
  for(int i=0;i<jw->indent;i++) out(jw,"  ");
  outc(jw,'}'); jw->sp--; jw->expect_value=0;
}

void jw_key(json_writer_t *jw, const char *s, size_t n){
  coma_if_needed(jw);
  print_escaped(jw,s,n);
  out(jw,": ");
  jw->expect_value=1;
}

void jw_string(json_writer_t *jw, const char *s, size_t n){
  if(!jw->expect_value) coma_if_needed(jw);
  print_escaped(jw,s,n);
  jw->expect_value=0;
}

void jw_number_i64(json_writer_t *jw, int64_t v){
  if(!jw->expect_value) coma_if_needed(jw);
  fprintf(jw->f,"%lld",(long long)v);
  jw->expect_value=0;
}
