if (CONFIG_LIBSODIUM_CRYPTO_BOX)

zephyr_sources(
    crypto_box.c
    crypto_box_easy.c
    crypto_box_seal.c
)

add_subdirectory(curve25519xchacha20poly1305)
add_subdirectory(curve25519xsalsa20poly1305)
endif()



