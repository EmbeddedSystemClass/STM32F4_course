#include "stm32f4xx_conf.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_dac.h"

#define ADC_1_ADDRESS_BASE 0x40012000
// ADC_DR = ADC regular Data Register
#define ADC_DR_ADDRESS_OFFSET 0x4C
volatile uint16_t valueFromADC;

void MY_DMA_initP2M(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	DMA_InitTypeDef strukturaDoInicjalizacjiDMA;
	// wyb�r kana�u DMA
	strukturaDoInicjalizacjiDMA.DMA_Channel = DMA_Channel_0;
	// ustalenie rodzaju transferu (memory2memory / peripheral2memory / memory2peripheral)
	strukturaDoInicjalizacjiDMA.DMA_DIR = DMA_DIR_PeripheralToMemory;
	// tryb pracy - pojedynczy transfer b�d� powtarzany
	strukturaDoInicjalizacjiDMA.DMA_Mode = DMA_Mode_Circular;
	// ustalenie priorytetu danego kana�u DMA
	strukturaDoInicjalizacjiDMA.DMA_Priority = DMA_Priority_Medium;
	// liczba danych do przes�ania
	strukturaDoInicjalizacjiDMA.DMA_BufferSize = (uint32_t)1;
	// adres �r�d�owy
	strukturaDoInicjalizacjiDMA.DMA_PeripheralBaseAddr = (uint32_t)(ADC_1_ADDRESS_BASE+ADC_DR_ADDRESS_OFFSET);
	// adres docelowy
	strukturaDoInicjalizacjiDMA.DMA_Memory0BaseAddr = (uint32_t)&valueFromADC;
	// okreslenie, czy adresy maj� by� inkrementowane po ka�dej przes�anej paczce danych
	strukturaDoInicjalizacjiDMA.DMA_MemoryInc = DMA_MemoryInc_Disable;
	strukturaDoInicjalizacjiDMA.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	// ustalenie rozmiaru przesy�anych danych
	strukturaDoInicjalizacjiDMA.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	strukturaDoInicjalizacjiDMA.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	// ustalenie trybu pracy - jednorazwe przes�anie danych
	strukturaDoInicjalizacjiDMA.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	strukturaDoInicjalizacjiDMA.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	// wy��czenie kolejki FIFO (nie u�ywana w tym przykadzie)
	strukturaDoInicjalizacjiDMA.DMA_FIFOMode = DMA_FIFOMode_Disable;
	// wype�nianie wszystkich p�l struktury jest niezb�dne w celu poprawnego dzia�ania, wpisanie jednej z dozwolonych wartosci
	strukturaDoInicjalizacjiDMA.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	// zapisanie wype�nionej struktury do rejestr�w wybranego po��czenia DMA
	DMA_Init(DMA2_Stream4, &strukturaDoInicjalizacjiDMA);
	// uruchomienie odpowiedniego po��czenia DMA
	DMA_Cmd(DMA2_Stream4, ENABLE);
}

void MY_ADC_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA , ENABLE); // wejscie ADC
	//inicjalizacja wej�cia ADC
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	// niezale�ny tryb pracy przetwornik�w
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	// zegar g��wny podzielony przez 2
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	// opcja istotna tylko dla tryby multi ADC
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	// czas przerwy pomi�dzy kolejnymi konwersjami
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //ADC
	ADC_InitTypeDef ADC_InitStructure;
	//ustawienie rozdzielczo�ci przetwornika na maksymaln� (12 bit�w)
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	//wy��czenie trybu skanowania (odczytywa� b�dziemy jedno wej�cie ADC
	//w trybie skanowania automatycznie wykonywana jest konwersja na wielu //wej�ciach/kana�ach)
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	//w��czenie ci�g�ego trybu pracy
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	//wy��czenie zewn�trznego wyzwalania
	//konwersja mo�e by� wyzwalana timerem, stanem wej�cia itd. (szczeg�y w //dokumentacji)
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	//warto�� binarna wyniku b�dzie podawana z wyr�wnaniem do prawej
	//funkcja do odczytu stanu przetwornika ADC zwraca warto�� 16-bitow�
	//dla przyk�adu, warto�� 0xFF wyr�wnana w prawo to 0x00FF, w lewo 0x0FF0
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	//liczba konwersji r�wna 1, bo 1 kana�
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	// zapisz wype�nion� struktur� do rejestr�w przetwornika numer 1
	ADC_Init(ADC1, &ADC_InitStructure);
	// konfiguracja czasu pr�bkowania sygna�u
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_84Cycles);
	// w��czenie wyzwalania DMA po ka�dym zako�czeniu konwersji
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

	// w��czenie DMA dla ADC
	ADC_DMACmd(ADC1, ENABLE);
	// uruchomienie modu�y ADC
	ADC_Cmd(ADC1, ENABLE);
}

