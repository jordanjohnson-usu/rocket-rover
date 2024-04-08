.PHONY: all rover_all rover_compile rover_size rover_fuse rover_flash cube_all cube_compile cube_size cube_fuse cube_flash trx_all trx_compile trx_size trx_fuse trx_flash

# As you program, you should only need to update these. List every file you'd throw under the "gcc" program.
rover_dependencies = rover/main.c rover/adc.h rover/adc.c common/uart.h common/uart.c
cube_dependencies = cube/standalone/main.c cube/common/cube_parameters.h cube/common/trx.h cube/common/trx.c cube/standalone/application.h cube/standalone/application.c common/uart.c common/uart.h common/spi.h common/spi.c cube/common/networking_constants.h cube/common/address_resolution.h cube/common/address_resolution.c cube/common/transport.h cube/common/transport.c cube/common/network.h cube/common/network.c cube/common/data_link.h cube/common/data_link.c
trx_dependencies = cube/rover_trx/main.c





# ============ Compile everything ==========

all: rover_compile cube_compile trx_compile

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
# to be determined by the fuses we need (avrdude command)
# example: avrdude -p m328p -c usbtiny -U lfuse:w:0xFF:m -U hfuse:w:0xDF:m -U efuse:w:0xFF:m -U lock:w:0xFF:m

rover_flash: build/rover.hex
	avrdude -p m328p -c usbtiny -U flash:w:build/rover.hex:i


# ============ Data Cube (standalone) =============

cube_all: cube_compile cube_fuse cube_flash

cube_compile: build/cube.hex cube_size

build/cube.hex: build/cube.out
	avr-objcopy -j .text -j .data -O ihex build/cube.out build/cube.hex

build/cube.out: $(cube_dependencies)
	avr-gcc -Icube/standalone -Icube/common -Icommon $(cube_dependencies) -mmcu=atmega328p -Os -o build/cube.out

cube_size: build/cube.out
	avr-size build/cube.out --format=avr --mcu=atmega328p -C

cube_fuse:
	avrdude -p m328p -c usbtiny -U lfuse:w:0x62:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m -U lock:w:0xFF:m

cube_flash: build/cube.hex
	avrdude -p m328p -c usbtiny -U flash:w:build/cube.hex:i

# ============ Data Cube (rover's transceiver) =============

trx_all: trx_compile trx_fuse trx_flash

trx_compile: build/trx.hex trx_size

build/trx.hex: build/trx.out
	avr-objcopy -j .text -j .data -O ihex build/trx.out build/trx.hex

build/trx.out: $(trx_dependencies)
	avr-gcc -Icube/rover_trx -Icube/common -Icommon $(trx_dependencies) -mmcu=atmega328p -o build/trx.out

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
