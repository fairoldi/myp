cmake_minimum_required(VERSION 3.17)
project(myp C)

set(CMAKE_C_STANDARD 99)

find_package(PkgConfig)
pkg_check_modules(GTK3 gtk+-3.0)
pkg_check_modules(INDICATOR3 appindicator3-0.1)
pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(PULSEAUDIO libpulse)

set(CFLAGS
        ${GTK3_CFLAGS} ${GTK3_CFLAGS_OTHER}
        ${X11_CFLAGS} ${X11_CFLAGS_OTHER}
        -s -O3 -w
        #	-g
        )


# notify application of presence of libappindicator
if (${INDICATOR_FOUND})
    LIST(APPEND CFLAGS -DHAVE_APPINDICATOR)
endif (${INDICATOR_FOUND})
if (${INDICATOR3_FOUND})
    LIST(APPEND CFLAGS -DHAVE_APPINDICATOR)
endif (${INDICATOR3_FOUND})

add_definitions(${CFLAGS})

set(LIBS
        ${GTK3_LIBRARIES}
        ${INDICATOR_LIBRARIES}
        ${INDICATOR3_LIBRARIES}
        ${X11_LIBRARIES}
        ${PULSEAUDIO_LIBRARIES}
        )
link_libraries(${LIBS})

set(LIB_PATHS
        ${GTK3_LIBRARY_DIRS}
        ${INDICATOR_LIBRARY_DIRS}
        ${INDICATOR3_LIBRARY_DIRS}
        ${X11_LIBRARY_DIRS}
        ${PULSEAUDIO_LIBRARY_DIRS}
        )
link_directories(${LIB_PATHS})

set(INCLUDE_PATHS
        ${GTK3_INCLUDE_DIRS}
        ${INDICATOR_INCLUDE_DIRS}
        ${INDICATOR3_INCLUDE_DIRS}
        ${X11_INCLUDE_DIRS}
        ${PULSEAUDIO_INCLUDE_DIRS}
        )
include_directories(${INCLUDE_PATHS})

add_executable(myp main.c tray.h pulse.c pulse.h)