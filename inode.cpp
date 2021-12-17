#include <stdatomic.h>

struct inode
{
    unsigned int ino;
    int link_count;
    atomic_int file_size;
    unsigned int nlinks[4];
    int single_indirect;

};
