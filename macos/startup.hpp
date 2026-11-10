#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace noxsdl::macos {

struct LaunchPaths {
    std::filesystem::path executable, user, log, home, configuredGame;
};
struct Launch {
    std::vector<std::string> arguments;
    std::filesystem::path settings;
};
/// Prepares user state and supplies defaults through upstream's existing options.
Launch prepareLaunch(std::vector<std::string> arguments, const LaunchPaths& paths);
void setSettingsDirectory(std::filesystem::path path);

} // namespace noxsdl::macos
