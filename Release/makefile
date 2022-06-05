################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include btvs/subdir.mk
-include btpsvend/subdir.mk
-include btpskrnl/subdir.mk
-include btpsfile/subdir.mk
-include USB_DEVICE/Target/subdir.mk
-include USB_DEVICE/App/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/subdir.mk
-include Middlewares/Third_Party/FatFs/src/option/subdir.mk
-include Middlewares/Third_Party/FatFs/src/subdir.mk
-include Middlewares/ST/STM32_USB_Device_Library/Core/Src/subdir.mk
-include Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/subdir.mk
-include FATFS/Target/subdir.mk
-include FATFS/App/subdir.mk
-include Drivers/STM32L4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/tests/subdir.mk
-include Core/Src/subdir.mk
-include Bluetooth/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := TestNucleoL4R5ZI_141021
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
TestNucleoL4R5ZI_141021.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
TestNucleoL4R5ZI_141021.list \

OBJCOPY_BIN += \
TestNucleoL4R5ZI_141021.bin \


# All Target
all: main-build

# Main-build Target
main-build: TestNucleoL4R5ZI_141021.elf secondary-outputs

# Tool invocations
TestNucleoL4R5ZI_141021.elf: $(OBJS) $(USER_OBJS) /Users/andrey/STM32CubeIDE/workspace_1.7.0/TestNucleoL4R5ZI_141021/STM32L4R5ZITX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-g++ -o "TestNucleoL4R5ZI_141021.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"/Users/andrey/STM32CubeIDE/workspace_1.7.0/TestNucleoL4R5ZI_141021/STM32L4R5ZITX_FLASH.ld" -Wl,-Map="TestNucleoL4R5ZI_141021.map" -Wl,--gc-sections -static -L../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/lib/gcc -L../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/SBC/lib/gcc -L../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/AUDIO/lib/gcc -L../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/AVRCP/lib/gcc -L../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/AVCTP/lib/gcc -L../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/GAVD/lib/gcc -L../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/GATT/lib/gcc -L../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/GAPS/lib/gcc -L../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/HDSET/lib/gcc -L../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/DIS/lib/gcc -L../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/HRS/lib/gcc --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -u _printf_float -Wl,--start-group -lc -lm -lstdc++ -lsupc++ -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

TestNucleoL4R5ZI_141021.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "TestNucleoL4R5ZI_141021.list"
	@echo 'Finished building: $@'
	@echo ' '

TestNucleoL4R5ZI_141021.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "TestNucleoL4R5ZI_141021.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJDUMP_LIST)$(C_UPPER_DEPS)$(S_DEPS)$(C_DEPS)$(OBJCOPY_BIN)$(CC_DEPS)$(SIZE_OUTPUT)$(C++_DEPS)$(EXECUTABLES)$(OBJS)$(CXX_DEPS)$(S_UPPER_DEPS)$(CPP_DEPS) TestNucleoL4R5ZI_141021.elf
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
