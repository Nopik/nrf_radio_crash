#ifndef NODE_THREAD_TASK_H
#define NODE_THREAD_TASK_H

#include "FreeRTOS.h"
#include "task.h"

#define DEFAULT_TASK_STACK_SIZE             ( 1024 / sizeof(StackType_t))           /**< FreeRTOS task stack size is determined in multiples of StackType_t. */
#define DEFAULT_TASK_PRIORITY               1

namespace Wrappers {

class Task {

public:

    enum class Status {
        NO_ERROR = 0,           ///< No error.
        GENERAL_ERROR,          ///< General error.
        UNINITIALIZED,          ///< Module not initialized.
    };

    Status IsInitialized();

    /**
     * Creates FreeRTOS task.
     *
     * @param aTaskCodeFn   Task's function.
     * @param aTaskCodeCtx  Task's context.
     * @param aName         Task's name.
     * @param aStackDepth   Task's stack size. Default value is DEFAULT_TASK_STACK_SIZE.
     * @param aPriority     Task's priority. Default value is DEFAULT_TASK_PRIORITY.
     * @param aSuspended    Whether created task is suspended (true) or starts running just after creation (false).
     * @return NO_ERROR on success.
     */
    Status Create(TaskFunction_t aTaskCodeFn, void *aTaskCodeCtx, const char *aName, uint16_t aStackDepth = DEFAULT_TASK_STACK_SIZE,
                  UBaseType_t aPriority = DEFAULT_TASK_PRIORITY, bool aSuspended = true);

    void Suspend();

    void Resume();

    void DelayUntil(TickType_t aPeriodMs);

    void Delete();

    static void SuspendCurrent();

    static void DelayCurrent(TickType_t aPeriodMs);

protected:

private:
    void InitDelayUntil();

    void TaskFunctionAdapter();

    static void StaticTaskFunctionAdapter(void *aContext)
    {
        static_cast<Task *>(aContext)->TaskFunctionAdapter();
    }

    /**
     * FreeRTOS task handle.
     */
    TaskHandle_t mTaskHandle = nullptr;
    void *mTaskCodeCtx = nullptr;
    TaskFunction_t mTaskCodeFn = nullptr;
    TickType_t mLastWakeTime = 0;
    bool mSuspended = true;
};

} // namespace Wrappers

#endif //NODE_THREAD_TASK_H
