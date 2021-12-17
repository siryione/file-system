#include <string>

class Driver{
    public:
        void static mk_fs(int n){

        };
        void get_descriptor(int id);
        void read_directory(std::string dir_path);
        void create_file(std::string file_path);
        int open(std::string file_path);
        void close(int nfd);
        char* read(int nfd, int offset, int size);
        void write(int nfd, int offset, char* data);
        void link(std::string file_path1, std::string file_path2);
        void unlink(std::string file_path);
        void truncate(std::string file_path, uint new_size);
};