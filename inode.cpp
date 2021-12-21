#ifndef INODE
#define INODE

#include "constants.hpp"
#define UNUSED_FILE 0
#define REGULAR_FILE 1
#define DIRECTORY_FILE 2

struct block_map{
    int nlinks[NLINK];
    int single_indirect;
};

struct inode{
    int type;  
    int ino;
    int link_count;
    int file_size;
    block_map blocks;
};

#endif //INODE