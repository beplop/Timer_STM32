#include "main.h"

#define DELAY_VAL 100
uint32_t reverse = 0;
int num = -1;
char RxBuffer[256];
char TxBuffer[256];
bool CommandRecieved = false;
uint32_t period_value = 1000;
char mode[8] = "UP";


void init_gpio(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN; //тактирование порта С и альтернативной функции потока ввода/вывода

	GPIOC->CRL &= ~(GPIO_CRL_CNF2|GPIO_CRL_CNF3|GPIO_CRL_CNF4|GPIO_CRL_CNF5|GPIO_CRL_CNF6); // pin2-6 push-pull ("00")
	GPIOC->CRH &= ~(GPIO_CRH_CNF8|GPIO_CRH_CNF9|GPIO_CRH_CNF10|GPIO_CRH_CNF11); // pin8-11 push-pull ("00")
	GPIOC->CRL |= GPIO_CRL_MODE2|GPIO_CRL_MODE3|GPIO_CRL_MODE4|GPIO_CRL_MODE5|GPIO_CRL_MODE6; // pin2-6 Output mode 50mHz ("11")
	GPIOC->CRH |= GPIO_CRH_MODE8|GPIO_CRH_MODE9|GPIO_CRH_MODE10|GPIO_CRH_MODE11; // pin8-11 Output mode 50mHz ("11")

	GPIOC->CRH |= GPIO_CRH_CNF13_1 | GPIO_CRH_CNF12_1; //см. ниже
	GPIOC->CRH &= ~(GPIO_CRH_CNF13_0|GPIO_CRH_MODE13|GPIO_CRH_CNF12_0|GPIO_CRH_MODE12); //pin13, pin 12 Input mode с подтяжкой, pull up/down ("1000")

	GPIOC->BSRR = GPIO_BSRR_BS13; //подтяжка pin13 с кнопкой к 1
	GPIOC->BSRR = GPIO_BSRR_BS12; //подтяжка pin12 с кнопкой к 1


}
void digit(uint32_t num)
{
	switch(num)
	{
	case 0:
		GPIOC->BSRR = GPIO_BSRR_BS2|GPIO_BSRR_BS3|GPIO_BSRR_BS4|GPIO_BSRR_BS5|GPIO_BSRR_BS6|GPIO_BSRR_BS8|GPIO_BSRR_BR9;
		break;
	case 1:
		GPIOC->BSRR = GPIO_BSRR_BR2|GPIO_BSRR_BS3|GPIO_BSRR_BS4|GPIO_BSRR_BR5|GPIO_BSRR_BR6|GPIO_BSRR_BR8|GPIO_BSRR_BR9;
		break;
	case 2:
		GPIOC->BSRR = GPIO_BSRR_BS2|GPIO_BSRR_BS3|GPIO_BSRR_BR4|GPIO_BSRR_BS5|GPIO_BSRR_BS6|GPIO_BSRR_BR8|GPIO_BSRR_BS9;
		break;
	case 3:
		GPIOC->BSRR = GPIO_BSRR_BS2|GPIO_BSRR_BS3|GPIO_BSRR_BS4|GPIO_BSRR_BS5|GPIO_BSRR_BR6|GPIO_BSRR_BR8|GPIO_BSRR_BS9;
		break;
	case 4:
		GPIOC->BSRR = GPIO_BSRR_BR2|GPIO_BSRR_BS3|GPIO_BSRR_BS4|GPIO_BSRR_BR5|GPIO_BSRR_BR6|GPIO_BSRR_BS8|GPIO_BSRR_BS9;
		break;
	case 5:
		GPIOC->BSRR = GPIO_BSRR_BS2|GPIO_BSRR_BR3|GPIO_BSRR_BS4|GPIO_BSRR_BS5|GPIO_BSRR_BR6|GPIO_BSRR_BS8|GPIO_BSRR_BS9;
		break;
	case 6:
		GPIOC->BSRR = GPIO_BSRR_BS2|GPIO_BSRR_BR3|GPIO_BSRR_BS4|GPIO_BSRR_BS5|GPIO_BSRR_BS6|GPIO_BSRR_BS8|GPIO_BSRR_BS9;
		break;
	case 7:
		GPIOC->BSRR = GPIO_BSRR_BS2|GPIO_BSRR_BS3|GPIO_BSRR_BS4|GPIO_BSRR_BR5|GPIO_BSRR_BR6|GPIO_BSRR_BR8|GPIO_BSRR_BR9;
		break;
	case 8:
		GPIOC->BSRR = GPIO_BSRR_BS2|GPIO_BSRR_BS3|GPIO_BSRR_BS4|GPIO_BSRR_BS5|GPIO_BSRR_BS6|GPIO_BSRR_BS8|GPIO_BSRR_BS9;
		break;
	case 9:
		GPIOC->BSRR = GPIO_BSRR_BS2|GPIO_BSRR_BS3|GPIO_BSRR_BS4|GPIO_BSRR_BS5|GPIO_BSRR_BR6|GPIO_BSRR_BS8|GPIO_BSRR_BS9;
		break;
	default:
		break;
	}
}

