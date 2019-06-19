/*
 * isr.h
 *
 *  Created on: Nov 19, 2018
 *      Author: carstenh
 */

#ifndef ISR_H_
#define ISR_H_

extern void (*BT_ISR_ptr)(void);

extern void EXTI9_5_IRQHandler(void);
extern void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

extern void HAL_GPIO_EXTI_Enable(void);

extern void USART2_IRQHandler(void);

extern void USART2_ISR_ENABLE(void);

extern void USART3_ISR_ENABLE(void);

#endif /* ISR_H_ */
