#include(CheckFunctionExists)

#check_function_exists(accept4 HAVE_ACCEPT4)
#if(NOT HAVE_ACCEPT4)
#  set_source_files_properties(SocketsOps.cc PROPERTIES COMPILE_FLAGS "-DNO_ACCEPT4")
#endif()

set(net_SRCS
  Signal.cc
  Async.cc
  Idle.cc
  Timer.cc
  Periodic.cc
  Child.cc
  EventLoop.cc
  EventLoopThread.cc
  EventLoopThreadPool.cc
  Channel.cc
  Buffer.cc
  SocketsOps.cc
  SockAddress.cc
  detail/Socket.cc
  detail/Acceptor.cc
  detail/Connector.cc
  SockConnection.cc
  SockServer.cc
  SockClient.cc
  )

add_library(flagwld_net ${net_SRCS})
#target_link_libraries(flagwld_net flagwld_base ev)
set_property(TARGET flagwld_net PROPERTY LINK_SEARCH_START_STATIC 1)
target_link_libraries(flagwld_net flagwld_base libev.a)

add_library(flagwld_net_cpp11 ${net_SRCS})
target_link_libraries(flagwld_net_cpp11 flagwld_base_cpp11 libev.a)
set_target_properties(flagwld_net_cpp11 PROPERTIES COMPILE_FLAGS "-std=c++0x")

install(TARGETS flagwld_net DESTINATION lib)
install(TARGETS flagwld_net_cpp11 DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/flagwld/net)

add_subdirectory(msgframe)
