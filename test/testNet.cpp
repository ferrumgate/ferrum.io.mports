#include "net.h"
#include <gtest/gtest.h>

namespace Ferrum {

TEST(TestNet, ipChecksum) {
  uint8_t packet_bytes[] = {0x45, 0x00, 0x00, 0x3c, 0x18, 0x61, 0x40,
                            0x00, 0x40, 0x11, 0xbe, 0x44, 0xc0, 0xa8,
                            0x58, 0xfa, 0x8e, 0xfa, 0xbb, 0x6e};
  auto result = Net::ipChecksum(reinterpret_cast<iphdr *>(packet_bytes));
  EXPECT_EQ(result, 0xbe44);
}

} // namespace Ferrum