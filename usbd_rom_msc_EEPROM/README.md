usbd_rom_msc_EEPROM 
================

When connected to PC, the board enumerates as mass storage device, and presents itself as 4Kb FAT formated disk.

In the root directory, there is file "EEPROM.BIN" which has the size of 4032 bytes, and contains EEPROM bytes.
(The last 64 bytes of 4Kb EEPROM is reserved, see UM10736 LPC15xx User manual, Rev. 1.1, page 577).

The file can be edited with a hex editor, and EEPROM is updated, when the file is saved.
