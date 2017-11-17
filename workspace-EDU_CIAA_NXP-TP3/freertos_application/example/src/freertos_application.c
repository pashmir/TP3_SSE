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

#define APPLICATION (APPLICATION_1)

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
/* Configura las interrupciones */
static void prvSetupSoftwareInterrupt()
{
	/* The interrupt service routine uses an (interrupt safe) FreeRTOS API
	 * function so the interrupt priority must be at or below the priority defined
	 * by configSYSCALL_INTERRUPT_PRIORITY. */
	NVIC_SetPriority(mainSW_INTERRUPT_ID, mainSOFTWARE_INTERRUPT_PRIORITY);

	/* Enable the interrupt. */
	NVIC_EnableIRQ(mainSW_INTERRUPT_ID);
}

#if (APPLICATION == APPLICATION_1)	/* T1 periodica 500ms + IRQ (productor) -> semaforo ->
 	 	 	 	 	 	 	 	   	   T2 (consumidor/productor) -> Cola -> T3 (consumidor)*/

const char *TextForTask1="Tarea 1 periódica ejecutandose. Activando interrupción\r\n";
const char *TextForIRQ="Otorgando semáforo.\r\n";
const char *TextForTask2="Semáforo recibido. Enviando datos a la cola.\r\n";
const char *QueueMessage="Mensaje transmitido por cola";
/* Se crea el handler del semaforo*/
xSemaphoreHandle xTask2Semaphore;
/* Se crea el handler de la cola */
xQueueHandle xQueue;

/* Tarea periodica de 500 ms */
void PeriodicTask(void *pvParameters){
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
	DEBUGOUT(TextForIRQ);
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

/* Tarea 2 consumidora del semaforo, productora de cola */
void InterruptHandlerTask(void *pvParameters){
	portBASE_TYPE xStatus;
	while(1)
	{
		vSemaphoreTake(xTask2Semaphore,portMAX_DELAY);
		DEBUGOUT(TextForTask2);
		xStatus = xQueueSendToBack(xQueue, QueueMessage,(portTickType)0);

		if (xStatus != pdPASS) {
			/* We could not write to the queue because it was full - this must
			 * be an error as the receiving task should make space in the queue
			 * as soon as both sending tasks are in the Blocked state. */
			DEBUGOUT("No se pudo enviar a la cola.\r\n");
		}
	}
}

/* Tarea 3 consumidora de cola*/
void QueueReceiverTask(void *pvParameters){
	const portTickType xTicksToWait = 100 / portTICK_RATE_MS;	// tiene que ser un tiempo
																// menor al de la interrupcion
	char *ReceivedText;
	/* As this task unblocks immediately that data is written to the queue this
	 * call should always find the queue empty. */
	if (uxQueueMessagesWaiting(xQueue) != 0) {
		DEBUGOUT("La cola debería estar vacía!\r\n");
	}

	/* The first parameter is the queue from which data is to be received.  The
	 * queue is created before the scheduler is started, and therefore before this
	 * task runs for the first time.
	 *
	 * The second parameter is the buffer into which the received data will be
	 * placed.  In this case the buffer is simply the address of a variable that
	 * has the required size to hold the received data.
	 *
	 * The last parameter is the block time � the maximum amount of time that the
	 * task should remain in the Blocked state to wait for data to be available should
	 * the queue already be empty. */
	xStatus = xQueueReceive(xQueue, ReceivedText, xTicksToWait);

	if (xStatus == pdPASS) {
	/* Data was successfully received from the queue, print out the received
	 * value. */
	DEBUGOUT(ReceivedText);
	}
	else {
		/* We did not receive anything from the queue even after waiting for 100ms.
		 * This must be an error as the sending tasks are free running and will be
		 * continuously writing to the queue. */
		DEBUGOUT("No se pudo recibir de la cola.\r\n");	// que mal suena esto
	}
}

int main(void){
	/* Sets up system hardware */
	prvSetupHardware();

	/* Se crea el semaforo */
	vSemaphoreCreateBinary(xTask2Semaphore);
	/* Se crea la cola */
	xQueue = xQueueCreate(5, sizeof(char *));

	/* Se comprueba que el semaforo y la cola se hayan creado con éxito */
	if (xTask2Semaphore != (xSemaphoreHandle) NULL && xQueue != (xQueueHandle)NULL) {
		DEBUGOUT("\r\n***** Ej 4: *****\n\r");
		/* Enable the software interrupt and set its priority. */
    	prvSetupSoftwareInterrupt();

		vTaskCreate(PeriodicTask,(char*)"Tarea periodica",configMINIMAL_STACK_SIZE,NULL,
   				(tskIDLE_PRIORITY + 1UL), (xTaskHandle *) NULL);
		vTaskCreate(InterruptHandlerTask,(char*)"Tarea asociada a interrupcion",
				configMINIMAL_STACK_SIZE,(tskIDLE_PRIORITY + 2UL),(xTaskHandle *)NULL);
		vTaskCreate(QueueReceiverTask,(char*)"Tarea receptora de cola",configMINIMAL_STACK_SIZE,
				(tskIDLE_PRIORITY + 3UL),(xTaskHandle *)NULL);
		vTaskStartScheduler();
	}
	while(1);
	return ((int) NULL);

}

#endif

#if (APPLICATION == APPLICATION_2)	/* Idem APP1 pero con direccion y roles invertidos */

#endif

#if (APPLICATION == APPLICATION_3)	/* Idem APP1 pero 3 tareas comparten el uso de un LED
								   por un periodo mayor al timeslice y no pueden mezclarse */

#endif

#if (APPLICATION == APPLICATION_4)	/* Porton de cochera usando el statechart */

#endif
