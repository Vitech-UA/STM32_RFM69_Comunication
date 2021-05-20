################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Startup/startup_stm32f030f4px.s 

S_DEPS += \
./Startup/startup_stm32f030f4px.d 

OBJS += \
./Startup/startup_stm32f030f4px.o 


# Each subdirectory must supply rules for building sources it contributes
Startup/startup_stm32f030f4px.o: ../Startup/startup_stm32f030f4px.s Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m0 -g3 -c -x assembler-with-cpp -MMD -MP -MF"Startup/startup_stm32f030f4px.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@" "$<"

