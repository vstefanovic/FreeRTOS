/*
    FreeRTOS V8.2.2 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Library includes. */
#include "board.h"

/*
 * The FreeRTOS tick handler.  This function must be installed as the handler
 * for the timer used to generate the tick interrupt.  Note that the interrupt
 * generated by the PIT is shared by other system peripherals, so if the PIT is
 * used for Tick generation then FreeRTOS_Tick_Handler() can only be installed
 * directly as the PIT handler if no other system interrupts need to be
 * serviced.  If system interrupts other than the PIT need to be serviced then
 * install System_Handler() as the PIT interrupt handler in place of
 * FreeRTOS_Tick_Handler() and add additional interrupt processing into the
 * implementation of System_Handler().
 */
extern void FreeRTOS_Tick_Handler( void );
static void System_Handler( void );

/*-----------------------------------------------------------*/

static void System_Handler( void )
{
	/* See the comments above the function prototype in this file. */
	FreeRTOS_Tick_Handler();
}
/*-----------------------------------------------------------*/

/*
 * The application must provide a function that configures a peripheral to
 * create the FreeRTOS tick interrupt, then define configSETUP_TICK_INTERRUPT()
 * in FreeRTOSConfig.h to call the function.  This file contains a function
 * that is suitable for use on the Atmel SAMA5.
 */
void vConfigureTickInterrupt( void )
{
	/* NOTE:  The PIT interrupt is cleared by the configCLEAR_TICK_INTERRUPT()
	macro in FreeRTOSConfig.h. */

	/* Enable the PIT clock. */
	PMC->PMC_PCER0 = 1 << ID_PIT;

	/* Initialize the PIT to the desired frequency - specified in uS. */
	PIT_Init( 1000000UL / configTICK_RATE_HZ, ( BOARD_MCK / 2 ) / 1000000 );

	/* Enable IRQ / select PIT interrupt. */
	PMC->PMC_PCER1 = ( 1 << ( ID_IRQ - 32 ) );
	AIC->AIC_SSR  = ID_PIT;

	/* Ensure interrupt is disabled before setting the mode and installing the
	handler.  The priority of the tick interrupt should always be set to the
	lowest possible. */
	AIC->AIC_IDCR = AIC_IDCR_INTD;
	AIC->AIC_SMR  = AIC_SMR_SRCTYPE_EXT_POSITIVE_EDGE;
	AIC->AIC_SVR = ( uint32_t ) FreeRTOS_Tick_Handler;

	/* Start with the interrupt clear. */
	AIC->AIC_ICCR = AIC_ICCR_INTCLR;

	/* Enable the interrupt in the AIC and peripheral. */
	AIC_EnableIT( ID_PIT );
	PIT_EnableIT();

	/* Enable the peripheral. */
	PIT_Enable();

	/* Prevent compiler warnings in the case where System_Handler() is not used
	as the handler.  See the comments above the System_Handler() function
	prototype at the top of this file. */
	( void ) System_Handler;
}
/*-----------------------------------------------------------*/
