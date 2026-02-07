#include <cstdlib>
#include <cstring>
#include <thread>

#include "PointerHookManager.h"
#include "SafePointerHook.h"
#include "Logger.h"
#include "KittyMemory/KittyInclude.hpp"

static ElfScanner g_UE4;

class AimAssistor : public SafePointerHook
{
public:
    AimAssistor() : SafePointerHook() {}
    ~AimAssistor() override = default;

    std::string GetName() const override { return "AimAssistor"; }

    uintptr_t FakeFunction(RegContext *ctx) override
    {
        LOGI("[%s] s0: %f", GetName().c_str(), ctx->floating.s[0]);
        ctx->floating.s[0] = 1.25f;
        return GetOrigFuncAddr();
    }

protected:
    uintptr_t GetElfBaseImpl() const override { return g_UE4.base(); }
    uintptr_t GetPtrAddrImpl() const override { return GetElfBaseImpl() + 0x14053D30; }
    uintptr_t GetFuncAddrImpl() const override { return GetElfBaseImpl() + 0x8E97190; }
};

class MemcpyHook : public SafePointerHook
{
public:
    MemcpyHook() : SafePointerHook() {}
    ~MemcpyHook() override = default;

    std::string GetName() const override { return "MemcpyHook"; }

    uintptr_t FakeFunction(RegContext *ctx) override
    {
        LOGI("[%s] memcpy( dst: %p, src: %p, size: %zu )", GetName().c_str(),
            (void *)ctx->general.x[0], (void *)ctx->general.x[1], ctx->general.x[2]);
        return GetOrigFuncAddr();
    }

protected:
    uintptr_t GetElfBaseImpl() const override { return g_UE4.base(); }
    uintptr_t GetPtrAddrImpl() const override { return GetElfBaseImpl() + 0x1646E2E8; }
    uintptr_t GetFuncAddrImpl() const override { return 0x0; }
};

void main_thread()
{
    do { std::this_thread::sleep_for(std::chrono::milliseconds(1));
        g_UE4 = ElfScanner::createWithPath("libUE4.so");
    } while (!g_UE4.isValid());

    LOGI("[+] libUE4.so base: %p", g_UE4.base());

    PointerHookManager::GetInstance().Add<AimAssistor>();
    PointerHookManager::GetInstance().Add<MemcpyHook>();
}

__attribute__((constructor))
void ctor()
{
    LOGI("ctor");
    std::thread(main_thread).detach();
}

__attribute__((destructor))
void dtor()
{
    LOGI("dtor");
}
