#include "flog.h"
#include "fsocketTcp.h"
using namespace Ferrum;

int main(int, char **) {
  FLog::info("starting application");
#ifdef FDEBUG
  FLog::info("setting log level to TRACE");
  FLog::setLevel(LogLevel::TRACE);
#endif
  FSocketTcp socket{};
  std::shared_ptr<FNatTable> natTable{new FNatTable{300000}};
  socket.configure("100.100.100.100", {}, natTable);
  auto result = socket.listen("192.168.105.105");
  if (result.isOk()) {
    FLog::info("socket listening");
  } else {
    FLog::error("socket listen error: %s", result.message.c_str());
    exit(1);
  }
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
