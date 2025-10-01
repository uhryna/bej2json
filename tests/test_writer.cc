#include <gtest/gtest.h>
#include <string>
#include "writer.h"
#include "types.h"

static std::string dump_json(void(*fn)(json_writer_t*)){
  FILE* out = tmpfile();
  json_writer_t jw; jw_init(&jw, out);
  fn(&jw);
  fflush(out); fseek(out,0,SEEK_SET);
  std::string s; s.resize(4096);
  size_t n=fread(s.data(),1,s.size(),out); s.resize(n); fclose(out);
  return s;
}

TEST(Writer, ObjectStringIntAndEscapes){
  auto out = dump_json([](json_writer_t* jw){
    jw_begin_object(jw);
      jw_key(jw, (char*)"key", 3);
      jw_string(jw, (char*)"A\"\\\b\f\n\r\t", 8);  // перевіряємо escape-и
      jw_key(jw, (char*)"n", 1);
      jw_number_i64(jw, 123456789);
    jw_end_object(jw);
  });

  EXPECT_NE(std::string::npos, out.find("\"key\""));
  EXPECT_NE(std::string::npos, out.find("\\\""));
  EXPECT_NE(std::string::npos, out.find("\\\\"));
  EXPECT_NE(std::string::npos, out.find("\\n"));
  EXPECT_NE(std::string::npos, out.find("\"n\""));
  EXPECT_NE(std::string::npos, out.find("123456789"));
}
