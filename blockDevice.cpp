#include <algorithm>
#include <iostream>
#include "constants.hpp"
#include "blockDevice.hpp"



BlockDevice::BlockDevice() {
  Block zero = {0};
  std::fill_n(memory, TOTAL_MEMORY, zero);
}

Block BlockDevice::read_block(int index){
  return memory[index];
}

void BlockDevice::write_block(int index, Block block){
  memory[index] = block;
}

