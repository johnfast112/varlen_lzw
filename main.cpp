// Args:
// - Binary file representing out string of bits
// - lzw_minimum_code_size
// Outputs:
// - List of indicies
// - Should store these indicies nicely too
// Returns:
// - 0 Good
// - 1 Missing args
// - 2 Bad arg

#include "options.h"
#include "varlen_lzw.h"

#include <cstddef>
#include <fstream>
#include <iostream>

void print_help(){
  //TODO: ME
}

int main(int argc, char** argv){
  //Read options
  Args args;
  int op_rv;
  if((op_rv = options(argc, argv, &args)) != 0){
    print_help();
    return op_rv;
  }

  //Verify options
  if(args.lzw_minimum_code_size<1){
    std::cerr << "varlen_lzw: requres a minimum code size of at least 1\n";
    return 2; //Bad argument
  }
  if(args.filename.empty()){
    std::cerr << "varlen_lzw: requires input file\n";
    return 1; 
  }

  //Read file
  std::ifstream file{args.filename, std::ios::in | std::ios::binary};
  if(!file.is_open()){
    std::cerr << "Could not open file \'" << args.filename << "\'\n";
    return 2; //Bad argument
  }

  file.seekg(0, std::ios::end);
  std::size_t size = file.tellg();
  char data[size];
  file.seekg(0, std::ios::beg);
  file.read(data, size);

  file.close();

  //TODO: Return indicies
  LZW::IndexStr indicies{};
  LZW::get_varlen_indicies(args.lzw_minimum_code_size, data, size, &indicies);

  //This is output. TODO: Remove me maybe?
  int count{0};
  for(LZW::IndexStr* p{&indicies}; p != nullptr; p = p->next){
    std::cout << "Index: " << p->index << '\n';
    ++count;
  }
  std::cout << "count: " << count << '\n';

  return 0;
}
