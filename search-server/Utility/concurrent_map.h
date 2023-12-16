#pragma once

#include <cstdint>
#include <map>
#include <mutex>
#include <vector>

template <typename Key, typename Value> class ConcurrentMap {
private:
  struct MapsCore {
    std::mutex guard;
    std::map<Key, Value> map_;
  };

  std::vector<MapsCore> core_;

public:
  static_assert(std::is_integral_v<Key>,
                "ConcurrentMap supports only integer keys");

  struct Access {
    std::lock_guard<std::mutex> lock;
    Value &ref_to_value;

    Access(const Key &key, MapsCore &map)
        : lock(map.guard), ref_to_value(map.map_[key]) {}
  };

  explicit ConcurrentMap(size_t bucket_count) : core_(bucket_count) {}

  Access operator[](const Key &key) {
    auto &core = core_[uint64_t(key) % core_.size()];
    return {key, core};
  }

  std::map<Key, Value> BuildOrdinaryMap() {
    std::map<Key, Value> to_ret;
    for (auto &[guard, map_] : core_) {
      std::lock_guard<std::mutex> lock(guard);
      to_ret.insert(map_.begin(), map_.end());
    }
    return to_ret;
  }

  void erase(const Key &key) {
    auto &core = core_[uint64_t(key) % core_.size()];
    std::lock_guard<std::mutex> lock(core.guard);
    core.map_.erase(key);
  }
};
