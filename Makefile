# Simple makefile for simple example
PROGRAM=firmware
EXTRA_COMPONENTS=extras/cpp_support extras/dhcpserver extras/mbedtls extras/rboot-ota extras/dht extras/onewire extras/ds18b20

include ../esp-open-rtos/common.mk
