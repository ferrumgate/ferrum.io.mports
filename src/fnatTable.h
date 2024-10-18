#ifndef __FNAT_TABLE_H__
#define __FNAT_TABLE_H__

#include <time.h>

#include "common.h"
#include "fcache.h"

namespace Ferrum {

/// @brief Nat table to handle port forwarding
class FNatTable {
 public:
  constexpr static uint16_t MIN_NAT_PORT = 1024;
  constexpr static uint16_t MAX_NAT_PORT = 65535;
  FNatTable(uint32_t timeoutMS = 300000);
  virtual ~FNatTable();
  Result<uint16_t> addNat(const FAddr &addr);
  Result<const FAddr &> getNat(uint16_t port);

 protected:
  uint32_t timeoutMS;
  uint16_t nextNatPort() const;
  FCache<FAddr, uint16_t> natCache;
};
}  // namespace Ferrum
#endif  // __FNAT_H__