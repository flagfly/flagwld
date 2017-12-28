set(libz_SRCS
  ZUtil.cc
  )

add_library(flagwld_z ${libz_SRCS})

install(TARGETS flagwld_z DESTINATION lib)
set(HEADERS
  ZUtil.h
  )
install(FILES ${HEADERS} DESTINATION include/flagwld/utils/zlib)

add_subdirectory(tests)
