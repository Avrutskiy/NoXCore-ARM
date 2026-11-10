#include "startup.hpp"
#include "user_data.hpp"

#include "noxcore/io/game_dir.hpp"

#include <cstdlib>
#include <optional>
#include <stdexcept>
#include <utility>

namespace noxsdl::macos {
namespace fs = std::filesystem;

namespace {
std::optional<std::string> option(const std::vector<std::string>& args, const char* name)
{
    std::optional<std::string> value;
    for (std::size_t i = 1; i < args.size(); ++i) {
        if (args[i] == name) {
            if (++i == args.size()) {
                throw std::runtime_error(std::string("Missing value for ") + name);
            }
            value = args[i];
        }
    }
    return value;
}
fs::path findGame(const std::vector<std::string>& args, const LaunchPaths& paths)
{
    if (const auto explicitPath = option(args, "--game-dir")) {
        if (nox::io::isGameDir(*explicitPath)) return fs::absolute(*explicitPath);
        throw std::runtime_error("No Nox game data in --game-dir: " + *explicitPath);
    }
    std::vector<fs::path> candidates;
    if (const char* env = std::getenv("NOX_DATA"); env && *env) candidates.emplace_back(env);
    auto base = paths.executable.lexically_normal();
    if (base.filename().empty()) base = base.parent_path();
    if (!base.empty()) candidates.push_back(base);
    const auto contents = base.parent_path(), bundle = contents.parent_path();
    if ((base.filename() == "Resources" || base.filename() == "MacOS") &&
        contents.filename() == "Contents" && bundle.extension() == ".app") {
        candidates.push_back(contents / "Resources/GameData");
        candidates.push_back(bundle.parent_path());
    }
    std::error_code error;
    const auto current = fs::current_path(error);
    if (!error) candidates.push_back(current);
    if (!paths.home.empty()) candidates.push_back(paths.home / "Library/Application Support/NoXCore/GameData");
    // Retain upstream's other install locations and configured last resort.
    for (const auto& candidate : nox::io::gameDirSearch(paths.executable, paths.configuredGame)) {
        candidates.push_back(candidate.dir);
    }
    for (const auto& candidate : candidates) {
        if (nox::io::isGameDir(candidate)) return fs::absolute(candidate);
    }
    throw std::runtime_error("Nox game data wasn't found. Place NoX Core.app in your Nox data folder, "
                             "copy the data to ~/Library/Application Support/NoXCore/GameData, "
                             "or use --game-dir.");
}
}

Launch prepareLaunch(std::vector<std::string> arguments, const LaunchPaths& paths)
{
    const auto game = findGame(arguments, paths);
    const auto userOption = option(arguments, "--user-dir");
    const fs::path user = userOption ? fs::path(*userOption) : paths.user;
    if (user.empty()) throw std::runtime_error("A writable user directory is required.");
    const bool separateSave = option(arguments, "--save-dir").has_value();
    noxsdl::prepareUserData(game, user, !separateSave);
    // The Mac-only option never reaches upstream's command-line parser.
    for (std::size_t i = 1; i < arguments.size();) {
        if (arguments[i] == "--user-dir") {
            arguments.erase(arguments.begin() + i, arguments.begin() + i + 2);
        } else {
            ++i;
        }
    }
    const auto defaultOption = [&](const char* name, const fs::path& value) {
        if (!option(arguments, name)) {
            arguments.emplace_back(name);
            arguments.push_back(value.string());
        }
    };
    defaultOption("--game-dir", game);
    defaultOption("--save-dir", user / "Save");
    defaultOption("--map-cache", user / "Cache/maps");
    defaultOption("--log", paths.log);
    return {std::move(arguments), user};
}
} // namespace noxsdl::macos
