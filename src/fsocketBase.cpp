#include "fsocketBase.h"

namespace Ferrum {

FSocketBase::FSocketBase(size_t bufferSize)
    : loop(nullptr),
      initialized(false),
      buffer(new uint8_t[bufferSize]),
      bufferSize(bufferSize),
      srcAddr(0, 0),
      dstAddr(0, 0) {
  loop = uv_default_loop();
}

FSocketBase::~FSocketBase() {
  close();
  delete[] buffer;
}

Result<bool> FSocketBase::initSocket() {
  return Result<bool>::Error("Please override this initSocket method");
}

const ::uv_os_fd_t FSocketBase::getSocketFd() const { return socketFd; }

void handleRead(uv_poll_t *handle, int status, int events) {
  Log::trace("poll callback status: %d events: %d", status, events);
  if (status < 0) {
    Log::error("poll error: %s", uv_strerror(status));
  }
  if (!status && events & UV_READABLE) {
    FSocketBase *socket = reinterpret_cast<FSocketBase *>(handle->data);
    auto socketFd = socket->getSocketFd();
    uint8_t *buffer = socket->buffer;
    auto readedLen = recv(socketFd, buffer, socket->bufferSize, 0);
    if (readedLen < 0) {
      Log::error("failed to read data: %s", strerror(errno));
      return;
    }
    if (readedLen == 0) {
      Log::trace("no data to read");
      return;
    }
    Log::trace("readed %d bytes", readedLen);
    socket->bufferSize = readedLen;
    socket->onRead();
  }
  if (!status && events & UV_WRITABLE) {
    Log::trace("socket writable");
  }
}

Result<bool> FSocketBase::listen(const std::string ip) {
  auto result = initSocket();
  if (result.isError()) {
    return result;
  }
  struct sockaddr_in saddr;
  auto error = uv_ip4_addr(ip.c_str(), 0, &saddr);
  if (error) {
    return Result<bool>::Error("Failed to parse ip address: " +
                               std::string(uv_strerror(error)));
  }
  saddr.sin_port = htons(9000);
  error = bind(socketFd, (struct sockaddr *)&saddr, sizeof(saddr));
  error = 0;
  if (error < 0) {
    return Result<bool>::Error("Failed to bind socket" +
                               std::string(strerror(errno)));
  }

  error = uv_poll_init_socket(uv_default_loop(), &poll, socketFd);
  if (error) {
    return Result<bool>::Error("Failed to initialize poll: " +
                               std::string(uv_strerror(error)));
  }
  poll.data = this;
  error = uv_poll_start(&poll, UV_READABLE, handleRead);
  if (error) {
    return Result<bool>::Error("Failed to poll start failed: " +
                               std::string(uv_strerror(error)));
  }
  return Result<bool>::Ok();
}

void FSocketBase::close() {
  if (initialized) {
    uv_poll_stop(&poll);
    uv_close((uv_handle_t *)&poll, [](uv_handle_t *handle) {
      Log::debug("poll closed");
      // close socket if opened
    });
  }
  initialized = false;
}

}  // namespace Ferrum