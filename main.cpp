// Args:
// - Binary file representing out string of bits
// - lzw_minimum_code_size
// Outputs:
// - List of indicies
// - Should store these indicies nicely too

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

struct Args{
  int lzw_minimum_code_size{0};
  std::string filename;
};

void print_help(){
  //TODO: ME
}

int options(int argc, char** argv, Args* args){
  // No args
  if(argc<2){
    std::cout << "varlen_lzw: no arguments given\n";
    return 1; //No args
  }

  //no switch on cstring
  //read options
  for(int i{1}; i<argc; ++i){
    // -m, --lzw_minimum_code_size <size>
    if(
      !strcmp(argv[i], "-m") ||
      !strcmp(argv[i], "--lzw_minimum_code_size")
    ){
      //Verify arg exists
      ++i;
      if(argc<=i){
        std::cout << "varlen_lzw: option \'" << argv[i-1] << "\' requires an argument\n";
        return 2; //Missing argument
      }

      //Cast to int and save in args
      char* p_end{};
      args->lzw_minimum_code_size = std::strtol(argv[i], &p_end, 10);
      if(argv[i] == p_end){
        std::cout << "varlen_lzw: option \'" << argv[i-1] << "\' requires an integer argument\n";
        return 3;
      }

      continue;
    }

    // -i, --file <file>
    if(
      !strcmp(argv[i], "-i") ||
      !strcmp(argv[i], "--file")
    ){
      ++i;
      if(argc<=i){
        std::cout << "varlen_lzw: option \'" << argv[i-1] << "\' requires an argument\n";
        return 2; //Missing argument
      }

      //save filename
      args->filename = argv[i];

      continue;
    }

    std::cerr << "varlen_lzw: unrecognized option \'" << argv[i] << "\'\n";
    return 3;
  }

  return 0;
}

int main(int argc, char** argv){
  Args args;
  int op_rv;
  if((op_rv = options(argc, argv, &args)) != 0){
    return op_rv;
  }

  std::cout << args.lzw_minimum_code_size << '\n';
  std::cout << args.filename << '\n';

  //TODO: Read file

  return 0;
}
