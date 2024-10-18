#ifndef __FSOCKET_BASE_H__
#define __FSOCKET_BASE_H__

#include <unordered_map>

#include "common.h"
#include "faddr.h"
#include "log.h"
#include "net.h"

namespace Ferrum {

class FSocketBase {
 public:
  FSocketBase(size_t bufferSize = std::numeric_limits<size_t>::max());
  virtual ~FSocketBase();
  virtual Result<bool> listen(const std::string ip);
  virtual void close();

 protected:
  virtual Result<bool> initSocket();
  virtual const ::uv_os_fd_t getSocketFd() const;
  virtual void onRead() = 0;

 protected:
  ::uv_poll_t poll;
  ::uv_loop_t *loop;
  ::uv_os_fd_t socketFd;
  bool initialized;

 protected:
  uint8_t *buffer;
  size_t bufferSize;
  FAddrSPtr srcAddr;
  FAddrSPtr dstAddr;

 private:
  friend void handleRead(uv_poll_t *handle, int status, int events);
};

}  // namespace Ferrum

#endif  // __FSOCKET_BASE_H__