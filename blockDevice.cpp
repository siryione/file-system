#include <string>
#include <stdio.h>
#include <fstream>
#include "constants.hpp"


struct Block
{
    //data has fixed size (BLOCK_SIZE)
    char* data;
};


class BlockDevice{
        std::string file_name;
    public:
        void read_block(int index, Block* block){
            std::ifstream f(file_name.c_str());
            f.seekg(index*BLOCK_SIZE);
            f.read(block->data, BLOCK_SIZE);
        };

        void write_block(int index, Block* block){
            std::ofstream f(file_name.c_str());
            f.open(file_name);
            f.seekp(index*BLOCK_SIZE);
            f.write(block->data, BLOCK_SIZE);
        };

        BlockDevice(std::string file_name){
            this->file_name = file_name;
            std::ifstream f(file_name.c_str());
            if(!f.good()){
                std::ofstream f(file_name.c_str(), std::ios::binary);
                f.seekp(BLOCK_SIZE * TOTAL_MEMORY - 1);
                f.write("", 1);
                f.close();
            }
        }
};
