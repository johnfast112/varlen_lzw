#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>

struct Args{
  int lzw_minimum_code_size{0};
  std::string filename;
};

int options(int argc, char** argv, Args* args);

#endif
