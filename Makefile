.PHONY: all rover_all rover_compile rover_size rover_fuse rover_flash cube_all cube_compile cube_size cube_fuse cube_flash trx_all trx_compile trx_size trx_fuse trx_flash

# As you program, you should only need to update these. List every file you'd throw under the "gcc" program.
common_dependencies = common/spi.c common/spi.h common/uart.c common/uart.h
cube_common_dependencies = cube/common/address_resolution.c cube/common/address_resolution.h cube/common/cube_parameters.h cube/common/data_link.c cube/common/data_link.h cube/common/network.c cube/common/network.h cube/common/networking_constants.h cube/common/transport.c cube/common/transport.h cube/common/trx.c cube/common/trx.h cube/common/digital_io.h cube/common/digital_io.c

rover_dependencies = $(common_dependencies) rover/adc.c rover/adc.h rover/main.c rover/digital_io.h rover/digital_io.c rover/motors.h rover/motors.c rover/timer.h rover/timer.c rover/accelerometer.h rover/accelerometer.c rover/test.h rover/test.c
trx_dependencies = $(common_dependencies) $(cube_common_dependencies) cube/rover_trx/address.h cube/rover_trx/application.c cube/rover_trx/application.h cube/rover_trx/main.c cube/rover_trx/routing_table.c cube/rover_trx/routing_table.h
cube0_dependencies = $(common_dependencies) $(cube_common_dependencies) cube/cube0/address.h cube/cube0/application.c cube/cube0/application.h cube/cube0/main.c cube/cube0/routing_table.c cube/cube0/routing_table.h
cube1_dependencies = $(common_dependencies) $(cube_common_dependencies) cube/cube1/address.h cube/cube1/application.c cube/cube1/application.h cube/cube1/main.c cube/cube1/routing_table.c cube/cube1/routing_table.h
cube2_dependencies = $(common_dependencies) $(cube_common_dependencies) cube/cube2/address.h cube/cube2/application.c cube/cube2/application.h cube/cube2/main.c cube/cube2/routing_table.c cube/cube2/routing_table.h




# ============ Compile everything ==========

all: rover_compile cube0_compile cube1_compile cube2_compile trx_compile

# ============ Reset the ATMega without writing anything ============

reset: 
	avrdude -p m328p -c usbtiny

# ============ Rover =============

rover_all: rover_compile rover_fuse rover_flash

rover_compile: build/rover.hex rover_size

build/rover.hex: build/rover.out
	avr-objcopy -j .text -j .data -O ihex build/rover.out build/rover.hex

build/rover.out: $(rover_dependencies)
	avr-gcc -Irover -Icommon $(rover_dependencies) -mmcu=atmega328p -Os -o build/rover.out

rover_size: build/rover.out
	avr-size build/rover.out --format=avr --mcu=atmega328p -C

rover_fuse:
# 1 MHz clock:
# avrdude -p m328p -c usbtiny -U lfuse:w:0x62:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m -U lock:w:0xFF:m
# 8 MHz clock:
	avrdude -p m328p -c usbtiny -U lfuse:w:0xE2:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m -U lock:w:0xFF:m

rover_flash: build/rover.hex
	avrdude -p m328p -c usbtiny -U flash:w:build/rover.hex:i


# ============ Cube 0 (standalone) =============

cube0_all: cube0_compile cube0_fuse cube0_flash

cube0_compile: build/cube0.hex cube0_size

build/cube0.hex: build/cube0.out
	avr-objcopy -j .text -j .data -O ihex build/cube0.out build/cube0.hex

build/cube0.out: $(cube0_dependencies)
	avr-gcc -Icube/cube0 -Icube/common -Icommon $(cube0_dependencies) -mmcu=atmega328p -Os -o build/cube0.out

cube0_size: build/cube0.out
	avr-size build/cube0.out --format=avr --mcu=atmega328p -C

cube0_fuse:
	avrdude -p m328p -c usbtiny -U lfuse:w:0x62:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m -U lock:w:0xFF:m

cube0_flash: build/cube0.hex
	avrdude -p m328p -c usbtiny -U flash:w:build/cube0.hex:i

# ============ Cube 0 (standalone) =============

cube1_all: cube1_compile cube1_fuse cube1_flash

cube1_compile: build/cube1.hex cube1_size

build/cube1.hex: build/cube1.out
	avr-objcopy -j .text -j .data -O ihex build/cube1.out build/cube1.hex

build/cube1.out: $(cube1_dependencies)
	avr-gcc -Icube/cube1 -Icube/common -Icommon $(cube1_dependencies) -mmcu=atmega328p -Os -o build/cube1.out

cube1_size: build/cube1.out
	avr-size build/cube1.out --format=avr --mcu=atmega328p -C

cube1_fuse:
	avrdude -p m328p -c usbtiny -U lfuse:w:0x62:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m -U lock:w:0xFF:m

cube1_flash: build/cube1.hex
	avrdude -p m328p -c usbtiny -U flash:w:build/cube1.hex:i

# ============ Cube 2 (standalone) =============

cube2_all: cube2_compile cube2_fuse cube2_flash

cube2_compile: build/cube2.hex cube2_size

build/cube2.hex: build/cube2.out
	avr-objcopy -j .text -j .data -O ihex build/cube2.out build/cube2.hex

build/cube2.out: $(cube2_dependencies)
	avr-gcc -Icube/cube2 -Icube/common -Icommon $(cube2_dependencies) -mmcu=atmega328p -Os -o build/cube2.out

cube2_size: build/cube2.out
	avr-size build/cube2.out --format=avr --mcu=atmega328p -C

cube2_fuse:
	avrdude -p m328p -c usbtiny -U lfuse:w:0x62:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m -U lock:w:0xFF:m

cube2_flash: build/cube2.hex
	avrdude -p m328p -c usbtiny -U flash:w:build/cube2.hex:i

# ============ Data Cube (rover's transceiver) =============

trx_all: trx_compile trx_fuse trx_flash

trx_compile: build/trx.hex trx_size

build/trx.hex: build/trx.out
	avr-objcopy -j .text -j .data -O ihex build/trx.out build/trx.hex

build/trx.out: $(trx_dependencies)
	avr-gcc -Icube/rover_trx -Icube/common -Icommon $(trx_dependencies) -mmcu=atmega328p -Os -o build/trx.out

trx_size: build/trx.out
	avr-size build/trx.out --format=avr --mcu=atmega328p -C

trx_fuse:
# to be determined by the fuses we need (avrdude command)
# example: avrdude -p m328p -c usbtiny -U lfuse:w:0xFF:m -U hfuse:w:0xDF:m -U efuse:w:0xFF:m -U lock:w:0xFF:m

trx_flash: build/trx.hex
	avrdude -p m328p -c usbtiny -U flash:w:build/trx.hex:i

# =============== General ========================
	
clean:
	rm -f build/rover.hex
	rm -f build/rover.out
	rm -f build/cube.hex
	rm -f build/cube.out
	rm -f build/trx.hex
	rm -f build/trx.out
