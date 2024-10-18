#ifndef __NET_H__
#define __NET_H__

#include "common.h"

namespace Ferrum {
class Net {
 private:
  Net();
  ~Net();

 public:
  static uint16_t checksum(uint16_t *addr, uint32_t byteCount);
  static uint16_t ipChecksum(struct iphdr *iphdr);
  static uint16_t tcpChecksum(struct iphdr *iphdr, struct tcphdr *tcphdrp);
  static uint16_t udpChecksum(struct iphdr *iphdr, struct udphdr *udphdrp);
};

}  // namespace Ferrum

#endif  // __NET_H__
