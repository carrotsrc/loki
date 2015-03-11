cmake_minimum_required (VERSION 2.6)
project (Loki)

set( GCC_CFLAG "-I/usr/include/libnl3 -I./ -ggdb" )

# Libs
set( EXTRA_LIBS ${EXTRA_LIBS} -lpcap -lnl-genl-3 -lnl-3 -lncurses -lpthread )

# Files
set( SRC 
	${CMAKE_CURRENT_SOURCE_DIR}/packet.c
	${CMAKE_CURRENT_SOURCE_DIR}/monitor.c
	${CMAKE_CURRENT_SOURCE_DIR}/frameproc.c
	${CMAKE_CURRENT_SOURCE_DIR}/capture.c
	${CMAKE_CURRENT_SOURCE_DIR}/state.c
	${CMAKE_CURRENT_SOURCE_DIR}/view/screen.c
	${CMAKE_CURRENT_SOURCE_DIR}/view/views.c
	${CMAKE_CURRENT_SOURCE_DIR}/controller/controllers.c
	${CMAKE_CURRENT_SOURCE_DIR}/init.c
	${CMAKE_CURRENT_SOURCE_DIR}/main.c
)

# Integration
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_CFLAG}" )

add_executable( loki ${SRC} )
target_link_libraries ( loki  ${EXTRA_LIBS} )
