# CMakeLists.txt for libsodium integration with Zephyr

# Only build for mark target, not mcuboot
if(CONFIG_LIBSODIUM)
    message(STATUS "Building libsodium for Zephyr")
    # Setup basic variables
    set(LIBSODIUM_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
    set(LIBSODIUM_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
    set(LIBSODIUM_INSTALL_DIR ${LIBSODIUM_BUILD_DIR}/install)
    set(LIBSODIUM_LIB_FILE ${LIBSODIUM_INSTALL_DIR}/lib/libsodium.a)

    include(ExternalProject)

    message(STATUS "Setting up libsodium build for Zephyr")
    message(STATUS "Target board: ${BOARD}")

    # Determine the appropriate toolchain based on the board
    if(BOARD MATCHES "qemu_x86")
        set(TOOLCHAIN_PREFIX "x86_64-zephyr-elf")
        set(TOOLCHAIN_HOST "x86_64-zephyr-elf")
        set(ARCH_CFLAGS "-m32 -march=i586 -mfpmath=sse -msse2 -ffunction-sections -fdata-sections -g -fno-pic -fno-pie")
    elseif(CONFIG_CPU_CORTEX_M4)
        set(TOOLCHAIN_PREFIX "arm-zephyr-eabi")
        set(TOOLCHAIN_HOST "arm-zephyr-eabi")
        set(ARCH_CFLAGS "-mcpu=cortex-m4 -mthumb -mabi=aapcs -mfloat-abi=soft -ffunction-sections -fdata-sections -g -fno-common -fmessage-length=0")
    else()
        # Default to ARM (can be expanded for other architectures)
        set(TOOLCHAIN_PREFIX "arm-zephyr-eabi")
        set(TOOLCHAIN_HOST "arm-zephyr-eabi")
        set(ARCH_CFLAGS "-ffunction-sections -fdata-sections -g -fno-common -fmessage-length=0")
        message(WARNING "Unknown board type: ${BOARD}, defaulting to ARM toolchain")
    endif()

    # Set up the toolchain variables
    set(TOOLCHAIN_CC "${ZEPHYR_SDK_INSTALL_DIR}/${TOOLCHAIN_PREFIX}/bin/${TOOLCHAIN_PREFIX}-gcc")
    set(TOOLCHAIN_CXX "${ZEPHYR_SDK_INSTALL_DIR}/${TOOLCHAIN_PREFIX}/bin/${TOOLCHAIN_PREFIX}-g++")
    set(TOOLCHAIN_AR "${ZEPHYR_SDK_INSTALL_DIR}/${TOOLCHAIN_PREFIX}/bin/${TOOLCHAIN_PREFIX}-ar")
    set(TOOLCHAIN_AS "${ZEPHYR_SDK_INSTALL_DIR}/${TOOLCHAIN_PREFIX}/bin/${TOOLCHAIN_PREFIX}-as")
    set(TOOLCHAIN_LD "${ZEPHYR_SDK_INSTALL_DIR}/${TOOLCHAIN_PREFIX}/bin/${TOOLCHAIN_PREFIX}-ld")
    set(TOOLCHAIN_RANLIB "${ZEPHYR_SDK_INSTALL_DIR}/${TOOLCHAIN_PREFIX}/bin/${TOOLCHAIN_PREFIX}-ranlib")
    set(TOOLCHAIN_STRIP "${ZEPHYR_SDK_INSTALL_DIR}/${TOOLCHAIN_PREFIX}/bin/${TOOLCHAIN_PREFIX}-strip")

    message(STATUS "Using toolchain prefix: ${TOOLCHAIN_PREFIX}")
    message(STATUS "Using CFLAGS: ${ARCH_CFLAGS}")

    # Prepare the build environment - create script to set environment variables
    file(WRITE ${LIBSODIUM_BUILD_DIR}/build_env.sh "#!/bin/bash
export CC=\"${TOOLCHAIN_CC}\"
export CXX=\"${TOOLCHAIN_CXX}\"
export AR=\"${TOOLCHAIN_AR}\"
export AS=\"${TOOLCHAIN_AS}\"
export LD=\"${TOOLCHAIN_LD}\"
export RANLIB=\"${TOOLCHAIN_RANLIB}\"
export STRIP=\"${TOOLCHAIN_STRIP}\"
export CFLAGS=\"${ARCH_CFLAGS}\"
export LDFLAGS=\"-nostartfiles -nodefaultlibs -nostdlib\"
")
    execute_process(COMMAND chmod +x ${LIBSODIUM_BUILD_DIR}/build_env.sh)

    message(STATUS "Building libsodium now during configure time...")

    message(STATUS "Configuring the build...")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E env bash -c "source ${LIBSODIUM_BUILD_DIR}/build_env.sh && cd ${LIBSODIUM_ROOT_DIR} && [ -f ./configure ] || autoreconf -i"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${LIBSODIUM_BUILD_DIR}/build
        COMMAND ${CMAKE_COMMAND} -E chdir ${LIBSODIUM_BUILD_DIR}/build ${CMAKE_COMMAND} -E env bash -c "source ${LIBSODIUM_BUILD_DIR}/build_env.sh && ${LIBSODIUM_ROOT_DIR}/configure --prefix=${LIBSODIUM_INSTALL_DIR} --host=${TOOLCHAIN_HOST} --enable-static --disable-shared --disable-ssp --disable-asm --disable-pie --with-pic=no --enable-minimal"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
    )
    if(NOT result EQUAL 0)
        message(STATUS "Output: ${output}")
        message(STATUS "Error: ${error}")
        message(FATAL_ERROR "Failed to build libsodium during configure phase")
    endif()

    message(STATUS "Building ...")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E chdir ${LIBSODIUM_BUILD_DIR}/build ${CMAKE_COMMAND} -E env bash -c "echo 'make' && source ${LIBSODIUM_BUILD_DIR}/build_env.sh && make -j$(nproc)"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
    )
    if(NOT result EQUAL 0)
        message(STATUS "Output: ${output}")
        message(STATUS "Error: ${error}")
        message(FATAL_ERROR "Failed to build libsodium during configure phase")
    endif()

    message(STATUS "Installing built files")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E chdir ${LIBSODIUM_BUILD_DIR}/build ${CMAKE_COMMAND} -E env bash -c "echo 'make install' && source ${LIBSODIUM_BUILD_DIR}/build_env.sh && make install"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
    )
    if(NOT result EQUAL 0)
        message(STATUS "Output: ${output}")
        message(STATUS "Error: ${error}")
        message(FATAL_ERROR "Failed to build libsodium during configure phase")
    endif()

    # Verify that the library file exists
    if(NOT EXISTS ${LIBSODIUM_LIB_FILE})
        message(FATAL_ERROR "libsodium library was not built successfully. File not found: ${LIBSODIUM_LIB_FILE}")
    endif()

    message(STATUS "Successfully built libsodium at ${LIBSODIUM_LIB_FILE}")

    # Create imported library target
    add_library(libsodium STATIC IMPORTED GLOBAL)
    set_target_properties(libsodium PROPERTIES
        IMPORTED_LOCATION ${LIBSODIUM_LIB_FILE}
        INTERFACE_INCLUDE_DIRECTORIES ${LIBSODIUM_INSTALL_DIR}/include
    )

    # Make the library's headers available to Zephyr applications
    zephyr_include_directories(${LIBSODIUM_INSTALL_DIR}/include)

    # Link with Zephyr
    zephyr_link_libraries(${LIBSODIUM_LIB_FILE})

    message(STATUS "libsodium build configured")
else()
    message(WARNING "libsodium build not configured, CONFIG_LIBSODIUM is not set")
endif()