cmake_minimum_required (VERSION 3.14)
set(CMAKE_CXX_STANDARD 20)
project(BORAINTERPRETER CXX OBJCXX)

set(SKIA_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../../global/contribs/bskia")
set(SKIA_BUILD_DIR "${CMAKE_SOURCE_DIR}/../../global/cached_libs/bskia")
include(${CMAKE_SOURCE_DIR}/../../global/cmake/skia.cmake)

if(WIN32)
    file(GLOB_RECURSE BORA_SOURCES_CONTRIB_TAZA ../../global/cpp/contribs/TAZA/code/*.cpp ../../global/cpp/contribs/TAZA/code/*.h ../../global/cpp/contribs/TAZA/code/*.hpp)
    file(GLOB_RECURSE BORA_SOURCES src/win32/*)
    file(GLOB_RECURSE BORA_SOURCES_3RDPARTYDLIB ../../global/libs/*.lib)
    file(GLOB_RECURSE BORA_SOURCES_3RDPARTYLIB ../../global/libs/*.dlib)

    include_directories(
            ../../global/cpp/contribs/TAZA/code
            ../../global/cpp/contribs
    )

    add_library(${PROJECT_NAME} SHARED
            ../../global/cpp/contribs/SysImageMgr.h
            ../../global/cpp/contribs/SysImageMgr.cpp
            ${BORA_SOURCES}
            ${BORA_SOURCES_CONTRIB_TAZA}
    )

    target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>>:/FI ${CMAKE_CURRENT_SOURCE_DIR}/../../global/cpp/contribs/TypeDefinitions.h>
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE
            TAZABASEDIR=\"${CMAKE_SOURCE_DIR}../global/cpp/contribs/TAZA/code\"
            NOMINMAX
    )

    # Specify Windows subsystem
    set_target_properties(${PROJECT_NAME} PROPERTIES
            OUTPUT_NAME "BORAINTERPRETERWIN32"
    )

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_libraries(${PROJECT_NAME} PRIVATE ${BORA_SOURCES_3RDPARTYDLIB})
    else ()
        target_link_libraries(${PROJECT_NAME} PRIVATE ${BORA_SOURCES_3RDPARTYLIB})
    endif()

endif()
# For macos code
if(APPLE)


    set(CMAKE_OSX_BUNDLE YES)
    set(CMAKE_OSX_BUNDLE_GUI_IDENTIFIER "com.bora.interpreter")
    set(CMAKE_OSX_BUNDLE_BUNDLE_VERSION "0.1.appleconfusion")
    set(CMAKE_OSX_BUNDLE_SHORT_VERSION_STRING "0.1")
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64") # universal

    file(GLOB_RECURSE BORA_SOURCES_CONTRIB_TAZA ../../global/cpp/contribs/TAZA/code/*.cpp ../../global/cpp/contribs/TAZA/code/*.h ../../global/cpp/contribs/TAZA/code/*.hpp)
    file(GLOB_RECURSE BORA_SOURCES src/osx/*)
    file(GLOB_RECURSE BORA_SOURCES_3RDPARTYDLIB ../../global/libs/__apple__/*.a ../../global/libs/__apple__x86/*.a)
    file(GLOB_RECURSE BORA_SOURCES_3RDPARTYLIB ../../global/libs/__apple__/*.a ../../global/libs/__apple__x86/*.a)



    include_directories(
            ../../global/cpp/contribs/TAZA/code
            ../../global/cpp/contribs
    )

    add_library(${PROJECT_NAME} MODULE
            ../../global/cpp/contribs/SysImageMgr.h
            ../../global/cpp/contribs/SysImageMgr.cpp
            ${BORA_SOURCES}
            ${BORA_SOURCES_CONTRIB_TAZA}
    )

    target_link_libraries(${PROJECT_NAME}
            ${BORA_SOURCES_3RDPARTYDLIB}
            "-framework CoreServices"
            "-framework CoreFoundation"
            "-framework CoreGraphics"
            "-framework Quartz"
    )


    target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:-include ${CMAKE_CURRENT_SOURCE_DIR}/../../global/cpp/contribs/TypeDefinitions.h>
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE
            TAZABASEDIR=\"${CMAKE_SOURCE_DIR}../global/cpp/contribs/TAZA/code\"
            NOMINMAX
    )

    # Specify Windows subsystem
    set_target_properties(${PROJECT_NAME} PROPERTIES
            OUTPUT_NAME "BORAINTERPRETEROSX"
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/src/osx/Info.plist
            BUNDLE TRUE
            BUNDLE_EXTENSION "qlgenerator"
            MACOSX_BUNDLE TRUE
    )


endif()