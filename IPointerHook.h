#pragma once

#include <cstddef>
#include <cstdint>
#include <shared_mutex>
#include <unordered_map>

#define MAKE_CRASH()     \
    __asm__ volatile (   \
        "mov x0, xzr;"   \
        "mov x29, x0;"   \
        "mov sp, x0;"    \
        "br x0;"         \
        : : :            \
    );

#ifdef __cplusplus
extern "C" {
#endif

void get_regs();
void get_regs_end();
void set_regs();
void set_regs_end();

#ifdef __cplusplus
}
#endif

struct RegContext {
    union FPReg {
        __int128_t q;
        struct {
            double d1;
            double d2;
        } d;
        struct {
            float f1;
            float f2;
            float f3;
            float f4;
        } f;
    };

    union {
        float s[32];
        FPReg q[32];
        struct {
        FPReg q0, q1, q2, q3, q4, q5, q6, q7;
        // [!!! READ ME !!!]
        // for Arm64, can't access q8 - q31, unless you enable full floating-point register pack
        FPReg q8, q9, q10, q11, q12, q13, q14, q15, q16, q17, q18, q19, q20, q21, q22, q23, q24, q25, q26, q27, q28, q29,
            q30, q31;
        } regs;
    } floating;

    union {
        uint64_t x[29];
        struct {
        uint64_t x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20, x21, x22,
            x23, x24, x25, x26, x27, x28;
        } regs;
    } general;

    uint64_t fp;
    uint64_t lr;
    uint64_t sp;

    uint64_t nzcv;

    uint64_t _pad;
};
static_assert(sizeof(RegContext) == 0x310, "Wrong size on RegContext");
static_assert(offsetof(RegContext, floating) == 0x0, "Wrong offset on floating");
static_assert(offsetof(RegContext, general) == 0x200, "Wrong offset on general");
static_assert(offsetof(RegContext, fp) == 0x2E8, "Wrong offset on fp");
static_assert(offsetof(RegContext, lr) == 0x2F0, "Wrong offset on lr");
static_assert(offsetof(RegContext, sp) == 0x2F8, "Wrong offset on sp");
static_assert(offsetof(RegContext, nzcv) == 0x300, "Wrong offset on nzcv");

class IPointerHook {
public:
    IPointerHook();
    virtual ~IPointerHook();

    virtual std::string GetName() const = 0;

    virtual uintptr_t FakeFunction(RegContext *ctx) = 0;

    virtual void Initialize();

    virtual void InstallHook();

    virtual void DestroyHook();

    uintptr_t GetElfBase() const { return GetElfBaseImpl(); }

    uintptr_t GetOrigPtrAddr() const { return GetPtrAddrImpl(); }

    uintptr_t GetOrigFuncAddr() const { return GetFuncAddrImpl(); }

    uintptr_t GetFakeFuncAddr() const { return fake_func_addr_; }

protected:
    virtual uintptr_t GetElfBaseImpl() const = 0;
    virtual uintptr_t GetPtrAddrImpl() const = 0;
    virtual uintptr_t GetFuncAddrImpl() const = 0;

    virtual void *x_mmap(size_t size);
    virtual bool x_munmap(void *addr, size_t size);
    virtual void x_write(void *dst, const void *src, size_t size);

private:
    static uintptr_t Dispatcher(RegContext *ctx, uint32_t index);

private:
    bool initialized_;

    int index_;

    uint64_t orig_ptr_addr_;
    uint64_t orig_func_addr_;
    uint64_t fake_func_addr_;

private:
    static std::shared_mutex g_Mutex_;
    static std::unordered_map<uint32_t, IPointerHook*> g_Hacks_;
};
