#include "net.h"

namespace Ferrum {

Net::Net() = default;
Net::~Net() = default;

/* Compute checksum for count bytes starting at addr, using one's complement of
 * one's complement sum*/
static unsigned short compute_checksum(unsigned short *addr,
                                       unsigned int count) {
  uint32_t sum = 0;
  while (count > 1) {
    sum += *addr++;
    count -= 2;
  }
  // if any bytes left, pad the bytes and add
  if (count > 0) {
    sum += ((*addr) & htons(0xFF00));
  }
  // Fold sum to 16 bits: add carrier to result
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }
  // one's complement
  sum = ~sum;
  return ((unsigned short)sum);
}

uint16_t Net::ipChecksum(struct iphdr *iphdrp) {
  return compute_checksum((unsigned short *)iphdrp, iphdrp->ihl << 2);
}

/* set tcp checksum: given IP header and tcp segment */
uint16_t Net::tcpChecksum(struct iphdr *pIph, uint16_t *tcpHeader) {
  uint32_t sum = 0;
  unsigned short tcpLen = ntohs(pIph->tot_len) - (pIph->ihl << 2);
  struct tcphdr *tcphdrp = reinterpret_cast<struct tcphdr *>(tcpHeader);
  // add the pseudo header
  // the source ip
  sum += (pIph->saddr >> 16) & 0xFFFF;
  sum += (pIph->saddr) & 0xFFFF;
  // the dest ip
  sum += (pIph->daddr >> 16) & 0xFFFF;
  sum += (pIph->daddr) & 0xFFFF;
  // protocol and reserved: 6
  sum += htons(IPPROTO_TCP);
  // the length
  sum += htons(tcpLen);

  // add the IP payload
  while (tcpLen > 1) {
    sum += *tcpHeader++;
    tcpLen -= 2;
  }
  // if any bytes left, pad the bytes and add
  if (tcpLen > 0) {
    // printf("+++++++++++padding, %dn", tcpLen);
    sum += ((*tcpHeader) & htons(0xFF00));
  }
  // Fold 32-bit sum to 16 bits: add carrier to result
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }
  sum = ~sum;
  // set computation result
  return static_cast<uint16_t>(sum);
}

/* set udp checksum: given IP header and UDP datagram */
uint16_t udpChecksum(struct iphdr *pIph, uint16_t *udpHeader) {
  uint32_t sum = 0;
  struct udphdr *udphdrp = reinterpret_cast<struct udphdr *>(udpHeader);
  unsigned short udpLen = htons(udphdrp->len);
  // printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~udp len=%dn", udpLen);
  // add the pseudo header
  // printf("add pseudo headern");
  // the source ip
  sum += (pIph->saddr >> 16) & 0xFFFF;
  sum += (pIph->saddr) & 0xFFFF;
  // the dest ip
  sum += (pIph->daddr >> 16) & 0xFFFF;
  sum += (pIph->daddr) & 0xFFFF;
  // protocol and reserved: 17
  sum += htons(IPPROTO_UDP);
  // the length
  sum += udphdrp->len;

  // add the IP payload
  // printf("add ip payloadn");
  // initialize checksum to 0
  udphdrp->check = 0;
  while (udpLen > 1) {
    sum += *udpHeader++;
    udpLen -= 2;
  }
  // if any bytes left, pad the bytes and add
  if (udpLen > 0) {
    // printf("+++++++++++++++padding: %dn", udpLen);
    sum += ((*udpHeader) & htons(0xFF00));
  }
  // Fold sum to 16 bits: add carrier to result
  // printf("add carriern");
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }
  // printf("one's complementn");
  sum = ~sum;
  // set computation result
  return (static_cast<uint16_t>(sum) == 0x0000) ? 0xFFFF
                                                : static_cast<uint16_t>(sum);
}

} // namespace Ferrum