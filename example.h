/**
  ******************************************************************************
  * file           : example.h
  * brief          : Header for example.c.
  *                  This file contains example-specific declarations to interface with main().
  ******************************************************************************
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef EXAMPLE_H
#define EXAMPLE_H

/* Includes ------------------------------------------------------------------*/
#include <inttypes.h> /* PRIx8 macros family */
#include <stdio.h> /* printf */
#include "string.h" /* memset */

#include "mx_hal_def.h"       /* aliases to the target-specific generated code */
#include "mx_m95p32.h"
#include "mx_basic_stdio_app.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Exported types ------------------------------------------------------------*/
/* The volatile qualifier ensures the content of variables of this type is always visible in the debugger. */
typedef volatile enum
{
  EXEC_STATUS_ERROR   = -1, /* problem encountered         */
  EXEC_STATUS_UNKNOWN = 0,  /* default value               */
  EXEC_STATUS_INIT_OK = 1,  /* app_init ran as expected    */
  EXEC_STATUS_OK      = 2   /* application ran as expected */
} app_status_t;

typedef enum
{
  M95P32_STATUS_OK = 0,              /* buff load ran as expected    */
  M95P32_EXEC_ERROR = -1,            /* execution error              */
  M95P32_MEMORY_PROTECTED   = -2,    /* memory protected             */
  M95P32_MEMORY_NOT_ERASED = -3,     /* memory not erased            */
  M95P32_SAFETY_REGISTER_ERROR = -4  /* memory not erased            */

} m95p32_buff_load_status;

/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/** brief:  User application initialization.
  * retval: example status (see app_status_t)
  */
app_status_t app_init(void);

/** brief:  User application processing.
  * retval: example status (see app_status_t)
  */
app_status_t app_process(void);
app_status_t MX_EEPROM_RW_Process(void);
app_status_t M95P32_TestWriteEnableDisable(void);
app_status_t M95P32_TestPage(void);
app_status_t M95P32_TestPageErase(void);
app_status_t M95P32_TestSectorErase(void);
app_status_t M95P32_TestBlockErase(void);
app_status_t M95P32_TestBufferMode(void);
m95p32_buff_load_status PageProgram_with_BufferLoad(m95p32_object_t *pObj, uint8_t *pData, uint32_t start_prog_add,
                                                    uint32_t nb_byte);
void M95P32_PageProg_with_SafetyRegisterCheck(void);
void M95P32_PageWrite_with_SafetyRegisterCheck(void);
void M95P32_16Word_align(void);
void M95P32_WriteProtectedArea(void);
void M95P32_PageProgram_with_BufferLoad(void);

void IntroScreen(void);
void User_Scenario_IntroScreen(void);
void Specific_Behaviour_IntroScreen(void);
hal_status_t UARTConsolePrint(char *puartmsg);
hal_status_t HAL_UART_Receive_Until_CR(hal_uart_handle_t *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);

/** brief:  User application de-init.
  * retval: example status (see app_status_t)
  */
app_status_t app_deinit(void);

/** logging macro - just redirects to printf()
  * libc's stdout is redirected to UART thanks to the Basic stdio utility.
  *
  * @user You can enable logs by defining USE_TRACE=1 in the build options.
  */
#if defined(USE_TRACE) && USE_TRACE != 0
#define PRINTF(...)    printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif /* defined(USE_TRACE) && USE_TRACE != 0 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EXAMPLE_H */
