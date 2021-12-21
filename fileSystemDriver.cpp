#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <string.h>
#include <iostream>
#include <sstream>
#include "fileSystemDriver.hpp"
#include "constants.hpp"
#define NUN 0

Driver::Driver(BlockDevice device){
    this->device = device;
    unique_file_descriptor = 0;
}

void Driver::mk_fs(int n){
    size_t metadata_size = TOTAL_MEMORY/8+sizeof(int)+n*sizeof(inode);
    int block_count = ceil((double)metadata_size/BLOCK_SIZE);
    Block zero_block;
    std::fill(zero_block.data, zero_block.data + sizeof(zero_block.data), 0);
    Block block_test;
    for(int block_index = 0; block_index < block_count; block_index++){
        device.write_block(block_index, zero_block);
        set_block_inuse(block_index);
    }
    set_max_descriptor_count(n);
    inode root = {.type = DIRECTORY_FILE, .ino=0, .link_count=1, .file_size=0};
    update_descriptor(root);
    add_link(root, root, ".");
}

inode Driver::get_descriptor(int ino){
    inode res;
    int inode_address = TOTAL_MEMORY/8+sizeof(int)+ino*sizeof(inode);
    int block_index = inode_address/BLOCK_SIZE;
    int block_offset = inode_address % BLOCK_SIZE;
    Block block = device.read_block(block_index);
    memcpy(&res, (char*)&block+block_offset, sizeof(inode));
    res.ino = ino;
    return res;
}


std::vector<dentry> Driver::read_directory(std::string dir_path){
    inode dir = lookup(dir_path);
    return read_directory(dir);
}


void Driver::create_file(std::string file_path){
    std::string file_name = file_path;
    inode dir = get_descriptor(0);
    inode file = get_first_unused_descriptor();
    file.type = REGULAR_FILE;
    update_descriptor(file);
    add_link(dir, file, file_name);
    
}

int Driver::open(std::string file_path){
    inode file = lookup(file_path);
    int nfd = unique_file_descriptor;
    unique_file_descriptor++;
    open_files[nfd] = file.ino;
    return nfd;
}

void Driver::close(int nfd){
    open_files.erase(open_files.find(nfd));
}

void Driver::read(int nfd, int offset, int size, char* buff){
    std::map<int,int>::iterator value = open_files.find(nfd);
    int ino = value->second;
    inode file = get_descriptor(ino);
    read(file, offset, size, buff);
}

void Driver::write(int nfd, int offset, char* data, int size){
    std::map<int,int>::iterator value = open_files.find(nfd);
    int ino = value->second;
    inode file = get_descriptor(ino);
    write(file, offset, data, size);
}


void Driver::link(std::string file_path1, std::string file_path2){
    inode dir = get_descriptor(0); //dir by file_path2
    inode file = lookup(file_path1);
    std::string file_name = file_path2;
    add_link(dir, file, file_name);
}

void Driver::unlink(std::string file_path){
    inode dir = get_descriptor(0); //dir by file_path
    std::string file_name = file_path;
    std::vector<dentry> dentries = read_directory(dir);
    for(int dentry_index = 0; dentry_index < dentries.size(); dentry_index++){
        if(dentries[dentry_index].file_name == file_name){
            dentries.erase(dentries.begin()+dentry_index);
            break;
        }
    }
    truncate(dir, dir.file_size-sizeof(dentry));
    dentry dentry_arr[dentries.size()];
    std::copy(dentries.begin(), dentries.end(), dentry_arr);
    write(dir, 0, (char*)&dentry_arr, dentries.size()*sizeof(dentry));
}



void Driver::truncate(std::string file_path, uint new_size){
    inode file = lookup(file_path);
    truncate(file, new_size);
}

//private


inode Driver::lookup(std::string file_path){
    inode dir = get_descriptor(0);
    std::vector<dentry> dentries = read_directory(dir);
    for(int dentry_index = 0; dentry_index < dentries.size(); dentry_index++){
        if(dentries[dentry_index].file_name == file_path){
            return get_descriptor(dentries[dentry_index].ino);
        }
    }
    throw std::invalid_argument("file does not exist");
}


// test inneed 100% buff[size]
void Driver::read(inode inode, int offset, int size, char* buff){
    int start_block = offset/BLOCK_SIZE;
    int end_block = (offset+size)/BLOCK_SIZE;
    std::vector<int> block_addresses = get_block_addresses(inode, start_block, end_block+1);
    int written_bytes = 0;
    for(int block_address_index; block_address_index < block_addresses.size(); block_address_index++){
        int offset_start = block_address_index == start_block ? offset % BLOCK_SIZE : 0;
        int offset_end = block_address_index == end_block ? BLOCK_SIZE - (offset+size) % BLOCK_SIZE : 0;
        Block block = device.read_block(block_addresses[block_address_index]);
        memcpy(buff+written_bytes, block.data + offset_start, BLOCK_SIZE - offset_start - offset_end);
        written_bytes += BLOCK_SIZE-offset_start-offset_end;
    }
}


