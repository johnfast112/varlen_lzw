#ifndef VARLEN_LZW_H
#define VARLEN_LZW_H

#include <cstddef>

namespace LZW{

//TODO: This can probably be improved but it's fine for now
struct IndexStr{
  int index;
  LZW::IndexStr* m_next{nullptr};
};

int toindicies(int lzw_minimum_code_size, const char* data, std::size_t size, LZW::IndexStr* out);

}

#endif
