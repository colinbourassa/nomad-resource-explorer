cmake_minimum_required (VERSION 3.13.4)
project (nomad-resource-explorer)

set (NRE_VER_MAJOR 1)
set (NRE_VER_MINOR 4)
set (NRE_VER_PATCH 0)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  message (FATAL_ERROR "Error: This project does not currently support MSVC compilers due to the handling of struct packing attributes. Windows builds are supported via MXE or MinGW.")
endif ()

find_package (Qt5 COMPONENTS Core Widgets Multimedia REQUIRED)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS} -s")
endif ()

include_directories ("${CMAKE_SOURCE_DIR}"
                     "${Qt5Widgets_INCLUDE_DIRS}")

qt5_wrap_ui (UI_SOURCE mainwindow.ui)
qt5_wrap_ui (UI_SOURCE aboutbox.ui)
qt5_add_resources (QSS_RESOURCE style.qrc)
message (STATUS "Generated: ${QSS_RESOURCE}")

add_definitions ("-DNRE_VER_MAJOR=${NRE_VER_MAJOR}"
                 "-DNRE_VER_MINOR=${NRE_VER_MINOR}"
                 "-DNRE_VER_PATCH=${NRE_VER_PATCH}")

add_executable (nomad-resource-explorer
    main.cpp
    aboutbox.cpp
    aboutbox.h
    dattable.h
    mainwindow.cpp
    mainwindow.h
    datlibrary.cpp
    datlibrary.h
    invobject.cpp
    invobject.h
    palette.cpp
    palette.h
    places.cpp
    places.h
    imageconverter.cpp
    imageconverter.h
    aliens.cpp
    aliens.h
    placeclasses.cpp
    placeclasses.h
    audio.cpp
    audio.h
    ships.cpp
    ships.h
    shipinventory.cpp
    shipinventory.h
    shipclasses.cpp
    shipclasses.h
    facts.cpp
    facts.h
    tablenumberitem.cpp
    tablenumberitem.h
    gametext.cpp
    gametext.h
    fullscreenimages.cpp
    fullscreenimages.h
    conversationtext.cpp
    conversationtext.h
    stampimages.cpp
    stampimages.h
    ${QSS_RESOURCE}
    ${UI_SOURCE})

message (STATUS "Build type is: ${CMAKE_BUILD_TYPE}")

