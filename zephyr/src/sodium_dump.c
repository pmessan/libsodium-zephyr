/**
 * Zephyr integration for libsodium
*/

#include <zephyr/kernel.h>
#include <zephyr/random/rand32.h>
#include <sodium/randombytes.h>
#include <sodium/core.h>
#include <sodium/utils.h>


#ifdef CONFIG_LIBSODIUM_CUSTOM_MEMORY
/* Override libsodium's memory allocation to use Zephyr's memory allocator */
void *sodium_malloc(size_t size)
{
    return k_malloc(size);
}

void sodium_free(void *ptr)
{
    k_free(ptr);
    return;
}

int sodium_mprotect_noaccess(void *ptr)
{
    /* Not supported on Zephyr */
    return 0;
}

int sodium_mprotect_readonly(void *ptr)
{
    /* Not supported on Zephyr */
    return 0;
}

int sodium_mprotect_readwrite(void *ptr)
{
    /* Not supported on Zephyr */
    return 0;
}
#endif /* CONFIG_LIBSODIUM_CUSTOM_MEMORY */

/* Initialize our custom RNG when the module starts */
static int libsodium_zephyr_init(void)
{
    int ret = 0;
    
#ifdef CONFIG_LIBSODIUM_USE_ZEPHYR_RANDOM
    /* Check if entropy source is available */
#ifdef CONFIG_ENTROPY_HAS_DRIVER
    const struct device *entropy = DEVICE_DT_GET(DT_CHOSEN_ZEPHYR_ENTROPY_LABEL);
    if (!device_is_ready(entropy)) {
        printk("ERROR: Entropy device not ready\n");
        return -ENODEV;
    }
#elif defined(CONFIG_TEST_RANDOM_GENERATOR)
    printk("WARNING: Using test random generator, not suitable for production!\n");
#else
    printk("ERROR: No entropy source available for libsodium\n");
    return -ENOTSUP;
#endif
    
    /* Set Zephyr's RNG as the default for libsodium */
    randombytes_set_implementation(&zephyr_randombytes_impl);
#endif

    /* Initialize libsodium */
    if (sodium_init() < 0) {
        printk("ERROR: Failed to initialize libsodium\n");
        return -EIO;
    }

    printk("libsodium initialized successfully\n");
    return ret;
}

SYS_INIT(libsodium_zephyr_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);