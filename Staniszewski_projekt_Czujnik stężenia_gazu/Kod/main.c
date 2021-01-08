/*-------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - laboratorium
					Temat: Czujnik steżenia gazów palnych z użyciem I2C oraz czujnika MQ-2
					autor projektu: Dariusz Staniszewski
					wersja: 04.12.2020r.
----------------------------------------------------------------------------*/
					
#include "MKL05Z4.h"
#include "ADC.h"
#include "frdm_bsp.h"
#include "lcd1602.h"
#include "pit.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LED_R 11
#define LED_Y 6
#define LED_G 7		
#define BUZZER 11 	//Sterowanie PTA 11


// Handshake
uint8_t wynik_ok=0; 																					// Handshake petla glowna-adc
uint8_t pit_ok = 0; 																					// Handshake pit-petla glowna

// Obliczenia
uint16_t temp;																								// Przechowuje zmienną tymczasową
float adc_volt_coeff = ((float)(((float)2.91) / 4095) );			// Współczynnik korekcji wyniku, w stosunku do napięcia referencyjnego przetwornika
																															// 12 bitów przetwornik i 2,91V
// Uśrednianie pomiaru
float	wynik , suma , wynik_fin; 															// Wynik - pojedyńczy pomiar, suma - suma z 5 okresów pomiarów, wynik_fin - gotowa zmienna o przeskalowanej i uśrednionej wartości
int count = 0; 																							  // Zliczanie ilości pomiarów

// Zmienne czas i alarm
char alarm = 0;
char czas =0;
//int a=0;

void ADC0_IRQHandler(){	
	temp = ADC0->R[0];	// Odczyt danej i skasowanie flagi COCO
	if(!wynik_ok){			// Handshake petla glowna-adc
	
		wynik = temp;			// Wyślij nową daną do petli glownej
		wynik_ok=1;
	}
}

void PIT_IRQHandler(){
	PIT->CHANNEL[0].TFLG &= PIT_TFLG_TIF_MASK;	// Skasuj flagę żądania przerwania
	wynik_fin = suma/count; 									  // Usrednianie
	count = 0;
	suma = 0;
	NVIC_ClearPendingIRQ(PIT_IRQn);
	pit_ok = 1; 																// Handshake pit-petla glowna
}

void state(){
		if (wynik_fin > 0.49 && wynik_fin <1.18) { alarm=1; }
		if (wynik_fin > 1.18 && wynik_fin <2.1)  { alarm=2; }
		if (wynik_fin > 2.1  && wynik_fin < 5)   { alarm=3; }
    if (wynik_fin <0.49) {alarm=0; }
		
}

void buzzer_mode(int a){
		if(a==1){
			DELAY(400);
			PTA->PTOR |= (1<<BUZZER);
		}else if(a==2){	
			DELAY(200);
			PTA->PTOR |= (1<<BUZZER);
		}else{	
			PTA->PCOR |= (1<<BUZZER);			
		}
}


void disp(){
			char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
			sprintf(display," %.2f V  ",wynik_fin);	
			LCD1602_SetCursor(0,0);
			LCD1602_Print(display);
			
			LCD1602_SetCursor(0,1);
			switch(alarm){
      case 0:
        LCD1602_Print("Poziom normalny  ");
				DELAY(4);
				PTB->PCOR |= (1<<LED_G); 	// Set to Low
				PTB->PSOR |= (1<<LED_R);
				PTB->PSOR |= (1<<LED_Y);
				buzzer_mode(0);
        break;
      case 1:
        LCD1602_Print("Poziom >100ppm   ");
				DELAY(4);
				PTB->PCOR |= (1<<LED_Y);
				PTB->PSOR |= (1<<LED_R);
				PTB->PSOR |= (1<<LED_G);
				buzzer_mode(0);
        break;
      case 2:
        LCD1602_Print("Poziom >1000ppm  ");
				DELAY(4);
				PTB->PCOR |= (1<<LED_R);
				PTB->PSOR |= (1<<LED_Y);
				PTB->PSOR |= (1<<LED_G);
				buzzer_mode(1);
        break;
      case 3:
        LCD1602_Print("Poziom >10000ppm ");
				DELAY(4);
				PTB->PCOR |= (1<<LED_R);
				PTB->PCOR |= (1<<LED_Y);
				PTB->PSOR |= (1<<LED_G);
				buzzer_mode(2);
        break;
				}
			
}

void heating(){								// Nagrzewanie czujnika
	    for (czas=59; czas>0; czas--){
      LCD1602_SetCursor(0,0);
      LCD1602_Print("Nagrzewanie...");
      LCD1602_SetCursor(0,1);
			char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
			sprintf(display,"Pozostalo: %d s ",czas);
			LCD1602_Print(display);
			DELAY(100); 	
			}
    LCD1602_ClearAll();
    czas=0;
}

int main (void){
	

	
	//LCD
	LCD1602_Init();		 					// Inicjalizacja wyświetlacza LCD
	LCD1602_Backlight(TRUE);
	LCD1602_Print("---");				// Ekran kontrolny - nie zniknie, jeśli dalsza część programu nie działa
	LCD1602_ClearAll();					
	
	//PIT
	PIT_Init();									// Inicjalizacja licznika PIT0
	
	// Kalibracje
	heating();									// Nagrzewanie czujnika
	
	uint8_t	kal_error;
	kal_error=ADC_Init();				// Inicjalizacja i kalibracja przetwornika A/C
	if(kal_error){	
		while(1);									// Klaibracja się nie powiodła
	}
	
	//GPIO start
	PORTB->PCR[LED_R]  |= PORT_PCR_MUX(1);			// GPIO
	PORTB->PCR[LED_Y]  |= PORT_PCR_MUX(1);	
	PORTB->PCR[LED_G]  |= PORT_PCR_MUX(1);	
	PTB->PDDR |= (1<<LED_R);								   	// Output
	PTB->PDDR |= (1<<LED_Y);								
	PTB->PDDR |= (1<<LED_G);								
	PTB->PSOR |= (1<<LED_R);										// Set to High
	PTB->PSOR |= (1<<LED_Y);								
	PTB->PSOR |= (1<<LED_G);
	
	PORTA->PCR[BUZZER]  |= PORT_PCR_MUX(1);
	PTA->PDDR |= (1<<BUZZER);		
	PTA->PCOR |= (1<<BUZZER);
	
	
	
	ADC0->SC1[0] = ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(12);		// Pierwsze wyzwolenie przetwornika ADC0 w kanale 12 i odblokowanie przerwania
	
	
	while(1){
		if(wynik_ok){
			suma += wynik*adc_volt_coeff;		// Dostosowanie wyniku do zakresu napięciowego
			count++;
			wynik_ok = 0;
		}
		if (pit_ok){											// Wykonaj po wystąpieniu przerwania PIT
			state();
			disp();
			pit_ok = 0;
		}
	}
}
