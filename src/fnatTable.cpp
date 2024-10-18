#include "fnatTable.h"

namespace Ferrum {

FNatTable::FNatTable(uint32_t timeoutMS)
    : timeoutMS(timeoutMS), natCache(timeoutMS) {
  // seed random number generator
  srand(time(NULL));
}

FNatTable::~FNatTable() {}

uint16_t FNatTable::nextNatPort() const {
  int counter = FNatTable::MAX_NAT_PORT;  // prevent infinite loop
  while (counter--) {
    uint16_t port =
        rand() % (FNatTable::MAX_NAT_PORT - FNatTable::MIN_NAT_PORT) +
        FNatTable::MIN_NAT_PORT;
    // if (revNatCache.getInstance().isExists(port).isOk()) {
    //   return port;
    // }
  }
  return 0;
}

}  // namespace Ferrum