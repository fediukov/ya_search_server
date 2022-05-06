#pragma once

#include <algorithm>
#include <cstdlib>
#include <execution>
#include <future>
#include <map>
#include <mutex>
#include <numeric>
#include <random>
#include <string>
#include <vector>

using namespace std::string_literals;

template <typename Key, typename Value>
class ConcurrentMap {
private:
    struct Buscket
    {
        std::mutex mutex_;
        std::map<Key, Value> map_;
    };
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");

    struct Access {
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;

        Access(const Key& key, Buscket& buscket)
            : guard(buscket.mutex_), ref_to_value(buscket.map_[key])
        {
        }
    };

    Access operator[](const Key& key)
    {
        uint64_t n = static_cast<uint64_t>(key) % busckets_.size();
        auto& buscket = busckets_[n];
        return { key, buscket };
    }//*/

    explicit ConcurrentMap(size_t bucket_count)
        : busckets_(bucket_count)
    {
    }

    std::map<Key, Value> BuildOrdinaryMap()
    {
        std::map<Key, Value> curr_map;
        for (auto& buscket : busckets_)
        {
            std::lock_guard g(buscket.mutex_);
            curr_map.insert(buscket.map_.begin(), buscket.map_.end());
        }
        return curr_map;
    }

    void Erase(const Key& key)
    {
        for (auto& b : busckets_)
        {
            if (b.map_.count(key))
            {
                b.map_.erase(key);
                break;
            }
        }
    }

private:
    std::vector<Buscket> busckets_;
};