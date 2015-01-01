#include "stm32f10x.h"
#include "task_485.h"
#include "bsp_delay.h"
/**
 * @brief UART Initialization
 * @param u32 baud - BaudRate,
 * @param u8 pre_priority - Preemption Priority
 * @param u8 sub_priority - SubPriority
 * @return NONE
 */
void Uart1_init(u32 baud,u8 pre_priority,u8 sub_priority)
{
        GPIO_InitTypeDef GPIO_InitStructure;
        USART_InitTypeDef USART_InitStructure;

        /* config USART1 clock */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

        /* USART1 GPIO config */
        /* Configure USART1 Tx (PA.09) as alternate function push-pull */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        
        /* Configure USART1 Rx (PA.10) as input floating */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        
        /* USART1 mode config */
        USART_InitStructure.USART_BaudRate = baud;
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        USART_InitStructure.USART_Parity = USART_Parity_No;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
        USART_Init(USART1, &USART_InitStructure);

        NVIC_InitTypeDef NVIC_InitStructure;

        /* Enable the USARTy Interrupt */
        NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = pre_priority;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = sub_priority;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

        USART_Cmd(USART1, ENABLE);
        
        USART_ClearFlag(USART1, USART_FLAG_TC); 
}

/**
 * @brief UART Send Byte
 * @param u8 data - Send Data,
 * @return NONE
 */
void Uart1_send_data(u8 data)
{
        USART_SendData(USART1, (uint8_t)data);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

static u8 data = 0;

/**
 * @brief UART IRQ Handler
 * @param NONE
 * @return NONE
 */
void USART1_IRQHandler(void)
{
        if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
                data = (u8)USART_ReceiveData(USART1);
                receive_protocol_data(data);
        }
}
