set(mysql_SRCS
  MysqlUtil.cc
  )

add_library(flagwld_mysql ${mysql_SRCS})

install(TARGETS flagwld_mysql DESTINATION lib)
set(HEADERS
  MysqlUtil.h
  )
install(FILES ${HEADERS} DESTINATION include/flagwld/utils/mysql)

add_subdirectory(tests)
