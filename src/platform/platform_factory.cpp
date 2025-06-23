#include "platform_interface.h"
#include "native_platform.h"
#include "web_platform.h"

namespace alice2 {
namespace platform {

std::unique_ptr<IPlatform> CreatePlatform() {
#ifdef __EMSCRIPTEN__
    return std::make_unique<WebPlatform>();
#else
    return std::make_unique<NativePlatform>();
#endif
}

} // namespace platform
} // namespace alice2
