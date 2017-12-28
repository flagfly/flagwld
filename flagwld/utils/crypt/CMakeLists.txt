include_directories( /opt/libz/include )
link_directories ( /opt/libz/include/lib )

include_directories( /opt/openssl/include )
link_directories ( /opt/openssl/include/lib )

set(libcrypt_SRCS
  RSAUtil.cc
  DESUtil.cc
  SHAUtil.cc
  MD5Util.cc
  XXTEAUtil.cc
  xxtea/xxtea.c
  )

add_library(flagwld_crypt ${libcrypt_SRCS})

install(TARGETS flagwld_crypt DESTINATION lib)
set(HEADERS
  RSAUtil.h
  DESUtil.h
  SHAUtil.h
  MD5Util.h
  XXTEAUtil.h
  )
install(FILES ${HEADERS} DESTINATION include/flagwld/utils/crypt)

add_subdirectory(tests)
