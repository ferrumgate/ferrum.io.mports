#ifndef __FNAT_TABLE_H__
#define __FNAT_TABLE_H__

#include <time.h>

#include "fcache.h"
#include "fcommon.h"

namespace Ferrum {

/// @brief Nat table to handle port forwarding
class FNatTable {
 public:
  constexpr static uint16_t MIN_NAT_PORT = 1024;
  constexpr static uint16_t MAX_NAT_PORT = 65535;
  FNatTable(uint32_t timeoutMS = 300000);
  virtual ~FNatTable();
  // port is always in host byte order
  FResult<uint16_t> addNat(FAddrSPtr &addr);
  /// @brief return the client address for the given port
  /// @param port
  /// @return
  FResult<FAddrSPtr> getNat(uint16_t port) const;
  /// @brief return the nat port for the given client address
  /// @param addr
  /// @return
  FResult<uint16_t> getNat(const FAddrSPtr &addr) const;

 protected:
  uint32_t timeoutMS;
  uint16_t nextNatPort();
  // client ip address to nat port
  FCache<FAddrSPtr, uint16_t> natCache;
  // nat port to client ip address
  FCache<uint16_t, FAddrSPtr> revNatCache;
};
}  // namespace Ferrum
#endif  // __FNAT_H__