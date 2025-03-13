/*
 * This code is problematic, because it does not account for struct alignment.
 * The CleanupTask structs have to be packed with an alignment of 1 byte,
 * because of this. I'm also not sure prefetching works backwards. It would be
 * better for the stack to extend down, but realloc doesn't support extension to
 * the left, so you would have to do a memcpy() each time the stack needed to be
 * resized. In hindsight, it might be better to use a linked list architecture
 * or something else.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "cleanupstack.h"

// https://github.com/microsoft/vscode-cpptools/issues/10696
#ifdef __INTELLISENSE__
    #define bool _Bool
    #define true 1
    #define false 0
#endif

int main(int /*argc*/, const char** /*argv*/) {
    // NOTE: All of the flushing is for if the program segfaults.

    printf("Initializing CleanupStack...");
    fflush(stdout);

    CleanupStack cleanupStack;
    if (!CleanupStackInit(&cleanupStack, 1)) {
        printf("Failed to initialize CleanupStack!\n");
        fflush(stdout);
        return 1;
    }

    printf(" Done.\nAllocating int...");
    fflush(stdout);

    int* ohNoA = malloc(sizeof(int));
    if (!CleanupStackPushMalloc(&cleanupStack, ohNoA)) {
        printf("Failed to add a resource to the CleanupStack!\n");
        fflush(stdout);
        free(ohNoA);
        return 1;
    }

    printf(" Done.\nFlushing CleanupStack...");
    fflush(stdout);

    CleanupStackFlush(&cleanupStack);

    printf(" Done.\nExiting...\n");
    fflush(stdout);
}
