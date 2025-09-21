#include "varlen_lzw.h"

//TODO: REMOVE
#include <iostream>
#include <bitset>

#include <cstddef>
#include <cstdint>
#include <vector>

void pstr(LZW::IndexStr* is);//TODO: Remove

uint16_t LZW::toindicies(uint16_t lzw_minimum_code_size, const char* data, std::size_t size, LZW::IndexStr* out){
  //Prints entire contents
  //for(int i{0}; i<size; ++i){
  //  std::cout << std::bitset<8>(data[i]) << '\n';
  //}

  //Information variables //TODO: Verify necessisity
  //static_cast to uint16_t makes warnings go away everything is scary
  uint16_t compression_code_starting_bit_size{static_cast<uint16_t>(lzw_minimum_code_size+1)}; 
  uint16_t clear_code{static_cast<uint16_t>(1<<lzw_minimum_code_size)};
  uint16_t end_of_information_code{static_cast<uint16_t>(clear_code+1)};
  uint16_t first_available_compression_code{static_cast<uint16_t>(clear_code+2)};
  //Max code value 4095 (0xFFF)
  uint16_t compression_code_bit_size{static_cast<uint16_t>(compression_code_starting_bit_size)};

  std::cout << "compression_code_starting_bit_size: " << compression_code_starting_bit_size << '\n';
  std::cout << "clear_code: " << clear_code << '\n';
  std::cout << "end_of_information_code: " << end_of_information_code << '\n';
  std::cout << "first_available_compression_code: " << first_available_compression_code << '\n';

  //Create our dictionary
  std::vector<LZW::IndexStr> dictionary{};
  for(int i{0}; i<first_available_compression_code; ++i){
    dictionary.push_back(LZW::IndexStr{static_cast<uint16_t>(i), nullptr});
  }

  //Indexing
  std::size_t data_index{0};
  char bufs[4]; //Worst case 12 bits per code will at most occupy 3 bytes, the fourth is a buffer for operations
  uint8_t bufs_index{0};
  //bufs_index = ( bufs_index+1 ) % 3; //Rotates index
  //bufs_index = ( bufs_index+2 ) % 3; //Rotates index backwards

  //results
  uint16_t lzw_code{};
  LZW::IndexStr baseRunningStr{};
  LZW::IndexStr* curRunningStr{&baseRunningStr};
  LZW::IndexStr* curOutStr{out};

  uint8_t bits_read{0};
  //while(data_index<size){
  while(data_index<1000){ //TODO: Replace me with above comment
    //TODO: Maybe off by one
    if(dictionary.size() > ((uint32_t)1 << compression_code_bit_size) - 1){
      compression_code_bit_size++;
    }

    //if(compression_code_bit_size>8){ //More than 1 byte codes
    //Copy at least enough bits for a code
    while(bits_read<compression_code_bit_size){
      bufs_index = ( bufs_index+1 ) % 3; //Rotates buffer index. Do this before so that bufs_index has the new value
      bufs[bufs_index] = data[data_index];
      bits_read+=8; // 1 byte(char) == 8 bits
      ++data_index; //Sets up for next byte in datastream
    }

    //TODO: Can this be better?
    //Cut off extra bits
    bufs[3] = static_cast<uint8_t>(bufs[bufs_index]) << (bits_read-compression_code_bit_size); //Cut off extra bits
    bufs[3] = static_cast<uint8_t>(bufs[3]) >> (bits_read-compression_code_bit_size); //Return bits to LSB side

    //Grab code
    //bufs MUST be cast to uint8 before uint16 to avoid filling new MSBs with 1s
    lzw_code = 
      static_cast<uint16_t>(static_cast<uint8_t>(bufs[(bufs_index+2) % 3])) | //Low bits = Previous index
      static_cast<uint16_t>(static_cast<uint8_t>(bufs[3])) << (bits_read-8); //High bits shifted to end of low bits

    //Setup next code
    bits_read-=compression_code_bit_size; //uncount bits
    bufs[bufs_index] = static_cast<uint8_t>(bufs[bufs_index]) >> (8-bits_read); //Current index will be previous in next code

    std::cout << "data_index " << data_index << '\n';
    std::cout << "dictionary.size() " << dictionary.size() << '\n';
    std::cout << "compression_code_bit_size " << compression_code_bit_size << '\n';
    std::cout << "lzw_code " << lzw_code << ' ' << std::bitset<16>(lzw_code) << '\n';
    std::cout << '\n';

    //Clear dictionary, reset paramaters, and continue
    if(lzw_code == clear_code){
      compression_code_bit_size = compression_code_starting_bit_size;
      while(dictionary.size()>first_available_compression_code){
        freeIndexStr(&dictionary.back());
        dictionary.pop_back();
      }
      continue;
    }

    //TODO: Cleanup
    //Create entry
    LZW::IndexStr baseEntryStr{};
    LZW::IndexStr* curEntryStr{&baseEntryStr};

    if(lzw_code<dictionary.size()){ //Code already exists
      std::cout << "Exists...\n";
      //Copy code to entry
      for(LZW::IndexStr* p{&dictionary.at(lzw_code)}; p != nullptr; p = p->next){
        curEntryStr->index = p->index;
        curEntryStr->next = new LZW::IndexStr{};
        curEntryStr = curEntryStr->next;
      }
    } else { //Code doesn't already exist
      std::cout << "Does not exist...\n";
      //Append running string first index to itself
      curRunningStr->index = baseRunningStr.index;
      
      //Copy running string to entry
      for(LZW::IndexStr* p{&baseRunningStr}; p != nullptr; p = p->next){
        curEntryStr->index = p->index;
        curEntryStr->next = new LZW::IndexStr{};
        curEntryStr = curEntryStr->next;
      }
    }

    //Append entry to result
    for(LZW::IndexStr* p{&baseEntryStr}; p->next != nullptr; p = p->next){
      curOutStr->index = p->index;
      curOutStr->next = new LZW::IndexStr{};
      curOutStr = curOutStr->next;
    }

    //Append new entry to dictionary
    if(baseRunningStr.next != nullptr){ //Only append a code if our running string is more than 1 index long (not first code)
      dictionary.push_back(LZW::IndexStr{});
      LZW::IndexStr* curDictionaryStr = &dictionary.back();
      //Copy running string first
      for(LZW::IndexStr* p{&baseRunningStr}; p->next != nullptr; p = p->next){
        curDictionaryStr->index = p->index;
        if(p->next != nullptr){
          curDictionaryStr->next = new LZW::IndexStr{};
          curDictionaryStr = curDictionaryStr->next;
        }
      }
      //Append first index of outputted entry to new dictionary entry
      curDictionaryStr->index = baseEntryStr.index;
    }

    //Update running string
    freeIndexStr(&baseRunningStr);
    curRunningStr = &baseRunningStr;
    //Copy current code to running string
    for(LZW::IndexStr* p{&dictionary.at(lzw_code)}; p != nullptr; p = p->next){
      curRunningStr->index = p->index;
      curRunningStr->next = new LZW::IndexStr{};
      curRunningStr = curRunningStr->next;
    }
  }
  //} else { //Less than 1 byte codes

  //} //Uncomment if we actually need the code size < or > 8 bits logic

  return 0;
}

//TODO: This is a stack overflow waiting to happen
void LZW::freeIndexStr(LZW::IndexStr* is){
  if(is->next == nullptr){
    return;
  } else {
    freeIndexStr(is->next);
    delete is->next;
    return;
  }
}

//TODO: Remove
void pstr(LZW::IndexStr* is){
  for(LZW::IndexStr* p{is}; p != nullptr; p = p->next){
    std::cout << p->index << '\n';
  }
  std::cout << "EOST\n";
}
