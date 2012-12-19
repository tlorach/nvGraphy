# Copyright (c) 2012, Tristan Lorach & NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived 
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 2.8)

PROJECT( nvGraphy )

set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin" )
# Specify the list of directories to search for cmake modules.
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set( CMAKE_PROGRAM_PATH "$ENV{PATH}")
set (SVCMFCUI_LOCATION "" CACHE PATH "...")
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set (ARCH "x64" CACHE STRING "CPU Architecture")
else ()
  set (ARCH "x86" CACHE STRING "CPU Architecture")
endif()

# Macro for adding files close to the executable
macro(_copy_files_to_target target thefiles)
    if(WIN32)
        foreach (FFF ${thefiles} )
			add_custom_command(
			    TARGET ${target} POST_BUILD
			    COMMAND ${CMAKE_COMMAND} -E copy_if_different
			        "${FFF}"
			        $<TARGET_FILE_DIR:${target}>
			        VERBATIM
			)
        endforeach()
    endif()

endmacro()


find_package(SvcMFCUI REQUIRED)
if(NOT SVCMFCUI_FOUND)
message(FATAL_ERROR "SvcMFC service for UI not found. Please install ( https://github.com/tlorach/SvcMFCUI ) and compile it first.
  If cmake could still not find it, please set SVCMFCUI_LOCATION in cmake or as Env. variable"
)
endif()

set(SOURCE_FILES
"Display.cpp"
"File_Fraps.cpp"
"File_Generic.cpp"
"File_PerfHUDAnnotations.cpp"
"File_PerfHUDReport.cpp"
"File_D3DAPICallsBenchmark.cpp"
"File_OGLAPICallsBenchmark.cpp"
"nvGraphy.cpp"
"nvGraphy.rc"
"regexp/Matcher.cpp"
"regexp/Pattern.cpp"
)
set(HEADER_FILES
"Display.h"
"nvGraphy.h"
"resource.h"
"stdAfx.h"
)

include_directories(
    ${SVCMFCUI_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/regexp
)

add_executable(nvGraphy
  ${SVCMFCUI_INCLUDE_DIR}/ISvcUI.h
  ${SVCMFCUI_INCLUDE_DIR}/ISvcBase.h
  ${SOURCE_FILES}
  ${HEADER_FILES}
)

# Copy the resources where the executable file will be created
_copy_files_to_target( nvGraphy "${SVCMFCUI_DLL}")

target_link_libraries(nvGraphy
    ${PLATFORM_LIBRARIES}
)

# setup entry point to be WinMain()
set_target_properties(nvGraphy PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
set_target_properties(nvGraphy PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")

