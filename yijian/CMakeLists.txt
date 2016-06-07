
# .so .a create & install
#aux_source_directory(. DIR_YIJIAN_SOURCE)

#add_library(yijian SHARED ${DIR_YIJIAN_SOURCE})
#add_library(yijian_static STATIC ${DIR_YIJIAN_SOURCE})

#set_target_properties(yijian_static PROPERTIES OUTPUT_NAME "yijian")
#set_target_properties(yijian PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#set_target_properties(yijian_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

#set_target_properties(yijian PROPERTIES version 1 soversion 1)

#install(TARGETS yijian yijian_static
#	LIBRARY DESTINATION lib
#	ARCHIVE DESTINATION lib)
# .so .a create & install 
file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/yijian)

