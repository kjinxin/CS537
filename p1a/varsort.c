/*************************************************************************
    > File Name: varsort.c
    > Author: Jin Xin
    > Mail: kjinxin@outlook.com
    > Created Time: Thu 22 Sep 2016 12:53:25 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "sort.h"


void usage() {
    fprintf(stderr, "Usage: varsort -i inputfile -o outputfile\n");
    exit(1);
}

int cmp(const void *a, const void *b) {
    return (((rec_data_t *) a)->key > ((rec_data_t *) b)->key)? 1: -1;
}
int main(int argc, char *argv[]) {
    // arguments
    char *inFile = "/no/such/file";
    char *outFile = "/no/such/file";
    int c;

    if (argc < 5) usage();
    if (strcmp(argv[1], "-i") != 0) usage();
    if (strcmp(argv[3], "-o") != 0) usage();
    opterr = 0;
    while ((c = getopt(argc, argv, "i:o:")) != -1) {
        switch (c) {
             case 'i':
                 inFile = strdup(optarg);
                 break;
             case 'o':
                 outFile = strdup(optarg);
                 break;
             default:
                 usage();
        }
    }
    
    // open input file
    int fd = open(inFile, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Error: Cannot open file %s\n", inFile);
        exit(1);
    }

    // output the number of keys as a header for this file
    int recordsLeft;
    int rc;

    rc = read(fd, &recordsLeft, sizeof(recordsLeft));
    if (rc != sizeof(recordsLeft)) {
        perror("read");
        exit(1);
    }

    rec_nodata_t r;
    rec_dataptr_t* datas = malloc(sizeof(rec_dataptr_t) *
                   recordsLeft);
    int i;
    unsigned int* pos;
    for (i = 0; i < recordsLeft; i ++) {
        // Read the fixed-sized portion of record: key and size of data
        rc = read(fd, &r, sizeof(rec_nodata_t));
        if (rc != sizeof(rec_nodata_t)) {
            perror("read");
            free(datas);
            free(inFile);
            free(outFile);
            (void) close(fd);
            exit(1);
        }
        assert(r.data_ints <= MAX_DATA_INTS);
        datas[i].key = r.key;
        datas[i].data_ints = r.data_ints;
        // Read the variable portion of the record
        pos = malloc(r.data_ints * sizeof(unsigned int));
        rc = read(fd, pos, r.data_ints * sizeof(unsigned int));
        if (rc !=  r.data_ints * sizeof(unsigned int)) {
            perror("read");
            free(datas);
            free(inFile);
            free(outFile);
            (void) close(fd);
            exit(1);
        }
        datas[i].data_ptr = pos;
    }
    qsort(datas, recordsLeft, sizeof(rec_dataptr_t), cmp);
    (void) close(fd);

    // write the sorted data to the file
    fd = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
    if (fd < 0) {
        fprintf(stderr, "Error: Cannot open file %s\n", outFile);
        free(datas);
        free(inFile);
        free(outFile);
        (void) close(fd);
        exit(1);
    }

    // output the number of keys as a header for this file
    rc = write(fd, &recordsLeft, sizeof(recordsLeft));
    if (rc != sizeof(recordsLeft)) {
        perror("write");
        free(datas);
        free(inFile);
        free(outFile);
        (void) close(fd);
        exit(1);
    }

    for (i = 0; i < recordsLeft; i++) {
        int data_size = 2*sizeof(unsigned int) +
            datas[i].data_ints*sizeof(unsigned int);
        rc = write(fd, &datas[i].key, sizeof(unsigned int));
        rc += write(fd, &datas[i].data_ints, sizeof(unsigned int));
        rc += write(fd, datas[i].data_ptr,
              datas[i].data_ints * sizeof(unsigned int));
        if (rc != data_size) {
            perror("write");
            free(datas);
            free(inFile);
            free(outFile);
            (void) close(fd);
            exit(1);
        }
        free(datas[i].data_ptr);
    }
    free(datas);
    free(inFile);
    free(outFile);
    (void) close(fd);
    return 0;
}

