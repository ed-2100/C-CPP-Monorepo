#pragma once

#include <stddef.h>

typedef struct {
  void* ptr;
  size_t used;
  size_t capacity;
} CleanupStack;

enum CleanupTask {
  CLEANUP_TASK_MALLOC,
  /*OTHER TASKS*/
};

#pragma pack(1)
typedef struct {
  void* ptr;
  enum CleanupTask sType;
} CleanupTaskMalloc;

/*OTHER TASK STRUCTS*/

bool CleanupStackInit(CleanupStack* cleanupStack, size_t capacity);
bool CleanupStackReserve(CleanupStack* cleanupStack, size_t size);
void CleanupStackFlush(CleanupStack* cleanupStack);
bool CleanupStackPushMalloc(CleanupStack* cleanupStack, void* size);
/*OTHER PUSH FUNCTIONS*/
