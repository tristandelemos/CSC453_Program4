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
    int i;
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


    mounted_disk = openDisk(filename, 0);

    char* buf = malloc(sizeof(char)*BLOCKSIZE);

    int rd = readBlock(mounted_disk, SUPERB, buf);

    if (buf[0] == 0x5A) {
        buf[3] = 0xFF;

        int w = writeBlock(mounted_disk, SUPERB, buf);

        return 0;
    }

    return -1;

    


}
int tfs_unmount(void) {

    char* buf = malloc(sizeof(char)*BLOCKSIZE);

    int rd = readBlock(mounted_disk, SUPERB, buf);

    buf[3] = 0x00;

    int w = writeBlock(mounted_disk, SUPERB, buf);

    closeDisk(mounted_disk);


}

/* Opens a file for reading and writing on the currently mounted file system. 
Creates a dynamic resource table entry for the file (the structure that tracks open files, the internal file pointer, etc.), 
and returns a file descriptor (integer) that can be used to reference this file while the filesystem is mounted. */
fileDescriptor tfs_open(char *name){

    // check if tinyFS is mounted

    char* buf = malloc(sizeof(char)*BLOCKSIZE);

    if (readBlock(mounted_disk, SUPERB, buf) != 0) {
        // disk not open
        return -1;
    }


    // open the file for reading and writing

    //find file inode from root inode

    int existing = 0;
    readBlock(mounted_disk, ROOT, buf);

    int j = 0;
    while (existing == 0 && j < BLOCKSIZE) {
        if (buf[9] == 0) {
            break;
        }
        int inode_num = buf[j];
        buf += 1;
        if (strcmp(name, buf) == 0) {
            existing = 1;
            return inode_num;
        }

        buf += 9;
        j += 10;
    }

    //not found

    //make inode

    //update root using j




    fileDescriptor file = open(name, O_CREAT |  O_RDWR);
    if(file < 0){
        return  -2;
    }

    // add inode block to tinyFS
    uint8_t inode[256];

    // add data blocks to tinyFS if any are 
    uint8_t data [256];
    

    // return filedescriptor
    return file;
}



/* Closes the file and removes dynamic resource table entry */
int tfs_close(fileDescriptor FD){
    // check if tinyFS is mounted

    // close the file

    // remove data blocks from tinyFS

    // remove inode block from tinyFS

    return 0;
}



/* Writes buffer ‘buffer’ of size ‘size’, which represents an entire file’s contents, to the file described by ‘FD’.
 Sets the file pointer to 0 (the start of file) when done. Returns success/error codes. */
int tfs_write(fileDescriptor FD, char *buffer, int size);

/* deletes a file and marks its blocks as free on disk. */
int tfs_delete(fileDescriptor FD);

/* reads one byte from the file and copies it to ‘buffer’, using the current file pointer location and incrementing it by one upon success. 
If the file pointer is already at the end of the file then tfs_readByte() should return an error and not increment the file pointer. */
int tfs_readByte(fileDescriptor FD, char *buffer);

/* change the file pointer location to offset (absolute). Returns success/error codes.*/
int tfs_seek(fileDescriptor FD, int offset);

