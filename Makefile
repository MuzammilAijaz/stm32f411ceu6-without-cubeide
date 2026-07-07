all: build

setup:
	# Clone the repository structure without downloading file contents (blobs)
	git submodule update --init --depth 1 --filter=blob:none vendor/STM32CubeF4
	
	# Configure sparse-checkout to only track Drivers and Middlewares
	cd vendor/STM32CubeF4 && \
	git sparse-checkout init --cone && \
	git sparse-checkout set Drivers Middlewares && \
	git checkout
	
	# Initialize and pull the specific nested driver submodules
	cd vendor/STM32CubeF4 && \
	git submodule update --init --depth 1 \
		Drivers/CMSIS/Device/ST/STM32F4xx/ \
		Drivers/STM32F4xx_HAL_Driver/ \

# Optional
#		Middlewares/Third_Party/FreeRTOS \
# 		Middlewares/ST/STM32_USB_Device_Library \
# 		Middlewares/ST/STM32_USB_Host_Library \

build:
	cmake -Bbuild -DCMAKE_TOOLCHAIN_FILE=cmake/stm32f411ce.cmake
	cmake --build build

flash:
	cmake --build build --target flash
