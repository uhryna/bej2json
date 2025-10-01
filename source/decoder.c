#include "decoder.h"
#include "sequence.h"
#include <string.h>
#include <stdio.h>

static int has(const decoder_t *dc, size_t need){
  return dc->off + need <= dc->bej_length;
}

static bej_status read_u8(decoder_t *dc, uint8_t *v){
  if(!has(dc, 1)) return E_RANGE; 
  *v = dc->bej[dc->off++]; 
  return E_OK;
}

static bej_status read_bytes(decoder_t *dc, size_t n, const uint8_t **p){
  if(!has(dc, n)) return E_RANGE; 
  *p = dc->bej + dc->off; 
  dc->off += n; 
  return E_OK;
}

static bej_status skip(decoder_t *dc, size_t n){
  if(!has(dc, n)) return E_RANGE; 
  dc->off += n; 
  return E_OK;
}

static int64_t sign_extend_le(const uint8_t *p, size_t L){
  if(L == 0) return 0;
  uint64_t v = 0;
  for(size_t i = 0; i < L && i < 8; i++) {
    v |= ((uint64_t)p[i]) << (8u * i);
  }
  int bits = (int)(L * 8);
  if(bits >= 64) return (int64_t)v;
  uint64_t sign = (uint64_t)1 << (bits - 1);
  if(v & sign) v |= ~(((uint64_t)1 << bits) - 1);
  return (int64_t)v;
}

static void print_fallback_key(json_writer_t *jw, uint32_t seq){
  char tmp[32];
  int n = snprintf(tmp, sizeof(tmp), "f%u", seq);
  jw_key(jw, tmp, (size_t)n);
}

static bej_status decode_tuple(decoder_t *dc, json_writer_t *jw, uint32_t parent_node_id, int depth, int emit_key);

static bej_status decode_set(decoder_t *dc, json_writer_t *jw, uint32_t child_node_id, int depth, uint64_t L){
  size_t end_off = dc->off + L; 
  uint64_t count = 0;
  bej_status e = read_nnint(dc->bej, dc->bej_length, &dc->off, &count);
  if(e) return e;

  jw_begin_object(jw);
  if(depth + 1 > dc->max_depth) return E_DEPTH;

  for(uint64_t i = 0; i < count; i++){
    e = decode_tuple(dc, jw, child_node_id, depth + 1, 1);
    if(e) return e;
  }

  jw_end_object(jw);

  if(dc->strict){
    if(dc->off != end_off) {
      if(dc->off < end_off){
        e = skip(dc, end_off - dc->off);
        if(e) return e;
      } else {
        return E_PARSE;
      }
    }
  } else {
    if(dc->off != end_off && end_off <= dc->bej_length) {
      dc->off = end_off;
    }
  }
  return E_OK;
}

static bej_status decode_tuple(decoder_t *dc, json_writer_t *jw, uint32_t parent_node_id, int depth, int emit_key){
  bej_status e;
  uint64_t S = 0, L = 0;
  uint8_t  F = 0;

  if(depth > dc->max_depth) return E_DEPTH;

  e = read_nnint(dc->bej, dc->bej_length, &dc->off, &S);
  if(e) return e;

  e = read_u8(dc, &F);
  if(e) return e;

  e = read_nnint(dc->bej, dc->bej_length, &dc->off, &L);
  if(e) return e;

  uint32_t dict_id = (uint32_t)(S & 1u);
  uint32_t seq     = (uint32_t)(S >> 1);
  uint32_t type    = (uint32_t)(F >> 4);

  if(dict_id == 1){
    return skip(dc, (size_t)L);
  }

  uint32_t child_id = 0;
  const char *key_ptr = NULL; 
  size_t key_len = 0;
  
  e = dict_child_by_seq(dc->dict, parent_node_id, seq, &child_id);
  if(e == E_OK){
    if(dict_get_name(dc->dict, child_id, &key_ptr, &key_len) != E_OK){
      key_ptr = NULL; 
      key_len = 0; 
    }
  } else if(e != E_PARSE){
    return e; 
  }

  if(emit_key){
    if(key_ptr) 
      jw_key(jw, (char*)key_ptr, key_len);  
    else        
      print_fallback_key(jw, seq);
  }
  
  switch(type){
    case BEJ_T_SET:
      return decode_set(dc, jw, (depth == 0 ? parent_node_id : (e == E_OK ? child_id : parent_node_id)),depth, L);
    case BEJ_T_STRING: {
      const uint8_t *p = NULL;
      e = read_bytes(dc, (size_t)L, &p);
      if(e) return e;
      size_t n = (size_t)L;
      if(n > 0 && p[n - 1] == 0x00) n--;  
      jw_string(jw, (const char*)p, n);
      return E_OK;
    }
    
    case BEJ_T_INTEGER: {
      if(dc->strict && L > 8) return E_UNSUPPORTED;
      const uint8_t *p = NULL;
      e = read_bytes(dc, (size_t)L, &p);
      if(e) return e;
      int64_t v = sign_extend_le(p, (size_t)L);
      jw_number_i64(jw, v);
      return E_OK;
    }
    
    default:
      if(dc->strict) return E_UNSUPPORTED;
      return skip(dc, (size_t)L);
  }
}

void decoder_init(decoder_t *dc, uint8_t *bej, size_t bej_len, dict_t *dict){
  memset(dc, 0, sizeof(*dc));
  dc->bej = bej; 
  dc->bej_length = bej_len; 
  dc->off = 0;
  dc->dict = dict; 
  dc->strict = 1; 
  dc->max_depth = 64;
  dc->last_error = E_OK; 
  dc->last_offset = 0;
}
  bej_status decoder_run(decoder_t *dc, json_writer_t *jw){
  if(!dc || !jw || !dc->dict) return E_RANGE;

  int has_header = 0;
  if(dc->bej_length >= 4){
    uint8_t b0 = dc->bej[0], b1 = dc->bej[1], b2 = dc->bej[2], b3 = dc->bej[3];
    uint32_t le = (uint32_t)b0 | ((uint32_t)b1<<8) | ((uint32_t)b2<<16) | ((uint32_t)b3<<24);
    uint32_t be = (uint32_t)b3 | ((uint32_t)b2<<8) | ((uint32_t)b1<<16) | ((uint32_t)b0<<24);

    if (le == 0xF0F1F000 || le == 0xF1F0F000 || be == 0xF0F1F000 || be == 0xF1F0F000) {
      has_header = 1;
    } else {
      if (b0 > 8) has_header = 1;
    }
  }

  if (has_header) {
    if(dc->bej_length < 7) return E_RANGE; 
    dc->off = 7;
  } else {
    dc->off = 0; 
  }

  bej_status e = decode_tuple(dc, jw, dict_root(dc->dict), 0, 0);
  dc->last_error = e;
  dc->last_offset = dc->off;
  return e;
}