void delay(uint32_t delay_value)
{
	for(uint32_t i = 0; i< delay_value; i++);
}

void init_interrupt()
{
	EXTI->IMR |= EXTI_IMR_MR13; //накладываем маску и разрешаем прерывания
	EXTI->FTSR |= EXTI_FTSR_TR13; //по спаду реагирует

	AFIO->EXTICR[3] |= AFIO_EXTICR4_EXTI13_PC; //включение альтернативной функции потока ввода/вывода 13 пина

	EXTI->IMR |= EXTI_IMR_MR12; //накладываем маску и разрешаем прерывания
	EXTI->FTSR |= EXTI_FTSR_TR12; //по спаду реагирует

	AFIO->EXTICR[3] |= AFIO_EXTICR4_EXTI12_PC; //включение альтернативной функции потока ввода/вывода 12 пина

	NVIC_EnableIRQ(EXTI15_10_IRQn);  //разрешаем прерывания на линии 10-15
	NVIC_SetPriority(EXTI15_10_IRQn,0); //приоритет прерывания
}


void EXTI15_10_IRQHandler(void)  //вызывается при нажатии на кнопку
{
	if(EXTI->PR & EXTI_PR_PR12) //проверка, что прерывание по 12 линии
	{
		TIM2->CR1 ^= TIM_CR1_CEN; // меняем значение работы таймера

		delay(DELAY_VAL);
		EXTI->PR |= EXTI_PR_PR12;
	}

	if(EXTI->PR & EXTI_PR_PR13) //проверка, что прерывание по 13 линии
	{
		reverse = !reverse; // меняем направление счета

		delay(DELAY_VAL);
		EXTI->PR |= EXTI_PR_PR13;
	}
}

void initClk(void)
{
	// Enable HSI
	RCC->CR |= RCC_CR_HSION;
	while(!(RCC->CR & RCC_CR_HSIRDY)){};

	// Enable Prefetch Buffer
	FLASH->ACR |= FLASH_ACR_PRFTBE;

	// Flash 2 wait state
	FLASH->ACR &= ~FLASH_ACR_LATENCY;
	FLASH->ACR |= FLASH_ACR_LATENCY_2;

	// HCLK = SYSCLK
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

	// PCLK2 = HCLK
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;

	// PCLK1 = HCLK
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

	// PLL configuration: PLLCLK = HSI/2 * 16 = 64 MHz
	RCC->CFGR &= ~RCC_CFGR_PLLSRC;
	RCC->CFGR |= RCC_CFGR_PLLMULL16;

	// Enable PLL
	RCC->CR |= RCC_CR_PLLON;

	// Wait till PLL is ready
	while((RCC->CR & RCC_CR_PLLRDY) == 0) {};

	// Select PLL as system clock source
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_PLL;

	// Wait till PLL is used as system clock source
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL){};
}

void init_tim2(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; //Включить тактирование TIM6
	//Частота APB1 для таймеров = APB1Clk * 2 = 32МГц * 2 = 64МГц
	TIM2->PSC = 64000-1;					//Предделитель частоты (64МГц/64000 = 1кГц)
	TIM2->ARR = 1000-1;						//Модуль счёта таймера (1кГц/1000 = 1с)
	TIM2->DIER |= TIM_DIER_UIE;				//Разрешить прерывание по переполнению таймера
	TIM2->CR1 |= TIM_CR1_CEN;				//Включить таймер

	NVIC_EnableIRQ(TIM2_IRQn);				//Разрешить прерывание от TIM2
	NVIC_SetPriority(TIM2_IRQn, 1);			//Выставляем приоритет
}



void TIM2_IRQHandler(void)
{
	if(reverse)
	{
		num--;
	}
	else num++;

	if(num > 30)
	{
		num = 0;
	}

	if((num < 0) && (reverse == 1))
	{
		num = 30;
	}
	TIM2->SR &= ~TIM_SR_UIF;			//Сброс флага переполнения
}

