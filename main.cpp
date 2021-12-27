#include <iostream>
#include "constants.hpp"
#include "blockDevice.hpp"
#include "fileSystemDriver.hpp"
#include "inode.cpp"

int main(){
    BlockDevice device = BlockDevice();
    Driver driver(device);
    driver.mk_fs(10);
    driver.create_file("/file");
    driver.truncate("/file", 30);
    int nfd = driver.open("/file");
    char str[100] = "hello world!";
    driver.write(nfd, 10, str, 12);
    std::vector<char> read_res = driver.read(nfd, 0, 30);
    for(int i = 0; i<30; i++){
        std::cout << (int)read_res[i] << std::endl;
    }
    std::vector<dentry> dentries = driver.read_directory("/");
    for(int dentry_index = 0; dentry_index < dentries.size(); dentry_index++){
        std::cout << dentries[dentry_index].file_name << " " << dentries[dentry_index].ino << std::endl;
    }
    inode root = driver.get_descriptor(0);
    std::cout << root.ino << " " << root.file_size << " " << root.link_count << " "<< std::endl;

    driver.mkdir("/dir1");
    driver.mkdir("/dir1/dir2");
    dentries = driver.read_directory("/");
    for(int dentry_index = 0; dentry_index < dentries.size(); dentry_index++){
        std::cout << dentries[dentry_index].file_name << " " << dentries[dentry_index].ino << std::endl;
    }
    dentries = driver.read_directory("/dir1");
    for(int dentry_index = 0; dentry_index < dentries.size(); dentry_index++){
        std::cout << dentries[dentry_index].file_name << " " << dentries[dentry_index].ino << std::endl;
    }

    return 0;
}