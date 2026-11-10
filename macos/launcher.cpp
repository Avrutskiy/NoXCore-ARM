// Supply SDL's standard callback entry point from a Mac-only source file.
// main.cpp still defines the unmodified upstream callbacks, without its entry-point wrapper.
#define SDL_MAIN_USE_CALLBACKS 1
#define SDL_AppInit MacAppInit
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#undef SDL_AppInit

#include "startup.hpp"

#include <cstdlib>
#include <stdexcept>
#include <string_view>
#include <utility>

extern "C" SDL_AppResult SDLCALL SDL_AppInit(void** state, int argc, char* argv[]);

namespace {
std::filesystem::path preferencePath(const char* organization, const char* application)
{
    char* value = SDL_GetPrefPath(organization, application);
    if (!value) throw std::runtime_error(SDL_GetError());
    std::filesystem::path path(value);
    SDL_free(value);
    return path;
}
}

extern "C" SDL_AppResult SDLCALL MacAppInit(void** state, int argc, char* argv[])
{
    try {
        const char* base = SDL_GetBasePath();
        const char* home = std::getenv("HOME");
        noxsdl::macos::LaunchPaths paths{base ? base : "", preferencePath("NoXCore", "UserData"),
            preferencePath("noxcpp", "noxsdl") / "noxcore-log.txt", home ? home : "", NOXSDL_DEFAULT_GAME_DIR};
        auto launch = noxsdl::macos::prepareLaunch(std::vector<std::string>(argv, argv + argc), paths);
        noxsdl::macos::setSettingsDirectory(std::move(launch.settings));
        // Keep argv storage valid for the lifetime of the callback application.
        static std::vector<std::string> arguments;
        static std::vector<char*> pointers;
        arguments = std::move(launch.arguments);
        pointers.clear();
        for (auto& arg : arguments) pointers.push_back(arg.data());
        pointers.push_back(nullptr);
        return SDL_AppInit(state, static_cast<int>(arguments.size()), pointers.data());
    } catch (const std::exception& error) {
        SDL_Log("Mac startup: %s", error.what());
        bool automated = false;
        for (int i = 1; i < argc; ++i) {
            automated = automated || std::string_view(argv[i]) == "--screenshot" ||
                        std::string_view(argv[i]) == "--map-picture";
        }
        if (!automated) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "NoX Core: loading failed", error.what(), nullptr);
        return SDL_APP_FAILURE;
    }
}
