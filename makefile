PROJECT         := trackMYmbed
DEVICES         := LPC1768
GCC4MBED_DIR    := gcc4mbed
NO_FLOAT_SCANF  := 1
NO_FLOAT_PRINTF := 1
SRC             := trackuino

include $(GCC4MBED_DIR)/build/gcc4mbed.mk
