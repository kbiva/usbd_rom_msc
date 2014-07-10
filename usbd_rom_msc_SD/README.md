usbd_rom_msc_SD
================

I used SD Card module from "LC studio".
When connected to PC, the board enumerates as mass storage device, and presents itself 
as a disk. The contents of the disk will be whatever is on the connected SD/MMC memory card.
The result is USB Card reader, but with rather slow speed. The speed depends on the memory card,
and can be ~265-435kb/s for reading, and 136-360kb/s for writing.
The memory card information from CID and CSD registers is written to UART, and can be viewed in
connected terminal.
Also there is 4Gb card size limit, because I couldn't figure out how to work with bigger sizes.

The cards tested:
| Type | Format | Version | Brand | Size | Size reported, MB | Date | SPI read kb/s | USB read kb/s | USB write kb/s |
|MultiMediaCard|MMCmobile| | |64Mb|61|2007-06|906|435|360|
|Secure Digital|microSD|SD2.0|Samsung|2Gb|1936|2009-10|851|420|210|
|Secure Digital|microSD|SD2.0|noname|1Gb|922|2009-12|801|325|55|
|Secure Digital|microSD|SD2.0|Nokia|128Mb|120|2007-06|723|382|240|
|MultiMediaCard|MMC| |Canon|16Mb|15|2007-11|653|377|139|
|Secure Digital|microSD|SD2.0|Sandisk|2Gb|1886|2009-11|617|369|182|
|Secure Digital|microSD|SD2.0|Sandisk|1Gb|942|2011-01|613|358|168|
|Secure Digital|SD|SD2.0|Silicon Power|1Gb|972|2007-07|601|370|138|
|Secure Digital|microSD|SDHC|Kingston|4Gb|3768|2013-07|534|282|136|
|MultiMediaCard|MMC| |Sandisk|16Mb|15|2003-06|391|265|305|
|Secure Digital|SD|SD2.0|Kingston|2Gb|1882|2011-01|282|325|13|

SPI speed was masured reading first 1000 sectors from the card.
USB speeds were measured copying 40Mb file from PC to the card and from the card to the PC.
For 16Mb cards file was 15Mb.