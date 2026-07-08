cmake_minimum_required(VERSION 3.21)

project(OpenSRA VERSION 1.0.0 LANGUAGES CXX C)

# ---- C++20 (mirror R2D; SimCenterCommon helper modules compile as C++20) ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    # Force-include msvc_fix.h before any Windows header (byte/RPC clash + lean+mean/nominmax)
    file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}/msvc_fix.h" MSVC_FIX_PATH)
    add_compile_options("/FI${MSVC_FIX_PATH}")
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    add_compile_definitions(NOMINMAX)
endif()

# ---- QGIS-fork dependency roots (sibling layout) appended to CMAKE_PREFIX_PATH ----
# QCA + Qt6Keychain + Qwt + QScintilla come from the SimCenter deps tree (QGIS/DEPS);
# gdal/geos/proj headers come from the QGIS fork's vcpkg tree. Appended (not prepended)
# so conan-provided packages keep priority.
set(PATH_TO_QGIS "${CMAKE_CURRENT_LIST_DIR}/../QGIS")
list(APPEND CMAKE_PREFIX_PATH
  "${PATH_TO_QGIS}/DEPS"
  "${PATH_TO_QGIS}/build/vcpkg_installed/x64-windows")

# ---- Qt ----
# Mirrors R2D's component set, minus SerialPort (OpenSRA's compiled sources do not use
# QtSerialPort; only the QGIS fork does, which is linked, not compiled, here).
find_package(Qt6 REQUIRED COMPONENTS
  Core Gui Widgets
  Charts Concurrent Network Sql Xml WebEngineWidgets
  3DCore 3DRender 3DExtras
  OpenGL OpenGLWidgets Svg
  Positioning QuickWidgets
)

