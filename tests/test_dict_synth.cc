#include <gtest/gtest.h>
#include <vector>
#include <string>
#include "dict.h"
#include "types.h"

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

  // Entry0: Root, має 2 дітей, покажчик на Entry1
  size_t e0 = entries_offset + 0*entry_size;
  d[e0+0]=0; wr16(e0+1,0); wr16(e0+3, entries_offset + 1*entry_size);
  wr16(e0+5, 2); d[e0+7]=(uint8_t)n0.size(); wr16(e0+8,(uint16_t)off0);

  // Entry1: Name (seq=1)
  size_t e1 = entries_offset + 1*entry_size;
  d[e1+0]=0; wr16(e1+1,1); wr16(e1+3,0); wr16(e1+5,0);
  d[e1+7]=(uint8_t)n1.size(); wr16(e1+8,(uint16_t)off1);

  // Entry2: Value (seq=2)
  size_t e2 = entries_offset + 2*entry_size;
  d[e2+0]=0; wr16(e2+1,2); wr16(e2+3,0); wr16(e2+5,0);
  d[e2+7]=(uint8_t)n2.size(); wr16(e2+8,(uint16_t)off2);

  // names
  std::copy(n0.begin(),n0.end(),d.begin()+off0);
  std::copy(n1.begin(),n1.end(),d.begin()+off1);
  std::copy(n2.begin(),n2.end(),d.begin()+off2);
  return d;
}

TEST(DictSynth, LoadsAndNames){
  auto bytes = make_synth_dict();
  dict_t dict; ASSERT_EQ(E_OK, dict_init_from_bytes(bytes.data(), bytes.size(), &dict));
  EXPECT_EQ(3u, dict.tuple_count);
  EXPECT_EQ(0u, dict.root_id);
  const char* s=nullptr; size_t n=0;
  ASSERT_EQ(E_OK, dict_get_name(&dict, 0, &s, &n)); EXPECT_EQ(std::string("Root"), std::string(s,n));
  ASSERT_EQ(E_OK, dict_get_name(&dict, 1, &s, &n)); EXPECT_EQ(std::string("Name"), std::string(s,n));
  ASSERT_EQ(E_OK, dict_get_name(&dict, 2, &s, &n)); EXPECT_EQ(std::string("Value"), std::string(s,n));
}
