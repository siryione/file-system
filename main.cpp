#include <iostream>
#include "constants.hpp"
#include "blockDevice.hpp"
#include "fileSystemDriver.hpp"
#include "inode.cpp"

int main(){
    BlockDevice device = BlockDevice();
    Driver driver(device);
    driver.mk_fs(10);
    driver.create_file("file");
    std::vector<dentry> dentries = driver.read_directory(".");
    for(int dentry_index = 0; dentry_index < dentries.size(); dentry_index++){
        std::cout << dentries[dentry_index].file_name << " " << dentries[dentry_index].ino << std::endl;
    }
    inode root = driver.get_descriptor(0);
    std::cout << root.ino << " " << root.file_size << " " << root.link_count << " "<< std::endl;
    return 0;
}