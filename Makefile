# Simple makefile for simple example
PROGRAM=rf
EXTRA_COMPONENTS=extras/cpp_support extras/spiffs extras/dhcpserver extras/mbedtls extras/paho_mqtt_c extras/rboot-ota

FLASH_SIZE = 32

# spiffs configuration
SPIFFS_BASE_ADDR = 0x210000
SPIFFS_SIZE = 0x010000
SPIFFS_SINGLETON = 0

include ../esp-open-rtos/common.mk

$(eval $(call make_spiffs_image,files))
