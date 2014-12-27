//#define NODE_SERVER
#define NODE_TEMPRATURE

#ifdef NODE_SERVER
#include "../lib/avr-uart/uart.h"
void uart_putstr(char *str) {
	uart_puts(str);
}
#endif

#ifdef NODE_TEMPRATURE
#include "../lib/avr-temperature/ds18x20_demo/onewire.h"
#include "../lib/avr-temperature/ds18x20_demo/ds18x20.h"

#define MAXSENSORS 1
uint8_t gSensorIDs[MAXSENSORS][OW_ROMCODE_SIZE];

uint8_t temprature_init();
#endif

#include "config/rfm12_config.h"
#include "../lib/librfm12/src/rfm12.h"
#include "../lib/librfm12/src/rfm12.c"
#include <avr/interrupt.h>
#include <util/delay.h>


int main() {
	uint8_t tv[10] = {0};
#ifdef NODE_SERVER
	uart0_init(UART_BAUD_SELECT(BAUD, F_CPU));
#endif

	_delay_ms(100);
	rfm12_init();
	sei();

#ifdef NODE_SERVER
	uart_puts ("\n" "RFM12 Server\n");
#endif

	uint8_t x = 0;
#ifdef NODE_TEMPRATURE
	x = temprature_init();
#endif
	
	while (1==1) {
#ifdef NODE_SERVER
                if (rfm12_rx_status() == STATUS_COMPLETE) {
			uint8_t *bufptr;
                        //so we have received a message
                        bufptr = rfm12_rx_buffer(); //get the address of the current rx buffer

                        // dump buffer contents to uart                 
                        for (uint8_t i=0;i<rfm12_rx_len();i++) {
                                uart_putc ( bufptr[i] );
                        }

                        uart_puts ("\n");

                        // tell the implementation that the buffer
                        // can be reused for the next data.
                        rfm12_rx_clear();
                	_delay_us(100); //small delay so loop doesn't run as fast
                }
#endif

#ifdef NODE_TEMPRATURE
		if (x == 1) {
			uint8_t i = 0;
			for (i = 0; i < 20; i++) {
               			_delay_ms(100); //small delay so loop doesn't run as fast
			}
			char s[10];
			uint8_t j=0;
                        int16_t decicelsius;
                
			DS18X20_start_meas(DS18X20_POWER_PARASITE, NULL);
			_delay_ms( DS18B20_TCONV_12BIT );
			DS18X20_read_decicelsius_single(gSensorIDs[0][0], &decicelsius);
			tv[2] = decicelsius;
			DS18X20_format_from_decicelsius(decicelsius, s, 10);
			while (s[j] != '\0' && j < 8) {
				tv[j] = s[j];
				j++;
			}
                        rfm12_tx (sizeof(tv), 0, tv);
		}
#endif
                //rfm12 needs to be called from your main loop periodically.
                //it checks if the rf channel is free (no one else transmitting), and then
                //sends packets, that have been queued by rfm12_tx above.
                rfm12_tick();
	}
}


#ifdef NODE_TEMPRATURE
uint8_t temprature_init() {
	uint8_t result = 0x00;
	uint8_t nSensors = 0;
	uint8_t diff, i;
        uint8_t id[OW_ROMCODE_SIZE];

	ow_set_bus(&PIND,&PORTD,&DDRD,PD4);

	ow_reset();

	_delay_ms(1000);
	diff = OW_SEARCH_FIRST;
	while ( diff != OW_LAST_DEVICE && nSensors < MAXSENSORS ) {
		DS18X20_find_sensor( &diff, &id[0] );

		if( diff == OW_PRESENCE_ERR ) {
			result = 0xFF;
			break;
		}

		if( diff == OW_DATA_ERR ) {
			result = 0xFE;
			break;
		}

                for ( i=0; i < OW_ROMCODE_SIZE; i++ ) {
                        gSensorIDs[nSensors][i] = id[i];
                }
                nSensors++;
        }
        _delay_ms(1000);

	if (result == 0x00) {
		result = nSensors;
	}

	return result;
}
#endif
