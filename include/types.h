#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum{
  E_OK=0,
  E_IO,
  E_RANGE,
  E_PARSE,
  E_DICT,
  E_UNSUPPORTED,
  E_DEPTH,
  E_ENOTIMPL
}bej_status;

typedef enum{
  BEJ_T_SET = 0x0,
  BEJ_T_INTEGER = 0x3,
  BEJ_T_STRING = 0x5
}bej_types;

#ifdef __cplusplus
}
#endif
