uint8_t point;
uint8_t s[128];

static uint8_t serDataFrom;
static uint8_t serChkSum;

static uint16_t serialNumber;


// ***********************************
// Interrupt driven UART transmitter for MIS_OSD
// ***********************************
static uint8_t tx_ptr;
static uint8_t tx_busy = 0;

ISR_UART{
	UDR0 = s[tx_ptr++];           /* Transmit next byte */
	if (tx_ptr == point) {      /* Check if all data is transmitted */
		UCSR0B &= ~(1 << UDRIE0);     /* Disable transmitter UDRE interrupt */
		tx_busy = 0;
	}
}

void UartSendData() {          // start of the data block transmission
	cli();
	tx_ptr = 0;
	UCSR0A |= (1 << UDRE0);        /* Clear UDRE interrupt flag */
	UCSR0B |= (1 << UDRIE0);       /* Enable transmitter UDRE interrupt */
	UDR0 = s[tx_ptr++];          /* Start transmission */
	tx_busy = 1;
	sei();
}
