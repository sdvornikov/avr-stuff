#include <OneWire.h>
#include <LiquidCrystal.h>

OneWire  ds(6);  // on pin 10 (a 4.7K resistor is necessary)
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);


struct DsROM {
  byte ROMCode[8];
};

struct DsScratchpad {
  float temperature;
  byte highT;
  byte lowT;
  byte conf;
};

int activeSensors = 0;
struct DsROM SensorROMCode[10]; // limited to 10 sensors

char *ftostr(float inValue,int precision) {
    char *buffer = (char*)malloc(10);
    dtostrf(inValue,4+precision,precision,buffer);
    return buffer;
}

void discoverSensors() 
{
  byte addrbuffer[8];
  int sensorCount = 0;
  while(ds.search(addrbuffer)) {

    if (OneWire::crc8(addrbuffer, 7) != addrbuffer[7]) {
      Serial.println("CRC is not valid!");
      continue;
    }

    for(int i=0;i<8;i++) {
      SensorROMCode[sensorCount].ROMCode[i] = addrbuffer[i];
    }
    activeSensors++;
    sensorCount++;
  }
}

void writeScratchpad(struct DsROM addr, int highT, int lowT, byte cfg) {
  if (!ds.reset()) return;
  ds.select(addr.ROMCode);
  ds.write(0x4E);
  ds.write((byte)(highT & 0xFF));
  ds.write((byte)(lowT & 0xFF));
  ds.write(cfg);
}

void copyScratchpad(struct DsROM addr) 
{
  if (!ds.reset()) return;
  ds.select(addr.ROMCode);
  ds.write(0x48, 1);
  delay(25);
  ds.depower(); 
}


void convertT(struct DsROM addr) 
{
  if (!ds.reset()) return;
  ds.select(addr.ROMCode);
  ds.write(0x44, 1);
  delay(800);
  ds.depower(); 
}

boolean readScratchpad(struct DsROM addr, struct DsScratchpad &outMemory) {
  byte databuffer[9];

  if (!ds.reset()) return false;
  ds.select(addr.ROMCode);    
  ds.write(0xBE);         // Read Scratchpad

  for (int i = 0; i < 9; i++) {           // we need 9 bytes
    databuffer[i] = ds.read();
  }
  if (OneWire::crc8(databuffer, 8) != databuffer[8]) {
    Serial.println("CRC is not valid!");
    return false;
  }


  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (databuffer[1] << 8) | databuffer[0];

  byte cfg = (databuffer[4] & 0x60);
  // at lower res, the low bits are undefined, so let's zero them
  if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
  else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
  else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
  //// default is 12 bit resolution, 750 ms conversion time

  outMemory.temperature=(float)raw / 16.0;
  outMemory.conf=databuffer[4];
  outMemory.highT=databuffer[2];
  outMemory.lowT=databuffer[3];
  return true; 
}


void setup(void) {
  lcd.begin(16, 2);
  Serial.begin(9600);
  Serial.print("ready...");
  delay(5000);
  Serial.println("GO!");
  discoverSensors();
}

void loop(void) {

  for(int i=0;i<activeSensors;i++) {
    convertT(SensorROMCode[i]);

    struct DsScratchpad memory;
    if(!readScratchpad(SensorROMCode[i], memory)) {
      Serial.println("Error reading memory");
      continue;
    }
    Serial.print("T = ");
    Serial.print(ftostr(memory.temperature,5));
    Serial.print(" config = ");
    Serial.print(memory.conf & 0x60, BIN);
    Serial.print(" High = ");
    Serial.print((char)memory.highT,DEC);
    Serial.print(" Low = ");
    Serial.print((char)memory.lowT,DEC);
    Serial.println();
    
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temperature: ");
  lcd.setCursor(0, 1);
  lcd.print(ftostr(memory.temperature,1));
  }
  delay(5000);
  runOnce();
}

void runOnce() {
  static boolean shouldRun = false;
  if (!shouldRun) return;
  int lowT = -1;
  int highT = 26;
  byte cfg = 0x60;
  writeScratchpad(SensorROMCode[0], highT , lowT , cfg);
  struct DsScratchpad memory;
    if(!readScratchpad(SensorROMCode[0], memory)) {
      Serial.println("runOnce: Error reading memory");
      return;
    }
    
    if(memory.lowT != (lowT & 0xFF) || memory.highT != (highT & 0xFF) || (memory.conf & 0x60) != cfg) {
      Serial.println("runOnce: Scratchpad do not contain intended values");
      return;
    }
    copyScratchpad(SensorROMCode[0]);
    Serial.println("runOnce completed");
  shouldRun=false;
}

