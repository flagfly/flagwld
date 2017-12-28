link_directories ( /opt/jemalloc/lib/ )

add_executable(msgserver_test MsgServer_test.cc)
target_link_libraries(msgserver_test flagwld_base flagwld_net flagwld_msgframe libjemalloc.a dl)

add_executable(msgserver_test2 MsgServer_test2.cc)
target_link_libraries(msgserver_test2 flagwld_base flagwld_net flagwld_msgframe libjemalloc.a dl)

add_executable(msgclient_test MsgClient_test.cc)
target_link_libraries(msgclient_test flagwld_base flagwld_net flagwld_msgframe libjemalloc.a dl)

add_executable(urlparser_test UrlParser_test.cc)
target_link_libraries(urlparser_test flagwld_msgframe)

add_executable(lineserver_test LineServer_test.cc)
target_link_libraries(lineserver_test flagwld_base flagwld_net flagwld_msgframe libjemalloc.a dl)

add_executable(websocketserver_test WebSocketServer_test.cc)
target_link_libraries(websocketserver_test flagwld_base flagwld_net flagwld_msgframe flagwld_crypt flagwld_base64 libcrypto.a libz.a dl libjemalloc.a dl)

add_executable(ircclient_test IrcClient_test.cc)
target_link_libraries(ircclient_test flagwld_base flagwld_net flagwld_msgframe libjemalloc.a dl)
