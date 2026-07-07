all: build

setup:
	git submodule update --init --depth 1 vendor/STM32CubeF4
	cd vendor/STM32CubeF4 && \
	git submodule update --init --depth 1 Drivers/CMSIS/Device/ST/STM32F4xx/ Drivers/STM32F4xx_HAL_Driver/ Middlewares/ST/STM32_USB_Device_Library Middlewares/ST/STM32_USB_Host_Library Middlewares/Third_Party/FreeRTOS

build:
	cmake -Bbuild -DCMAKE_TOOLCHAIN_FILE=cmake/stm32f411ce.cmake
	cmake --build build

flash:
	cmake --build build --target flash
