#include "startup.hpp"
#include "noxcore/io/game_dir.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>

namespace fs = std::filesystem;
void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
std::optional<std::string> option(const std::vector<std::string>& args, const char* name)
{
    for (std::size_t i = 1; i + 1 < args.size(); ++i) if (args[i] == name) return args[i + 1];
    return {};
}
void game(const fs::path& path)
{
    fs::create_directories(path);
    for (const char* name : {"gamedata.bin", "modifier.bin", "monster.bin", "thing.bin"}) std::ofstream(path / name) << "fixture";
    std::ofstream(path / "DEFAULT.CFG") << "Factory = 1\n";
    std::ofstream(path / "nox.cfg") << "Original = 1\n";
    fs::create_directory(path / "Save");
    std::ofstream(path / "Save/Hero.plr") << "original character";
}
int main()
{
    const auto root = fs::temp_directory_path() / ("nox-mac-startup-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    require(fs::create_directory(root), "unique test directory");
    const auto oldCwd = fs::current_path();
    const char* existingEnv = std::getenv("NOX_DATA");
    const std::optional<std::string> oldEnv = existingEnv ? std::optional<std::string>(existingEnv) : std::nullopt;
    unsetenv("NOX_DATA");
    int result = 0;
    try {
        fs::current_path(root);
        const auto assets = root / "assets", user = root / "user", custom = root / "custom";
        game(assets);
        noxsdl::macos::LaunchPaths paths{root / "NoX Core.app/Contents/Resources", user, root / "launch.log", root / "home", {}};
        auto launch = noxsdl::macos::prepareLaunch({"noxcore", "--game-dir", assets.string()}, paths);
        require(launch.settings == user, "settings use the writable location");
        require(option(launch.arguments, "--save-dir") == (user / "Save").string(), "save default passed to upstream");
        require(option(launch.arguments, "--map-cache") == (user / "Cache/maps").string(), "map cache default passed to upstream");
        require(option(launch.arguments, "--log") == paths.log.string(), "log default passed to upstream");
        require(fs::exists(user / "Save/Hero.plr") && fs::exists(user / "default.cfg"), "settings and characters migrated");
        fs::remove(user / "Save/Hero.plr");
        std::ofstream(user / "nox.cfg") << "User = 2\n";
        noxsdl::macos::prepareLaunch({"noxcore", "--game-dir", assets.string()}, paths);
        require(!fs::exists(user / "Save/Hero.plr"), "deleted character must not reappear");
        std::ifstream saved(user / "nox.cfg"); std::string line; std::getline(saved, line);
        require(line == "User = 2", "existing user configuration must not be overwritten");
        launch = noxsdl::macos::prepareLaunch({"noxcore", "--game-dir", assets.string(), "--user-dir", custom.string(),
            "--save-dir", (root / "external/Save").string(), "--log", "chosen.log", "--map-cache", "chosen-cache"}, paths);
        require(launch.settings == custom && !option(launch.arguments, "--user-dir"), "Mac option consumed before upstream parsing");
        require(option(launch.arguments, "--save-dir") == (root / "external/Save").string(), "explicit saves preserved");
        require(option(launch.arguments, "--log") == "chosen.log" && option(launch.arguments, "--map-cache") == "chosen-cache", "explicit log/cache preserved");
        require(!fs::exists(custom / "Save/Hero.plr"), "independent saves must not be migrated");
        require(!fs::exists(custom / ".saves-import-complete"), "independent saves must not mark migration complete");
        fs::create_directories(paths.executable);
        const auto bundled = paths.executable / "GameData";
        game(bundled);
        launch = noxsdl::macos::prepareLaunch({"noxcore"}, paths);
        require(option(launch.arguments, "--game-dir") == bundled.string(), "bundle data discovery");
        setenv("NOX_DATA", assets.c_str(), 1);
        launch = noxsdl::macos::prepareLaunch({"noxcore"}, paths);
        require(option(launch.arguments, "--game-dir") == assets.string(), "NOX_DATA precedes bundle discovery");
        launch = noxsdl::macos::prepareLaunch({"noxcore", "--game-dir", bundled.string()}, paths);
        require(option(launch.arguments, "--game-dir") == bundled.string(), "explicit data precedes NOX_DATA");
        bool rejected = false;
        try { noxsdl::macos::prepareLaunch({"noxcore", "--user-dir"}, paths); } catch (const std::exception&) { rejected = true; }
        require(rejected, "missing user-dir argument rejected");
        std::cout << "PASS: launch defaults, explicit overrides, migration, deletion, bundle discovery and argument validation\n";
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; result = 1; }
    fs::current_path(oldCwd);
    if (oldEnv) setenv("NOX_DATA", oldEnv->c_str(), 1); else unsetenv("NOX_DATA");
    fs::remove_all(root);
    return result;
}
