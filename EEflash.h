
#ifndef EEflash_h
#define EEflash_h

#include <stddef.h>
#include <stdint.h>
#include <string.h>

class EEflashClass {
public:
  EEflashClass(uint32_t sector);

  void begin(size_t size);
  uint8_t read(int address);
  void write(int address, uint8_t val);
  bool commit();
  void end();
  bool save(int address, uint8_t * data, int len);

  // template<typename T> 
  // T &get(int address, T &t) {
  //   if (address < 0 || address + sizeof(T) > _size)
  //     return t;

  //   memcpy((uint8_t*) &t, _data + address, sizeof(T));
  //   return t;
  // }

  // template<typename T> 
  // const T &put(int address, const T &t) {
  //   if (address < 0 || address + sizeof(T) > _size)
  //     return t;
  //   if (memcmp(_data + address, (const uint8_t*)&t, sizeof(T)) != 0) {
	// _dirty = true;
	// memcpy(_data + address, (const uint8_t*)&t, sizeof(T));
  //   }

  //   return t;
  // }

protected:
  uint32_t _sector;
  uint8_t* _data;
  size_t _size;
  bool _dirty;
};

#endif

