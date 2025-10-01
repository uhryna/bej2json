#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <algorithm>
#include "decoder.h"
#include "dict.h"
#include "writer.h"
#include "types.h"

// локальний конструктор синтетичного словника
static std::vector<uint8_t> make_synth_dict(){
  const uint16_t entry_count = 3;
  const uint32_t entries_offset = 12;
  const uint32_t entry_size = 10;
  const uint32_t entries_size = entry_count * entry_size;
  const uint32_t names_offset = entries_offset + entries_size;

  std::string n0 = "Root";  n0.push_back('\0');
  std::string n1 = "Name";  n1.push_back('\0');
  std::string n2 = "Value"; n2.push_back('\0');

  uint32_t off0 = names_offset;
  uint32_t off1 = off0 + (uint32_t)n0.size();
  uint32_t off2 = off1 + (uint32_t)n1.size();
  uint32_t dict_size = off2 + (uint32_t)n2.size();

  std::vector<uint8_t> d(dict_size, 0);
  auto wr16=[&](size_t p,uint16_t v){ d[p]=uint8_t(v&0xFF); d[p+1]=uint8_t(v>>8); };
  auto wr32=[&](size_t p,uint32_t v){ d[p]=v&0xFF; d[p+1]=(v>>8)&0xFF; d[p+2]=(v>>16)&0xFF; d[p+3]=(v>>24)&0xFF; };

  // header
  wr16(2, entry_count);
  wr32(8, dict_size);

  // Root
  size_t e0 = entries_offset + 0*entry_size;
  d[e0+0]=0; wr16(e0+1,0); wr16(e0+3, entries_offset + 1*entry_size);
  wr16(e0+5, 2); d[e0+7]=(uint8_t)n0.size(); wr16(e0+8,(uint16_t)off0);

  // Name (seq=1)
  size_t e1 = entries_offset + 1*entry_size;
  d[e1+0]=0; wr16(e1+1,1); wr16(e1+3,0); wr16(e1+5,0);
  d[e1+7]=(uint8_t)n1.size(); wr16(e1+8,(uint16_t)off1);

  // Value (seq=2)
  size_t e2 = entries_offset + 2*entry_size;
  d[e2+0]=0; wr16(e2+1,2); wr16(e2+3,0); wr16(e2+5,0);
  d[e2+7]=(uint8_t)n2.size(); wr16(e2+8,(uint16_t)off2);

  std::copy(n0.begin(),n0.end(),d.begin()+off0);
  std::copy(n1.begin(),n1.end(),d.begin()+off1);
  std::copy(n2.begin(),n2.end(),d.begin()+off2);
  return d;
}

static void push_nnint(std::vector<uint8_t>& v, uint64_t val, uint8_t n){
  v.push_back(n);
  for(uint8_t i=0;i<n;i++) v.push_back((uint8_t)((val>>(8*i))&0xFF));
}

TEST(DecoderSynth, RootSet_String_Int){
  auto dict_bytes = make_synth_dict();
  dict_t dict; ASSERT_EQ(E_OK, dict_init_from_bytes(dict_bytes.data(), dict_bytes.size(), &dict));

  std::vector<uint8_t> bej;
  bej.insert(bej.end(), {0x00,0xF0,0xF1,0xF0, 0x00,0x00,0x00}); // header

  bej.push_back(0x00); // S
  bej.push_back(0x00); // F (SET)
  size_t L_pos = bej.size(); bej.push_back(0x01); bej.push_back(0x00); // L placeholder

  std::vector<uint8_t> pl;
  push_nnint(pl, 2, 1); // count

  // Name (seq=1 => S=2), STRING "ABC\0"
  push_nnint(pl, 2, 1);
  pl.push_back(0x50);
  push_nnint(pl, 4, 1);
  pl.insert(pl.end(), {'A','B','C','\0'});

  // Value (seq=2 => S=4), INTEGER 3200 (0x0C80)
  push_nnint(pl, 4, 1);
  pl.push_back(0x30);
  push_nnint(pl, 2, 1);
  pl.insert(pl.end(), {0x80, 0x0C});

  bej[L_pos+0]=0x01; bej[L_pos+1]=(uint8_t)pl.size();
  bej.insert(bej.end(), pl.begin(), pl.end());

  FILE* out=tmpfile(); json_writer_t jw; jw_init(&jw, out);
  decoder_t dc; decoder_init(&dc, bej.data(), bej.size(), &dict);
  bej_status e = decoder_run(&dc, &jw);
  ASSERT_EQ(E_OK, e);

  fflush(out); fseek(out,0,SEEK_SET);
  std::string s; s.resize(4096);
  size_t n=fread(s.data(),1,s.size(),out); s.resize(n); fclose(out);

  EXPECT_NE(std::string::npos, s.find("\"Name\""));
  EXPECT_NE(std::string::npos, s.find("\"ABC\""));
  EXPECT_NE(std::string::npos, s.find("\"Value\""));
  EXPECT_NE(std::string::npos, s.find("3200"));
}
