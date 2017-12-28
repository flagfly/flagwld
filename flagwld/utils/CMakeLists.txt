set(base62_SRCS
  base62.cc
  )

add_library(flagwld_base62 ${base62_SRCS})
target_link_libraries(flagwld_base62)

install(TARGETS flagwld_base62 DESTINATION lib)
set(HEADERS
  base62.h
  )
install(FILES ${HEADERS} DESTINATION include/flagwld/utils)

set(base64_SRCS
  base64.cc
  )

add_library(flagwld_base64 ${base64_SRCS})
target_link_libraries(flagwld_base64)

install(TARGETS flagwld_base64 DESTINATION lib)
set(HEADERS
  base64.h
  )
install(FILES ${HEADERS} DESTINATION include/flagwld/utils)

set(urlcode_SRCS
  urlcode.cc
  )

add_library(flagwld_urlcode ${urlcode_SRCS})
target_link_libraries(flagwld_urlcode)

install(TARGETS flagwld_urlcode DESTINATION lib)
set(HEADERS
  urlcode.h
  )
install(FILES ${HEADERS} DESTINATION include/flagwld/utils)

set(realpath_SRCS
  realpath.cc
  )

add_library(flagwld_realpath ${realpath_SRCS})
target_link_libraries(flagwld_realpath)

install(TARGETS flagwld_realpath DESTINATION lib)
set(HEADERS
  realpath.h
  )
install(FILES ${HEADERS} DESTINATION include/flagwld/utils)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/flagwld/utils)

file(GLOB SRCS "*.cc")
install(FILES ${SRCS} DESTINATION include/flagwld/utils)

add_subdirectory(mysql)
add_subdirectory(zlib)
add_subdirectory(crypt)

