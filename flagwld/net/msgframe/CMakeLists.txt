link_directories ( /opt/jemalloc/lib/ )

set(msg_SRCS
  TextLineParser.cc
  TextMsgParser.cc
  UrlParser.cc
  MsgBody.cc
  MsgRequest.cc
  MsgResponse.cc
  TextMsgCodec.cc
  TextMsgCodec2.cc
  TextLineCodec.cc
  WebSocketCodec.cc
  WebSocketFrame.cc
  WebSocketFrameParser.cc
  )

add_library(flagwld_msgframe ${msg_SRCS})
target_link_libraries(flagwld_msgframe)

install(TARGETS flagwld_msgframe DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/flagwld/net/msgframe)

add_subdirectory(tests)
