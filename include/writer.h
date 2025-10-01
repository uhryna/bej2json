#pragma once
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  FILE *f;
  int   indent;
  int   need_comma_stack[128];
  int   sp;
  int   expect_value;
} json_writer_t;

void jw_init(json_writer_t *jw, FILE *f);
void jw_begin_object(json_writer_t *jw);
void jw_end_object(json_writer_t *jw);

void jw_key(json_writer_t *jw, const char *s, size_t n);
void jw_string(json_writer_t *jw, const char *s, size_t n);
void jw_number_i64(json_writer_t *jw, int64_t v);

#ifdef __cplusplus
}
#endif
