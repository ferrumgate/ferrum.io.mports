#include "fnatTable.h"

namespace Ferrum {

FNatTable::FNatTable(uint32_t timeoutMS)
    : timeoutMS(timeoutMS), natCache(timeoutMS) {
  // seed random number generator
  srand(time(NULL));
}

FNatTable::~FNatTable() {}

uint16_t FNatTable::nextNatPort() {
  int counter = FNatTable::MAX_NAT_PORT;  // prevent infinite loop
  while (counter--) {
    uint16_t port =
        rand() % (FNatTable::MAX_NAT_PORT - FNatTable::MIN_NAT_PORT) +
        FNatTable::MIN_NAT_PORT;
    if (!revNatCache.isExists(port).isOk()) {
      return port;
    }
  }
  return 0;
}

FResult<uint16_t> FNatTable::addNat(FAddrSPtr &addr) {
  uint16_t port = nextNatPort();
  if (port == 0) {
    return FResult<uint16_t>::Error("Failed to find a free port");
  }
  FResult<bool> result = natCache.add(addr, port);
  if (result.isError()) {
    return FResult<uint16_t>::Error(std::move(result.message));
  }
  FResult<bool> result2 = revNatCache.add(port, addr);
  if (result2.isError()) {
    // @todo we need to remove the port from the natCache
    return FResult<uint16_t>::Error(std::move(result2.message));
  }
  return FResult<uint16_t>::Ok(port);
}

FResult<FAddrSPtr> FNatTable::getNat(uint16_t port) const {
  auto result = revNatCache.get(port);
  if (result.isError()) {
    return FResult<FAddrSPtr>::Error(std::move(result.message));
  }
  return FResult<FAddrSPtr>::Ok(result.data);
}

FResult<uint16_t> FNatTable::getNat(const FAddrSPtr &addr) const {
  auto result = natCache.get(addr);
  if (result.isError()) {
    return FResult<uint16_t>::Error(std::move(result.message));
  }
  return FResult<uint16_t>::Ok(result.data);
}

}  // namespace Ferrum