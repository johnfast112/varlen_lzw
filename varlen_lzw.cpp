#include "varlen_lzw.h"

//TODO: REMOVE
#include <iostream>
#include <bitset>

#include <cstddef>
#include <cstdint>
#include <vector>

void pstr(LZW::IndexStr* is);//TODO: Remove

int LZW::toindicies(uint16_t lzw_minimum_code_size, const char* data_stream, std::size_t data_stream_size, LZW::IndexStr* out_str){
  LZW::IndexStr* cur_out_str{out_str};
  LZW::IndexStr held_code{};
  LZW::IndexStr* cur_held_code{nullptr};
  //TODO: Remove this debug info
  //Prints entire contents
  //for(int i{0}; i<size; ++i){
  //  std::cout << std::bitset<8>(data[i]) << '\n';
  //}

  //Information variables //TODO: Verify necessisity
  //static_cast to uint16_t makes warnings go away
  //
  //The first byte of the Compressed Data stream is a value indicating the minimum
  //number of bits required to represent the set of actual pixel values. Normally
  //this will be the same as the number of color bits. Because of some algorithmic
  //constraints however, black & white images which have one color bit must be
  //indicated as having a code size of 2.
  //This code size value also implies that the compression codes must start out one
  //bit longer.
  uint16_t compression_code_starting_bit_size{static_cast<uint16_t>(lzw_minimum_code_size+1)}; 

  //A special Clear code is defined which resets all compression/decompression
  //parameters and tables to a start-up state. The value of this code is 2**<code
  //size>. For example if the code size indicated was 4 (image was 4 bits/pixel)
  //the Clear code value would be 16 (10000 binary). The Clear code can appear at
  //any point in the image data stream and therefore requires the LZW algorithm to
  //process succeeding codes as if a new data stream was starting. Encoders should
  //output a Clear code as the first code of each image data stream.
  uint16_t clear_code{static_cast<uint16_t>(1<<lzw_minimum_code_size)};

  //An End of Information code is defined that explicitly indicates the end of
  //the image data stream. LZW processing terminates when this code is encountered.
  //It must be the last code output by the encoder for an image. The value of this
  //code is <Clear code>+1.
  uint16_t end_of_information_code{static_cast<uint16_t>(clear_code+1)};

  //The first available compression code value is <Clear code>+2.
  uint16_t first_available_compression_code{static_cast<uint16_t>(clear_code+2)};

  //The out* string will start with the value of the clear code since special
  //codes can be removed from the linked list later.
  //TODO: Remove special codes from the linked list
  out_str->index = clear_code;

  //The output codes are of variable length, starting at <code size>+1 bits per
  //code, up to 12 bits per code. This defines a maximum code value of 4095
  //(0xFFF). Whenever the LZW code value would exceed the current code length, the
  //code length is increased by one. The packing/unpacking of these codes must then
  //be altered to reflect the new code length.
  uint16_t compression_code_bit_size{static_cast<uint16_t>(compression_code_starting_bit_size)};

  //TODO: Remove this debug code
  std::cout << "binary size: " << data_stream_size << '\n';
  std::cout << "compression_code_starting_bit_size: " << compression_code_starting_bit_size << '\n';
  std::cout << "clear_code: " << clear_code << '\n';
  std::cout << "end_of_information_code: " << end_of_information_code << '\n';
  std::cout << "first_available_compression_code: " << first_available_compression_code << '\n';
  std::cout << "compression_code_bit_size: " << compression_code_bit_size << '\n';
  std::cout << '\n';

  //The size of the dictionary will start out as the value of <first available
  //compression code>-1. Each code smaller than the first compression code will
  //index into an entry containing it's own index as it's value. For example if
  //the first available compression code were 258, the dictionary 250th entry
  //would contain the string of indexes: 250. A larger code would index into an
  //entry containing multiple indexes, for example the 280th entry could contain
  //the string on indexes: 6, 251, 139.
  std::vector<LZW::IndexStr> dictionary{};
  for(int i{0}; i<first_available_compression_code; ++i){ //Populate the dictionary with entries where their index is it's value
    dictionary.push_back(LZW::IndexStr{static_cast<uint16_t>(i), nullptr}); //Index 0 has value 0, 1 has 1...
  }

  //The codes are formed into a stream of bits as if they were packed right to
  //left and then picked off 8 bits at a time to have their code extracted
  //Worst case 12 bits per code will at most occupy 3 bytes.
  char bufs[6]{0}; 
  //Breakdown of extra bufs index.
  //bufs[3] contains most significant bits
  //bufs[4] could contain the least significant bits, unless it cannot contain
  //all of them in which case it will contain 8 bits to exist somewhere in the
  //middle of the code and
  //bufs[5] will contain the least significant bits

  uint8_t bufs_index{0}; //index into which byte is the most significant bits
  //bufs_index = ( bufs_index+1 ) % 3; //Rotates index forward
  //bufs_index = ( bufs_index+2 ) % 3; //Rotates index backwards

  //Read 8 bits at a time until bits_read >= <compression code bit size>
  //Extract lzw code from string of 8 bit bytes
  //Index into dictionary using the extracted code
  //Append the indexed dictionary entry to the output
  //Hold on to the code for the next iteration
  //If there is already a code being held on to:
  //  Append the first index in the dictionary entry at the index of the current
  //  code to the entry at the index of the previous code
  //  Add this new string of indexes to the dictionary
  //If the extracted code refers to an entry that does not yet exist in the 
  //dictionary
  //  Append the first index in the dictionary entry at the index of the previous
  //  code to itself
  //  Add this new string of indexes to the dictionary
  //  Append this new dictionary entry to the output
  //If the new size of the dictionary does not fix within the number of bits for
  //lzw codes, increase the size of the codes by 1 bit
  uint8_t bits_read{0};
  uint16_t lzw_code{};
  std::size_t data_stream_index{0};
  while(data_stream_index<data_stream_size){
  //while(data_stream_index<5697){ //TODO: Replace me with above comment
  //while(data_stream_index<287){ //TODO: Replace me with above comment
    //TODO:Remove the following debug
    //std::cout << "Full string: ";
    //pstr(out_str);
    //std::cout << '\n';
    std::cout << "Held string: ";
    pstr(&held_code);
    std::cout << '\n';

    //If the size of the dictionary does not fit within the number of bits for lzw
    //codes, increase the size of the codes by 1 bit
    //TODO: Does this need cleaning up?
    if(dictionary.size() > ((uint32_t)1 << compression_code_bit_size) - 1){
      if(compression_code_bit_size < 12){
        compression_code_bit_size++;
      }
    }

    //Read 8 bits at a time until bits_read >= <compression code bit size>
    while(bits_read<compression_code_bit_size){

      //Read 8 bits
      bufs_index = ( bufs_index+1 ) % 3; //Buffer index is rotated before 8 bits are copied from the datastream so that bufs_index can point to the most significant bits of the lzw code
      bufs[bufs_index] = data_stream[data_stream_index]; //Copy 8 bits
      bits_read+=8;
      ++data_stream_index;
    }

    //Extract lzw code from string of 8 bit bytes
    //The most significant bits are at index bufs_index, though it could contain
    //extra bits that need to get cut off. We'll copy the byte containing the most
    //significant bits of the code to the extra bufs index, bit shift left to
    //remove the extra bits, then bit shift right to leave zeros in place of the
    //extra bits. Math for bit shifting the most significant bits can be found in
    //the MATH.txt file
    //
    //TODO: Most significant bits are not right maybe
    bufs[3] = static_cast<uint8_t>(bufs[bufs_index]) << (bits_read-compression_code_bit_size);
    bufs[3] = static_cast<uint8_t>(bufs[3]) >> (bits_read-compression_code_bit_size);
    //if(bits_read>8){
    //  bufs[3] = static_cast<uint8_t>(bufs[3]) >> (bits_read-compression_code_bit_size);
    //} else {
    //  bufs[3] = static_cast<uint8_t>(bufs[3]) >> (8-compression_code_bit_size);
    //}

    //The least significant bits are at the index before bufs_index. Remember the
    //bufs array can roll over from the end back to the start and vice versa with
    //the included operations. The least significant bits could also be spread
    //across one or two bytes in the bufs array. These bits could also contain extra
    //bits that need to get cut off. Bit shifting is still used to remove the extra
    //bits, however the operations aren't quite the same. Math for these can also
    //be found in the MATH.txt file.


    //bufs_index = ( bufs_index+2 ) % 3; //Rotates index backwards
    //bufs_index = ( bufs_index+1 ) % 3; //Rotates index forward. Is equivilant to rolling back twice
    if(bits_read>16){ //A full byte appears in bufs
      bufs[4] = bufs[( bufs_index+2 ) % 3]; //Previous byte is a full 8 bits
      bufs[5] = static_cast<uint8_t>(bufs[( bufs_index+1 ) % 3]) >> (24 - bits_read);
    } else if(bits_read>8){
      //TODO: This apperas to be correct
      bufs[4] = static_cast<uint8_t>(bufs[( bufs_index+2 ) % 3]) >> (16 - bits_read);
    } else {
      //TODO: This might be uneeded
      bufs[4] = static_cast<uint8_t>(bufs[( bufs_index+2 ) % 3]) >> (8 - bits_read);
    }

    //Create LZW Code
    //See MATH.txt
    if(bits_read>16){
      lzw_code = 
        ( static_cast<uint16_t>(static_cast<uint8_t>(bufs[5])                  ) ) | //Least significant bits
        ( static_cast<uint16_t>(static_cast<uint8_t>(bufs[4]) << (bits_read - 16)) ) | //More significant bits
        ( static_cast<uint16_t>(static_cast<uint8_t>(bufs[3]) << (bits_read - 8)) ); //Most significant bits
    } else if(bits_read>8){
      lzw_code = 
        ( static_cast<uint16_t>(static_cast<uint8_t>(bufs[4])                  ) ) | //Least significant bits
        ( static_cast<uint16_t>(static_cast<uint8_t>(bufs[3]) << (bits_read - 8)) ); //Most significant bits
    } else {
      lzw_code = 
        static_cast<uint16_t>(static_cast<uint8_t>(bufs[3])); //Most significant bits
    }



    std::cout << "data_stream_index " << data_stream_index << '\n';
    std::cout << "dictionary.size() " << dictionary.size() << '\n';
    std::cout << "compression_code_bit_size " << compression_code_bit_size << '\n';
    std::cout << "bits_read " << static_cast<uint16_t>(bits_read) << '\n';
    std::cout << "bufs_index " << static_cast<uint16_t>(bufs_index) << '\n';
    std::cout << "og code bits [0]: " << std::bitset<8>(bufs[0]) << " [1]: " << std::bitset<8>(bufs[1]) << " [2]: " << std::bitset<8>(bufs[2]) << '\n';
    std::cout << "lzw code bits [3]: " << std::bitset<8>(bufs[3]) << " [4]: " << std::bitset<8>(bufs[4]) << " [5]: " << std::bitset<8>(bufs[5]) << '\n';
    std::cout << "lzw_code " << lzw_code << ' ' << std::bitset<16>(lzw_code) << '\n';
    std::cout << '\n';

    bits_read -= compression_code_bit_size; //The next lzw code will only count the extra bits in the most recently used buffer byte

    //Special case code
    //TODO: Special case code
    if(lzw_code == clear_code){
      compression_code_bit_size = compression_code_starting_bit_size;
      cur_held_code = nullptr;
      std::cout << "HERE\n";
      freeIndexStr(&held_code);
      std::cout << "HERE\n";
      while(dictionary.size()>first_available_compression_code){
        freeIndexStr(&dictionary.back());
        dictionary.pop_back();
      }
      std::cout << "CLEAR CODE\n";
      continue;
    }
    if(lzw_code == end_of_information_code){
      //TODO: This
      std::cout << "END OF INFORMATION CODE\n";
      return 0;
      continue;
    }

    //TODO: Remove
    //Index into dictionary using the extracted code
    if(lzw_code<dictionary.size()){
      std::cout << "String " << static_cast<uint16_t>(lzw_code) << ": ";
      pstr(&dictionary.at(lzw_code));
      std::cout << '\n';
    }

    //If the extracted code refers to an entry that does not yet exist in the 
    //dictionary
    if(lzw_code>=dictionary.size()){
      //  Append the first index in the dictionary entry at the index of the previous
      //  code to itself
      cur_held_code->next = new LZW::IndexStr{};
      cur_held_code = cur_held_code->next;
      cur_held_code->index = held_code.index;

      //  Add this new string of indexes to the dictionary
      dictionary.push_back(LZW::IndexStr{});
      LZW::IndexStr* cur_dictionary_str = &dictionary.back();
      for(LZW::IndexStr* p{&held_code}; ; p = p->next){
        cur_dictionary_str->index = p->index;
        if(p->next == nullptr){
          break;
        }
        cur_dictionary_str->next = new LZW::IndexStr{};
        cur_dictionary_str = cur_dictionary_str->next;
      }

      //  Append this new dictionary entry to the output
      for(LZW::IndexStr* p{&dictionary.at(lzw_code)}; ; p = p->next){
        cur_out_str->next = new LZW::IndexStr{};
        cur_out_str = cur_out_str->next;
        cur_out_str->index = p->index;
        if(p->next == nullptr){
          break;
        }
      }
      std::cout << "New entry " << static_cast<uint16_t>(lzw_code) << ": ";
      pstr(&dictionary.at(lzw_code));
      std::cout << '\n';
      continue;
    }

    //Append the indexed dictionary entry to the output
    for(LZW::IndexStr* p{&dictionary.at(lzw_code)}; ; p = p->next){
      cur_out_str->next = new LZW::IndexStr{};
      cur_out_str = cur_out_str->next;
      cur_out_str->index = p->index;
      if(p->next == nullptr){
        break;
      }
    }

    //If there is already a code being held on to:
    if(cur_held_code != nullptr){
      //  Append the first index in the dictionary entry at the index of the current
      //  code to the entry at the index of the previous code
      cur_held_code->next = new LZW::IndexStr{};
      cur_held_code = cur_held_code->next;
      cur_held_code->index = dictionary.at(lzw_code).index;

      //  Add this new string of indexes to the dictionary
      dictionary.push_back(LZW::IndexStr{});
      LZW::IndexStr* cur_dictionary_str = &dictionary.back();
      for(LZW::IndexStr* p{&held_code}; ; p = p->next){
        cur_dictionary_str->index = p->index;
        if(p->next == nullptr){
          break;
        }
        cur_dictionary_str->next = new LZW::IndexStr{};
        cur_dictionary_str = cur_dictionary_str->next;
      }
      std::cout << "New entry " << dictionary.size()-1 << ": ";
      pstr(&dictionary.back());
      std::cout << '\n';
    }

    //Hold on to the code for the next iteration
    freeIndexStr(&held_code);
    cur_held_code = &held_code;
    for(LZW::IndexStr* p{&dictionary.at(lzw_code)}; ; p = p->next){
      cur_held_code->index = p->index;
      if(p->next == nullptr){
        break;
      }
      cur_held_code->next = new LZW::IndexStr{};
      cur_held_code = cur_held_code->next;
    }
  }








  //TODO The following is old code and may not be needed
  //
  //while(data_index<size){
  //while(data_index<1000){ //TODO: Replace me with above comment
  //  //TODO: Maybe off by one
  //  if(dictionary.size() > ((uint32_t)1 << compression_code_bit_size) - 1){
  //    compression_code_bit_size++;
  //  }

  //  //if(compression_code_bit_size>8){ //More than 1 byte codes
  //  //Copy at least enough bits for a code
  //  while(bits_read<compression_code_bit_size){
  //    bufs_index = ( bufs_index+1 ) % 3; //Rotates buffer index. Do this before so that bufs_index has the new value
  //    bufs[bufs_index] = data[data_index];
  //    bits_read+=8; // 1 byte(char) == 8 bits
  //    ++data_index; //Sets up for next byte in datastream
  //  }

  //  //TODO: Can this be better?
  //  //Cut off extra bits
  //  bufs[3] = static_cast<uint8_t>(bufs[bufs_index]) << (bits_read-compression_code_bit_size); //Cut off extra bits
  //  bufs[3] = static_cast<uint8_t>(bufs[3]) >> (bits_read-compression_code_bit_size); //Return bits to LSB side

  //  //Grab code
  //  //bufs MUST be cast to uint8 before uint16 to avoid filling new MSBs with 1s
  //  lzw_code = 
  //    static_cast<uint16_t>(static_cast<uint8_t>(bufs[(bufs_index+2) % 3])) | //Low bits = Previous index
  //    static_cast<uint16_t>(static_cast<uint8_t>(bufs[3])) << (bits_read-8); //High bits shifted to end of low bits

  //  //Setup next code
  //  bits_read-=compression_code_bit_size; //uncount bits
  //  bufs[bufs_index] = static_cast<uint8_t>(bufs[bufs_index]) >> (8-bits_read); //Current index will be previous in next code

  //  std::cout << "data_index " << data_index << '\n';
  //  std::cout << "dictionary.size() " << dictionary.size() << '\n';
  //  std::cout << "compression_code_bit_size " << compression_code_bit_size << '\n';
  //  std::cout << "lzw_code " << lzw_code << ' ' << std::bitset<16>(lzw_code) << '\n';
  //  std::cout << '\n';

  //  //Clear dictionary, reset paramaters, and continue
  //  if(lzw_code == clear_code){
  //    compression_code_bit_size = compression_code_starting_bit_size;
  //    while(dictionary.size()>first_available_compression_code){
  //      freeIndexStr(&dictionary.back());
  //      dictionary.pop_back();
  //    }
  //    continue;
  //  }

  //  //TODO: Cleanup
  //  //Create entry
  //  LZW::IndexStr baseEntryStr{};
  //  LZW::IndexStr* curEntryStr{&baseEntryStr};

  //  if(lzw_code<dictionary.size()){ //Code already exists
  //    std::cout << "Exists...\n";
  //    //Copy code to entry
  //    for(LZW::IndexStr* p{&dictionary.at(lzw_code)}; p != nullptr; p = p->next){
  //      curEntryStr->index = p->index;
  //      curEntryStr->next = new LZW::IndexStr{};
  //      curEntryStr = curEntryStr->next;
  //    }
  //  } else { //Code doesn't already exist
  //    std::cout << "Does not exist...\n";
  //    //Append running string first index to itself
  //    curRunningStr->index = baseRunningStr.index;
  //    
  //    //Copy running string to entry
  //    for(LZW::IndexStr* p{&baseRunningStr}; p != nullptr; p = p->next){
  //      curEntryStr->index = p->index;
  //      curEntryStr->next = new LZW::IndexStr{};
  //      curEntryStr = curEntryStr->next;
  //    }
  //  }

  //  //Append entry to result
  //  for(LZW::IndexStr* p{&baseEntryStr}; p->next != nullptr; p = p->next){
  //    curOutStr->index = p->index;
  //    curOutStr->next = new LZW::IndexStr{};
  //    curOutStr = curOutStr->next;
  //  }

  //  //Append new entry to dictionary
  //  if(baseRunningStr.next != nullptr){ //Only append a code if our running string is more than 1 index long (not first code)
  //    dictionary.push_back(LZW::IndexStr{});
  //    LZW::IndexStr* curDictionaryStr = &dictionary.back();
  //    //Copy running string first
  //    for(LZW::IndexStr* p{&baseRunningStr}; p->next != nullptr; p = p->next){
  //      curDictionaryStr->index = p->index;
  //      if(p->next != nullptr){
  //        curDictionaryStr->next = new LZW::IndexStr{};
  //        curDictionaryStr = curDictionaryStr->next;
  //      }
  //    }
  //    //Append first index of outputted entry to new dictionary entry
  //    curDictionaryStr->index = baseEntryStr.index;
  //  }

  //  //Update running string
  //  freeIndexStr(&baseRunningStr);
  //  curRunningStr = &baseRunningStr;
  //  //Copy current code to running string
  //  for(LZW::IndexStr* p{&dictionary.at(lzw_code)}; p != nullptr; p = p->next){
  //    curRunningStr->index = p->index;
  //    curRunningStr->next = new LZW::IndexStr{};
  //    curRunningStr = curRunningStr->next;
  //  }
  //}
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
    is->next = nullptr;
    return;
  }
}

//TODO: Remove
void pstr(LZW::IndexStr* is){
  for(LZW::IndexStr* p{is}; p != nullptr; p = p->next){
    std::cout << p->index << ' ';
    //std::cout << p->index << '\n';
  }
  std::cout << "EOSTR\n";
}
