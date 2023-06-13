
#include "libTinyFS.h"

int main(int argc, char * argv[]){


    tfs_mkfs("testing1", 0);

    tfs_mount("testing1");

    fileDescriptor file = tfs_open("testing2");

    tfs_close(file);

    printf("\ni did everything!!!!!!!!!!\n");

    return 0;

}