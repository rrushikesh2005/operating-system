#include <stdio.h>
#include <stdbool.h>
#include "ealloc.h"

struct tableEntry {
    char *tablePtr;
    int prevOffset;
};

bool pehliBaar = false;

struct pageAllocs {
    struct tableEntry table [PAGESIZE/MINALLOC];
    void *ptr;
    bool fullPageAsked;
};

struct pageAllocs pages[4];

void init_alloc() {
    for (int i=0; i<4; i++) {
        pages[i].ptr = NULL;
        pages[i].fullPageAsked = false;
        for (int j=0; j<PAGESIZE/MINALLOC; j++) {
            pages[i].table[j].tablePtr = NULL;
            pages[i].table[j].prevOffset = 0;
        }
    }
}

char *alloc(int offset) {

    if (offset%MINALLOC != 0) {
        return NULL;
    }
    if (!pehliBaar) {
        init_alloc();
        pehliBaar = true;
    }

    int numEntries = offset/MINALLOC;
    // printf("numEntries is %d and PAGESIZE/MINALLOC is %d\n",numEntries,PAGESIZE/MINALLOC);

    for (int i=0; i<4; i++) {

        if (numEntries == PAGESIZE/MINALLOC) {
            if (pages[i].ptr != NULL) {
                bool isFree = true;
                //check to see if the page is free
                for (int j=0; j<PAGESIZE/MINALLOC;j++) {
                    if (pages[i].table[j].tablePtr != NULL) {
                        isFree = false;
                    }
                }
                if (isFree) {
                    for (int j=0; j<PAGESIZE/MINALLOC; j++) {
                        pages[i].table[j].tablePtr = (char*)(pages[i].ptr + 8*j);
                        pages[i].table[j].prevOffset = j;
                    }
                    pages[i].fullPageAsked = true;
                    return pages[i].ptr;
                }
            }
            else {
                pages[i].ptr = mmap(0,PAGESIZE,PROT_READ | PROT_WRITE, MAP_PRIVATE + MAP_ANONYMOUS, -1, 0);
                for (int j=0; j<PAGESIZE/MINALLOC; j++) {
                    pages[i].table[j].tablePtr = (char*)(pages[i].ptr + 8*j);
                    pages[i].table[j].prevOffset = j;
                }
                pages[i].fullPageAsked = true;
                return pages[i].ptr;
            }
        }

        if (pages[i].ptr == NULL) {
            pages[i].ptr = mmap(0,PAGESIZE,PROT_READ | PROT_WRITE, MAP_PRIVATE + MAP_ANONYMOUS, -1, 0);
        }
        //printf("yaha pohoche and ptr is %p\n",pages[i].ptr);
        //exit(0);
        bool foundWithinThisPage = false;

        //int numEntries = offset/MINALLOC;
        for (int j=0; j<=PAGESIZE/MINALLOC - numEntries; j++) {
            bool thisOne = true;
            for (int k=0; k<numEntries; k++) {
                if (pages[i].table[j+k].tablePtr != NULL) {
                    thisOne = false;
                  //  printf("tf\n");

                }
            }
            if (thisOne) {
                // printf("hmm j = %d\n",j);
                foundWithinThisPage = true;
                for (int k=0; k<numEntries; k++) {
                    pages[i].table[j+k].tablePtr = (pages[i].ptr + 8*(j + k));
                    pages[i].table[j+k].prevOffset = k;
                }
                return (pages[i].ptr + 8*j); 
            }
        }
    }
    // printf("returning Null\n");
    return NULL;
}

void dealloc (char* thingToRemove) {
    //printf("dealloc called\n");
    for (int i=0; i<4; i++) {

        if (pages[i].fullPageAsked) {
            //printf("full page asked i is %d\n",i);
            if ((char*)pages[i].ptr == thingToRemove) {
                //printf("nikal raha hu i is %d\n",i);
                for (int j=0; j<PAGESIZE/MINALLOC; j++) {
                    pages[i].table[j].tablePtr = NULL;
                    pages[i].table[j].prevOffset = 0;
                }
                // pages[i].ptr = NULL;
                pages[i].fullPageAsked = false;
                return;
            }
            continue;
        }

        for (int j=0; j<PAGESIZE/MINALLOC; j++) {
            if (pages[i].table[j].tablePtr == thingToRemove) {
                //printf("deallocating something wait j is %d\n",j);
                pages[i].table[j].tablePtr = NULL;
                pages[i].table[j].prevOffset = 0;
                int k = 1;
                while (pages[i].table[j+k].prevOffset != 0 && j + k < PAGESIZE/MINALLOC) {
                    pages[i].table[j+k].tablePtr = NULL;
                    pages[i].table[j+k].prevOffset = 0;
                    k++;
                }
                return;
            }
        }
    }
}

void cleanup() {
    for (int i=0; i<4; i++) {
        munmap(pages[i].ptr,PAGESIZE);
        // pages[i].ptr = NULL;
        for (int j=0; j<PAGESIZE/MINALLOC; j++) {
            pages[i].table[j].tablePtr = NULL;
            pages[i].table[j].prevOffset = 0;
        }
    }
}