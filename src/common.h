#ifndef __COMMON_H__
#define __COMMON_H__

#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <limits>
#include <linux/if_packet.h>
#include <linux/udp.h>
#include <memory>
#include <net/ethernet.h> /* the L2 protocols */
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string>
#include <sys/socket.h>
#include <uv.h>

namespace Ferrum {

enum class RCode : uint8_t {
  OK = 0,
  ERROR = 1,
};

template <typename T> struct Result {
  RCode code;
  std::shared_ptr<std::string> codeMessage;
  std::shared_ptr<T> data;
  bool isOk() { return code == RCode::OK; }
  bool isError() { return code == RCode::ERROR; }

  static struct Result<T> Ok() {
    return {RCode::OK, std::make_shared<std::string>(), std::make_shared<T>()};
  } static struct Result<T> Ok(T &&data) {
    return {RCode::OK, std::make_shared<std::string>(),
            std::make_shared<T>(data)};
  } static struct Result<T> Error(const std::string &&message) {
    return {RCode::ERROR, std::make_shared<std::string>(message),
            std::make_shared<T>()};
  }
};

} // namespace Ferrum
#endif // __LIB_H__