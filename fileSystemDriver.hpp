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
        int ino=0;
        void set_block_inuse(int block_index);
        void set_block_unuse(int block_index);
        void set_max_descriptor_count(int n);
        int get_max_descriptor_count();
        void update_descriptor(inode);
        std::vector<char> read(inode inode, int offset,int  size);
        void write(inode inode,int offset, const char* data, int size);
        std::vector<int> get_block_addresses(inode inode, int start_index, int end_index);
        inode lookup(const std::string& file_path, bool resolve_symlink=false, int origin_dir_ino=-1, int * symlink_depth=nullptr);
        inode get_first_unused_descriptor();
        void add_link(inode dir, inode file, std::string file_name);
        std::vector<dentry> read_directory(inode dir);
        int get_unused_block();
        inode add_block(inode inode, int block_address);
        inode remove_last_block(inode file);
        void truncate(inode inode,int new_size);
        std::string parent_dir(std::string path);
        std::string get_file_name(std::string path);
        bool is_path_abs(std::string file_path);
        void unlink(inode inode, std::string file_name);
        void clean_block(int block_index);

    public:
    Driver(BlockDevice device);
        void mk_fs(int n);
        inode get_descriptor(int ino);
        std::vector<dentry> read_directory(std::string dir_path);
        void create_file(std::string file_path);
        int open(std::string file_path);
        void close(int nfd);
        std::vector<char> read(int nfd, int offset, int size);
        void write(int nfd, int offset, const char* data, int size);
        void link(std::string file_path1, std::string file_path2);
        void unlink(std::string file_path);
        void truncate(std::string file_path, uint new_size);
        void mkdir(std::string dir_path);
        void rmdir(std::string dir_path);
        void symlink(std::string file_path, std::string sim_path);
        void cd(std::string dir_path);
};

#endif // FILE_SYSTEM_DRIVER 