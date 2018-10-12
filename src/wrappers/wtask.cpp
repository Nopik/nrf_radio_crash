
#include "wtask.h"

using namespace Wrappers;

Task::Status Task::IsInitialized()
{
    // Mutex for mTaskHandle is not needed when all tasks are Created in main() function, before calling vTaskStartScheduler()
    if( !mTaskHandle ) {
        return Status::UNINITIALIZED;
    }
    return Status::NO_ERROR;
}

Task::Status Task::Create(TaskFunction_t aTaskCodeFn, void *aTaskCodeCtx, const char *aName, uint16_t aStackDepth, UBaseType_t aPriority, bool aSuspended)
{
    Status status = Status::GENERAL_ERROR;

    if((IsInitialized() == Status::UNINITIALIZED) && (aTaskCodeFn != nullptr)) {
        if( xTaskCreate(StaticTaskFunctionAdapter, aName, aStackDepth, this, aPriority, &mTaskHandle) == pdPASS) {
            mTaskCodeCtx = aTaskCodeCtx;
            mTaskCodeFn = aTaskCodeFn;
            mSuspended = aSuspended;
            status = Status::NO_ERROR;
        }
    }

    return status;
}

void Task::InitDelayUntil()
{
    mLastWakeTime = xTaskGetTickCount();
}

void Task::DelayUntil(TickType_t aPeriodMs)
{
    vTaskDelayUntil(&mLastWakeTime, pdMS_TO_TICKS(aPeriodMs));
}

void Task::TaskFunctionAdapter()
{
    if( mSuspended ) {
        Suspend();
    }
    InitDelayUntil();
    mTaskCodeFn(mTaskCodeCtx);
    vTaskDelete(mTaskHandle);
}

void Task::Suspend()
{
    if( IsInitialized() != Status::UNINITIALIZED ) {
        vTaskSuspend(mTaskHandle);
    }
}

void Task::Resume()
{
    if( IsInitialized() != Status::UNINITIALIZED ) {
        InitDelayUntil();
        vTaskResume(mTaskHandle);
    }
}

void Task::Delete()
{
    if( IsInitialized() != Status::UNINITIALIZED ) {
        vTaskDelete(mTaskHandle);
    }
}

void Task::DelayCurrent(TickType_t aPeriodMs)
{
    vTaskDelay(pdMS_TO_TICKS(aPeriodMs));
}

void Task::SuspendCurrent()
{
    vTaskSuspend(nullptr);
}
