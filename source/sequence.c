#include "sequence.h"

bej_status read_nnint(uint8_t *data, size_t length, size_t *offset, uint64_t *out){
  if(!data || !offset ||!out) return E_RANGE;
  if(*offset >= length) return E_RANGE;

  uint8_t n = data[(*offset)++];
  if(n > 8) return E_PARSE;
  if(length - *offset < n) return E_RANGE;

  uint64_t v = 0;
  for(uint8_t i = 0; i<n; i++){
    v |= ((uint64_t)data[*offset+i]) << (8u*i);
  }
  *offset += n;
  *out = v;
  return E_OK;
}