# Qt6Keychain resolves from the SimCenter deps tree (QGIS/DEPS/lib/cmake/Qt6Keychain),
# now on CMAKE_PREFIX_PATH. REQUIRED for the link (Session 4).
find_package(Qt6Keychain REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ---- Paths (single-".." sibling layout: OpenSRA_GUI/{OpenSRAFrontEnd,R2DTool,SimCenterCommon,QGIS}) ----
set(PATH_TO_COMMON "${CMAKE_CURRENT_LIST_DIR}/../SimCenterCommon")
set(PATH_TO_R2D    "${CMAKE_CURRENT_LIST_DIR}/../R2DTool")
# NOTE: no PATH_TO_QGIS_DEPS. SimCenterQGIS.cmake sources QGIS dependency headers from the
# QGIS fork's own build/vcpkg_installed tree, so OpenSRA needs no separate QGIS_DEPS repo.

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# ---- Conan packages (via conanfile.py -> CMakeDeps) ----
find_package(jansson REQUIRED)
find_package(ZLIB REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# ---- QCA (Qt Cryptography Architecture) from the SimCenter deps tree ----
# Uses the installed CMake config package QGIS/DEPS/lib/cmake/Qca-qt6 (imported target qca-qt6,
# which carries its QtCrypto include dir + Qt6::Core). REQUIRED for the link (Session 4).
find_package(Qca-qt6 CONFIG REQUIRED)
set(QCA_FOUND TRUE)
set(QCA_LIBRARY qca-qt6)                                              # imported target
set(QCA_INCLUDE_DIR "${PATH_TO_QGIS}/DEPS/include/Qca-qt6/QtCrypto")

# ---- Qwt + QScintilla from the SimCenter deps tree (mirror R2D's -D flags) ----
set(QWT_INCLUDE_DIR        "${PATH_TO_QGIS}/DEPS/include/qwt"             CACHE PATH "Qwt include dir")
set(QWT_LIBRARY            "${PATH_TO_QGIS}/DEPS/lib/qwt.lib"             CACHE FILEPATH "Qwt import library")
set(QSCINTILLA_INCLUDE_DIR "${PATH_TO_QGIS}/DEPS/include"                 CACHE PATH "QScintilla include root (has Qsci/)")
set(QSCINTILLA_LIBRARY     "${PATH_TO_QGIS}/DEPS/lib/qscintilla2_qt6.lib" CACHE FILEPATH "QScintilla import library")

find_package(QCA QUIET)
if(QCA_FOUND AND Qt6Keychain_FOUND)
  set(OPENSRA_HAVE_QGIS_DEPS TRUE)
else()
  set(OPENSRA_HAVE_QGIS_DEPS FALSE)
  message(WARNING
    "OpenSRA: QCA and/or Qt6Keychain not found yet (QCA_FOUND=${QCA_FOUND}, "
    "Qt6Keychain_FOUND=${Qt6Keychain_FOUND}). These come from the QGIS fork's vcpkg build. "
    "Configure proceeds, but a full LINK requires them — add the QGIS vcpkg tree to "
    "CMAKE_PREFIX_PATH (e.g. <workspace>/QGIS/build/vcpkg_installed/x64-windows) once built.")
endif()

# ---- Target ----
set(TARGET_NAME ${PROJECT_NAME})   # "OpenSRA"

set(SOURCES
  main.cpp
)

set(HEADERS
  # populated by the helper modules below
)

set(QRC_FILES
  images.qrc
  styles.qrc
)

# ---- App icon ----
set(APP_ICON "")
if(APPLE)
  set(APP_ICON "${CMAKE_CURRENT_LIST_DIR}/icons/openSRA-icon.icns")
  if(EXISTS "${APP_ICON}")
    set_source_files_properties("${APP_ICON}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  else()
    message(WARNING "App icon not found: ${APP_ICON}")
    set(APP_ICON "")
  endif()
endif()

# ---- Executable ----
qt_add_executable(${TARGET_NAME}
  WIN32
  MACOSX_BUNDLE
  ${SOURCES}
  ${HEADERS}
  ${QRC_FILES}
  ${APP_ICON}
)

# ---- Defines (mirror OpenSRA.pro) ----
target_compile_definitions(${TARGET_NAME} PRIVATE
  APP_VERSION="${PROJECT_VERSION}"
  OpenSRA
  Q_GIS
)

if(WIN32)
  target_compile_definitions(${TARGET_NAME} PRIVATE CURL_STATICLIB)
endif()

# ---- Shared modules + QGIS + app (mirror R2D's include/call order) ----
# Common
include("${PATH_TO_COMMON}/Common/Common.cmake")
simcenter_add_common(${TARGET_NAME})

# RandomVariables
include("${PATH_TO_COMMON}/RandomVariables/RandomVariables.cmake")
simcenter_add_randomvariables(${TARGET_NAME})

# SimCenterQGIS (shared QGIS widgets + QGIS fork include paths)
include("${PATH_TO_COMMON}/QGIS/SimCenterQGIS.cmake")
simcenter_add_qgis(${TARGET_NAME})

# OpenSRA app sources (own + borrowed R2D subset)
include("${CMAKE_CURRENT_LIST_DIR}/OpenSRA.cmake")
simcenter_add_opensra(${TARGET_NAME})

# Common Workflow — OpenSRA-SELECTIVE set (NOT the full simcenter_add_workflow, which would
# double-compile OpenSRA's own LocalApplication/MainWindowWorkflowApp and pull in unused SIM
# building models -> duplicate-class/moc conflicts). Mirrors OpenSRACommon.pri's Workflow entries.
include("${CMAKE_CURRENT_LIST_DIR}/OpenSRAWorkflow.cmake")
simcenter_add_opensra_workflow(${TARGET_NAME})

# ---- Include dirs ----
if(QCA_FOUND)
  target_include_directories(${TARGET_NAME} PRIVATE ${QCA_INCLUDE_DIR})
endif()
target_include_directories(${TARGET_NAME} PRIVATE
  ${QSCINTILLA_INCLUDE_DIR}
  ${QWT_INCLUDE_DIR}
)

# minizip/ZipUtils (SimCenterCommon) must see conan's STATIC zlib.h (plain symbols), not the
# vcpkg SHARED zlib.h that SimCenterQGIS adds to the include path (which yields __imp_* dllimport
# refs unsatisfied by the linked static conan zlib). BEFORE puts conan's zlib include first.
get_target_property(_opensra_zlib_inc ZLIB::ZLIB INTERFACE_INCLUDE_DIRECTORIES)
if(_opensra_zlib_inc)
  target_include_directories(${TARGET_NAME} BEFORE PRIVATE ${_opensra_zlib_inc})
endif()
message(STATUS "OpenSRA: prepended conan zlib include = ${_opensra_zlib_inc}")

# ---- Qt link libs ----
target_link_libraries(${TARGET_NAME} PRIVATE
  Qt6::Core Qt6::Gui Qt6::Widgets
  Qt6::Charts Qt6::Concurrent Qt6::Network Qt6::Sql Qt6::Xml
  Qt6::WebEngineWidgets
  Qt6::3DCore Qt6::3DRender Qt6::3DExtras
  Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Svg
  Qt6::Positioning Qt6::QuickWidgets
)

# qtkeychain (from the QGIS vcpkg tree) — linked only when available; required for link.
if(Qt6Keychain_FOUND)
  target_link_libraries(${TARGET_NAME} PRIVATE qt6keychain)
endif()

# ---- QGIS libraries (PARAMETERIZED; USER sets QGIS_LIB_DIR in Session 4) ----
# Empty default keeps the configure clean and skips QGIS linking until the fork is built.
set(QGIS_LIB_DIR "${PATH_TO_QGIS}/build/src" CACHE PATH "Path to built QGIS libs (Windows: <workspace>/QGIS/build/src; macOS: .../Frameworks)")
set(QGIS_VERSION "4.1.0" CACHE STRING "QGIS library version")

if(QGIS_LIB_DIR)
  if(APPLE)
    foreach(m app gui native analysis 3d core)
      target_link_libraries(${TARGET_NAME} PRIVATE "${QGIS_LIB_DIR}/libqgis_${m}.${QGIS_VERSION}.dylib")
    endforeach()
  elseif(WIN32)
    foreach(m app gui native core)
      target_link_libraries(${TARGET_NAME} PRIVATE "${QGIS_LIB_DIR}/${m}/qgis_${m}.lib")
    endforeach()
  endif()
else()
  message(STATUS "OpenSRA: QGIS_LIB_DIR not set -> skipping QGIS library linking (set it in Session 4).")
endif()

# ---- External libs (prefer find_package results) ----
target_link_libraries(${TARGET_NAME} PRIVATE
  ${QWT_LIBRARY}
  ${QSCINTILLA_LIBRARY}
  CURL::libcurl
  ZLIB::ZLIB
  jansson::jansson
  nlohmann_json::nlohmann_json
)

# minizip/ZipUtils (SimCenterCommon) compiles against the vcpkg (shared) zlib headers on the
# QGIS include path and references __imp_* zlib symbols that conan's STATIC zlib doesn't provide.
# Satisfy them with the vcpkg zlib import lib (zlib1.dll is already in-process via the QGIS libs).
if(WIN32 AND EXISTS "${PATH_TO_QGIS}/build/vcpkg_installed/x64-windows/lib/zlib.lib")
  target_link_libraries(${TARGET_NAME} PRIVATE "${PATH_TO_QGIS}/build/vcpkg_installed/x64-windows/lib/zlib.lib")
endif()

# QCA (from the QGIS vcpkg tree) — linked only when available; required for link.
if(QCA_FOUND)
  target_link_libraries(${TARGET_NAME} PRIVATE ${QCA_LIBRARY})
endif()

if(WIN32)
  target_link_libraries(${TARGET_NAME} PRIVATE Advapi32)
elseif(APPLE)
  target_link_libraries(${TARGET_NAME} PRIVATE lapack blas)
endif()

# Qt 6.10 auto-finalizes qt_add_executable targets; an explicit qt_finalize_executable here
# would double-finalize. Left to automatic finalization.
