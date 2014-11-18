#include <VirtualWire.h>

#define LED_PIN 2

void setup()
{
  pinMode(LED_PIN, OUTPUT);
    vw_set_ptt_inverted(true);  // Required by the RF module=
    vw_setup(1000);            // bps connection speed
    vw_set_tx_pin(0);         // Arduino pin to connect the receiver data pin
    vw_set_ptt_pin(1);
    vw_set_rx_pin(3); 
    
}
 
void loop()
{
   //Message to send:
   const char *msg = "IT WORKS!!!";
   digitalWrite(LED_PIN, HIGH);
   vw_send((uint8_t *)msg, strlen(msg));

   vw_wait_tx();        // We wait to finish sending the message
   digitalWrite(LED_PIN, LOW);
   delay(200);         // We wait to send the message again                
}
