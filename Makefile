# C compiler
CC:= mipsel-openwrt-linux-g++
# path where the toolchain is
TOOLCHAIN_ROOT_DIR:= /home/max/source/staging_dir/target-mipsel_24kc_musl-1.1.16

# additional includes from toolchain
INCLUDE_DIRS:=$(TOOLCHAIN_ROOT_DIR)/usr/include
LIB_DIRS:=$(TOOLCHAIN_ROOT_DIR)/usr/lib

CFLAGS:= -O3 -ggdb -g -Wall -Wextra -std=c++17
IFLAGS:= -I $(INCLUDE_DIRS) -I arduino-lmic/src/ -I.

EXAMPLE_SOURCE = dht11_lora
PROGRAM_SOURCES = $(EXAMPLE_SOURCE).cpp 
EXECUTABLE:= $(EXAMPLE_SOURCE)

CBOR_SOURCES := \
	cbor-cpp/src/decoder.cpp \
	cbor-cpp/src/encoder.cpp \
	cbor-cpp/src/input.cpp \
	cbor-cpp/src/listener_debug.cpp \
	cbor-cpp/src/output_dynamic.cpp \
	cbor-cpp/src/output_static.cpp 
	
export STAGING_DIR="$TOOLCHAIN_ROOT_DIR/staging_dir/"

.PHONY : program all clean all

program:
	$(CC) -o $(EXECUTABLE) $(CFLAGS) $(IFLAGS) -L $(LIB_DIRS) $(PROGRAM_SOURCES) $(CBOR_SOURCES)

all: | program

upload: | all
	sshpass -p "onioneer" scp $(EXECUTABLE) root@192.168.1.150:/root/.

clean:
	rm -rf $(EXECUTABLE)
