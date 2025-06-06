if (CONFIG_LIBSODIUM_CRYPTO_STREAM)
zephyr_sources(
    crypto_stream.c
)

add_subdirectory(chacha20)
add_subdirectory(salsa20)
add_subdirectory(salsa208)
add_subdirectory(salsa2012)
add_subdirectory(xchacha20)
add_subdirectory(xsalsa20)

endif()