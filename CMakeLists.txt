cmake_minimum_required(VERSION 3.17)
project(myp C)

set(CMAKE_C_STANDARD 99)

find_package(PkgConfig)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(XCB REQUIRED xcb)
pkg_check_modules(PULSEAUDIO REQUIRED libpulse)
pkg_check_modules(PULSEAUDIO_GLIB REQUIRED libpulse-mainloop-glib)
pkg_check_modules(INDICATOR3 REQUIRED appindicator3-0.1)

#set(CFLAGS
#        ${GTK3_CFLAGS} ${GTK3_CFLAGS_OTHER}
#        ${X11_CFLAGS} ${X11_CFLAGS_OTHER}
#        -s -O3 -w
#        #	-g
#        )


add_definitions(${CFLAGS})

set(LIBS
        ${GTK3_LIBRARIES}
        ${X11_LIBRARIES}
        ${PULSEAUDIO_LIBRARIES}
        ${PULSEAUDIO_GLIB_LIBRARIES}
        ${INDICATOR3_LIBRARIES}
        ${XCB_LIBRARIES}
        )
link_libraries(${LIBS})

set(LIB_PATHS
        ${GTK3_LIBRARY_DIRS}
        ${X11_LIBRARY_DIRS}
        ${PULSEAUDIO_LIBRARY_DIRS}
        ${PULSEAUDIO_GLIB_LIBRARY_DIRS}
        ${INDICATOR3_LIBRARY_DIRS}
        ${XCB_LIBRARY_DIRS}
        )
link_directories(${LIB_PATHS})

set(INCLUDE_PATHS
        ${GTK3_INCLUDE_DIRS}
        ${X11_INCLUDE_DIRS}
        ${PULSEAUDIO_INCLUDE_DIRS}
        ${INDICATOR3_INCLUDE_DIRS}
        ${XCB_INCLUDE_DIRS}
        )
include_directories(${INCLUDE_PATHS})

add_executable(myp main.c)