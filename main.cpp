#include "fsocketTcp.h"
#include "log.h"
using namespace Ferrum;

int main(int, char **) {

  Log::info("starting application");
#ifdef FDEBUG
  Log::info("setting log level to TRACE");
  Log::setLevel(LogLevel::TRACE);
#endif
  FSocketTcp socket{};
  socket.configure("100.100.100.100", {});
  auto result = socket.listen("192.168.105.105");
  if (result.isOk()) {
    Log::info("socket listening");
  } else {
    Log::error("socket listen error: %s", result.message.c_str());
    exit(1);
  }
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
