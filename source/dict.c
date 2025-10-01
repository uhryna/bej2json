#include "dict.h"
#include <string.h>
#include <stdint.h>

enum{
  DICT_HDR_SIZE = 12,
  ENTRY_SIZE = 10,
  OFF_FM = 0,
  OFF_SEQ = 1,
  OFF_CHILD_POINTER = 3,
  OFF_CHILD_COUNTER = 5,
  OFF_NAME_LENGTH = 7,
  OFF_NAME_OFFSET = 8
};

static uint16_t read16_le(uint8_t *pointer){
  return (uint16_t)(pointer[0] | (pointer[1] << 8));
} 

static uint32_t read32_le(uint8_t *pointer){
  return (uint32_t)(pointer[0] | 
                   (pointer[1] << 8) | 
                   (pointer[2] << 16) | 
                   (pointer[3] << 24));
}

static int in_bounds(dict_t *d, uint32_t off, uint32_t need){
  return d && d->dict_base && off <= d->dict_length && need <= d->dict_length - off;
}

static uint32_t entry_off(dict_t *d, uint32_t i){
  return d->entries_offset + i * d->entry_jump;
}

static int entry_in_bounds(dict_t *d, uint32_t i){
  return in_bounds(d, d->entries_offset + i * d->entry_jump, d->entry_jump);
}

bej_status dict_init_from_bytes(uint8_t *bytes, size_t n, dict_t *out){
  if(!bytes || !out) return E_DICT;
  memset(out, 0, sizeof(*out));
  out->dict_base = bytes;
  out->dict_length = n;

  uint16_t entry_count = read16_le(&bytes[2]);
  uint32_t dict_size = read32_le(&bytes[8]);

  if(dict_size == 0 || dict_size > n){
    dict_size = (uint32_t)n;
  }
  
  out->entries_offset = DICT_HDR_SIZE;
  out->entry_jump = ENTRY_SIZE;
  out->tuple_count = entry_count;
  out->root_id = 0;

  uint64_t check_size = (uint64_t)out->entries_offset + 
                        (uint64_t)out->tuple_count * (uint64_t)out->entry_jump;
  
  if(check_size > dict_size) return E_DICT;
  
  out->names_offset = (uint32_t)check_size;
  out->names_length = dict_size - out->names_offset;

  for(uint32_t i = 0; i < entry_count; i++){
    if(!entry_in_bounds(out, i)) return E_DICT; 
    uint32_t eoff = entry_off(out, i);

    uint16_t child_ptr = read16_le(&bytes[eoff + OFF_CHILD_POINTER]);
    uint16_t child_counter = read16_le(&bytes[eoff + OFF_CHILD_COUNTER]);

    if(child_ptr){
      if(child_ptr < out->entries_offset) return E_DICT;
      uint32_t rel = child_ptr - out->entries_offset;
      if(rel % out->entry_jump != 0) return E_DICT;
      uint32_t first_index = rel / out->entry_jump;
      if(first_index > out->tuple_count || 
         child_counter > out->tuple_count - first_index) return E_DICT;
    }

    uint8_t name_len = bytes[eoff + OFF_NAME_LENGTH];
    uint16_t name_off = read16_le(&bytes[eoff + OFF_NAME_OFFSET]);
    
    if(name_len){
      if(name_off < out->names_offset) return E_DICT;
      if(!in_bounds(out, name_off, name_len)) return E_DICT;
    }
  }
  return E_OK;
}

bej_status dict_get_name(dict_t *dict, uint32_t node_id, const char **s, size_t *len){
  if(!dict || !s || !len) return E_RANGE;
  if(node_id >= dict->tuple_count) return E_RANGE;
  if(!entry_in_bounds(dict, node_id)) return E_DICT;
  
  uint32_t eoff = entry_off(dict, node_id);
  uint8_t name_len = dict->dict_base[eoff + OFF_NAME_LENGTH];
  uint16_t name_off = read16_le(&dict->dict_base[eoff + OFF_NAME_OFFSET]);

  if(name_len == 0 || name_off == 0) return E_PARSE;
  if(name_off < dict->names_offset) return E_DICT;
  if(!in_bounds(dict, name_off, name_len)) return E_DICT;

  size_t out_len = (size_t)name_len;
  if(out_len > 0 && dict->dict_base[name_off + out_len - 1] == '\0') out_len--;

  *s = (const char*)(dict->dict_base + name_off);
  *len = out_len;

  return E_OK;
}

bej_status dict_children_range(dict_t *dict, uint32_t node_id, uint32_t *first, uint32_t *count){
  if(!dict || !first || !count) return E_RANGE;
  if(node_id >= dict->tuple_count) return E_RANGE;
  if(!entry_in_bounds(dict, node_id)) return E_DICT;
  
  uint32_t eoff = entry_off(dict, node_id);
  uint16_t pointer = read16_le(&dict->dict_base[eoff + OFF_CHILD_POINTER]);
  uint16_t cnt = read16_le(&dict->dict_base[eoff + OFF_CHILD_COUNTER]);

  if(pointer == 0 || cnt == 0){
    *first = 0;
    *count = 0;
    return E_OK;
  }
  
  if(pointer < dict->entries_offset) return E_DICT;

  uint32_t rel = pointer - dict->entries_offset;
  if(rel % dict->entry_jump != 0) return E_DICT;
  uint32_t index = rel / dict->entry_jump;
  if(index > dict->tuple_count || cnt > dict->tuple_count - index) return E_DICT;

  *first = index;
  *count = cnt;

  return E_OK;
}

bej_status dict_child_by_seq(dict_t *dict, uint32_t parent_id, uint32_t seq, uint32_t *out_child){
  if(!dict || !out_child) return E_RANGE;

  uint32_t first = 0;
  uint32_t count = 0;
  bej_status e = dict_children_range(dict, parent_id, &first, &count);
  if(e != E_OK) return e;
  
  for(uint32_t i = 0; i < count; i++){
    uint32_t nid = first + i;
    if(!entry_in_bounds(dict, nid)) return E_DICT;
    uint32_t eoff = entry_off(dict, nid);
    uint16_t s = read16_le(&dict->dict_base[eoff + OFF_SEQ]);
    
    if(s == (uint16_t)seq){
      *out_child = nid;
      return E_OK;
    }
  }
  return E_PARSE;
}