void Driver::write(inode inode, int offset, char* data, int size){
    int start_block = offset/BLOCK_SIZE;
    int end_block = (offset+size)/BLOCK_SIZE;
    std::vector<int> block_addresses = get_block_addresses(inode, start_block, end_block+1);
    int written_bytes = 0;
    int need_towrite_bytes = size;
    for(int block_address_index = 0; block_address_index < block_addresses.size(); block_address_index++){
        int offset_start = block_address_index == start_block ? offset % BLOCK_SIZE : 0;
        int offset_end = block_address_index == end_block ? BLOCK_SIZE - (offset+size) % BLOCK_SIZE : 0;
        int write_toblock = std::min(BLOCK_SIZE - offset_start - offset_end, need_towrite_bytes);
        Block block = device.read_block(block_addresses[block_address_index]);
        memcpy(block.data + offset_start, data + written_bytes, write_toblock);
        device.write_block(block_addresses[block_address_index], block);
        written_bytes += write_toblock;
        need_towrite_bytes -= write_toblock;
        if(need_towrite_bytes <= 0){
            break;
        }
    } 
}


std::vector<int> Driver::get_block_addresses(inode inode, int start_index, int end_index){
    std::vector<int> res;
    while(start_index < NLINK){
        if(start_index >= end_index){
            return res;
        }
        if(inode.blocks.nlinks[start_index] == NUN){
            return res;
        }
        res.push_back(inode.blocks.nlinks[start_index]);
        start_index++;
    }
    if(start_index >= end_index){
        return res;
    }
    Block single_indirect = device.read_block(inode.blocks.single_indirect);
    for(int address_index = 0; address_index < BLOCK_SIZE/sizeof(int); address_index++){
        int address;
        memcpy(&address, (int*)&single_indirect+address_index, sizeof(int));
        if(start_index >= end_index){
            return res;
        }
        if(inode.blocks.nlinks[start_index] == NUN){
            return res;
        }
        res.push_back(address);
    }
    return res;
}


void Driver::truncate(inode file,int new_size){
    if(file.file_size < new_size){
        int need_block_count = ceil((double)new_size/BLOCK_SIZE);
        int block_count = ceil((double)file.file_size/BLOCK_SIZE);
        std::vector<int> free_block_addresses;
        for(int i = 0; i < need_block_count- block_count; i++){
            free_block_addresses.push_back(get_unused_block());
        }
        for(int i = 0; i < need_block_count- block_count; i++){
            set_block_inuse(free_block_addresses[i]);
            file = add_block(file, free_block_addresses[i]);
        }
        file.file_size = new_size;
        update_descriptor(file);
    }
    else{
        int need_block_count = ceil((double)new_size/BLOCK_SIZE);
        int block_count = ceil((double)file.file_size/BLOCK_SIZE);
        for(int i = 0; i < block_count-need_block_count; i++){
            file = remove_last_block(file);
        }
        if(need_block_count > 0){
            int last_block_address = get_block_addresses(file, need_block_count - 1, need_block_count)[0];
            Block last_block = device.read_block(last_block_address);
            int need_tonull_bytes = BLOCK_SIZE - new_size % BLOCK_SIZE;
            int offset = new_size % BLOCK_SIZE;
            std::fill_n(last_block.data+offset,need_tonull_bytes, 0);
            device.write_block(last_block_address, last_block);
        }
        file.file_size = new_size;
        update_descriptor(file);
    }
}

inode Driver::add_block(inode inode, int block_address){
    int block_count = ceil((double)inode.file_size/BLOCK_SIZE);
    if(block_count==NLINK+BLOCK_SIZE/sizeof(int)){
        throw std::overflow_error("already max mem used by inode");
    }
    if(block_count<NLINK){
        inode.blocks.nlinks[block_count] = block_address;
        update_descriptor(inode);
    }
    else{
        if(inode.blocks.single_indirect == NUN){
            inode.blocks.single_indirect = get_unused_block();
            set_block_inuse(inode.blocks.single_indirect);
            update_descriptor(inode);
        }
        Block single_indirect_block = device.read_block(inode.blocks.single_indirect);
        memcpy(single_indirect_block.data + sizeof(int) * (block_count - NLINK - 1), &block_address, sizeof(int));
        device.write_block(inode.blocks.single_indirect, single_indirect_block);
    }
    return inode;
}

