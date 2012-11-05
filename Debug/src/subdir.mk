################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/gameoflife-openmp-stitch.cpp \
../src/readfile.cpp 

OBJS += \
./src/gameoflife-openmp-stitch.o \
./src/readfile.o 

CPP_DEPS += \
./src/gameoflife-openmp-stitch.d \
./src/readfile.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -g -rdynamic -Wall -c -fmessage-length=0 -fopenmp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


