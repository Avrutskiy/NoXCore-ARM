# CMake's existing project injection/deferred-call API keeps upstream build files intact.
cmake_language(EVAL CODE "cmake_language(DEFER CALL include [[${CMAKE_CURRENT_LIST_DIR}/client.cmake]])")