inode Driver::remove_last_block(inode inode){
    int block_count = ceil((double)inode.file_size/BLOCK_SIZE);
    if(block_count<=NLINK){
        inode.blocks.nlinks[block_count] = NUN;
        update_descriptor(inode);
    }
    else{
        Block single_indirect_block = device.read_block(inode.blocks.single_indirect);
        std::fill_n(single_indirect_block.data + sizeof(int) * (block_count - NLINK - 1), sizeof(int), 0);
        device.write_block(inode.blocks.single_indirect, single_indirect_block);
        if(block_count == NLINK + 1){
            set_block_unuse(inode.blocks.single_indirect);
            inode.blocks.single_indirect = NUN;
            update_descriptor(inode);
        }
    }
    return inode;
}


void Driver::set_block_inuse(int block_index){
    int bit_map_block_index = block_index / 8 / BLOCK_SIZE;
    int bit_map_byte_index = (block_index / 8) % BLOCK_SIZE;
    int bit_map_bit_index = block_index % 8;
    Block block = device.read_block(bit_map_block_index);
    block.data[bit_map_byte_index] |= (1<<bit_map_bit_index);
    device.write_block(bit_map_block_index, block);
}

void Driver::set_block_unuse(int block_index){
    int bit_map_block_index = block_index / 8 / BLOCK_SIZE;
    int bit_map_byte_index = (block_index / 8) % BLOCK_SIZE;
    int bit_map_bit_index = block_index % 8;
    Block block = device.read_block(bit_map_block_index);
    block.data[bit_map_byte_index] &= ~(1<<bit_map_bit_index);
    device.write_block(bit_map_block_index, block);
}


void Driver::set_max_descriptor_count(int n){
    int n_address = TOTAL_MEMORY/8;
    int block_index = n_address/BLOCK_SIZE;
    int block_offset = n_address % BLOCK_SIZE;
    Block block = device.read_block(block_index);
    memcpy((char*)&block+block_offset, &n, sizeof(int));
    device.write_block(block_index, block);
}

int Driver::get_max_descriptor_count(){
    int res;
    int n_address = TOTAL_MEMORY/8;
    int block_index = n_address/BLOCK_SIZE;
    int block_offset = n_address % BLOCK_SIZE;
    Block block = device.read_block(block_index);
    memcpy(&res, (char*)&block+block_offset, sizeof(int));
    return res;
}

void Driver::update_descriptor(inode descriptor){
    int inode_address = TOTAL_MEMORY/8+sizeof(int)+descriptor.ino*sizeof(inode);
    int block_index = inode_address/BLOCK_SIZE;
    int block_offset = inode_address % BLOCK_SIZE;
    Block block = device.read_block(block_index);
    memcpy((char*)&block+block_offset, &descriptor, sizeof(inode));
    device.write_block(block_index, block);
}


inode Driver::get_first_unused_descriptor(){
    int n = get_max_descriptor_count();
    for(int i = 0; i < n; i++){
        inode file = get_descriptor(i);
        if(file.type == UNUSED_FILE){
            return file;
        }
    }
    throw std::overflow_error("unused descriptor not found");
}

void Driver::add_link(inode dir, inode file, std::string file_name){
    if(file_name.length() > 28){
        throw std::overflow_error("max file_name size is greater than allowed");        
    }
    std::vector<dentry> dentries = read_directory(dir);
    for(int dentry_index = 0; dentry_index < dentries.size(); dentry_index++){
        if(dentries[dentry_index].file_name == file_name){
            throw std::overflow_error("file already exists");
        }
    }
    dentry dentry = {.ino = file.ino};
    strcpy(dentry.file_name, file_name.c_str());
    truncate(dir, dir.file_size+sizeof(dentry));
    dir = get_descriptor(dir.ino);
    write(dir, dir.file_size-sizeof(dentry), (char*)&dentry, sizeof(dentry));
    file = get_descriptor(file.ino);
    file.link_count++;
    update_descriptor(file);
}

std::vector<dentry> Driver::read_directory(inode dir){
    char read_res[dir.file_size];
    read(dir, 0, dir.file_size, read_res);
    std::vector<dentry> result;
    for(int dentry_index = 0; dentry_index < dir.file_size/sizeof(dentry); dentry_index++){
        dentry d;
        memcpy(&d, read_res+dentry_index*sizeof(dentry), sizeof(dentry));
        result.push_back(d);
    }
    return result;
}


int Driver::get_unused_block(){
    int block_count = ceil((double)BITMAP_BYTES/BLOCK_SIZE);
    Block block;
    for(int byte_index = 0; byte_index<BITMAP_BYTES; byte_index++){
        if(byte_index % BLOCK_SIZE == 0){
            block = device.read_block(byte_index/BLOCK_SIZE);
        }
        if(block.data[byte_index%BLOCK_SIZE] != 255){
            for(int bit_index = 0; bit_index < 8; bit_index++){
                if((block.data[byte_index%BLOCK_SIZE] & (1<<bit_index)) == 0){
                    return byte_index*8 + bit_index;
                }
            }
        }
    }
    throw std::overflow_error("no unused blocks left");
}
