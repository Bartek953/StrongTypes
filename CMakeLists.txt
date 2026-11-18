cmake_minimum_required(VERSION 3.14)
project(StrongLibTests CXX)

# Wymaganie standardu C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Definicja biblioteki nagłówkowej 'strong' jako targetu INTERFACE
add_library(strong INTERFACE)
target_include_directories(strong INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# --- GoogleTest Configuration ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

# Zapobieganie nadpisywaniu flag MSVC w systemach Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Automatyczne wyszukiwanie wszystkich plików .cpp rekurencyjnie w folderze tests/
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "tests/*.cpp")

# Plik wykonywalny z testami zbudowany z odnalezionych plików
add_executable(unit_tests ${TEST_SOURCES})

# Linkowanie biblioteki strong oraz GoogleTest
target_link_libraries(unit_tests PRIVATE strong GTest::gtest_main)

# Integracja z CTest
include(GoogleTest)
gtest_discover_tests(unit_tests)