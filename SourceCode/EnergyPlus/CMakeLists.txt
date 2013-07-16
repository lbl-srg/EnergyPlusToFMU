
INCLUDE_DIRECTORIES( "${CMAKE_SOURCE_DIR}/SourceCode/Expat/lib")
#INCLUDE_DIRECTORIES( "${CMAKE_SOURCE_DIR}/SourceCode/")

SET(SRC 
  main.c
  stack.c
  stack.h
  util.c
  util.h
  xml_parser_cosim.c
  xml_parser_cosim.h
  fmiFunctions.h
  fmiModelTypes.h
  reader.h
  reader.c
  utilSocket.c
  utilSocket.h
  defines.h
)


ADD_LIBRARY( epfmiexport SHARED ${SRC} )
IF( WIN32 )
LIST( APPEND SRC "dirent_win.h" )
ENDIF()
#ADD_EXECUTABLE( epfmiexport ${SRC} )

TARGET_LINK_LIBRARIES( epfmiexport epexpat)

INSTALL( TARGETS epfmiexport 
  RUNTIME DESTINATION bin 
  LIBRARY DESTINATION lib 
)

if( UNIX )
set_target_properties(
  epfmiexport
  PROPERTIES
  INSTALL_NAME_DIR "@executable_path/../lib"
)
endif()
