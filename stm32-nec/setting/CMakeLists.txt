cmake_minimum_required(VERSION 3.13)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools")
include(arm-none-eabi-gcc)

set(PRJ_NAME stm32h7-gfx-fw)


project(${PRJ_NAME}
  LANGUAGES ASM C CXX
)


set(EXECUTABLE ${PRJ_NAME}.elf)


file(GLOB SRC_FILES CONFIGURE_DEPENDS
  src/*.cpp
  src/*.c
  src/bsp/*.c
  src/bsp/device/*.c
  src/bsp/rtos/*.c

  src/lib/FatFs/src/*.c
  src/lib/FatFs/src/option/syscall.c
  src/lib/FatFs/src/option/unicode.c

  src/lib/STM32_USB_Device_Library/Core/Src/*.c

  src/lib/FreeRTOS/Source/*.c
  src/lib/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/*.c
  src/lib/FreeRTOS/Source/CMSIS_RTOS/*.c  

  src/lib/FreeRTOS/Source/portable/MemMang/heap_4.c

  src/lib/CMSIS/DSP/Source/FastMathFunctions/arm_cos_f32.c
  src/lib/CMSIS/DSP/Source/FastMathFunctions/arm_sin_f32.c
  src/lib/CMSIS/DSP/Source/SupportFunctions/arm_q15_to_float.c
  src/lib/CMSIS/DSP/Source/SupportFunctions/arm_fill_q15.c
  src/lib/CMSIS/DSP/Source/TransformFunctions/arm_rfft_fast_f32.c  
  src/lib/CMSIS/DSP/Source/TransformFunctions/arm_rfft_fast_init_f32.c
  src/lib/CMSIS/DSP/Source/ComplexMathFunctions/arm_cmplx_dot_prod_f32.c
  src/lib/CMSIS/DSP/Source/ComplexMathFunctions/arm_cmplx_mag_f32.c
  src/lib/CMSIS/DSP/Source/StatisticsFunctions/arm_power_q15.c
)

file(GLOB_RECURSE SRC_FILES_RECURSE CONFIGURE_DEPENDS
  src/ap/*.cpp
  src/ap/*.c
  src/bsp/*.s
  src/common/*.c
  src/hw/*.c
  src/lib/STM32H7xx_HAL_Driver/Src/*.c
  src/lib/littlefs/*.c

  src/ap/touch-gfx/*.cpp
  src/lib/touchgfx/framework/source/touchgfx/*.cpp  
)

# 특정 폴더를 빌드에서 제외한다.
#
set(EXCLUDE_PATHS 
  "src/ap/touch-gfx/build"
  "src/ap/touch-gfx/simulator"
  "src/ap/touch-gfx/generated/simulator"
)

foreach(TMP_PATH ${SRC_FILES_RECURSE}) 
  foreach(EXCLUDE_PATH ${EXCLUDE_PATHS}) 
    string(FIND ${TMP_PATH} ${EXCLUDE_PATH} RESULT) 
    if(NOT ${RESULT} EQUAL -1) 
      list(REMOVE_ITEM SRC_FILES_RECURSE ${TMP_PATH}) 
    endif() 
  endforeach(EXCLUDE_PATH) 
endforeach(TMP_PATH)


add_executable(${EXECUTABLE} 
  ${SRC_FILES}
  ${SRC_FILES_RECURSE}
)



target_include_directories(${EXECUTABLE} PRIVATE 
  Source
  Source/Core 
  Source/Application
  Source/Drivers
  Source/Drivers/STM32F1xx_HAL_Driver
  Source/Drivers/STM32F1xx_HAL_Driver/Inc
  Source/Drivers/CMSIS
  Source/Drivers/CMSIS/Include
  Source/Drivers/CMSIS/Device
  Source/Drivers/CMSIS/Device/ST
  Source/Drivers/CMSIS/Device/ST/STM32F1xx
  Source/Drivers/CMSIS/Device/ST/STM32F1xx/Include
)

target_compile_definitions(${EXECUTABLE} PRIVATE
  -DSTM32H723xx 
  -DLFS_THREADSAFE
  -D__PV_LANGUAGE_KOREAN__
  )

target_compile_options(${EXECUTABLE} PRIVATE
  # startup 코드 가장 상단 참고 (ARM-Cortex Setting)
  -mcpu cortex-m3
  -mfpu softvfp
  -mthumb

  # GNAT 정적 라이브러리 -> 사용하지 않는 데드 코드 제거
  # https://gcc.gnu.org/onlinedocs/gnat_ugn/Compilation-options.html
  -fdata-sections
  -ffunction-sections

   # GCC 옵션 FLAG
   # https://www.rapidtables.com/code/linux/gcc/gcc-g.html
  -Wall # gcc -Wall enables all compiler's warning messages.
  -g3 # -g3 : Maximal debug information
  -Og # Set the compiler's optimization level.
  )

target_link_options(${EXECUTABLE} PRIVATE
  -T../Source/STM32F103C6TX_FLASH.ld
  -mcpu=cortex-m7
  -mthumb
  -mfpu=fpv5-d16 
  -mfloat-abi=hard
  -specs=nano.specs
  -lc
  -lm
  -lstdc++
  -lsupc++
  -fno-rtti
  #-lnosys
  -Wl,-Map=${PRJ_NAME}.map,--cref
  -Wl,--gc-sections
  -Xlinker -print-memory-usage -Xlinker
  )


set (CMAKE_CXX_FLAGS "-fno-rtti")

target_link_libraries( ${EXECUTABLE} PRIVATE
  ${CMAKE_SOURCE_DIR}/src/lib/CMSIS/DSP/Lib/GCC/libarm_cortexM7lfdp_math.a 
  ${CMAKE_SOURCE_DIR}/src/lib/STM32_Audio/Addons/HP_PDM/Lib/libPDMFilter_CM7_GCC_wc32.a
  ${CMAKE_SOURCE_DIR}/src/lib/STM32_AcousticSL_Library/Lib/libAcousticSL_CM7F_wc32_ot.a
  ${CMAKE_SOURCE_DIR}/src/lib/picovoice/ko/libpicovoice.a
  ${CMAKE_SOURCE_DIR}/src/lib/touchgfx/lib/core/cortex_m7/gcc/libtouchgfx-float-abi-hard.a
  )

add_custom_command(TARGET ${EXECUTABLE} 
  POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} ARGS -O binary ${EXECUTABLE} ${PROJECT_NAME}.bin
  COMMENT "Invoking: Make Binary"
  )  