#include <stdio.h>
#include <stdbool.h>
#include "alloc.h"

void *ptr = NULL;
int whereWithinPtr;
void *ogPtr;
bool pehliBaar = false;

struct tableEntry
{
    char *tablePtr;
    int prevOffset;
};

struct tableEntry table[PAGESIZE / MINALLOC];

int init_alloc()
{
    ptr = mmap(0, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE + MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED)
    {
        return -1;
    }
    for (int i = 0; i < PAGESIZE / MINALLOC; i++)
    {
        table[i].tablePtr = NULL;
        table[i].prevOffset = 0;
    }
    whereWithinPtr = 0;
    ogPtr = ptr;
    return 0;
}

int cleanup()
{
    ptr = NULL;
    whereWithinPtr = 0;
    for (int i = 0; i < PAGESIZE / MINALLOC; i++)
    {
        table[i].tablePtr = NULL;
        table[i].prevOffset = 0;
    }
    return munmap(ogPtr, PAGESIZE);
}

char *alloc(int offset)
{
    if (offset % MINALLOC != 0)
    {
        return NULL;
    }
    init_alloc();

    int numEntries = offset / MINALLOC;

    for (int i = 0; i < PAGESIZE / MINALLOC - numEntries; i++)
    {
        bool thisOne = true;
        for (int j = 0; j < numEntries; j++)
        {
            if (table[i + j].tablePtr != NULL)
            {
                thisOne = false;
            }
        }
        if (thisOne)
        {
            for (int j = 0; j < numEntries; j++)
            {
                table[i + j].tablePtr = (char *)(ogPtr + 8 * (i + j));
                table[i + j].prevOffset = j;
            }
            // char *tmp = (char*)ptr;
            return ogPtr + 8 * i;
        }
    }

    return NULL;
}

void dealloc(char *thingToRemove)
{
    for (int i = 0; i < PAGESIZE / MINALLOC; i++)
    {
        if (table[i].tablePtr == thingToRemove)
        {
            table[i].tablePtr = NULL;
            table[i].prevOffset = 0;
            int j = 1;
            while (table[i + j].prevOffset != 0 && i + j < PAGESIZE / MINALLOC)
            {
                table[i + j].tablePtr = NULL;
                table[i + j].prevOffset = 0;
                j++;
            }
            return;
        }
    }
}