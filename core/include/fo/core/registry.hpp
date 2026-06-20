#pragma once

#include "interfaces.hpp"
#include <memory>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <iostream>
#include <vector>
#include <string>

namespace fo::core {

template <typename T>
class Registry {
public:
    using Factory = std::function<std::unique_ptr<T>()>;

    static Registry& instance() {
        static Registry r;
        return r;
    }

    void add(const std::string& name, Factory f) {
        std::lock_guard<std::mutex> lock(mtx_);
        factories_[name] = std::move(f);
    }

    std::unique_ptr<T> create(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = factories_.find(name);
        if (it == factories_.end()) return nullptr;
        return (it->second)();
    }

    std::shared_ptr<T> get_shared(const std::string& name) {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = shared_instances_.find(name);
        if (it != shared_instances_.end()) return it->second;

        auto it_f = factories_.find(name);
        if (it_f == factories_.end()) return nullptr;
        
        std::shared_ptr<T> shared = (it_f->second)();
        shared_instances_[name] = shared;
        return shared;
    }

    std::vector<std::string> names() const {
        std::lock_guard<std::mutex> lock(mtx_);
        std::vector<std::string> out;
        out.reserve(factories_.size());
        for (auto& kv : factories_) out.push_back(kv.first);
        return out;
    }

    void clear_shared_instances() {
        std::lock_guard<std::mutex> lock(mtx_);
        shared_instances_.clear();
    }

private:
    mutable std::mutex mtx_;
    std::unordered_map<std::string, Factory> factories_;
    std::unordered_map<std::string, std::shared_ptr<T>> shared_instances_;
};

} // namespace fo::core
