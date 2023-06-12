/*
Created by Tristan de Lemos and Trenten Spicer
CSC 453 Program 4

*/

#include "libDisk.h"

/* The default size of the disk and file system block */
#define BLOCKSIZE 256

/* Your program should use a 10240 byte disk size giving you 40 blocks total. 
This is the default size. 
You must be able to support different possible values, or report an error if it exceeds the limits of your implementation. */
#define DEFAULT_DISK_SIZE 10240 

/* use this name for a default disk file name */
#define DEFAULT_DISK_NAME “tinyFSDisk” 	
typedef int fileDescriptor;

#define SUPERB 0
#define ROOT 1

#define BLOCK_ALLOC sizeof(char)*BLOCKSIZE

// dynamic resource table
uint8_t ** resource_table;


/* Makes an empty TinyFS file system of size nBytes on the file specified by ‘filename’. 
This function should use the emulated disk library to open the specified file, and upon success, format the file to be mountable. 
This includes initializing all data to 0x00, setting magic numbers, initializing and writing the superblock and other metadata, etc. 
Must return a specified success/error code. */
int tfs_mkfs(char *filename, int nBytes){
    int disk = 0;
    if (nBytes == 0){
        disk = openDisk(filename, DEFAULT_DISK_SIZE);
    }
    else{
        disk = openDisk(filename, nBytes);
    }
    
    if(disk < 0){
        printf("something is wrong with openDisk.\n");
        return -1;
    } 

    // malloc memory for dynamic resource table
    resource_table = malloc(DEFAULT_DISK_SIZE/BLOCKSIZE);
    int i;
    // malloc memory for each entry in table and set each entry to NULL
    for ( i = 0; i < DEFAULT_DISK_SIZE/BLOCKSIZE; i++){
        resource_table[i] = malloc(sizeof(uint8_t) * 3);
        resource_table[i][0] = '/0';
        resource_table[i][1] = '/0';
        resource_table[i][2] = '/0';
    }
    
    
    uint8_t zeros[256];
    for (i = 0; i <= 256; i++){
        zeros[i] = 0x00;
    }

    uint8_t superblock[256];
    for (i = 0; i <= 256; i++){
        superblock[i] = 0x00;
        if(i == 0){
            writeBlock(disk, i, 0x5A);
        }
    }
    // add special inode block for root
    
    for (i = 0; i < nBytes/BLOCKSIZE; i++){
        writeBlock(disk, i, zeros);
        if(i == 0){
            writeBlock(disk, i, superblock);
        }
    }

    closeDisk(disk);

    return 0;
    
}

/* tfs_mount(char *filename) “mounts” a TinyFS file system located within ‘filename’. 
tfs_unmount(void) “unmounts” the currently mounted file system. 
As part of the mount operation, tfs_mount should verify the file system is the correct type. 
Only one file system may be mounted at a time. Use tfs_unmount to cleanly unmount the currently mounted file system. 
Must return a specified success/error code. */

int mounted_disk = -1;

int tfs_mount(char *filename) {

    //set global fd for disk
    mounted_disk = openDisk(filename, 0);
    //buffer
    char* buf = malloc(BLOCK_ALLOC);
    //read superblock
    int rd = readBlock(mounted_disk, SUPERB, buf);
    //verify TinyFS compatible
    if (buf[0] == 0x5A) {
        //fill mounted byte
        buf[4] = 0xFF;
        //write to superblock
        int w = writeBlock(mounted_disk, SUPERB, buf);
        //free buf
        free(buf);
        //return
        return 0;
    }
    //error return
    return -1;


}
int tfs_unmount(void) {

    //buffer
    char* buf = malloc(BLOCK_ALLOC);
    //read superblock
    int rd = readBlock(mounted_disk, SUPERB, buf);
    //set mounted byte to 0
    buf[4] = 0x00;
    //write to superblock
    int w = writeBlock(mounted_disk, SUPERB, buf);
    //free buf
    free(buf);
    //close disk
    closeDisk(mounted_disk);


}

