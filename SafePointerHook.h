#pragma once

#include <sys/mman.h>
#include <sys/prctl.h>

#include "IPointerHook.h"

class SafePointerHook : virtual public IPointerHook {
public:
    SafePointerHook() : IPointerHook() {}
    virtual ~SafePointerHook() override {
        IPointerHook::~IPointerHook();
    }

    void InstallHook() override {
        IPointerHook::InstallHook();
        // TODO: 添加过检测逻辑
    }

    void DestroyHook() override {
        IPointerHook::DestroyHook();
    }

protected:
    void *x_mmap(size_t size) override {
        return IPointerHook::x_mmap(size);
    }

    bool x_munmap(void *addr, size_t size) override {
        return IPointerHook::x_munmap(addr, size);
    }
};
