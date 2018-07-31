# Simple makefile for simple example
PROGRAM=firmware
EXTRA_COMPONENTS=extras/cpp_support extras/dhcpserver extras/mbedtls extras/rboot-ota extras/paho_mqtt_c extras/http_client_ota
#extras/dht extras/onewire extras/ds18b20

# LIB_ARGS=espnow

include ../esp-open-rtos/common.mk
