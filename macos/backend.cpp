// The Mac implementation of the upstream client-backend contract.
#include "backend_hooks.hpp"
#include "startup.hpp"

#include <utility>

namespace {
std::filesystem::path userDirectory;
}

namespace noxsdl::macos {
void setSettingsDirectory(std::filesystem::path path) { userDirectory = std::move(path); }
}

namespace noxsdl::backend {
const std::filesystem::path& settingsDirectory(const std::filesystem::path& gameDir)
{
    return userDirectory.empty() ? gameDir : userDirectory;
}
SDL_WindowFlags additionalWindowFlags() { return SDL_WINDOW_HIGH_PIXEL_DENSITY; }
} // namespace noxsdl::backend
