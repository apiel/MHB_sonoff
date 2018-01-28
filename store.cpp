
#include "EEPROM.h"
#include "config.h"

void save_store(int address, char * data) 
{
    int len = strlen(data);
    for(int pos = 0; pos < len; pos++, address++) {
        if (address > EEPROM_SIZE) {
            break;
        }
        EEPROM.write(address, data[pos]);
    }
    EEPROM.commit();
}
