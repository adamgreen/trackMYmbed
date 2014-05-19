PROJECT         := trackMYmbed
DEVICES         := LPC1768
GCC4MBED_DIR    := gcc4mbed
NO_FLOAT_SCANF  := 1
NO_FLOAT_PRINTF := 1
SRC             := firmware

# Pull config.h settings from priv_config.h if it exists.
PRIV_CONFIG := $(wildcard firmware/priv_config.h)
ifneq "$(PRIV_CONFIG)" ""
    GPFLAGS := -include $(PRIV_CONFIG)
endif

include $(GCC4MBED_DIR)/build/gcc4mbed.mk
