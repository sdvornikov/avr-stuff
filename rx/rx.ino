#include <VirtualWire.h>
 
void setup()
{
    Serial.begin(9600);          // Configure the serial connection to the computer
    vw_set_ptt_inverted(true);  // Required by the RF module
    vw_setup(2000/8);            // bps connection speed - divided by 8 to work with ATtiny45 (programmed for 8Mhz)
    vw_set_rx_pin(3);         // Arduino pin to connect the receiver data pin
    vw_rx_start();           // Start the receiver
}
 
void loop()
{

  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  if (vw_get_message(buf, &buflen))      // We check if we have received data
  {
    int i;
    // Message with proper check    
    for (i = 0; i < buflen; i++)
    {
      Serial.write(buf[i]);  // The received data is stored in the buffer
                            // and sent through the serial port to the computer
    }
    Serial.println();
   
  }
}


