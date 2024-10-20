#include "faddr.h"

namespace Ferrum {

#define resetAddr()                                        \
  auto *ptr = reinterpret_cast<uint8_t *>(&this->addr.v6); \
  std::memset(ptr, 0, sizeof(addr))

FAddr::FAddr(uint32_t ip, uint16_t port) : ipStr{} {
  resetAddr();
  this->addr.v4.sin_family = AF_INET;
  this->addr.v4.sin_port = port;
  this->addr.v4.sin_addr.s_addr = ip;
  this->isV4Addr = true;
}

FAddr::FAddr(sockaddr_in &addr) : ipStr{} {
  resetAddr();
  this->addr.v4 = addr;
  this->isV4Addr = true;
}

FAddr::FAddr(sockaddr_in6 &addr) : ipStr{} {
  resetAddr();
  this->addr.v6 = addr;
  this->isV4Addr = false;
}

FAddr::FAddr(const FAddr &other) = default;

FAddr &FAddr::operator=(const FAddr &other) = default;

FAddr::~FAddr() {}

bool operator<(const FAddr &thiss, const FAddr &other) {
  return memcmp(&thiss.addr, &other.addr, sizeof(thiss.addr)) < 0;
}

bool FAddr::isV4() const {
  return this->isV4Addr;
}

bool FAddr::isV6() const {
  return !this->isV4Addr;
}

const std::string &FAddr::toString() {
  if (this->ipStr.size() > 0) {
    return this->ipStr;
  }
  if (this->isV4Addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &this->addr.v4.sin_addr, ip, INET_ADDRSTRLEN);
    ipStr = ip;
  } else {
    char ip[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &this->addr.v6.sin6_addr, ip, INET6_ADDRSTRLEN);
    ipStr = ip;
  }
  return this->ipStr;
}

const std::string &FAddr::toStringWithPort() {
  if (this->ipStrWithPort.size() > 0) {
    return this->ipStrWithPort;
  }
  if (this->isV4Addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &this->addr.v4.sin_addr, ip, INET_ADDRSTRLEN);
    ipStr = ip;
    ipStr += ":";
    ipStr += std::to_string(ntohs(this->addr.v4.sin_port));
  } else {
    char ip[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &this->addr.v6.sin6_addr, ip, INET6_ADDRSTRLEN);
    ipStr = ip;
    ipStr += ":[";
    ipStr += std::to_string(ntohs(this->addr.v6.sin6_port));
    ipStr += "]";
  }
  return this->ipStr;
}

bool FAddr::operator==(const FAddr &other) const {
  if (this->isV4() != other.isV4()) {
    return false;
  }
  if (this->isV4()) {
    return std::memcmp(&this->addr.v4, &other.addr.v4, sizeof(sockaddr_in)) ==
           0;
  }
  return memcmp(&this->addr.v6.sin6_addr, &other.addr.v6.sin6_addr,
                sizeof(struct in6_addr)) == 0;
}

sockaddr_in &FAddr::getV4Addr() {
  return this->addr.v4;
}

sockaddr_in6 &FAddr::getV6Addr() {
  return this->addr.v6;
}

}  // namespace Ferrum
