#ifndef BLOCK_DEVICE
#define BLOCK_DEVICE
#include <string>
#include "constants.hpp"


struct Block
{
    char data[BLOCK_SIZE];
};

class BlockDevice {
  public:
    BlockDevice();
    Block read_block(int index);
    void write_block(int index, Block block);
  private:
    Block memory[TOTAL_MEMORY];
};
#endif // BLOCK_DEVICE