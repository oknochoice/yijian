cmake_minimum_required(VERSION 2.8.8)
project(catch_builder CXX)
include(ExternalProject)
find_package(Git REQUIRED)

ExternalProject_Add(
	catch
	PREFIX ${CMAKE_BINARY_DIR}/catch
  #GIT_REPOSITORY https://github.com/philsquared/Catch.git
  GIT_REPOSITORY ""
	TIMEOUT 10
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
	LOG_DOWNLOAD ON
	)
ExternalProject_Get_Property(catch source_dir)
set(CATCH_INCLUDE_DIR ${source_dir}/include CACHE INTERNAL "Path to include folder for Catch")
