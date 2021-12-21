#ifndef FILE_SYSTEM_DRIVER 
#define FILE_SYSTEM_DRIVER 
#include <string>
#include <vector>
#include <map>
#include "blockDevice.hpp"
#include "dentry.cpp"
#include "inode.cpp"

class Driver{
    private:
        BlockDevice device;
        std::map<int,int> open_files;
        int unique_file_descriptor;
        void set_block_inuse(int block_index);
        void set_block_unuse(int block_index);
        void set_max_descriptor_count(int n);
        int get_max_descriptor_count();
        void update_descriptor(inode);
        void read(inode inode, int offset,int  size, char* buff);
        void write(inode inode,int offset, char* data, int size);
        std::vector<int> get_block_addresses(inode inode, int start_index, int end_index);
        inode lookup(std::string file_path);
        inode get_first_unused_descriptor();
        void add_link(inode dir, inode file, std::string file_name);
        std::vector<dentry> read_directory(inode dir);
        int get_unused_block();
        inode add_block(inode inode, int block_address);
        inode remove_last_block(inode file);
        void truncate(inode inode,int new_size);

    public:
    Driver(BlockDevice device);
        void mk_fs(int n);
        inode get_descriptor(int ino);
        std::vector<dentry> read_directory(std::string dir_path);
        void create_file(std::string file_path);
        int open(std::string file_path);
        void close(int nfd);
        void read(int nfd, int offset, int size, char* buff);
        void write(int nfd, int offset, char* data, int size);
        void link(std::string file_path1, std::string file_path2);
        void unlink(std::string file_path);
        void truncate(std::string file_path, uint new_size);
};

#endif // FILE_SYSTEM_DRIVER 