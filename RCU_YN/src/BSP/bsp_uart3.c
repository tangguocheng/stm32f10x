#include "stm32f10x.h"
#include "task_485.h"

/*
 * 函数名：USARTx_Config
 * 描述  ：USART3 GPIO 配置,工作模式配置
 * 输入  ：无
 * 输出  : 无
 * 调用  ：外部调用
 */
void Uart3_init(u32 baud,u8 pre_priority,u8 sub_priority)
{
        GPIO_InitTypeDef GPIO_InitStructure;
        USART_InitTypeDef USART_InitStructure;

        /* config USART3 clock */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
        
        /* USART3 GPIO config */
        /* Configure USART3 Tx (PB.10) as alternate function push-pull */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        /* Configure USART3 Rx (PB.11) as input floating */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        /* USART3 mode config */
        USART_InitStructure.USART_BaudRate = baud;
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        USART_InitStructure.USART_Parity = USART_Parity_No;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

        USART_Init(USART3, &USART_InitStructure);

        NVIC_InitTypeDef NVIC_InitStructure;

        /* Enable the USARTy Interrupt */
        NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = pre_priority;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = sub_priority;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

        USART_Cmd(USART3, ENABLE);
        
        USART_ClearFlag(USART3, USART_FLAG_TC);
}

void Uart3_send_data(u8 data)
{
        USART_SendData(USART3, data);

        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}

static u8 data = 0;
void USART3_IRQHandler(void)
{
        if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
                data = (u8)USART_ReceiveData(USART3);
                receive_protocol_data(data);
        }
}