if (MINGW)
  message (STATUS "Found Windows/MinGW/MXE platform.")

  # prevent the final executable from launching a terminal window in parallel with the main GUI,
  # and force gcc to follow the struct packing attribute in the intended manner
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows -mno-ms-bitfields")

  # -fPIC appears to be redundant when building win32 binaries, so disable that flag
  string (REGEX REPLACE "-fPIC" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

  # When we call find_library() below, we're only looking for DLLs, so set the suffix list appropriately
  set (CMAKE_FIND_LIBRARY_SUFFIXES ".dll")
  message (STATUS "Limiting find_library() suffixes to ${CMAKE_FIND_LIBRARY_SUFFIXES}")

  find_library (LIBZSTD libzstd)
  if (LIBZSTD)
    message (STATUS "Found libzstd at ${LIBZSTD}")
  else ()
    message (SEND_ERROR "Could not find libzstd!")
  endif ()

  find_library (LIBGCC NAMES libgcc_s_sjlj-1 libgcc_s_seh-1 libgcc_s_dw2-1)
  if (LIBGCC)
    message (STATUS "Found libgcc at ${LIBGCC}")
  else ()
    message (SEND_ERROR "Could not find libgcc!")
  endif ()

  find_library (LIBSTDCPP libstdc++-6)
  if (LIBSTDCPP)
    message (STATUS "Found libstdc++ at ${LIBSTDCPP}")
  else ()
    message (SEND_ERROR "Could not find libstdc++!")
  endif ()

  find_library (LIBWINPTHREAD libwinpthread-1)
  if (LIBWINPTHREAD)
    message (STATUS "Found libwinpthread at ${LIBWINPTHREAD}")
  else ()
    message (SEND_ERROR "Could not find libwinpthread!")
  endif ()

  find_package (ZLIB REQUIRED)
  if (ZLIB_FOUND)
    message ("ZLIB location is ${ZLIB_LIBRARIES}")
  else ()
    message (SEND_ERROR "Could not find zlib!")
  endif ()

  get_target_property (QT5CORE_LIB Qt5::Core LOCATION)
  if (QT5CORE_LIB)
    message (STATUS "Qt5::Core location is ${QT5CORE_LIB}")
  else ()
    message (SEND_ERROR "Could not find Qt5Core library!")
  endif ()

  get_target_property (QT5WIDGETS_LIB Qt5::Widgets LOCATION)
  if (QT5WIDGETS_LIB)
    message (STATUS "Qt5::Widgets location is ${QT5WIDGETS_LIB}")
  else ()
    message (SEND_ERROR "Could not find Qt5Widgets library!")
  endif ()

  get_target_property (QT5GUI_LIB Qt5::Gui LOCATION)
  if (QT5GUI_LIB)
    message (STATUS "Qt5::Gui location is ${QT5GUI_LIB}")
  else ()
    message (SEND_ERROR "Could not find Qt5Gui library!")
  endif ()

  get_target_property (QT5MULTIMEDIA_LIB Qt5::Multimedia LOCATION)
  if (QT5MULTIMEDIA_LIB)
    message (STATUS "Qt5::Multimedia location is ${QT5MULTIMEDIA_LIB}")
  else ()
    message (SEND_ERROR "Could not find Qt5Multimedia library!")
  endif ()

  get_target_property (QT5NETWORK_LIB Qt5::Network LOCATION)
  if (QT5NETWORK_LIB)
    message (STATUS "Qt5::Network location is ${QT5NETWORK_LIB}")
  else ()
    message (SEND_ERROR "Could not find Qt5Network library!")
  endif ()

  get_target_property (QT5WINDOWS_LIB Qt5::QWindowsIntegrationPlugin LOCATION)
  if (QT5WINDOWS_LIB)
    message (STATUS "Qt5::QWindows location is ${QT5WINDOWS_LIB}")
  else ()
    message (SEND_ERROR "Could not find Qt5 Windows GUI plugin!")
  endif ()

  get_target_property (QT5WINDOWSAUDIO_LIB Qt5::QWindowsAudioPlugin LOCATION)
  if (QT5WINDOWSAUDIO_LIB)
    message (STATUS "Qt5::QWindowsAudio location is ${QT5WINDOWSAUDIO_LIB}")
  else ()
    message (SEND_ERROR "Could not find Qt5 Windows Audio plugin!")
  endif ()

  get_target_property (QT5WINDOWSVISTA_LIB Qt5::QWindowsVistaStylePlugin LOCATION)
  if (QT5WINDOWSVISTA_LIB)
    message (STATUS "Qt5::QWindowsVistaStyle location is ${QT5WINDOWSVISTA_LIB}")
  else ()
    message (WARNING "Could not find Qt5 Windows Vista style GUI plugin!")
  endif ()

  target_link_libraries (nomad-resource-explorer Qt5::Widgets Qt5::Multimedia)

  install (FILES "${CMAKE_BINARY_DIR}/nomad-resource-explorer.exe"
                  ${LIBGCC}
                  ${LIBSTDCPP}
                  ${LIBWINPTHREAD}
                  ${LIBZSTD}
                  ${ZLIB_LIBRARIES}
                  ${QT5CORE_LIB}
                  ${QT5WIDGETS_LIB}
                  ${QT5MULTIMEDIA_LIB}
                  ${QT5NETWORK_LIB}
                  ${QT5GUI_LIB}
           DESTINATION ".")
  install (FILES ${QT5WINDOWS_LIB}      DESTINATION "platforms")
  if (QT5WINDOWSAUDIO_LIB)
    install (FILES ${QT5WINDOWSAUDIO_LIB} DESTINATION "audio")
  endif ()
  if (QT5WINDOWSVISTA_LIB)
    install (FILES ${QT5WINDOWSVISTA_LIB} DESTINATION "styles")
  endif ()

  set (CPACK_GENERATOR "ZIP")
  set (CPACK_PACKAGE_NAME ${PROJECT_NAME})
  set (CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
  set (CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${NRE_VER_MAJOR}.${NRE_VER_MINOR}.${NRE_VER_PATCH}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
  include (CPack)

else()
  message (STATUS "Defaulting to Linux build environment.")

  target_link_libraries (nomad-resource-explorer Qt5::Widgets Qt5::Multimedia)

  set (CMAKE_SKIP_RPATH TRUE)
  set (CMAKE_INSTALL_PREFIX "/usr")

  # set the installation destinations for the header files,
  # shared library binaries, and reference utility
  install (FILES "${CMAKE_CURRENT_BINARY_DIR}/nomad-resource-explorer"
           DESTINATION "bin"
           PERMISSIONS
            OWNER_READ OWNER_EXECUTE OWNER_WRITE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)

  # set the architecture flag that will be used by the Debian package builder
  if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set (CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
  elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
    set (CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386)
  endif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")

  set (CPACK_GENERATOR "DEB")
  set (CPACK_PACKAGE_NAME ${PROJECT_NAME})
  set (CPACK_PACKAGE_VERSION_MAJOR ${NRE_VER_MAJOR})
  set (CPACK_PACKAGE_VERSION_MINOR ${NRE_VER_MINOR})
  set (CPACK_PACKAGE_VERSION_PATCH ${NRE_VER_PATCH})
  set (CPACK_DEBIAN_PACKAGE_MAINTAINER "Colin Bourassa <colin.bourassa@gmail.com>")
  set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Graphical data file explorer for the game resources from the 1993 space trading adventure 'Nomad'")
  set (CPACK_DEBIAN_PACKAGE_SECTION "Miscellaneous")
  set (CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.13), libstdc++6 (>= 4.6.3), libqt5core5 (>= 5.12.4) | libqt5core5a (>= 5.12.4), libqt5gui5 (>= 5.12.4), libqt5widgets5 (>= 5.12.4), libqt5network5 (>= 5.12.4), libqt5multimedia5 (>= 5.12.4)")
  set (CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${NRE_VER_MAJOR}.${NRE_VER_MINOR}.${NRE_VER_PATCH}-${CMAKE_SYSTEM_NAME}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
  set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

  include(CPack)

endif()

