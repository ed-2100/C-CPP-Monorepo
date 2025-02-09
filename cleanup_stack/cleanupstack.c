#include <stdlib.h>

#include "cleanupstack.h"

/// Initializes ECleanupStack.
bool CleanupStackInit(CleanupStack* cleanupStack, size_t capacity) {
  if (0 == capacity) {
    return false;
  }

  cleanupStack->ptr = malloc(capacity);

  if (NULL == cleanupStack->ptr) {
    return false;
  }

  cleanupStack->used = 0;
  cleanupStack->capacity = capacity;
  return true;
}

/// Automatically reserves extra space in the ECleanupStack, if necessary.
bool CleanupStackReserve(CleanupStack* cleanupStack, size_t size) {
  if (size > cleanupStack->capacity - cleanupStack->used) {
    size_t newCapacity = cleanupStack->capacity * 2;
    if (size > newCapacity) {
      newCapacity = cleanupStack->capacity + size;
    }

    void* newLocation = realloc(cleanupStack->ptr, newCapacity);
    if (NULL == newLocation) {
      return false;
    }

    cleanupStack->ptr = newLocation;
    cleanupStack->capacity = newCapacity;
  }

  return true;
}

/// Cleans up all memory referenced on the ECleanupStack.
void CleanupStackFlush(CleanupStack* cleanupStack) {
  void* pEnd = cleanupStack->ptr + cleanupStack->used;
  while (pEnd > cleanupStack->ptr) {
    enum CleanupTask sType =
        *(enum CleanupTask*)(pEnd -= sizeof(enum CleanupTask));

    switch (sType) {
      case CLEANUP_TASK_MALLOC: {
        void* ptr = *(void**)(pEnd -= sizeof(void*));
        free(ptr);
        break;
      }
        /*OTHER TASK HANDLING*/
    }
  }
}

/// Pushed a malloc-ed pointer to the ECleanupStack.
bool CleanupStackPushMalloc(CleanupStack* cleanupStack, void* ptr) {
  if (!CleanupStackReserve(cleanupStack, sizeof(CleanupTaskMalloc))) {
    return false;
  }

  CleanupTaskMalloc* newStruct =
      (CleanupTaskMalloc*)(cleanupStack->ptr + cleanupStack->used);

  newStruct->ptr = ptr;
  newStruct->sType = CLEANUP_TASK_MALLOC;

  cleanupStack->used += sizeof(CleanupTaskMalloc);

  return true;
}
