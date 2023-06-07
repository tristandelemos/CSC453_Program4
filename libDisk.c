/*
Created by Tristan de Lemos and Trenten Spicer
CSC 453 Program 4

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

int BLOCKSIZE = 256;

/* This function opens a regular UNIX file and designates the first nBytes of it as space for the emulated disk. 
nBytes should be a number that is evenly divisible by the block size. 
If nBytes > 0 and there is already a file by the given filename, that disk is resized to nBytes, 
and that file’s contents may be overwritten. If nBytes is 0, an existing disk is opened, 
and should not be overwritten. There is no requirement to maintain integrity of any content beyond nBytes. 
Errors must be returned for any other failures, as defined by your own error code system.  */
int openDisk(char *filename, int nBytes){
    int FILE;
    // check if nBytes is present
    if(nBytes == 0){
        FILE = fopen(filename, "r");
        // check if fopen was successful
        if(FILE < 0){
            // return error code -1, error on read only
            return -1;
        } 
    }
    // check if nBytes is evenly divisible
    else if(BLOCKSIZE%nBytes != 0){
        // return error code -2, error on block size
        return -2;
    }

    // check if file exists already
    if(access(filename, F_OK) != 0){
        // if file does not exist, create it
        FILE = fopen(filename, "w+");
        if(FILE < 0){
            // return error code -3, error on creating new file
            return -3;
        } 
    }
    // if file does exist
    else{
        FILE = fopen(filename, "r+");
    }

    return FILE;
}

/* readBlock() reads an entire block of BLOCKSIZE bytes from the open disk (identified by ‘disk’)
 and copies the result into a local buffer (must be at least of BLOCKSIZE bytes). 
 The bNum is a logical block number, which must be translated into a byte offset within the disk. 
 The translation from logical to physical block is straightforward: bNum=0 is the very first byte of the file. 
 bNum=1 is BLOCKSIZE bytes into the disk, bNum=n is n*BLOCKSIZE bytes into the disk. On success, it returns 0. 
 Errors must be returned if ‘disk’ is not available (i.e. hasn’t been opened) or for any other failures, as defined by your own error code system. */
int readBlock(int disk, int bNum, void *block) {

    //block consists of bytes
    char* buf = block;
    //set offset
    off_t offset = bNum * BLOCKSIZE;
    //seek to block offset
    off_t seek_val = lseek(disk, offset, SEEK_SET);
    //error check
    if (seek_val == -1) {
        printf("Error with read lseek\n");
        return -1;
    }
    //read from disk
    ssize_t read_size = read(disk, buf, BLOCKSIZE);
    //error check
    if (read_size == -1) {
        printf("Error with read\n");
        return -1;
    }
    //return finished
    return 0;



}

/* writeBlock() takes disk number ‘disk’ and logical block number ‘bNum’ and writes the content of the buffer ‘block’ to that location. 
BLOCKSIZE bytes will be written from ‘block’ regardless of its actual size. The disk must be open. 
Just as in readBlock(), writeBlock() must translate the logical block bNum to the correct byte position in the file. 
On success, it returns 0. Errors must be returned if ‘disk’ is not available (i.e. hasn’t been opened) or for any other failures, 
as defined by your own error code system. */
int writeBlock(int disk, int bNum, void *block) {

    //block consists of bytes
    char* buf = block;
    //set offset
    off_t offset = bNum * BLOCKSIZE;
    //seek to block offset
    off_t seek_val = lseek(disk, offset, SEEK_SET);
    //error check
    if (seek_val == -1) {
        printf("Error with write lseek\n");
        return -1;
    }
    //write to disk
    ssize_t write_size = write(disk, buf, BLOCKSIZE);
    //error check
    if (write_size == -1) {
        printf("Error with write\n");
        return -1;
    }
    //return finished
    return 0;

}

/* closeDisk() takes a disk number ‘disk’ and makes the disk closed to further I/O;
 i.e. any subsequent reads or writes to a closed disk should return an error. 
Closing a disk should also close the underlying file, committing any writes being buffered by the real OS. */
void closeDisk(int disk){
    close(disk);
}
