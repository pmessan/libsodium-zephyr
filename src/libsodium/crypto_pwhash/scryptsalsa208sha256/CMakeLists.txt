zephyr_sources(
	crypto_scrypt-common.c
	pbkdf2-sha256.c
	pwhash_scryptsalsa208sha256.c
	scrypt_platform.c
)

add_subdirectory(nosse)
add_subdirectory(sse)