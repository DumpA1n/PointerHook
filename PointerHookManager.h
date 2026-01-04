#pragma once

#include <typeindex>
#include <shared_mutex>
#include <unordered_map>

#include "IPointerHook.h"
#include "Logger.h"

class PointerHookManager {
public:
    PointerHookManager(const PointerHookManager&) = delete;
    PointerHookManager& operator=(const PointerHookManager&) = delete;

    static PointerHookManager& GetInstance() {
        static PointerHookManager gInstance;
        return gInstance;
    }

    template<class T, class... Args, std::enable_if_t<std::is_base_of_v<IPointerHook, T>, int> = 0>
    void Add(Args&&... args) {
        std::type_index idx(typeid(T));

        auto [it, inserted] = m_hookMap.try_emplace(idx, nullptr);
        if (!inserted) {
            LOGI("[PointerHookManager] Hook %s already exists", it->second->GetName().c_str());
            return;
        }

        auto hack = std::make_unique<T>(std::forward<Args>(args)...);
        hack->Initialize();
        hack->InstallHook();

        LOGI("[PointerHookManager] Add: %s", hack->GetName().c_str());
        it->second = std::move(hack);
    }

    template<class T, std::enable_if_t<std::is_base_of_v<IPointerHook, T>, int> = 0>
    void Remove() {
        std::type_index idx(typeid(T));
        if (auto it = m_hookMap.find(idx); it != m_hookMap.end()) {
            LOGI("[PointerHookManager] Remove: %s", it->second->GetName().c_str());
            m_hookMap.erase(it);
        } else {
            LOGI("[PointerHookManager] Hook %s not found", idx.name());
        }
    }

protected:
    PointerHookManager() {}
    ~PointerHookManager() {}

private:
    std::shared_mutex m_mutex;
    std::unordered_map<std::type_index, std::unique_ptr<IPointerHook>> m_hookMap;
};
