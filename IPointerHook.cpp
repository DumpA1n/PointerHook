#include "IPointerHook.h"

#include <sys/mman.h>

#include "xHook/xh_util.h"
#include "Logger.h"

std::shared_mutex IPointerHook::g_Mutex_;
std::unordered_map<uint32_t, IPointerHook*> IPointerHook::g_Hacks_;

IPointerHook::IPointerHook()
    : initialized_(false),
      index_(-1),
      orig_ptr_addr_(0),
      orig_func_addr_(0),
      fake_func_addr_(0)
{
}

IPointerHook::~IPointerHook()
{
    DestroyHook();
}

void IPointerHook::Initialize()
{
    orig_ptr_addr_ = GetPtrAddrImpl();

    if (GetFuncAddrImpl()) {
        orig_func_addr_ = GetFuncAddrImpl();
    } else {
        uint32_t prot = 0;
        xh_util_get_addr_protect(orig_ptr_addr_, NULL, &prot);
        if (prot & PROT_READ) {
            if (uintptr_t temp = *(uintptr_t*)orig_ptr_addr_; temp != 0) {
                orig_func_addr_ = temp;
            } else {
                LOGE("[%s] Failed to initialize: orig_func_addr_ is null", GetName().c_str());
                return;
            }
        } else {
            LOGE("[%s] Failed to initialize: cannot read orig_ptr_addr_", GetName().c_str());
            return;
        }
    }

    initialized_ = true;

    LOGI("[%s] Initialized", GetName().c_str());
}

void IPointerHook::InstallHook()
{
    if (!initialized_) {
        return;
    }

    void *mapped_address = x_mmap(4096);
    LOGI("[%s] mmap: %p", GetName().c_str(), mapped_address);

    {
        std::unique_lock lock(g_Mutex_);
        index_ = g_Hacks_.size();
        g_Hacks_[index_] = this;
    }

    uintptr_t addr = (uintptr_t)mapped_address;

    size_t get_size = (size_t)((uintptr_t)get_regs_end - (uintptr_t)get_regs);
    memcpy((void*)addr, (const void*)&get_regs, get_size);

    uintptr_t d_addr = (uintptr_t)&IPointerHook::Dispatcher;
    uint32_t shellcode[] = {
        // mov x1, #index
        (uint32_t)(0xD2800001 | ((index_ & 0xFFFF) << 5)),

        // mov x16, #(addr & 0xFFFF)
        (uint32_t)(0xD2800010 | (((d_addr >> 0)  & 0xFFFF) << 5)),

        // movk x16, #((addr >> 16) & 0xFFFF, lsl #16
        (uint32_t)(0xF2A00010 | (((d_addr >> 16) & 0xFFFF) << 5)),

        // movk x16, #((addr >> 32) & 0xFFFF, lsl #32
        (uint32_t)(0xF2C00010 | (((d_addr >> 32) & 0xFFFF) << 5)),

        // movk x16, #((addr >> 48) & 0xFFFF, lsl #48
        (uint32_t)(0xF2E00010 | (((d_addr >> 48) & 0xFFFF) << 5)),

        // blr x16
        (uint32_t)0xD63F0200,
    };
    memcpy((void*)(addr + get_size), (const void*)shellcode, sizeof(shellcode));

    size_t set_size = (size_t)((uintptr_t)set_regs_end - (uintptr_t)set_regs);
    memcpy((void*)(addr + get_size + sizeof(shellcode)), (const void*)&set_regs, set_size);

    fake_func_addr_ = addr;

    x_write((void*)orig_ptr_addr_, &fake_func_addr_, sizeof(uintptr_t));

    LOGI("[%s] InstallHook: orig_ptr_addr_: %p, orig_func_addr_: %p, fake_func_addr_: %p",
        GetName().c_str(),
        (void*)orig_ptr_addr_,
        (void*)orig_func_addr_,
        (void*)fake_func_addr_);
}

void IPointerHook::DestroyHook()
{
    if (initialized_) {
        x_write((void*)orig_ptr_addr_, &orig_func_addr_, sizeof(uintptr_t));
        x_munmap((void*)fake_func_addr_, 4096);
        {
            std::unique_lock lock(g_Mutex_);
            g_Hacks_.erase(index_);
        }
        index_ = -1;
        orig_ptr_addr_ = 0;
        orig_func_addr_ = 0;
        fake_func_addr_ = 0;
        initialized_ = false;

        LOGI("[%s] DestroyHook", GetName().c_str());
    }
}

void *IPointerHook::x_mmap(size_t size)
{
    void *addr = mmap(nullptr, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) {
        LOGE("[!] mmap failed: %s", strerror(errno));
        MAKE_CRASH();
    }
    return addr;
}

bool IPointerHook::x_munmap(void *addr, size_t size)
{
    if (munmap(addr, size) == -1) {
        LOGE("[!] munmap failed: %s", strerror(errno));
        MAKE_CRASH();
    }
    return true;
}

void IPointerHook::x_write(void *dst, const void *src, size_t size)
{
    uint32_t prot = 0;
    xh_util_get_addr_protect((uintptr_t)dst, NULL, &prot);
    if (prot & PROT_WRITE) {
        memcpy(dst, src, size);
    } else {
        xh_util_set_addr_protect((uintptr_t)dst, prot | PROT_WRITE);
        memcpy(dst, src, size);
        xh_util_set_addr_protect((uintptr_t)dst, prot);
    }
}

uintptr_t IPointerHook::Dispatcher(RegContext *ctx, uint32_t index)
{
    uintptr_t return_address = 0;
    {
        std::shared_lock lock(g_Mutex_);
        if (auto it = g_Hacks_.find(index); it != g_Hacks_.end()) {
            return_address = it->second->FakeFunction(ctx);
        }
    }
    return return_address;
}
