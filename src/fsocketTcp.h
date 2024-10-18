#ifndef __FSOCKET_TCP_H__
#define __FSOCKET_TCP_H__

#include "fcache.h"
#include "fnatTable.h"
#include "fsocketBase.h"

namespace Ferrum {
/// @brief FSocket class
class FSocketTcp : public FSocketBase {
 public:
  FSocketTcp(size_t bufferSize = 4096);
  virtual ~FSocketTcp() override;
  Result<bool> configure(const std::string ip, const std::string ports[],
                         std::shared_ptr<FNatTable> natTable);
  Result<bool> listen(const std::string ip) override;
  void close() override;

 protected:
  Result<bool> initSocket() override;
  void onRead() override;
  FAddr fwdAddr;
  FAddr listenAddr;
  std::shared_ptr<FNatTable> natTable;
};

}  // namespace Ferrum
#endif  // __FSOCKET_TCP_H__