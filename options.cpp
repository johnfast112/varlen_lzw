#include "options.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

int options(int argc, char** argv, Args* args){
  // No args
  if(argc<2){
    std::cerr << "varlen_lzw: no arguments given\n";
    return 1; //No args
  }

  //no switch on cstring
  //read options
  for(int i{1}; i<argc; ++i){
    // -m, --lzw_minimum_code_size <size>
    if(
      !std::strcmp(argv[i], "-m") ||
      !std::strcmp(argv[i], "--lzw_minimum_code_size")
    ){
      //Verify arg exists
      ++i;
      if(argc<=i){
        std::cerr << "varlen_lzw: option \'" << argv[i-1] << "\' requires an argument\n";
        return 1; //Missing argument
      }

      //Cast to int and save in args
      char* p_end{};
      args->lzw_minimum_code_size = std::strtol(argv[i], &p_end, 10);
      if(argv[i] == p_end){
        std::cerr << "varlen_lzw: option \'" << argv[i-1] << "\' requires an integer argument\n";
        return 2; //Bad argument
      }

      continue;
    }

    // -i, --file <file>
    if(
      !std::strcmp(argv[i], "-i") ||
      !std::strcmp(argv[i], "--file")
    ){
      ++i;
      if(argc<=i){
        std::cerr << "varlen_lzw: option \'" << argv[i-1] << "\' requires an argument\n";
        return 1; //Missing argument
      }

      //save filename
      args->filename = argv[i];

      continue;
    }

    std::cerr << "varlen_lzw: unrecognized option \'" << argv[i] << "\'\n";
    return 2; //Bad argument
  }

  return 0;
}
