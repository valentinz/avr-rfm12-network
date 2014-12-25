#include "../lib/avr-uart/uart.h"
void uart_putstr(char *str) {
	uart_puts(str);
}
#include "config/rfm12_config.h"
#include "../lib/librfm12/src/rfm12.h"
#include "../lib/librfm12/src/rfm12.c"
#include <avr/interrupt.h>
#include <util/delay.h>


int main() {
	uint8_t *bufptr;
	uint16_t ticker = 0;
	uint8_t tv[] = "foobar";
	uint8_t packettype = 0xEE;
	uart0_init(UART_BAUD_SELECT(BAUD, F_CPU));
	_delay_ms(100);
	rfm12_init();
	sei();
	uint8_t i = 0;

	uart_puts ("\r\n" "RFM12 Pingpong test\r\n");

	
	while (1==1) {
                if (rfm12_rx_status() == STATUS_COMPLETE)
                {
                        //so we have received a message

                        uart_puts ("new packet: \"");

                        bufptr = rfm12_rx_buffer(); //get the address of the current rx buffer

                        // dump buffer contents to uart                 
                        for (i=0;i<rfm12_rx_len();i++)
                        {
                                uart_putc ( bufptr[i] );
                        }

                        uart_puts ("\"\r\n");

                        // tell the implementation that the buffer
                        // can be reused for the next data.
                        rfm12_rx_clear();
                }


                ticker ++;
                if(ticker == 3000){
                        ticker = 0;
                        uart_puts (".");
                        rfm12_tx (sizeof(tv), 0, tv);
                }

                //rfm12 needs to be called from your main loop periodically.
                //it checks if the rf channel is free (no one else transmitting), and then
                //sends packets, that have been queued by rfm12_tx above.
                rfm12_tick();

                _delay_us(100); //small delay so loop doesn't run as fast
	}
}
