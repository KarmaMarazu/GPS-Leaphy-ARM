################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include USB_HOST/Target/subdir.mk
-include USB_HOST/App/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/subdir.mk
-include Middlewares/ST/STM32_USB_Host_Library/Core/Src/subdir.mk
-include Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Src/subdir.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include Core/MyApp/Ports/subdir.mk
-include Core/MyApp/App/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


<<<<<<< HEAD
BUILD_ARTIFACT_NAME := FreeRTOS_GPS_407
=======
BUILD_ARTIFACT_NAME := Free
>>>>>>> branch 'master' of https://github.com/KarmaMarazu/GPS-Leaphy-ARM.git
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
<<<<<<< HEAD
FreeRTOS_GPS_407.elf \
=======
Free.elf \
>>>>>>> branch 'master' of https://github.com/KarmaMarazu/GPS-Leaphy-ARM.git

MAP_FILES += \
<<<<<<< HEAD
FreeRTOS_GPS_407.map \
=======
Free.map \
>>>>>>> branch 'master' of https://github.com/KarmaMarazu/GPS-Leaphy-ARM.git

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
<<<<<<< HEAD
FreeRTOS_GPS_407.list \


# All Target
all: main-build

# Main-build Target
main-build: FreeRTOS_GPS_407.elf secondary-outputs

# Tool invocations
FreeRTOS_GPS_407.elf FreeRTOS_GPS_407.map: $(OBJS) $(USER_OBJS) C:\Users\Thomas\ van\ Ooijen\git\GPS-Leaphy-ARM\FreeRTOS_GPS_407\STM32F407VGTX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "FreeRTOS_GPS_407.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\Users\Thomas van Ooijen\git\GPS-Leaphy-ARM\FreeRTOS_GPS_407\STM32F407VGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="FreeRTOS_GPS_407.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

FreeRTOS_GPS_407.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "FreeRTOS_GPS_407.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) FreeRTOS_GPS_407.elf FreeRTOS_GPS_407.list FreeRTOS_GPS_407.map default.size.stdout
=======
Free.list \


# All Target
all: main-build

# Main-build Target
main-build: Free.elf secondary-outputs

# Tool invocations
Free.elf Free.map: $(OBJS) $(USER_OBJS) E:\HBO\3A\Reflectieve\ Professional\ Embedded\ sys\RTOS\FreeRTOS_GPS_407\FreeRTOS_GPS_407\STM32F407VGTX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "Free.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"E:\HBO\3A\Reflectieve Professional Embedded sys\RTOS\FreeRTOS_GPS_407\FreeRTOS_GPS_407\STM32F407VGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="Free.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -u _printf_float -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

Free.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "Free.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) Free.elf Free.list Free.map default.size.stdout
>>>>>>> branch 'master' of https://github.com/KarmaMarazu/GPS-Leaphy-ARM.git
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
