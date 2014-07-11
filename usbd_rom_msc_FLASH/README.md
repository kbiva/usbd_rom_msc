usbd_rom_msc_FLASH 
================

When connected to PC, the board enumerates as mass storage device, and presents itself as 258Kb FAT formated disk.

In the root directory, there is file "FIRMWARE.BIN" which has the size of 262144 bytes (256Kb), and contains image of microcontroller flash memory. The file can be copied to PC. 

The file can be edited, deleted, but there will be no writing back to the microcontroller flash memory.

This result of running example is simillar to what you see when running USB bootloader, except that there will never be writing to the flash memory.


