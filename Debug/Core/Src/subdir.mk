################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/AUDIO.c \
../Core/Src/HAL.c \
../Core/Src/adc.c \
../Core/Src/crc.c \
../Core/Src/dac.c \
../Core/Src/dma.c \
../Core/Src/freertos.c \
../Core/Src/gpio.c \
../Core/Src/i2c.c \
../Core/Src/main.c \
../Core/Src/opamp.c \
../Core/Src/rtc.c \
../Core/Src/sai.c \
../Core/Src/sdmmc.c \
../Core/Src/spi.c \
../Core/Src/stm32l4xx_hal_msp.c \
../Core/Src/stm32l4xx_hal_timebase_tim.c \
../Core/Src/stm32l4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32l4xx.c \
../Core/Src/usart.c 

OBJS += \
./Core/Src/AUDIO.o \
./Core/Src/HAL.o \
./Core/Src/adc.o \
./Core/Src/crc.o \
./Core/Src/dac.o \
./Core/Src/dma.o \
./Core/Src/freertos.o \
./Core/Src/gpio.o \
./Core/Src/i2c.o \
./Core/Src/main.o \
./Core/Src/opamp.o \
./Core/Src/rtc.o \
./Core/Src/sai.o \
./Core/Src/sdmmc.o \
./Core/Src/spi.o \
./Core/Src/stm32l4xx_hal_msp.o \
./Core/Src/stm32l4xx_hal_timebase_tim.o \
./Core/Src/stm32l4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32l4xx.o \
./Core/Src/usart.o 

C_DEPS += \
./Core/Src/AUDIO.d \
./Core/Src/HAL.d \
./Core/Src/adc.d \
./Core/Src/crc.d \
./Core/Src/dac.d \
./Core/Src/dma.d \
./Core/Src/freertos.d \
./Core/Src/gpio.d \
./Core/Src/i2c.d \
./Core/Src/main.d \
./Core/Src/opamp.d \
./Core/Src/rtc.d \
./Core/Src/sai.d \
./Core/Src/sdmmc.d \
./Core/Src/spi.d \
./Core/Src/stm32l4xx_hal_msp.d \
./Core/Src/stm32l4xx_hal_timebase_tim.d \
./Core/Src/stm32l4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32l4xx.d \
./Core/Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L4R5xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Bluetooth/Inc -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/btpskrnl -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/btvs -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/SBC/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/GAVD/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/AUDIO/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/AVCTP/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/A2DP/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/HDSET/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/AVRCP/include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

