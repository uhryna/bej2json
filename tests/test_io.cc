#include <gtest/gtest.h>
#include "io.h"
#include "types.h"

TEST(IO, MissingFile){
  uint8_t* p=nullptr; size_t n=0;
  EXPECT_EQ(E_IO, io_read_all((char*)"tests/fixtures/no_such.bin", &p, &n));
  EXPECT_EQ(nullptr, p);
  EXPECT_EQ(0u, n);
}
