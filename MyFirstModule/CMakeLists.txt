# - Basic CMake setup
# Check version meets ou requirements
# Declare project, which will configure compiler for us
cmake_minimum_required(VERSION 3.3)
project(MyFirstModule)

# Modules use Falaise, so we need to locate this or fail
# Locating Falaise will automatically locate all of its
# dependencies such as Bayeux, ROOT and Boost.
find_package(Falaise REQUIRED)

# Build a dynamic library from our sources
add_library(MyFirstModule SHARED MyFirstModule.h MyFirstModule.cc)

# Link it to the FalaiseModule library
# This ensures the correct compiler flags, include paths
# and linker flags are applied to our dynamic library.
target_link_libraries(MyFirstModule PUBLIC Falaise Falaise_ParticleIdentification)

# Install it:
install(TARGETS MyFirstModule DESTINATION ${CMAKE_INSTALL_LIBDIR}/Falaise/modules)
