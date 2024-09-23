#include "fsocketTcp.h"

namespace Ferrum {

FSocketTcp::FSocketTcp(size_t bufferSize) : FSocketBase(bufferSize) {}
FSocketTcp::~FSocketTcp() {}

Result<bool> FSocketTcp::initSocket() {
  socketFd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (socketFd < 0) {
    return Result<bool>::Error("Failed to create socket");
  }
  Log::trace("socket created: %d", socketFd);
  auto socketStatus = fcntl(socketFd, F_GETFL);
  auto error = fcntl(socketFd, F_SETFL, socketStatus | O_NONBLOCK);
  if (error < 0) {
    ::close(socketFd);
    return Result<bool>::Error("Failed to set socket to non-blocking");
  }
  return Result<bool>::Ok();
}
Result<bool> FSocketTcp::listen(const std::string ip) {
  return FSocketBase::listen(ip);
}
void FSocketTcp::close() { FSocketBase::close(); }

void FSocketTcp::onRead() {
  Log::trace("onRead called");
  Log::trace("socket readed buf size: %d", this->bufferSize);
  struct iphdr *ip_packet = (struct iphdr *)buffer;
  Log::debug("IP Packet Id: %d from %s", ip_packet->id,
             inet_ntoa(*(struct in_addr *)&ip_packet->saddr));
  struct tcphdr *tcp_packet = (struct tcphdr *)(buffer + sizeof(struct iphdr));
  Log::debug("TCP Packet Src Port: %d Dst Port: %d", tcp_packet->source,
             tcp_packet->dest);
  // filter if port is not in our target list, don't log
  ip_packet->daddr = inet_addr("100.100.100.100");
}

} // namespace Ferrum