#ifndef NODE_THREAD_CMUTEX_H
#define NODE_THREAD_CMUTEX_H

#include "FreeRTOS.h"
#include "semphr.h"

namespace Wrappers {

class MutexBase {

public:

    bool Take(TickType_t aTimeout = portMAX_DELAY);

    bool TakeFromISR(BaseType_t *xHigherPriorityTaskWoken = nullptr);

    bool Give();

    bool GiveFromISR(BaseType_t *xHigherPriorityTaskWoken = nullptr);

protected:
    SemaphoreHandle_t mMutexHandle = nullptr;
};

class Mutex : public MutexBase {

public:

    Mutex();

};

class RecursiveMutex {

public:

    RecursiveMutex();

    bool Take(TickType_t aTimeout = portMAX_DELAY);

    bool Give();

protected:
    SemaphoreHandle_t mMutexHandle = nullptr;
};

class Semaphore : public MutexBase {

public:

    Semaphore();

};

class LockGuard {

public:
    explicit LockGuard(Mutex& aMutex);
    ~LockGuard();

private:
    /**
     *  Remove copy constructor.
     */
    LockGuard(const LockGuard&);

    Mutex& mMutex;
};

class LockGuardRecursive {

public:
    explicit LockGuardRecursive(RecursiveMutex& aMutex);
    ~LockGuardRecursive();

private:
    /**
     *  Remove copy constructor.
     */
    LockGuardRecursive(const LockGuardRecursive&);

    RecursiveMutex& mMutex;
};

class LockGuardFromISR {

public:
    explicit LockGuardFromISR(Mutex& aMutex);
    ~LockGuardFromISR();

private:
    /**
     *  Remove copy constructor.
     */
    LockGuardFromISR(const LockGuardFromISR&);

    Mutex& mMutex;
};

} // namespace Wrappers

#endif //NODE_THREAD_CMUTEX_H
