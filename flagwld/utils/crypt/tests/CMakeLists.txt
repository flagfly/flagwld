include_directories( /opt/openssl/include/ )
link_directories ( /opt/openssl/lib /opt/libz/lib )

add_executable(rsa_test rsa_test.cc)
target_link_libraries(rsa_test flagwld_base flagwld_crypt libcrypto.a libz.a dl)

add_executable(rsa_test1 rsa_test1.cc)
target_link_libraries(rsa_test1 flagwld_crypt flagwld_base libcrypto.a libz.a dl)

add_executable(des_test des_test.cc)
target_link_libraries(des_test flagwld_base flagwld_crypt libcrypto.a libz.a dl)

add_executable(sha_test sha_test.cc)
target_link_libraries(sha_test flagwld_base flagwld_crypt libcrypto.a libz.a dl)

add_executable(md5_test md5_test.cc)
target_link_libraries(md5_test flagwld_base flagwld_crypt libcrypto.a libz.a dl)

add_executable(xxtea_test xxtea_test.cc)
target_link_libraries(xxtea_test flagwld_base flagwld_crypt flagwld_base62)
