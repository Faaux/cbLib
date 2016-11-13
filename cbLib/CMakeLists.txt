cmake_minimum_required (VERSION 3.6)
project (cbLib)
 
set (SOURCE_FILES 	cbPlatform.cpp)
set (HEADER_FILES	cbBasic.h
					cbDebug.h
					cbInclude.h
					cbKeys.h
					cbMemory.h
					cbPlatform.h)

# add the executable
add_executable(cbLib ${SOURCE_FILES} ${HEADER_FILES})
configure_file(cbLib.vcxproj.user.in ${CMAKE_BINARY_DIR}/cbLib/cbLib.vcxproj.user @ONLY) 
target_link_libraries(cbLib glew32 opengl32 cbGame)