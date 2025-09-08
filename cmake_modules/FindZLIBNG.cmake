# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# ZLIB_NG_HOME environmental variable is used to check for ZLIB_NG headers and static library

# ZLIB_NG_INCLUDE_DIR: directory containing headers
# ZLIB_NG_LIBRARY: path to libz-ng
# ZLIB_NG_STATIC_LIB: path to libz-ng.a
# ZLIB_NG_FOUND: whether ZLIB_NG has been found

if( NOT "${ZLIB_NG_HOME}" STREQUAL "")
    file (TO_CMAKE_PATH "${ZLIB_NG_HOME}" _zlib_ng_path)
endif()

message (STATUS "ZLIB_NG_HOME: ${ZLIB_NG_HOME}")

find_path (ZLIB_NG_INCLUDE_DIR zlib-ng.h HINTS
  ${_zlib_ng_path}
  NO_DEFAULT_PATH
  PATH_SUFFIXES "include")

if (NOT ZLIB_NG_STATIC_LIB_NAME)
  set (ZLIB_NG_STATIC_LIB_NAME z-ng)
endif()

find_library (ZLIB_NG_LIBRARY NAMES z-ng ${ZLIB_NG_STATIC_LIB_NAME} HINTS
  ${_zlib_ng_path}
  PATH_SUFFIXES "lib")

find_library (ZLIB_NG_STATIC_LIB NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}${ZLIB_NG_STATIC_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX} HINTS
  ${_zlib_ng_path}
  PATH_SUFFIXES "lib")

if (ZLIB_NG_INCLUDE_DIR AND ZLIB_NG_LIBRARY)
  set (ZLIB_NG_FOUND TRUE)
  set (ZLIB_NG_HEADER_NAME zlib-ng.h)
  set (ZLIB_NG_HEADER ${ZLIB_NG_INCLUDE_DIR}/${ZLIB_NG_HEADER_NAME})
else ()
  set (ZLIB_NG_FOUND FALSE)
endif ()

if (ZLIB_NG_FOUND)
  message (STATUS "Found the ZLIB_NG header: ${ZLIB_NG_HEADER}")
  message (STATUS "Found the ZLIB_NG library: ${ZLIB_NG_LIBRARY}")
  if (ZLIB_NG_STATIC_LIB)
    message (STATUS "Found the ZLIB_NG static library: ${ZLIB_NG_STATIC_LIB}")
  endif ()
else()
  if (_zlib_ng_path)
    set (ZLIB_NG_ERR_MSG "Could not find ZLIB_NG. Looked in ${_zlib_ng_path}.")
  else ()
    set (ZLIB_NG_ERR_MSG "Could not find ZLIB_NG in system search paths.")
  endif()

  if (ZLIB_NG_FIND_REQUIRED)
    message (FATAL_ERROR "${ZLIB_NG_ERR_MSG}")
  else ()
    message (STATUS "${ZLIB_NG_ERR_MSG}")
  endif ()
endif()

mark_as_advanced (
  ZLIB_NG_INCLUDE_DIR
  ZLIB_NG_STATIC_LIB
  ZLIB_NG_LIBRARY
)
