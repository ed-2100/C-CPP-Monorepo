#pragma once

#include <stddef.h>

/// The `CleanupStack` structure.
typedef struct {
    void* ptr;
    size_t used;
    size_t capacity;
} CleanupStack;

/// An enumeration of all the cleanup tasks.
enum CleanupTask {
    CLEANUP_TASK_MALLOC,
    /*OTHER TASKS*/
};

/// Represents a `malloc` task.
#pragma pack(1)

typedef struct {
    void* ptr;
    enum CleanupTask sType;
} CleanupTaskMalloc;

/*OTHER TASK STRUCTS*/

/// Initializes `CleanupStack`.
bool CleanupStackInit(CleanupStack* cleanupStack, size_t capacity);

/// Automatically reserves extra space in the `CleanupStack`, if necessary.
bool CleanupStackReserve(CleanupStack* cleanupStack, size_t size);

/// Cleans up all memory referenced on the `CleanupStack`.
void CleanupStackFlush(CleanupStack* cleanupStack);

/// Pushes a `malloc` pointer to the `CleanupStack`.
bool CleanupStackPushMalloc(CleanupStack* cleanupStack, void* size);

/*OTHER PUSH FUNCTIONS*/
