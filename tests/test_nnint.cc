#include <gtest/gtest.h>
#include "sequence.h"
#include "types.h"

TEST(NNINT, BasicLengths){
  // n=1 → 42
  uint8_t d1[] = {1, 0x2A};
  size_t off=0; uint64_t v=0;
  ASSERT_EQ(E_OK, read_nnint(d1, sizeof(d1), &off, &v));
  EXPECT_EQ(42u, v); EXPECT_EQ(2u, off);

  // n=2 → 0x1234 (LE)
  uint8_t d2[] = {2, 0x34, 0x12};
  off=0; v=0; ASSERT_EQ(E_OK, read_nnint(d2, sizeof(d2), &off, &v));
  EXPECT_EQ(0x1234u, v); EXPECT_EQ(3u, off);
}

TEST(NNINT, Errors){
  uint8_t bad_len[] = {9};           // n>8
  size_t off=0; uint64_t v=0;
  EXPECT_EQ(E_PARSE, read_nnint(bad_len, sizeof(bad_len), &off, &v));

  uint8_t trunc[] = {2, 0x00};       // не вистачає байтів
  off=0; v=0;
  EXPECT_EQ(E_RANGE, read_nnint(trunc, sizeof(trunc), &off, &v));
}
