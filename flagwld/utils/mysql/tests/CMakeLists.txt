add_executable(mysql_test mysql_test.cc)
target_link_libraries(mysql_test flagwld_base flagwld_mysql mysqlclient)

