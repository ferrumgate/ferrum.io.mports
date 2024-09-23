#ifndef __NET_H__
#define __NET_H__

#include "common.h"

namespace Ferrum {
class Net {
private:
  Net();
  ~Net();

public:
  static uint16_t ipChecksum(struct iphdr *iphdr);
  static uint16_t tcpChecksum(struct iphdr *iphdr, uint16_t *tcpHeader);
  static uint16_t udpChecksum(struct iphdr *iphdr, uint16_t *udpHeader);
};

} // namespace Ferrum

#endif // __NET_H__
