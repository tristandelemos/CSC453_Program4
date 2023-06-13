#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

int BLOCKSIZE = 256;

int openDisk(char *filename, int nBytes);

int readBlock(int disk, int bNum, void *block);

int writeBlock(int disk, int bNum, void *block);

void closeDisk(int disk);