/* Opens a file for reading and writing on the currently mounted file system. 
Creates a dynamic resource table entry for the file (the structure that tracks open files, the internal file pointer, etc.), 
and returns a file descriptor (integer) that can be used to reference this file while the filesystem is mounted. */
fileDescriptor tfs_open(char *name){

    // check if tinyFS is mounted
    //buffer
    char* buf = malloc(BLOCK_ALLOC);

    if (readBlock(mounted_disk, SUPERB, buf) != 0) {
        // disk not open
        free(buf);
        return -1;
    }

    int existing = 0;
    //read root
    readBlock(mounted_disk, ROOT, buf);

    //look for inode in root
    int j = 0;
    while (existing == 0 && j < BLOCKSIZE) {
        if (buf[9] == 0) {
            break;
        }
        int inode_num = buf[j];
        buf += 1;
        if (strcmp(name, buf) == 0) {
            existing = 1;
            free(buf);
            return inode_num;
        }

        buf += 9;
        j += 10;
    }

    //not found

    //get next free block for inode block
    int inode_bNum = getFreeBlock();
    //make inode
    int i = 0;
    for (i=0; i<BLOCKSIZE; i++) {
        buf[i] = 0;
    }
    buf = makeInode(inode_bNum, buf);
    //fill block, get next free, updates superblock
    int over = overwriteFreeBlock(inode_bNum, buf);
    

    //update root using j
    int inode_rooted = addInodeToRoot(inode_bNum, name);


    // add dynamic resource table entry
    for (i = 0; i < DEFAULT_DISK_SIZE/BLOCKSIZE; i++){
        if(resource_table[i][0] == NULL){
            resource_table[i][0] = inode_bNum;
            resource_table[i][1] = buf[29];
            resource_table[i][2] = 0;
            break;
        }
    }

    free(buf);

    // return filedescriptor
    return inode_bNum;
}



int makeInode(int bNum, uint8_t* buf) {

    int i = 0;

    int unknown = 0;

    //inode identifier
    buf[0] = 0xCC;
    buf[1] = 0xCC;
    buf[2] = 0xCC;
    buf[3] = 0xCC;

    //inode number
    buf[4] = bNum;

    //mode
    buf[5] = unknown;
    buf[6] = unknown;
    buf[7] = unknown;
    buf[8] = unknown;

    //uid
    buf[9] = unknown;
    buf[10] = unknown;
    buf[11] = unknown;
    buf[12] = unknown;

    //gid
    buf[13] = unknown;
    buf[14] = unknown;
    buf[15] = unknown;
    buf[16] = unknown;

    //permissions
    //base 777
    buf[17] = 0x03;
    buf[18] = 0x09;

    //size
    buf[19] = 0;
    buf[20] = 0;

    //access time
    buf[21] = unknown;
    buf[22] = unknown;
    buf[23] = unknown;
    buf[24] = unknown;

    //modified time
    buf[25] = unknown;
    buf[26] = unknown;
    buf[27] = unknown;
    buf[28] = unknown;

    //data pointer
    buf[29] = unknown;

    //num_blocks
    buf[30] = unknown;

    //ending null
    buf[31] = '\0';

}

int addInodeToRoot(int bNum, char* name) {

    //buffer
    uint8_t* buf = malloc(BLOCKSIZE);
    //read root inode
    buf = readBlock(mounted_disk, ROOT, buf);
    //seek to end of existing inodes
    int i = 0;
    while (buf[i+9] == '\0') {
        i += 10;
    }
    //set inode number in root
    buf[i] = bNum;
    //fill root with first 8 of name
    i += 1;
    int j = 0;
    for (j=0; j<8; j++) {
        if (length(name) >= j) {
            buf[i+j] = name[j];
        } else {
            buf[i+j] = '\0';
        }
    }
    //append final null
    buf[i+8] = '\0';
    //rewrite root node
    int rd = writeBlock(mounted_disk, ROOT, buf);
    //free buf
    free(buf);
    //return
    return 0;

}


int overwriteFreeBlock(int bNum, void* data) {

    //buffer
    char* buf = malloc(BLOCK_ALLOC);
    //seek to free block
    int rd = readBlock(mounted_disk, bNum, buf);
    //take next free block info, write to superblock
    int next_free = buf[0];
    updateBlock(SUPERB, 2, next_free);
    //write data to block
    int w = writeBlock(mounted_disk, bNum, data);
    //free buffer
    free(buf);
    //return
    return 0;
    
}

int updateBlock(int bNum, int byte, char data) {
    
    //buffer
    char* buf = malloc(BLOCK_ALLOC);
    //read in block
    int rd = readBlock(mounted_disk, bNum, buf);
    //change byte in block
    buf[byte] = data;
    //rewrite block
    int w = writeBlock(mounted_disk, bNum, buf);
    //free buffer
    free(buf);
    //return
    return 0;

}

int getFreeBlock() {

    //buffer
    char* buf = malloc(BLOCK_ALLOC);
    //read in supernode
    int rd = readBlock(mounted_disk, SUPERB, buf);
    //read next free block
    int next_free = buf[2];
    //free buffer
    free(buf);
    //return
    return next_free;

}



/* Closes the file and removes dynamic resource table entry */
int tfs_close(fileDescriptor FD){
    // remove from dynamic resource table
    int i;
    for (i = 0; i < sizeof(resource_table); i++){
        uint8_t * entry = resource_table[i];
        // if the inode numbers are the same, set everything in table to NULL
        if(FD == entry[0]){
            entry[0] = '/0';
            entry[1] = '/0';
            entry[2] = '/0';
        }
    }

    return 0;
}



