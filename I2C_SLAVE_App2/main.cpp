/*
 * I2C_SLAVE.cpp
 *
 * Created: 01.08.2024 09:26:36
 * Author : Adrian
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <string.h> 
#include <util/delay.h>

#define F_CPU 16000000

#define SLA 0x08 

#define DATA_BUFFER_SIZE 3 
#define MESSAGE_BUFFER_SIZE 10

char data_buffer[DATA_BUFFER_SIZE];
uint8_t data_buffer_index = 0;

char message_buffer[MESSAGE_BUFFER_SIZE];
uint8_t message_buffer_index = 0;

#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

void USART_Init(unsigned int ubrr)
{
	//Set baud rate
	UBRR0H = (unsigned char) (ubrr >>8);
	UBRR0L = (unsigned char) ubrr;
	
	//Enable transmission and reception
	UCSR0B = (1 << RXEN0) | (1 <<TXEN0);

	//Set frame format:8data, 2 stop bit
	UCSR0C = (1 << USBS0) | (1 << UCSZ00) | (1 << UCSZ01) | (1 << UPM01);
	
}

void USART_Transmitter(char data)
{
		while(!(UCSR0A & (1 << UDRE0)));
		UDR0 = data;
}

void USART_TransmitString(const char* str) {
	while (*str) { 
		USART_Transmitter(*str); 
		str++; 
	}
}

void i2c_init(void) 
{   
    TWAR = (SLA << 1);  // Setare adresa slave
    TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWIE);  // Activare TWI, activare ACK, activare intrerupere
}

ISR(TWI_vect) {
	cli();  // Dezactiveazã întreruperile globale pentru a preveni conflictele

	switch (TWSR & 0xF8)
	{
		case TW_SR_SLA_ACK: // Adresã recunoscutã
		//USART_TransmitString("\r\n");
		TWDR = 0;
		TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE); // Trimitere ACK
		data_buffer_index = 0;
		break;
		
		case TW_SR_DATA_ACK: // Date primite
		{
			char received = TWDR;
			
			// Adaugã caracterul primit la buffer
			if (data_buffer_index < DATA_BUFFER_SIZE - 1)
			{
				data_buffer[data_buffer_index++] = received;
			}

			// Trimitere caracter pe UART
			//USART_Transmitter(received);
			

			if (data_buffer_index == DATA_BUFFER_SIZE - 1)
			{
				
				switch (data_buffer[0])
				{
					case 's':
					switch (data_buffer[1]) {
						case '1':
						PORTB |= (1 << 0);
						//USART_TransmitString("\r\n");
						strcpy(message_buffer, "LED-1-ON");
						break;
						case '2':
						PORTB |= (1 << 1);
						//USART_TransmitString("\r\n");
						strcpy(message_buffer, "LED-2-ON");
						break;
						case '3':
						PORTB |= (1 << 2);
						//USART_TransmitString("\r\n");
						strcpy(message_buffer, "LED-3-ON");
						break;
						default:
						//USART_TransmitString("\r\n");
						strcpy(message_buffer, "--ERROR--");
						break;
					}
					break;
					case 'r':
					switch (data_buffer[1]) {
						case '1':
						PORTB &= ~(1 << 0);
						//USART_TransmitString("\r\n");
						strcpy(message_buffer, "LED-1-OFF");
						break;
						case '2':
						PORTB &= ~(1 << 1);
						//USART_TransmitString("\r\n");
						strcpy(message_buffer, "LED-2-OFF");
						break;
						case '3':
						PORTB &= ~(1 << 2);
						//USART_TransmitString("\r\n");
						strcpy(message_buffer, "LED-3-OFF");
						break;
						default:
						//USART_TransmitString("\r\n");
						
						strcpy(message_buffer, "--ERROR--");
						break;
					}
					break;
					default:
					//USART_TransmitString("\r\n");
					strcpy(message_buffer, "--ERROR--");
					break;
				}
				data_buffer_index = 0;
				message_buffer_index = 0;
			}
			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE); // Trimitere ACK
		}
		break;
		
		case TW_ST_DATA_ACK:  // Master solicitã date
		if (message_buffer_index < MESSAGE_BUFFER_SIZE)
		{
			
			TWDR = message_buffer[message_buffer_index++];
			//USART_Transmitter(TWDR);

		} else
		{
			TWDR = '\0';
		}
		
		
		TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);  // Trimitere ACK
		break;

		case TW_SR_STOP: // Stop sau Repeated Start
		TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
		break;
		
		 case TW_ST_SLA_ACK:
		 //USART_TransmitString("Eroare");
		  if (message_buffer_index < MESSAGE_BUFFER_SIZE) 
		  {
			  TWDR = message_buffer[message_buffer_index++];
			  } else 
			  {
			  TWDR = '\0';
		  }
		  TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);  // Pregãtire pentru urmãtoarea transmisie
		 break;
		
		default:
		TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
	}
	sei();  // Reactivare întreruperi globale
}

int main(void)
{
	USART_Init(MYUBRR);
    i2c_init();
    sei();  
    DDRB = 0xFF;  
    PORTB = 0x00;  
    while (1) 
    {
     
    }
}