void init_UART2(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

	//PA2 - OUT, PA3 - IN
	GPIOA->CRL &= ~ (GPIO_CRL_CNF2 | GPIO_CRL_MODE2);
	GPIOA->CRL |= GPIO_CRL_CNF2_1| GPIO_CRL_MODE2_1; //PA2 AFO push-pull, output 2MHz

	GPIOA->CRL &= ~ (GPIO_CRL_CNF3 | GPIO_CRL_MODE3);
	GPIOA->CRL |= GPIO_CRL_CNF3_0; //PA3 floating input

	//частота APB1 (f(APB2)/2) /(16*28800) = 69.4 |||| 69 в hex = 0x45; 0.4*16=6.4 в hex в итоге = 0x456
	USART2->BRR = 0x456; //28800 бод
	USART2->CR1 |= USART_CR1_UE|USART_CR1_RE|USART_CR1_TE|USART_CR1_RXNEIE; //вкл uart, вкл Rx, Tx, прерывания при приеме

	NVIC_EnableIRQ(USART2_IRQn);			//Разрешить прерывание от UART2ы
}

void USART2_IRQHandler(void)
{
	if((USART2->SR & USART_SR_RXNE)!=0)
	{
		uint8_t len = strlen(RxBuffer);
		RxBuffer[len] = USART2->DR;

		if (RxBuffer[len]==0x0D)
		{
			CommandRecieved = true;
		}
	}
}

void TxStr(char *str, bool crlf)
{
	if (crlf)
	{
		strcat(str, "\r"); //конкатинация символов конца строки
	}
	for(uint16_t i = 0; i<strlen(str); i++)
	{
		USART2->DR = str[i]; //побайтово отправляем
		while((USART2->SR & USART_SR_TC) == 0) {}; //ждем подтверждения что байт передан
	}
}

void ExecuteCommand(void)
{
	memset(TxBuffer,0,256);

	if(strncmp(RxBuffer, "*IDN?",5) == 0)
	{
		strcpy(TxBuffer, "Khamidullin, Khodzhich, Islomboev IU4-72B");

	}
	else if(strncmp(RxBuffer, "SET",3) == 0)
	{
		strcpy(TxBuffer, "Ok");
		int value;
		sscanf(RxBuffer, "%*s %u", &value); //звездочка игнорирует строку s; u - беззнаковое число
		if ((0<value)&&(value<31))
		{
			num = value;
		}
		else
		{
			strcpy(TxBuffer, "Invalid value");
		}

	}
	else if (strncmp(RxBuffer, "PERIOD ",7) == 0)
	{
		strcpy(TxBuffer, "Ok");
		sscanf(RxBuffer, "%*s %u", &period_value); //звездочка игнорирует строку s; u - беззнаковое число
		if ((100<=period_value)&&(period_value<=5000))
		{
			TIM2->ARR = period_value;
			TIM2->CNT = 0;
		}
		else
		{
			strcpy(TxBuffer, "Invalid value");
		}
	}
	else if(strncmp(RxBuffer, "GET?",4) == 0)
		{
			sprintf(TxBuffer, "%d", num);
		}
	else if(strncmp(RxBuffer, "PERIOD?",7) == 0)
		{
			sprintf(TxBuffer, "%d", period_value);
		}
	else if(strncmp(RxBuffer, "DIR ",4) == 0)
		{
			sscanf(RxBuffer, "%*s %s", &mode); //звездочка игнорирует строку s
			if (strncmp(mode, "UP",2) == 0)
			{
				strcpy(TxBuffer, "Ok");
				reverse = 0;
			}
			if (strncmp(mode, "DOWN",4) == 0)
			{
				strcpy(TxBuffer, "Ok");
				reverse = 1;
			}
			else
			{
				strcpy(TxBuffer, "Invalid parameter");
			}
		}
	else if(strncmp(RxBuffer, "DIR?",4) == 0)
		{
			if (strncmp(mode, "UP",2) == 0)
			{
				strcpy(TxBuffer, "UP");
			}
			if (strncmp(mode, "DOWN",4) == 0)
			{
				strcpy(TxBuffer, "DOWN");
			}
		}
	else
	{
		strcpy(TxBuffer, "Unknown command");
	}

	TxStr(TxBuffer, true);
	memset(RxBuffer,0,256);
	CommandRecieved = false;
}

int main(void)
{
	initClk();
	init_gpio();
	init_interrupt();
	init_tim2();
	init_UART2();

    while(true)
    {
    	if(CommandRecieved)
		{
			ExecuteCommand();
		}
    	GPIOC->BSRR = GPIO_BSRR_BS10|GPIO_BSRR_BR11;
    	digit(num % 10);
    	delay(DELAY_VAL);

    	GPIOC->BSRR = GPIO_BSRR_BR10|GPIO_BSRR_BS11;
    	digit(num / 10);
    	delay(DELAY_VAL);
    }
}
