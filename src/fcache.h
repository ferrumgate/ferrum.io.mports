#ifndef __FCACHE_H__
#define __FCACHE_H__

#include <chrono>
#include <map>
#include <memory>

#include "common.h"
#include "faddr.h"

namespace Ferrum {

/// @brief time base caching with pages
/// by this way, we can easily delete a timedout page in O(1) time
/// @tparam TKey
/// @tparam TValue
template <typename TKey, typename TValue>
class FCachePage {
 public:
  FCachePage(uint64_t expireTime) : _expireTime(expireTime), _cache() {}

  virtual ~FCachePage() {}

  uint64_t getExpireTime() const {
    return _expireTime;
  }

  Result<bool> add(const TKey key, const TValue value) {
    _cache[key] = value;
    return Result<bool>::Ok();
  }

  Result<bool> remove(TKey &key) {
    auto it = _cache.find(key);
    if (it != _cache.end()) {
      _cache.erase(it);
      return Result<bool>::Ok();
    }
    return Result<bool>::Error("Key not found");
  }

  Result<bool> isExists(TKey &key) {
    auto it = _cache.find(key);
    if (it != _cache.end()) {
      return Result<bool>::Ok();
    }
    return Result<bool>::Error("Key not found");
  }

  Result<TValue> get(TKey &key) {
    auto it = _cache.find(key);
    if (it != _cache.end()) {
      auto k = it->second;
      return Result<TValue>::Ok(it->second);
    }
    return Result<TValue>::Error("Key not found");
  }

  size_t getSize() {
    return _cache.size();
  }

 protected:
  uint64_t _expireTime;
  std::map<TKey, TValue> _cache;
};

template <typename TKey, typename TValue>
class FCache {
 public:
  using CachePage = FCachePage<TKey, TValue>;
  using CacheNow = std::unique_ptr<FCachePage<TKey, TValue>>;
  using CacheFuture = std::unique_ptr<FCachePage<TKey, TValue>>;

 public:
  FCache(uint64_t timeoutMS = 5 * 60 * 1000)
      : _timeoutMS(timeoutMS), _nowPage(nullptr), _futurePage(nullptr) {
    init();
  }
  virtual ~FCache() {
    clear();
  }

  /*   static FCache &getInstance() {
      if (!_instance) {
        _instance = std::make_unique<FCache<TKey, TValue>>();
      }
      return *_instance;
    } */

  virtual void clear() {
    if (_nowPage) {
      _nowPage.reset();
    }
    if (_futurePage) {
      _futurePage.reset();
    }
  }
  virtual void init() {
    auto nowMS = this->now();
    auto nowStart = (nowMS / _timeoutMS) * _timeoutMS;
    if (!_nowPage) {
      _nowPage =
          std::make_unique<FCachePage<TKey, TValue>>(nowStart + _timeoutMS);
    }
    if (!_futurePage) {
      _futurePage = std::make_unique<FCachePage<TKey, TValue>>(
          nowStart + _timeoutMS + _timeoutMS);
    }
  }
  virtual Result<bool> clearTimedOut() {
    auto nowMS = this->now();
    auto nowStart = (nowMS / _timeoutMS) * _timeoutMS;
    if (_nowPage && _nowPage->getExpireTime() < nowStart) {
      _nowPage.reset();
      _nowPage = std::move(_futurePage);
      _futurePage = nullptr;
    }
    init();
    return Result<bool>::Ok();
  }

  virtual Result<bool> isExists(const TKey &key) {
    auto result = _nowPage->isExists(key);
    return result;
  }

  virtual Result<TValue> get(TKey &key) {
    return _nowPage->get(key);
  }

  virtual Result<bool> add(TKey key, TValue value) {
    _nowPage->add(key, value);
    _futurePage->add(key, value);
    return Result<bool>::Ok();
  }

 protected:
  uint64_t _timeoutMS;
  CacheNow _nowPage;
  CacheFuture _futurePage;
  // static std::unique_ptr<FCache<TKey, TValue>> _instance;

 public:
  int64_t now() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch())
        .count();
  }
  CachePage *getCacheNow() {
    return _nowPage.get();
  }
  CachePage *getCacheFuture() {
    return _futurePage.get();
  }
};
}  // namespace Ferrum

#endif  // __FCACHE_H__