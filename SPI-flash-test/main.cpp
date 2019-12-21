#include "mbed.h"
#include "LittleFileSystem.h"
#include "SPIFBlockDevicePD.h"

#include "pindefs.h"

// Physical block device, can be any device that supports the BlockDevice API
SPIFBlockDevicePD bd(FLASH_MOSI, FLASH_MISO, FLASH_SCK, FLASH_CS, 24000000);

// Storage for the littlefs
LittleFileSystem fs("fs");
DigitalOut resetter(RESET_CONTROLLER_EN);

// Entry point
int main() {
    resetter = 0;
    // Mount the filesystem
    int err = fs.mount(&bd);
    if (err) {
        // Reformat if we can't mount the filesystem,
        // this should only happen on the first boot
        LittleFileSystem::format(&bd);
        fs.mount(&bd);
    }

    // Read the boot count
    uint32_t boot_count = 0;
    FILE *f = fopen("/fs/boot_count", "r+");
    if (!f) {
        // Create the file if it doesn't exist
        f = fopen("/fs/boot_count", "w+");
    }
    fread(&boot_count, sizeof(boot_count), 1, f);

    // Update the boot count
    boot_count += 1;
    rewind(f);
    fwrite(&boot_count, sizeof(boot_count), 1, f);

    // Remember that storage may not be updated until the file
    // is closed successfully
    fclose(f);

    // Release any resources we were using
    fs.unmount();

    // Print the boot count
    printf("boot_count: %ld\n", boot_count);
}