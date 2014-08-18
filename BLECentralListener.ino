
#include <stdarg.h>
#include "typedef.h"
#include "biscuit_central.h"
#include "ble_hci.h"

#if defined(__AVR_ATmega328P__) // Arduino UNO?
  #include <AltSoftSerial.h>
  AltSoftSerial Serial1;
  // Refer to this: http://www.pjrc.com/teensy/td_libs_AltSoftSerial.html
#endif

uint8_t found_address[6];
uint8_t last_address[6] = { 143,26,194,251,140,48};


int LED = 13;
int DiscoveredLED = 2;
int DiscoveredButton = A4;
int DiscoveredButton5V = A1;

unsigned long lastMillis;


int scan_delay = 20000;





void flashLED(int led) {
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(200);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(200); 
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(200);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(200); 
  // wait for a second
}


void p(char *fmt, ... )
{
  char tmp[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(tmp, 128, fmt, args);
  va_end (args);
  Serial.print(tmp);
}

void ble_event_poll()
{
}

byte ble_event_available()
{
  return Serial1.available(); 
}

byte ble_event_process()
{
  Serial.print("\r\n millis:");
  Serial.println(millis());

  uint8_t type, event_code, data_len, status1;
  uint16_t event;
  uint8_t buf[255];
  
  

  
  
  
  
  //get the details of the incoming message
  type = Serial1.read();//first byte
  //after first by, wait 35 seconds for the rest of the message to come in
  delay(35);
  event_code = Serial1.read();//second byte
  data_len = Serial1.read(); //third byte

  p("-----------------------------\r\n");
  p("-Type        : 0x%02X\r\n", type);
  p("-EventCode   : 0x%02X\r\n", event_code);
  p("-Data Length : 0x%02X\r\n", data_len);
  p("-Data: \r\n");
    
  Serial.print(type);
  Serial.print("::");
  Serial.print(event_code);
  Serial.print("::");
  Serial.print(data_len);
  Serial.print("::");
  
  //iterate all data to specified data_len
  for (int i = 0; i < data_len; i++){
    buf[i] = Serial1.read();
    Serial.print(buf[i]);
    Serial.print(":");
  }
  Serial.println();
  
  //all bytes have been loaded at this point
  //translate buffer to info
  event = BUILD_UINT16(buf[0], buf[1]);
  status1 = buf[2];

  p(" Event       : 0x%04X\r\n", event);
  p(" Status      : 0x%02X\r\n", status1);
  
  
  p("\r\n");    


  switch (event)
  {
    case 0x0601: // GAP_DeviceDiscoveryDone
    {    p("GAP_DeviceDiscoveryDone\r\n");

        uint8_t num_devs = buf[3];
        p(" NumDevs     : 0x%02X\r\n", num_devs);
        
        if (num_devs > 0){
          //copies adddress to store
          memcpy(found_address, &buf[6], 6); // store 1 device address only in this demo

          boolean same = true;
          //copy all bytes from found_address to last_address
          for(int k = 0; k < 6; k++) {
              if(last_address[k] != found_address[k]){
               same = false; 
              }
           }

          //see if the bytes of last_address match the found address 
          Serial.print("same: " );
          Serial.println(same);
          
          
          if(same){
              flashLED(DiscoveredLED);
          }
          
          
          
          //added
          int currentIndex = 6;
          for(int i = 1; i <= num_devs; i++) { 
            Serial.print("Address found:");
            for(int k = 0; k < 6; k++) {
              Serial.print(buf[currentIndex]);
              currentIndex++;
              Serial.print(":");
            }
            currentIndex += 2;  //3 because skip 2, start the currentIndex at the beginning of the next byte
            Serial.println();
          }
          //added

 
         Serial.print("found_address: ");
         for(int k = 0; k < 6; k++) {
              Serial.print(found_address[k]);
              Serial.print(":");
         }
         
         
         
         
         Serial.println();
 
       }else{
         //digitalWrite(DiscoveredLED, LOW);
 
       }
        break;
    }
    case 0x051B: // ATT_HandleValueNotification
    {
          p("ATT_HandleValueNotification\r\n");
          
          uint8_t data[21] = {0};
          uint8_t len = data_len - 8;
          
          if (len > 20)
            len = 20;
            
          memcpy(data, &buf[8], len);
          
          p(" -------------> Received from Biscuit peripheral: %s\r\n", data);
  
        break;
    }
    case 0x067F: // GAP_HCI_ExtentionCommandStatus
    {
        p("GAP_HCI_ExtentionCommandStatus\r\n");
         
        break;
    }
    case 0x060D: // GAP_DeviceInformation
    {
        p("GAP_DeviceInformation\r\n");
        
        Serial.print("************RSSI: ");
        Serial.println(buf[11]);
         
        break;
    }
    case 0x0600: // GAP_DeviceInitDone
    {
        p("GAP_DeviceInitDone\r\n");
         
        break;
    }
    default:
      p(" -> Not handled yet.\r\n");
  }  
}



void setup()
{ 
    pinMode(LED, OUTPUT); 
    pinMode(DiscoveredLED, OUTPUT); 
    
    
    pinMode(DiscoveredButton, INPUT); 
    pinMode(DiscoveredButton5V, OUTPUT); 

    digitalWrite(DiscoveredButton5V, HIGH);
  
  
    #if defined(__AVR_ATmega328P__)
      Serial1.begin(57600);
      Serial.begin(57600);

      ble_hci_init(&Serial1);
    #else
      Serial1.begin(57600);
      Serial.begin(115200);
 
      while (!Serial);
 
      Serial.write("BLE Central Listener Started\r\n");
   
      flashLED(LED);

      Serial.write("Send D to start discovery mode\r\n");
      Serial.write("\r\n");
      Serial.print("millis:");
      Serial.println(millis());
   
      ble_hci_init();
    #endif

  biscuit_central_init();

  lastMillis = millis();
  
}

void loop()
{
    while (ble_event_available())
      ble_event_process();
    
    while (Serial.available())
    {  
      byte cmd = Serial.read();
      switch(cmd)
      {
        case 'D':
        case 'd':
          p(" -> Start discovery...\r\n");
          Serial.print("millis:");
          Serial.println(millis());
          biscuit_central_start_discovery();
          break;
        
        case 'E':
        case 'e':
          p(" -> Connecting to Biscuit peripheral...\r\n");
          biscuit_central_connect(found_address);
          break;
      
        case 'N':
        case 'n':
          p(" -> Enable notification to receive data...\r\n");
          biscuit_central_enable_notification();
          break;
      
        case '1':
          p(" -> Send \"Hello World!\" to the Biscuit peripheral...\r\n");
          biscuit_central_write_bytes((uint8 *)"Hello World!\r\n", 14);
          break;
        
        case '2':
          p(" -> Send \"I love BLE!\" to the Biscuit peripheral...\r\n");
          biscuit_central_write_bytes((uint8 *)"I love BLE!\r\n", 13);
          break;

        default:
          p(" -> Invalid command: %s\r\n", cmd);      
       }
  }
  
  if(analogRead(DiscoveredButton)>100){
    p("BUTTON PRESSED\r\n");
    p(" -> Start discovery...\r\n");
    Serial.print("millis:");
    Serial.println(millis());
    biscuit_central_start_discovery();
    
  }
  
  if((millis()-lastMillis)>scan_delay){
      lastMillis = millis(); 
      p("TIME EXPIRED\r\n");
      p(" -> Start discovery...\r\n");
      biscuit_central_start_discovery();
  }
  
  flashLED(LED);
}


