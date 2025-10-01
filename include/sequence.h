#pragma once
#include <stddef.h>
#include <stdint.h>
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif
bej_status read_nnint(uint8_t *data, size_t length, size_t *offset, uint64_t *out);
#ifdef __cplusplus
}
#endif
