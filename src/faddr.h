#ifndef __FADDR_H__
#define __FADDR_H__

#include "common.h"

namespace Ferrum {
class FAddr {
 public:
  FAddr(uint32_t ip, uint16_t port);
  FAddr(sockaddr_in &addr);
  FAddr(sockaddr_in6 &addr);
  FAddr(const FAddr &other);
  FAddr &operator=(const FAddr &other);
  virtual ~FAddr();
  virtual const std::string &toString();
  virtual const std::string &toStringWithPort();
  bool isV4() const;
  bool isV6() const;
  bool operator==(const FAddr &other) const;
  friend bool operator<(const FAddr &thiss, const FAddr &other);
  sockaddr_in &getV4Addr();
  sockaddr_in6 &getV6Addr();

 protected:
  union {
    sockaddr_in v4;
    sockaddr_in6 v6;
  } addr;
  bool isV4Addr;
  std::string ipStr;
  std::string ipStrWithPort;
};

}  // namespace Ferrum

#endif  // __FADDR_H__