#include "fsocketTcp.h"

namespace Ferrum {

FSocketTcp::FSocketTcp(size_t bufferSize)
    : FSocketBase(bufferSize),
      fwdAddr(0, 0),
      listenAddr(0, 0),
      natTable(nullptr) {}

FSocketTcp::~FSocketTcp() {}

FResult<bool> FSocketTcp::initSocket() {
  socketFd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (socketFd < 0) {
    return FResult<bool>::Error("Failed to create socket");
  }
  FLog::trace("socket created: %d", socketFd);
  auto socketStatus = fcntl(socketFd, F_GETFL);
  auto error = fcntl(socketFd, F_SETFL, socketStatus | O_NONBLOCK);
  if (error < 0) {
    ::close(socketFd);
    return FResult<bool>::Error("Failed to set socket to non-blocking");
  }
  const int32_t on = 1;
  error = setsockopt(socketFd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));
  if (error < 0) {
    ::close(socketFd);
    return FResult<bool>::Error("Failed to set socket to include IP header");
  }
  return FResult<bool>::Ok();
}

FResult<bool> FSocketTcp::listen(const std::string ip) {
  this->listenAddr = FAddr{inet_addr(ip.c_str()), htons(8080)};
  return FSocketBase::listen(ip);
}

FResult<bool> FSocketTcp::configure(const std::string fwdIp,
                                    const std::string ports[],
                                    std::shared_ptr<FNatTable> natTable) {
  this->fwdAddr = FAddr{inet_addr(fwdIp.c_str()), 0};
  this->natTable = natTable;
  FLog::trace("fwdAddr: %s", this->fwdAddr.toString().c_str());
  return FResult<bool>::Ok();
}

void FSocketTcp::close() {
  FSocketBase::close();
}

void FSocketTcp::onRead() {
  // static FAddr destAddr{inet_addr("100.100.100.1000"), 8080};
  struct iphdr *ip_packet = (struct iphdr *)buffer;
  struct tcphdr *tcp_packet = (struct tcphdr *)(buffer + sizeof(struct iphdr));
  this->srcAddr = FAddrSPtr{new FAddr{ip_packet->saddr, tcp_packet->source}};
  this->dstAddr = FAddrSPtr{new FAddr{ip_packet->daddr, tcp_packet->dest}};
  // filter if port is not in our target list, don't log
  auto &fwd = this->fwdAddr.getV4Addr();
  auto isPacketComingToListening =
      ip_packet->daddr == listenAddr.getV4Addr().sin_addr.s_addr &&
      tcp_packet->dest == listenAddr.getV4Addr().sin_port;

  auto isPacketComingFromForward = ip_packet->saddr == fwd.sin_addr.s_addr &&
                                   tcp_packet->source == fwd.sin_port;

  if (!(isPacketComingToListening || isPacketComingFromForward)) {
    // FLog::debug("packet is not coming from the forward or client %d %d",
    //            isPacketComingToListening, isPacketComingFromForward);
    return;
  }
  FLog::debug("***************************************");
  FLog::trace("onRead called");
  FLog::trace("socket readed buf size: %d", this->bufferSize);
  FLog::debug("ip packet id: %d from %s to %s", ip_packet->id,
              srcAddr->toStringWithPort().c_str(),
              dstAddr->toStringWithPort().c_str());
  FLog::debug("tcp Packet Src Port: %d Dst Port: %d", ntohs(tcp_packet->source),
              ntohs(tcp_packet->dest));
  // if packet is not coming from the destination,
  // it is coming from client, forward to destination to
  if (ip_packet->saddr != fwd.sin_addr.s_addr) {
    FLog::debug("packet is coming from the client");
    uint16_t natPort = 0;
    auto natPortFResult = natTable->getNat(this->srcAddr);
    if (natPortFResult.isError()) {
      FLog::debug("failed to get nat port: %s", natPortFResult.message.c_str());
      natPortFResult = natTable->addNat(this->srcAddr);
      if (natPortFResult.isError()) {
        FLog::error("failed to add nat port: %s",
                    natPortFResult.message.c_str());
        return;
      }
    }
    natPort = natPortFResult.data;
    FLog::debug("assigned nat port: %d", ntohs(natPort));
    // change the source address to our address
    ip_packet->saddr = listenAddr.getV4Addr().sin_addr.s_addr;
    tcp_packet->source = natPort;
    // change the destination address to the forward address
    ip_packet->daddr = fwd.sin_addr.s_addr;
    // recalculate checksum
    ip_packet->check = FNet::ipChecksum(ip_packet);
    tcp_packet->check = FNet::tcpChecksum(ip_packet, tcp_packet);

    fwd.sin_port = tcp_packet->dest;
    FLog::debug("packet will sent to %s packet size: %d",
                this->fwdAddr.toStringWithPort().c_str(), this->bufferSize);
    auto sendedSize = sendto(socketFd, buffer, this->bufferSize, 0,
                             reinterpret_cast<const struct sockaddr *>(&fwd),
                             sizeof(struct sockaddr_in));
    if (sendedSize < 0) {
      FLog::error("failed to send data: %s", strerror(errno));
      return;
    }
    FLog::debug("packet sent to %s", this->fwdAddr.toStringWithPort().c_str());

  } else {
    // find the destination client and redirect the packet
    FLog::debug("packet is coming from the forward");
    auto natPort = tcp_packet->dest;
    auto natFResult = natTable->getNat(natPort);
    if (natFResult.isError()) {
      FLog::error("failed to get nat port: %s", natFResult.message.c_str());
      return;
    }
    // change the source address to our address
    tcp_packet->source = listenAddr.getV4Addr().sin_port;
    ip_packet->saddr = listenAddr.getV4Addr().sin_addr.s_addr;
    // change the destination address to the client address
    tcp_packet->dest = natFResult.data->getV4Addr().sin_port;
    ip_packet->daddr = natFResult.data->getV4Addr().sin_addr.s_addr;
    // recalculate checksum
    ip_packet->check = FNet::ipChecksum(ip_packet);
    tcp_packet->check = FNet::tcpChecksum(ip_packet, tcp_packet);

    FLog::debug("packet will sent to %s",
                natFResult.data->toStringWithPort().c_str());
    auto sendedSize = sendto(socketFd, buffer, this->bufferSize, 0,
                             reinterpret_cast<const struct sockaddr *>(
                                 &natFResult.data.get()->getV4Addr()),
                             sizeof(struct sockaddr_in));
    if (sendedSize < 0) {
      FLog::error("failed to send data: %s", strerror(errno));
      return;
    }
    FLog::debug("packet sent to %s",
                natFResult.data.get()->toStringWithPort().c_str());
  }
}

}  // namespace Ferrum