void lab10_P2M(void)
{
	/* GPIOD Periph clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitTypeDef  GPIO_InitStructure;
	/* Configure PD12, PD13, PD14 and PD15 in output push-pull mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	MY_DMA_initP2M();
	MY_ADC_init();

	ADC_SoftwareStartConv(ADC1);

	const uint16_t fullRange = 0xFFF;
	const uint16_t oneFifthOfRange = (fullRange/5);
	const uint16_t twoFifthsOfRange = (2*fullRange/5);
	const uint16_t threeFifthsOfRange = (3*fullRange/5);
	const uint16_t fourFifthsOfRange = (4*fullRange/5);

	for(;;)
	{
		if(valueFromADC < oneFifthOfRange)
		{
			GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
		}
		else if(valueFromADC < twoFifthsOfRange)
		{
			GPIO_SetBits(GPIOD, GPIO_Pin_12);
			GPIO_ResetBits(GPIOD, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
		}
		else if(valueFromADC < threeFifthsOfRange)
		{
			GPIO_SetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13);
			GPIO_ResetBits(GPIOD, GPIO_Pin_14 | GPIO_Pin_15);
		}
		else if(valueFromADC < fourFifthsOfRange)
		{
			GPIO_SetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14);
			GPIO_ResetBits(GPIOD, GPIO_Pin_15);
		}
		else
		{
			GPIO_SetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
		}
	}
}

#define DAC_CHANNEL_1_ADDRESS_BASE 0x40007400
// DAC_DHR12R1 = DAC Data Holding Register 12 bits, Right aligned channel 1
#define DAC_DHR12R1_ADDRESS_OFFSET 0x08
#define DMA_DAC_SIGNAL_SIZE 32
const uint16_t DAC_DMA_sine12bit[DMA_DAC_SIGNAL_SIZE] = {
2047, 2447, 2831, 3185, 3498, 3750, 3939, 4056, 4095, 4056,
3939, 3750, 3495, 3185, 2831, 2447, 2047, 1647, 1263, 909,
599, 344, 155, 38, 0, 38, 155, 344, 599, 909, 1263, 1647
};

void MY_DMA_initM2P(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	DMA_InitTypeDef strukturaDoInicjalizacjiDMA;
	// wyb�r kana�u DMA
	strukturaDoInicjalizacjiDMA.DMA_Channel = DMA_Channel_7;
	// ustalenie rodzaju transferu (memory2memory / peripheral2memory / memory2peripheral)
	strukturaDoInicjalizacjiDMA.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	// tryb pracy - pojedynczy transfer b�d� powtarzany
	strukturaDoInicjalizacjiDMA.DMA_Mode = DMA_Mode_Circular;
	// ustalenie priorytetu danego kana�u DMA
	strukturaDoInicjalizacjiDMA.DMA_Priority = DMA_Priority_Medium;

	// liczba danych do przes�ania
	strukturaDoInicjalizacjiDMA.DMA_BufferSize = (uint32_t)DMA_DAC_SIGNAL_SIZE;
	// adres �r�d�owy
	strukturaDoInicjalizacjiDMA.DMA_Memory0BaseAddr = (uint32_t)DAC_DMA_sine12bit;
	// adres docelowy
	strukturaDoInicjalizacjiDMA.DMA_PeripheralBaseAddr = (uint32_t)(DAC_CHANNEL_1_ADDRESS_BASE + DAC_DHR12R1_ADDRESS_OFFSET);
	// zezwolenie na inkrementacje adresu po ka�dej przes�anej paczce danych
	strukturaDoInicjalizacjiDMA.DMA_MemoryInc = DMA_MemoryInc_Enable;
	strukturaDoInicjalizacjiDMA.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	// ustalenie rozmiaru przesy�anych danych
	strukturaDoInicjalizacjiDMA.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	strukturaDoInicjalizacjiDMA.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	// ustalenie trybu pracy - jednorazwe przes�anie danych
	strukturaDoInicjalizacjiDMA.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	strukturaDoInicjalizacjiDMA.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	// wy��czenie kolejki FIFO (nie u�ywana w tym przykadzie)
	strukturaDoInicjalizacjiDMA.DMA_FIFOMode = DMA_FIFOMode_Disable;
	// wype�nianie wszystkich p�l struktury jest niezb�dne w celu poprawnego dzia�ania, wpisanie jednej z dozwolonych wartosci
	strukturaDoInicjalizacjiDMA.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	// zapisanie wype�nionej struktury do rejestr�w wybranego po��czenia DMA
	DMA_Init(DMA1_Stream5, &strukturaDoInicjalizacjiDMA);
	// uruchomienie odpowiedniego po��czenia DMA
	DMA_Cmd(DMA1_Stream5, ENABLE);
	// uruchomienie DMA dla pierwszego kana�u DAC
	DAC_DMACmd(DAC_Channel_1, ENABLE);
}

void MY_DAC_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA , ENABLE); // wyjscie DAC
	//inicjalizacja wyj�cia DAC
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE); //DAC
	DAC_InitTypeDef DAC_InitStructure;
	//wy��czenie zewn�trznego wyzwalania
	//konwersja mo�e by� wyzwalana timerem, stanem wej�cia itd. (szczeg�y w //dokumentacji)
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
	//wy��czamy generator predefiniowanych przebieg�w wyj�ciowych (warto�ci //zadajemy sami, za pomoc� opowiedniej funkcji)
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	//w��czamy buforowanie sygna�u wyj�ciowego
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);
	DAC_SetChannel1Data(DAC_Align_12b_R, 0x000);
	DAC_Cmd(DAC_Channel_1, ENABLE);
}

void MY_DAC_initTimerForUpdating(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Prescaler = 42-1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 20;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
	TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);
	TIM_SetCounter(TIM6, 0);
	TIM_Cmd(TIM6, ENABLE);
}

void lab10_M2P(void)
{
	MY_DAC_init();
	MY_DAC_initTimerForUpdating();
	MY_DMA_initM2P();

	for(;;)
	{
		asm("nop");
	}
}

void waitUntilDMATransferHasFinished(void)
{
	while(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0) == SET)
	{
		asm("nop");
	}
}