/* Writes buffer ‘buffer’ of size ‘size’, which represents an entire file’s contents, to the file described by ‘FD’.
 Sets the file pointer to 0 (the start of file) when done. Returns success/error codes. */
int tfs_write(fileDescriptor FD, char *buffer, int size){
    int i;
    uint8_t data_block_num; 
    uint8_t * offset;
    for (i = 0; i < sizeof(resource_table); i++){
        uint8_t * entry = resource_table[i];
        // if the inode numbers are the same, get data block num and byte offset
        if(FD == entry[0]){
            data_block_num = entry[1]; 
            offset = entry[2];
        }
   
    }

    // based on 

     writeBlock(mounted_disk, data_block_num, buffer);

}


/* deletes a file and marks its blocks as free on disk. */
int tfs_delete(fileDescriptor FD){

    // check if tinyFS is mounted

    // remove data blocks from tinyFS
    //check inode for associated blocks
    int data_blocks[40] = malloc(sizeof(int)*40);
    int num_connected[40] = malloc(sizeof(int)*40);
    int i = 0;
    for (i=0; i<40; i++) {
        data_blocks[i] = -1;
        num_connected[i] = -1;
    }
    uint8_t* buf = malloc(BLOCK_ALLOC);
    int rd = readBlock(mounted_disk, FD, buf);

    int j = 29;
    int k = 0;
    //read through list of fragmented data portions in inode
    while (buf[j] != 0) {
        data_blocks[k] = buf[j];
        num_connected[k] = buf[j+1];
        k += 1;
        j += 2;
    }

    i = 0;
    while(data_blocks[i] != -1) {
        
        //for every run of data, delete each block
        for (j=0; j<num_connected[i]; j++) {
            deleteBlock(data_blocks[i]+j);
        }
    i++;

    }

    // remove inode block from tinyFS
    deleteBlock(FD);

    // save inode number for removing from dynamic resource table
    return 0;
}

int deleteBlock(int bNum) {

    //make empty block
    uint8_t* buf = malloc(BLOCK_ALLOC);
    int i = 0;
    for (i=0; i<256; i++) {
        buf[i] = 0;
    }

    //overwrite block with empty block
    int w = writeBlock(mounted_disk, bNum, buf);

    //connect last free block to here
    uint8_t* superb = malloc(BLOCK_ALLOC);
    int rd = readBlock(mounted_disk, SUPERB, superb);
    int prev_last = superb[3];
    buf[0] = bNum;
    w = writeBlock(mounted_disk, prev_last, buf);

    //this is now last free block
    updateBlock(SUPERB, 3, bNum);

    return 0;

}


/* reads one byte from the file and copies it to ‘buffer’, using the current file pointer location and incrementing it by one upon success. 
If the file pointer is already at the end of the file then tfs_readByte() should return an error and not increment the file pointer. */
int tfs_readByte(fileDescriptor FD, char *buffer){
    int i;
    uint8_t data_block_num = 0; 
    uint8_t offset = 0;
    // search through resource table for same file descriptor numbers
    for (i = 0; i < sizeof(resource_table); i++){
        uint8_t * entry = resource_table[i];
        // if the inode numbers are the same, get data block num and byte offset
        if(FD == entry[0]){
            data_block_num = entry[1]; 
            offset = entry[2];
            // return error if past 256,  have to change this later
            if(offset > 256){
                // find what data block we should be at
                int next_data_block = (offset / 256) + 29;

                // read in inode block to find where that next data block should be
                uint8_t * block;
                readBlock(mounted_disk, entry[0], block);

                // if we are trying to read past the end of the file
                if(block[next_data_block] == 0){
                    printf("EOF error in tfs_readByte.\n");
                    return -1;
                }

                // else, set correct data block number
                data_block_num = block[next_data_block];
            }
        }
    }
    // if data block is still 0, then we did not find the file in the resource table
    if(data_block_num == 0){
        printf("File not found when reading byte.\n");
        return -1;
    }
    // read in specified block
    uint8_t * block;
    readBlock(mounted_disk, data_block_num, block);

    // copy specified byte to return buffer and increase offset
    strcpy(buffer, block[offset++]);
    
    return 0;
}

/* change the file pointer location to offset (absolute). Returns success/error codes.*/
int tfs_seek(fileDescriptor FD, int offset){
    int i;
    uint8_t data_block_num = 0; 
    uint8_t offset = 0;
    // search through resource table for same file descriptor numbers
    for (i = 0; i < sizeof(resource_table); i++){
        uint8_t * entry = resource_table[i];
        // if the inode numbers are the same, get set new offset
        if(FD == entry[0]){
            entry[2] = offset;
            return 0;
        }
    }
    printf("Error could not find file in resource table in tfs_seek.\n");
    return -1;
}

