//////////////////////////////////////////////////
// rBoot open source boot loader for ESP8266.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
//////////////////////////////////////////////////

#ifdef RBOOT_INTEGRATION
#include <rboot-integration.h>
#endif

#include "rboot-private.h"
#include <rboot-hex2a.h>

void sleep(void) {
	uint8 loop;
	for(loop = 2; loop > 0; loop--) {
		ets_printf("sleep %d\r\n", loop);
		ets_delay_us(1000000);
	}
}

// Found magic at 8192.
// Found magic at 131488.
// Found magic at 393952.
// Found magic at 532480.
// Found magic at 656096.

// Found magic at 8192.
// Found magic at 294912.
// Booting at ram 550432.

// Found magic at 8192.
// Header size: 255520
// Booting at ram 263712.

// Found magic at 8192.
// Found magic at 294912.
// Header size: 255520
// Booting at ram 550432.


static uint32 search(uint32 readpos, uint8 number) {
	uint32 romaddr = 0;
	uint8 buffer[2];

	// for(;readpos < 0x100000;readpos++) {
	for(;readpos < 0x100000;readpos += SECTOR_SIZE) {
		if (buffer[0] == ROM_MAGIC_NEW1 && buffer[1] == ROM_MAGIC_NEW2) {
			ets_printf("Found magic at %d.\r\n", readpos);
			romaddr = readpos;
			if (number == 0) {
				ets_printf("Start here.\r\n");
				break;
			} else {
				number--;
			}
		}
	}
	// wifi_station_get_connect_status();
	// wifi_station_connect();
	return romaddr;
}

static uint32 check_image(uint32 readpos) {
	uint8 buffer[BUFFER_SIZE];
	uint8 sectcount;
	uint8 sectcurrent;
	uint8 *writepos;
	uint8 chksum = CHKSUM_INIT;
	uint32 loop;
	uint32 remaining;
	uint32 romaddr;

	rom_header_new *header = (rom_header_new*)buffer;
	// section_header *section = (section_header*)buffer;

	if (SPIRead(readpos, header, sizeof(rom_header_new)) != 0) {
		return 0;
	}

	ets_printf("Header size: %d\r\n", header->len + sizeof(rom_header_new));
	// new type, has extra header and irom section first
	romaddr = readpos + header->len + sizeof(rom_header_new);

	return romaddr;
}

#define ROM_TO_BOOT_ON 1

void write_romconf(uint32 romAddr) {
	uint8 buffer[SECTOR_SIZE];

	rboot_config *romconf = (rboot_config*)buffer;
	// rom_header *header = (rom_header*)buffer;

	// read rom header
	// SPIRead(0, header, sizeof(rom_header));
	// flashsize = 0x100000;
	uint32 flashsize = 0x200000;

	// read boot config
	SPIRead(BOOT_CONFIG_SECTOR * SECTOR_SIZE, buffer, SECTOR_SIZE);

	// create a default config for a standard 2 rom setup
	ets_printf("Writing default boot config.\r\n");
	ets_memset(romconf, 0x00, sizeof(rboot_config));
	romconf->magic = BOOT_CONFIG_MAGIC;
	romconf->version = BOOT_CONFIG_VERSION;
	romconf->count = 2;
	romconf->roms[0] = SECTOR_SIZE * (BOOT_CONFIG_SECTOR + 1);
	romconf->roms[1] = romAddr;
	romconf->current_rom = ROM_TO_BOOT_ON;
	// romconf->count = 2;
	// romconf->roms[0] = SECTOR_SIZE * (BOOT_CONFIG_SECTOR + 1);
	// romconf->roms[1] = (flashsize / 2) + (SECTOR_SIZE * (BOOT_CONFIG_SECTOR + 1));
	// romconf->current_rom = ROM_TO_BOOT_ON;
	// write new config sector
	SPIEraseSector(BOOT_CONFIG_SECTOR);
	SPIWrite(BOOT_CONFIG_SECTOR * SECTOR_SIZE, buffer, SECTOR_SIZE);
}

// prevent this function being placed inline with main
// to keep main's stack size as small as possible
// don't mark as static or it'll be optimised out when
// using the assembler stub
uint32 NOINLINE find_image(void) {

	uint8 flag;
	uint32 runAddr;
	int32 romToBoot;
	uint8 updateConfig = FALSE;

	uint32 romAddr = search(SECTOR_SIZE, ROM_TO_BOOT_ON);
	// write_romconf(romAddr);
	runAddr = check_image(romAddr);

	ets_printf("Booting at ram %d.\r\n", runAddr);
	// copy the loader to top of iram
	ets_memcpy((void*)_text_addr, _text_data, _text_len);
	// return address to load from

	return runAddr;

}

#ifdef BOOT_NO_ASM

// small stub method to ensure minimum stack space used
void call_user_start(void) {
	uint32 addr;
	stage2a *loader;

	addr = find_image();
	if (addr != 0) {
		loader = (stage2a*)entry_addr;
		loader(addr);
	}
}

#else

// assembler stub uses no stack space
// works with gcc
void call_user_start(void) {
	while(1) {
		sleep();
	}
	__asm volatile (
		"mov a15, a0\n"          // store return addr, hope nobody wanted a15!
		"call0 find_image\n"     // find a good rom to boot
		"mov a0, a15\n"          // restore return addr
		"bnez a2, 1f\n"          // ?success
		"ret\n"                  // no, return
		"1:\n"                   // yes...
		"movi a3, entry_addr\n"  // get pointer to entry_addr
		"l32i a3, a3, 0\n"       // get value of entry_addr
		"jx a3\n"                // now jump to it
	);
}

#endif
