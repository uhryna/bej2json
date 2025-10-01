#pragma once

#include <stddef.h>
#include <stdint.h>
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif
bej_status io_read_all(char *path, uint8_t **out_data, size_t *out_len);
void  io_free(uint8_t *p);
#ifdef __cplusplus
}
#endif
