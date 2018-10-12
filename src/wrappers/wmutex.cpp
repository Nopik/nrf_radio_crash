#include "wmutex.h"

using namespace Wrappers;

bool MutexBase::Take(TickType_t aTimeout)
{
    return xSemaphoreTake(mMutexHandle, aTimeout) == pdTRUE;
}

bool MutexBase::TakeFromISR(BaseType_t *xHigherPriorityTaskWoken)
{
    return xSemaphoreTakeFromISR(mMutexHandle, xHigherPriorityTaskWoken) == pdTRUE;
}

bool MutexBase::Give()
{
    return xSemaphoreGive(mMutexHandle) == pdTRUE;
}

bool MutexBase::GiveFromISR(BaseType_t *xHigherPriorityTaskWoken)
{
    return xSemaphoreGiveFromISR(mMutexHandle, xHigherPriorityTaskWoken) == pdTRUE;
}

Mutex::Mutex() : MutexBase()
{
    mMutexHandle = xSemaphoreCreateMutex();
}

Semaphore::Semaphore() : MutexBase()
{
    mMutexHandle = xSemaphoreCreateBinary();
}

LockGuard::LockGuard(Mutex &aMutex)
        : mMutex(aMutex)
{
    mMutex.Take();
}

LockGuard::~LockGuard()
{
    mMutex.Give();
}

LockGuardFromISR::LockGuardFromISR(Mutex &aMutex)
        : mMutex(aMutex)
{
    mMutex.TakeFromISR();
}

LockGuardFromISR::~LockGuardFromISR()
{
    mMutex.GiveFromISR();
}

RecursiveMutex::RecursiveMutex()
{
    mMutexHandle = xSemaphoreCreateRecursiveMutex();
}

bool RecursiveMutex::Take(TickType_t aTimeout)
{
    return xSemaphoreTakeRecursive(mMutexHandle, aTimeout) == pdTRUE;
}

bool RecursiveMutex::Give()
{
    return xSemaphoreGiveRecursive(mMutexHandle) == pdTRUE;
}

LockGuardRecursive::LockGuardRecursive(RecursiveMutex &aMutex)
        : mMutex(aMutex)
{
    mMutex.Take();
}

LockGuardRecursive::~LockGuardRecursive()
{
    mMutex.Give();
}
