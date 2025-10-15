#ifndef VARLEN_LZW_H
#define VARLEN_LZW_H

#include <cstddef>
#include <cstdint>

namespace LZW{

//TODO: This can probably be improved but it's fine for now
struct IndexStr{
  uint16_t index;
  LZW::IndexStr* next{nullptr};
};

void freeIndexStr(LZW::IndexStr* is);

int toindicies(uint16_t lzw_minimum_code_size, const char* data, std::size_t size, LZW::IndexStr* out);

}

#endif
