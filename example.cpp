#include <thread>

#include "IPointerHook.h"

void main_thread()
{
    IPointerHook::SelfTest();
}

__attribute__((constructor))
void ctor() { std::thread(main_thread).detach(); }

__attribute__((destructor))
void dtor() { }
