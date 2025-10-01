#pragma once

#include <stddef.h>
#include <stdint.h>
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct{
  uint8_t *dict_base;
  size_t dict_length;

  uint32_t tuple_count;
  uint32_t root_id;
  uint32_t entries_offset;
  uint32_t entry_jump;

  uint32_t names_offset;
  uint32_t names_length;
}dict_t;

bej_status dict_init_from_bytes(uint8_t *bytes, size_t n, dict_t *out);

bej_status dict_get_name(dict_t *dict, uint32_t node_id, const char **s, size_t *length);

bej_status dict_children_range(dict_t *dict, uint32_t node_id, uint32_t *first, uint32_t *count);

bej_status dict_child_by_seq(dict_t *dict, uint32_t parent_id, uint32_t seq, uint32_t *out_child);

static inline uint32_t dict_root(dict_t *dict){return dict ? dict->root_id : 0;}
#ifdef __cplusplus
}
#endif
