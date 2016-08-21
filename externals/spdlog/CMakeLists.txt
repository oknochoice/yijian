cmake_minimum_required(VERSION 2.8.8)
project(spdlog_builder CXX)
include(ExternalProject)
find_package(Git REQUIRED)

ExternalProject_Add(
	spdlog
	PREFIX ${CMAKE_BINARY_DIR}/spdlog
	GIT_REPOSITORY https://github.com/gabime/spdlog.git
  #GIT_REPOSITORY ""
	TIMEOUT 10
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
	LOG_DOWNLOAD ON
	)
ExternalProject_Get_Property(spdlog source_dir)
set(SPDLOG_INCLUDE_DIR ${source_dir}/include CACHE INTERNAL "Path to include folder for Spdlog")
