cmake_minimum_required (VERSION 3.0)

set                   ( CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebinfo" CACHE STRING "limited configs" FORCE )

project               ( Launcher )
set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include               ( CMake-common )

find_package          ( QT5 REQUIRED  COMPONENTS Network WebKitWidgets)

add_subdirectory      ( "${CMAKE_CURRENT_LIST_DIR}/ThirdParty/quazip" ${CMAKE_CURRENT_BINARY_DIR}/quazip )
add_subdirectory      ( "${CMAKE_CURRENT_LIST_DIR}/ThirdParty/yaml" ${CMAKE_CURRENT_BINARY_DIR}/yaml )

include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )
include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )

include_directories   (.)             
include_directories   ( "Classes" )
include_directories   ( "ThirdParty/quazip" )
include_directories   ( "ThirdParty/yaml/include" )

link_directories      ("ThirdParty" )

add_definitions       ( -DQUAZIP_STATIC )


if ( WIN32 )
    set( LIB_RELEASE  "msvcrt.lib"  "zlib.lib"   )
    set( LIB_DEBUG    "msvcrtd.lib" "zlibd.lib"  )
endif()

file                  ( GLOB UI_LIST "DataQt/*.ui" )
qt5_wrap_ui           ( UI_APP_HDRS  ${UI_LIST} )
                   
file                  ( GLOB RCC_LIST "DataQt/*.qrc" )
qt5_add_resources     ( RCC_APP_SRCS  ${RCC_LIST} )

set                   ( Generated  ${UI_APP_HDRS} ${RCC_APP_SRCS} )

define_source_folders          ( SRC_ROOT "Classes"  )
generate_source_groups_project ( SRC_ROOT "Classes" "DataQt"  )

set( MACOS_PLIST          ${CMAKE_CURRENT_LIST_DIR}/Info.plist )
set( MACOS_ICO            ${CMAKE_CURRENT_LIST_DIR}/icon.icns  )

set( ADDED_SRC            ${UI_APP_HDRS} ${RCC_APP_SRCS}  Launcher.rc )

set( LIBRARIES            quazip yaml-cpp  )
set( LIBRARIES_RELEASE    ${LIB_RELEASE} )
set( LIBRARIES_DEBUG      ${LIB_DEBUG} )
set( ADDED_BINARY_DIR     ${QT5_PATH_WIN}/bin )

setup_main_executable()



