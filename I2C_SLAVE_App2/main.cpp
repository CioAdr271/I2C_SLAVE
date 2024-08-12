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

#define F_CPU 16000000

#define SLA 0x08 

#define DATA_BUFFER_SIZE 3 
#define MESSAGE_BUFFER_SIZE 10

char data_buffer[DATA_BUFFER_SIZE];
uint8_t data_buffer_index = 0;

char message_buffer[MESSAGE_BUFFER_SIZE];
uint8_t message_buffer_index = 0;

void i2c_init(void) 
{   
    TWAR = (SLA << 1);  // Setare adresa slave
    TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWIE);  // Activare TWI, activare ACK, activare intrerupere
}

ISR(TWI_vect) {
	cli();
    switch (TWSR & 0xF8) 
    {
        case TW_SR_SLA_ACK: // Adresã recunoscutã
			TWDR = 0;
            TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE); // Trimitere ACK
            data_buffer_index = 0; 
            break;
			
        case TW_SR_DATA_ACK: // Date primite
        {
			char received = TWDR;
			
            if (data_buffer_index < DATA_BUFFER_SIZE - 1) 
			{
                data_buffer[data_buffer_index++] = received; 
            }

            if (data_buffer_index == DATA_BUFFER_SIZE - 1) 
			{
                switch (data_buffer[0]) 
				{
                    case 's': 
                        switch (data_buffer[1]) {
                            case '1':
                                PORTB |= (1 << 0);  
							    strcpy(message_buffer, "");
                                break;
                            case '2':
                                PORTB |= (1 << 1);  
								strcpy(message_buffer, "LED-2-ON"); 
                                break;
                            case '3':
                                PORTB |= (1 << 2);  
								strcpy(message_buffer, "LED-3-ON"); 
                                break;
                            default:
								strcpy(message_buffer, "ERROR");
                                break;
                        }
                        break;
                    case 'r': 
                        switch (data_buffer[1]) {
                            case '1':
                                PORTB &= ~(1 << 0);  
								strcpy(message_buffer, "LED-1-OFF");
                                break;
                            case '2':
                                PORTB &= ~(1 << 1);  
								strcpy(message_buffer, "LED-2-OFF"); 
                                break;
                            case '3':
                                PORTB &= ~(1 << 2);  
								strcpy(message_buffer, "LED-3-OFF"); 
                                break;
                            default:
                                strcpy(message_buffer, "ERROR");
                                break;
                        }
                        break;
                    default:
                        strcpy(message_buffer, "ERROR");
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
            } else 
			{
                TWDR = '\0';  
            }
            TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);  // Trimitere ACK
            break;

        case TW_SR_STOP: // Stop sau Repeated Start
            TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
            break;
			
        default:
            TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);  
    }
	sei();
}

int main(void)
{
    i2c_init();
    sei();  
    DDRB = 0xFF;  
    PORTB = 0x00;  
    while (1) 
    {
     
    }
}
