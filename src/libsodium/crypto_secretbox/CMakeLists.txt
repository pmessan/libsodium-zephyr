zephyr_sources(
    crypto_secretbox.c
    crypto_secretbox_easy.c
)

add_subdirectory(xchacha20poly1305)
add_subdirectory(xsalsa20poly1305)