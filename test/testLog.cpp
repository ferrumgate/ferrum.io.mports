#include "log.h"
#include <gtest/gtest.h>
namespace Ferrum {

TEST(TestLog, info) { Log::info("hello"); }
TEST(TestLog, debug) { Log::debug("hello"); }
TEST(TestLog, set_level) { Log::setLevel(LogLevel::DEBUG); }

} // namespace Ferrum