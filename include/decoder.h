#pragma once

#include <stddef.h>
#include <stdint.h>
#include "types.h"
#include "dict.h"
#include "writer.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct{
  uint8_t *bej;
  size_t bej_length;
  size_t off;
  dict_t *dict;
  int strict;
  int max_depth;
  bej_status last_error;
  size_t last_offset;

}decoder_t;

void decoder_init(decoder_t *dc, uint8_t *bej, size_t bej_length, dict_t *dict);

bej_status decoder_run(decoder_t *dc, json_writer_t *jw);
#ifdef __cplusplus
}
#endif
