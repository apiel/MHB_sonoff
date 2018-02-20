
#include "EEflash.h"

#include <espressif/spi_flash.h>

#include "config.h"

#include <espressif/esp_common.h> // printf

#ifndef __STRINGIFY
#define __STRINGIFY(a) #a
#endif
#define xt_rsil(level) (__extension__({uint32_t state; __asm__ __volatile__("rsil %0," __STRINGIFY(level) : "=a" (state)); state;}))
#define interrupts() xt_rsil(0)
#define noInterrupts() xt_rsil(15)

EEflashClass::EEflashClass(uint32_t sector)
: _sector(sector)
, _data(0)
, _size(0)
, _dirty(false)
{
  printf("EEflashClass A");
}

void EEflashClass::begin(size_t size) {
  if (size <= 0)
    return;
  if (size > SPI_FLASH_SEC_SIZE)
    size = SPI_FLASH_SEC_SIZE;

  size = (size + 3) & (~3);

  if (_data) {
    delete[] _data;
  }

printf("new datatatata %d\n", size);
  _data = new uint8_t[size];
  _size = size;

  noInterrupts();
  sdk_spi_flash_read(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(_data), _size);
  interrupts();
}

void EEflashClass::end() {
  if (!_size)
    return;

  commit();
  if(_data) {
    delete[] _data;
  }
  _data = 0;
  _size = 0;
}


uint8_t EEflashClass::read(int address) {
  if (address < 0 || (size_t)address >= _size)
    return 0;
  if(!_data)
    return 0;

  return _data[address];
}

void EEflashClass::write(int address, uint8_t value) {
  printf("a");
  if (address < 0 || (size_t)address >= _size)
    return;
  printf("b");
  if(!_data)
    return;

printf("c");
  // Optimise _dirty. Only flagged if data written is different.
  uint8_t* pData = &_data[address];
  printf("d");
  if (*pData != value)
  {
    printf("e");
    *pData = value;
    printf("f");
    _dirty = true;
  }
  printf("g\n");
}

bool EEflashClass::commit() {
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

bool EEflashClass::save(int address, uint8_t * data, int len)
{
  printf("save eeflash %d\n", len);
    for(int pos = 0; pos < len; pos++, address++) {
        if (address > _size) {
            break;
        }
        printf("write data %d %d %d\n", pos, address, data[pos]);
        write(address, data[pos]);
    }
  printf("now commit\n");
    return commit();
}
