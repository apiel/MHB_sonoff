/*
Originaly written for Arduino updated for esp-open-rtos
Still not sure about the EEPROM_START
*/
/*
  EEPROM.cpp - esp8266 EEPROM emulation

  Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
  This file is part of the esp8266 core for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "EEPROM.h"

#include <espressif/spi_flash.h>

#include "config.h"

#ifndef __STRINGIFY
#define __STRINGIFY(a) #a
#endif
#define xt_rsil(level) (__extension__({uint32_t state; __asm__ __volatile__("rsil %0," __STRINGIFY(level) : "=a" (state)); state;}))
#define interrupts() xt_rsil(0)
#define noInterrupts() xt_rsil(15)

EEPROMClass::EEPROMClass(uint32_t sector)
: _sector(sector)
, _data(0)
, _size(0)
, _dirty(false)
{
}

EEPROMClass::EEPROMClass(void)
: _sector(EEPROM_START / SPI_FLASH_SEC_SIZE)
// : _sector((0x402EB000 - 0x40200000) / SPI_FLASH_SEC_SIZE)
, _data(0)
, _size(0)
, _dirty(false)
{
}

void EEPROMClass::begin(size_t size) {
  if (size <= 0)
    return;
  if (size > SPI_FLASH_SEC_SIZE)
    size = SPI_FLASH_SEC_SIZE;

  size = (size + 3) & (~3);

  if (_data) {
    delete[] _data;
  }

  _data = new uint8_t[size];
  _size = size;

  noInterrupts();
  sdk_spi_flash_read(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(_data), _size);
  interrupts();
}

void EEPROMClass::end() {
  if (!_size)
    return;

  commit();
  if(_data) {
    delete[] _data;
  }
  _data = 0;
  _size = 0;
}


uint8_t EEPROMClass::read(int address) {
  if (address < 0 || (size_t)address >= _size)
    return 0;
  if(!_data)
    return 0;

  return _data[address];
}

void EEPROMClass::write(int address, uint8_t value) {
  if (address < 0 || (size_t)address >= _size)
    return;
  if(!_data)
    return;

  // Optimise _dirty. Only flagged if data written is different.
  uint8_t* pData = &_data[address];
  if (*pData != value)
  {
    *pData = value;
    _dirty = true;
  }
}

bool EEPROMClass::commit() {
  bool ret = false;
  if (!_size)
    return false;
  if(!_dirty)
    return true;
  if(!_data)
    return false;

  noInterrupts();
  if(sdk_spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK) {
    if(sdk_spi_flash_write(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(_data), _size) == SPI_FLASH_RESULT_OK) {
      _dirty = false;
      ret = true;
    }
  }
  interrupts();

  return ret;
}

uint8_t * EEPROMClass::getDataPtr() {
  _dirty = true;
  return &_data[0];
}

bool EEPROMClass::save(int address, char * data)
{
    int len = strlen(data);
    for(int pos = 0; pos < len; pos++, address++) {
        if (address > EEPROM_SIZE) {
            break;
        }
        write(address, data[pos]);
    }
    return commit();
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EEPROM)
EEPROMClass EEPROM;
#endif
