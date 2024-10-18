#include "fsocketTcp.h"

namespace Ferrum {

FSocketTcp::FSocketTcp(size_t bufferSize)
    : FSocketBase(bufferSize), fwdAddr(0, 0) {}

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
  const int32_t on = 1;
  error = setsockopt(socketFd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));
  if (error < 0) {
    ::close(socketFd);
    return Result<bool>::Error("Failed to set socket to include IP header");
  }
  return Result<bool>::Ok();
}

Result<bool> FSocketTcp::listen(const std::string ip) {
  return FSocketBase::listen(ip);
}

Result<bool> FSocketTcp::configure(const std::string ip,
                                   const std::string ports[]) {
  this->fwdAddr = FAddr{inet_addr(ip.c_str()), 0};
  Log::trace("fwdAddr: %s", this->fwdAddr.toString().c_str());
  return Result<bool>::Ok();
}

void FSocketTcp::close() { FSocketBase::close(); }

void FSocketTcp::onRead() {
  Log::trace("onRead called");
  Log::trace("socket readed buf size: %d", this->bufferSize);

  // static FAddr destAddr{inet_addr("100.100.100.1000"), 8080};
  struct iphdr *ip_packet = (struct iphdr *)buffer;
  struct tcphdr *tcp_packet = (struct tcphdr *)(buffer + sizeof(struct iphdr));
  this->srcAddr = FAddr{ip_packet->saddr, ntohs(tcp_packet->source)};
  this->dstAddr = FAddr{ip_packet->daddr, ntohs(tcp_packet->dest)};
  Log::debug("IP Packet Id: %d from %s to %s", ip_packet->id,
             srcAddr.toStringWithPort().c_str(),
             dstAddr.toStringWithPort().c_str());
  Log::debug("TCP Packet Src Port: %d Dst Port: %d", tcp_packet->source,
             tcp_packet->dest);
  // filter if port is not in our target list, don't log
  auto &fwd = this->fwdAddr.getV4Addr();
  // if packet is not coming from the destination, change the destination to
  if (ip_packet->saddr != fwd.sin_addr.s_addr) {
    Log::debug("packet is not coming from the destination");

    ip_packet->daddr = fwd.sin_addr.s_addr;
    ip_packet->check = Net::ipChecksum(ip_packet);
    tcp_packet->check = Net::tcpChecksum(ip_packet, tcp_packet);

    fwd.sin_port = tcp_packet->dest;
    Log::debug("packet will sent to %s",
               this->fwdAddr.toStringWithPort().c_str());
    auto sendedSize = sendto(socketFd, buffer, this->bufferSize, 0,
                             reinterpret_cast<const struct sockaddr *>(&fwd),
                             sizeof(struct sockaddr_in));
    if (sendedSize < 0) {
      Log::error("failed to send data: %s", strerror(errno));
      return;
    }
    Log::debug("packet sent to %s", this->fwdAddr.toStringWithPort().c_str());

  } else {
    // find the destination client and redirect the packet
  }
}

}  // namespace Ferrum