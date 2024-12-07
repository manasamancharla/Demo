#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <unordered_map>
#include <list>
#include <string>

class LRUCache
{
public:
  LRUCache(size_t capacity) : capacity_(capacity) {}

  std::string get(const std::string &key)
  {
    auto it = cacheMap_.find(key);
    if (it == cacheMap_.end())
      return ""; // Cache miss

    cacheList_.splice(cacheList_.begin(), cacheList_, it->second);
    return it->second->second;
  }

  void put(const std::string &key, const std::string &value)
  {
    auto it = cacheMap_.find(key);
    if (it != cacheMap_.end())
    {
      it->second->second = value;
      cacheList_.splice(cacheList_.begin(), cacheList_, it->second);
      return;
    }

    if (cacheList_.size() == capacity_)
    {
      auto last = cacheList_.back();
      cacheMap_.erase(last.first);
      cacheList_.pop_back();
    }

    cacheList_.emplace_front(key, value);
    cacheMap_[key] = cacheList_.begin();
  }

private:
  size_t capacity_;
  std::list<std::pair<std::string, std::string>> cacheList_;
  std::unordered_map<std::string, std::list<std::pair<std::string, std::string>>::iterator> cacheMap_;
};

#endif
