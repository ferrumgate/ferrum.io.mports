#ifndef __FSOCKET_BASE_H__
#define __FSOCKET_BASE_H__

#include <unordered_map>

#include "faddr.h"
#include "fcommon.h"
#include "flog.h"
#include "fnet.h"

namespace Ferrum {

class FSocketBase {
 public:
  FSocketBase(size_t bufferCapacity = std::numeric_limits<uint16_t>::max());
  virtual ~FSocketBase();
  virtual FResult<bool> listen(const std::string ip);
  virtual void close();

 protected:
  virtual FResult<bool> initSocket();
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
  size_t bufferCapacity;
  FAddrSPtr srcAddr;
  FAddrSPtr dstAddr;

 private:
  friend void handleRead(uv_poll_t *handle, int status, int events);
};

}  // namespace Ferrum

#endif  // __FSOCKET_BASE_H__