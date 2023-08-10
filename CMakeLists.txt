# minimum required cmake version
cmake_minimum_required( VERSION 3.5 FATAL_ERROR )

project( Processing )

# set default CMAKE_BUILD_TYPE to Release if not set
if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE )
endif()

if( CMAKE_COMPILER_IS_GNUCC )
  set( BUILD_STATIC OFF CACHE BOOL "Build static executables" )
endif()

# set c++11
set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

# compile everything position independent (even static libraries)
set( CMAKE_POSITION_INDEPENDENT_CODE TRUE )

# use folders in IDEs for projects (e.g. lib sample app test)
set_property( GLOBAL PROPERTY USE_FOLDERS ON )

# add needed subdirectories
add_subdirectory( "src/Lib/" )
add_subdirectory( "src/App/NaluProcessing" )
add_subdirectory( "src/App/StreamProcessing" )

