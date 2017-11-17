/*
 * @brief FreeRTOS examples
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2014
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdlib.h>

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
#define APPLICATION_1 (1)		/* T1 periodica 500ms + IRQ (productor) -> semaforo ->
 	 	 	 	 	 	 	 	   T2 (consumidor/productor) -> Cola -> T3 (consumidor)*/
#define APPLICATION_2 (2)		/* Idem APP1 pero con direccion y roles invertidos */
#define APPLICATION_3 (3)		/* Idem APP1 pero 3 tareas comparten el uso de un LED
								   por un periodo mayor al timeslice y no pueden mezclarse */
#define APPLICATION_4 (4)		/* Porton de cochera usando el statechart */

#define APPLICATION (APPLICATION_1)	/* T1 periodica 500ms + IRQ (productor) -> semaforo ->
 	 	 	 	 	 	 	 	   	   T2 (consumidor/productor) -> Cola -> T3 (consumidor)*/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED state is off */
	Board_LED_Set(LED3, LED_OFF);
}

#if (APPLICATION == APPLICATION_1)

const char *TextForTask1="Tarea 1 periódica ejecutandose. Activando interrupción\r\n";
const char *TextForIRQ="Otorgando semáforo.\r\n";

xSemaphoreHandle xTask2Semaphore;

/* Tarea periodica de 500 ms */
void TareaPeriodica(void *pvParameters){
	portTickType xLastExecutionTime;
	while (1)
	{
		vTaskDelayUntil(&xLastExecutionTime, 500 / portTICK_RATE_MS);	// 500 ms de delay
		DEBUGOUT(TextForTask1);
		NVIC_SetPendingIRQ(mainSW_INTERRUPT_ID);
	}
}

/*Handler de la interrupcion productora */
void DAC_IRQHandler(void){
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	/* 'Give' the semaphore to unblock the task. */
    xSemaphoreGiveFromISR(xTask2Semaphore, &xHigherPriorityTaskWoken);

    /* Clear the software interrupt bit using the interrupt controllers
	 * Clear Pending register. */
    NVIC_ClearPendingIRQ(mainSW_INTERRUPT_ID);

	/* Giving the semaphore may have unblocked a task - if it did and the
	 * unblocked task has a priority equal to or above the currently executing
     * task then xHigherPriorityTaskWoken will have been set to pdTRUE and
	 * portEND_SWITCHING_ISR() will force a context switch to the newly unblocked
	 * higher priority task.
	 *
	 * NOTE: The syntax for forcing a context switch within an ISR varies between
	 * FreeRTOS ports.  The portEND_SWITCHING_ISR() macro is provided as part of
	 * the Cortex-M3 port layer for this purpose.  taskYIELD() must never be called
	 * from an ISR! */
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

/* Tarea 2 consumidora del semaforo */


#endif

#if (APPLICATION == APPLICATION_2)	/* Idem APP1 pero con direccion y roles invertidos */

#endif

#if (APPLICATION == APPLICATION_3)	/* Idem APP1 pero 3 tareas comparten el uso de un LED
								   por un periodo mayor al timeslice y no pueden mezclarse */

#endif

#if (APPLICATION == APPLICATION_4)	/* Porton de cochera usando el statechart */

#endif
