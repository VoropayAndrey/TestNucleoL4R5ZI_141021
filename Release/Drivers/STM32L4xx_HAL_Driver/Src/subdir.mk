################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc_ex.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_crc.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_crc_ex.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dac.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dac_ex.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma_ex.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_exti.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ramfunc.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_opamp.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_opamp_ex.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pcd.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pcd_ex.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc_ex.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sai.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sai_ex.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sd.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sd_ex.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi_ex.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart_ex.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_sdmmc.c \
../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_usb.c 

C_DEPS += \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc_ex.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_crc.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_crc_ex.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dac.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dac_ex.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma_ex.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_exti.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ramfunc.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_opamp.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_opamp_ex.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pcd.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pcd_ex.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc_ex.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sai.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sai_ex.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sd.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sd_ex.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi_ex.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart_ex.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_sdmmc.d \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_usb.d 

OBJS += \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc_ex.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_crc.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_crc_ex.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dac.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dac_ex.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma_ex.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_exti.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ramfunc.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_opamp.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_opamp_ex.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pcd.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pcd_ex.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc_ex.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sai.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sai_ex.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sd.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sd_ex.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi_ex.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart_ex.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_sdmmc.o \
./Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_usb.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/STM32L4xx_HAL_Driver/Src/%.o: ../Drivers/STM32L4xx_HAL_Driver/Src/%.c Drivers/STM32L4xx_HAL_Driver/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32L4R5xx -D__SUPPORT_LOW_ENERGY__ -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Bluetooth/Inc -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/btpskrnl -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/btvs -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/SBC/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/GAVD/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/AUDIO/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/AVCTP/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/A2DP/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/HDSET/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/AVRCP/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles -I"/Users/andrey/Documents/Common/bluetopia/CC256x STM32 Bluetopia SDK/v4.0.2.2/FreeRTOS/Bluetopia/profiles/HRS/include" -I"/Users/andrey/Documents/Common/bluetopia/CC256x STM32 Bluetopia SDK/v4.0.2.2/FreeRTOS/Bluetopia/profiles/GATT/include" -I"/Users/andrey/Documents/Common/bluetopia/CC256x STM32 Bluetopia SDK/v4.0.2.2/FreeRTOS/Bluetopia/profiles/GAPS/include" -I"/Users/andrey/Documents/Common/bluetopia/CC256x STM32 Bluetopia SDK/v4.0.2.2/FreeRTOS/Bluetopia/profiles/DIS/include" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

