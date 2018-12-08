
#include <SPI.h>
#define LE 3 // Latch Enable (Chip Select)
#define extPin A0 // External input pin
#define delayVCO 300

String inputString = "";         
boolean stringComplete = false;  
String commandString = "";
uint32_t regs[13]; // holds the initial ADF5355 register values
uint32_t regsHop[13]; // holds the registers being hopped to
int msDelay = 100; // default hop delay
int hopCycles = 1; 
unsigned int extIntMin = 100; // default minimum INT value in Ext mode
int extTime = 5000; // default external sweep time

boolean isConnected = false;

void setup() {
  
  Serial.begin(9600);
  pinMode(LE,OUTPUT);
  digitalWrite(LE, HIGH);
  
}

void loop() {

if(stringComplete)
{
  stringComplete = false;
  getCommand();
  
  if(commandString.equals("STAR"))
  {
    // startup blank for now
  }
  else if(commandString.equals("REGS")) // Set registers
  {
    String regNum = inputString.substring(5,7);
    writeRegs(regNum);
  }
  else if(commandString.equals("INIT")) // Initialization sequence
  {
    setADF5355();
  }
  else if(commandString.equals("FREQ")) // Frequency update sequence
  {
    updateADF5355();
  }
  else if(commandString.equals("HOPS")) // Set hop registers
  {
    String regNum = inputString.substring(5,7);
    writeRegsHop(regNum);
  }
  else if(commandString.equals("HOPD")) // Get hop delay
  {
    char charBuf[inputString.length()-6];
    inputString.substring(5, inputString.length()-2).toCharArray(charBuf, inputString.length()-6);
    msDelay = strtoul(charBuf, NULL, 0);
  }
  else if (commandString.equals("HOPC"))
  {
    char charBuf[inputString.length()-6];
    inputString.substring(5, inputString.length()-2).toCharArray(charBuf, inputString.length()-6);
    hopCycles = strtoul(charBuf, NULL, 0);
  }
  else if(commandString.equals("HOPB")) // Begin hopping
  {
    for (int i = 0; i <= hopCycles; i++)
    {
      updateADF5355();
      delay(msDelay);
      hopUpdateADF5355();
      delay(msDelay);
    }
    for (int i = 0; i<100; i++) // Need to send the callback several times to make sure it is read
    {
      Serial.write("STPH");
    }
  }
  else if(commandString.equals("EXTI")) //Set Ext integer
  {
    char charBuf[inputString.length()-6];
    inputString.substring(5, inputString.length()-2).toCharArray(charBuf, inputString.length()-6);
    extIntMin = (int)strtoul(charBuf, NULL, 0);
  }
  else if(commandString.equals("EXTT")) //Set Ext sweep time (ms)
  {
    char charBuf[inputString.length()-6];
    inputString.substring(5, inputString.length()-2).toCharArray(charBuf, inputString.length()-6);
    extTime = strtoul(charBuf, NULL, 0);
  }
  else if(commandString.equals("EXTS")) // Start Ext sweeping
  {
    int extIntMax = bitExtracted(regs[0], 16, 5); //take the INT value from REG0, this is the maximum frequency
    int extIntRange = extIntMax - extIntMin;
    unsigned long startTime = millis();
    while ((millis() - startTime) < extTime)
    {
      updateADF5355();
      int extVin = analogRead(extPin); // 0-1023
      double frac = (double) extVin / 1023.0; 
      double prod = (double)frac * (double) extIntRange;
      int updateInt = (int)prod + (int)extIntMin;
      //int updateInt = (int)((extVin * extIntRange)/1023) + extIntMin; // between min and max 
      regs[0] = updateInt << 4;
      //regs[0] = extIntMin << 4;
      //regs[0] = extIntMax << 4;
      //regs[0] = extVin << 4;
      delay(10);
    }
    for (int i = 0; i<100; i++) // Need to send the callback several times to make sure it is read
    {
      Serial.write("STPH");
    }
  }
  inputString = ""; // refresh the input string
}

}

boolean getState()
{
  boolean state = false;
  if(inputString.substring(5,7).equals("ON"))
  {
    state = true;
  }else
  {
    state = false;
  }
  return state;
}

void getCommand()
{
  if(inputString.length()>0)
  {
     commandString = inputString.substring(1,5);
  }
}

void turnOn(int pin)
{
  digitalWrite(pin,HIGH);
}

void turnOff(int pin)
{
  digitalWrite(pin,LOW);
}

void writeRegs(String regNum)
{
  if (regNum == "00"){char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regs[0] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "01") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regs[1] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "02") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regs[2] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "03") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regs[3] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "04") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regs[4] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "05") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regs[5] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "06") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regs[6] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "07") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regs[7] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "08") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regs[8] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "09") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regs[9] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "10") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regs[10] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "11") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regs[11] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "12") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regs[12] = strtoul(charBuf, NULL, 0);}
}

void writeRegsHop(String regNum)
{
  if (regNum == "00"){char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regsHop[0] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "01") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regsHop[1] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "02") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regsHop[2] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "03") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regsHop[3] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "04") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regsHop[4] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "05") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regsHop[5] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "06") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regsHop[6] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "07") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regsHop[7] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "08") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regsHop[8] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "09") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regsHop[9] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "10") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regsHop[10] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "11") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regsHop[11] = strtoul(charBuf, NULL, 0);}
  else if (regNum == "12") {char charBuf[inputString.length()-8]; inputString.substring(7, inputString.length()-2).toCharArray(charBuf, inputString.length()-8); regsHop[12] = strtoul(charBuf, NULL, 0);}
}

void setADF5355()
{
  for (int i = 12; i >= 1; i--)
  { 
    WriteRegister32(regs[i]);
  }
  delayMicroseconds(delayVCO);
  WriteRegister32(regs[0]);
  /*digitalWrite(LE, HIGH);
  digitalWrite(LE, LOW);
  digitalWrite(LE, HIGH);*/
}

void updateADF5355()
{
   WriteRegister32(regs[10]);                       //R10
   WriteRegister32(regs[4]|=1UL<<4);                //R4  DB4=1
   WriteRegister32(regs[2]);                        //R2
   WriteRegister32(regs[1]);                        //R1
   WriteRegister32(regs[0]&=~(1UL<<21));            //R0  DB21=0   
   WriteRegister32(regs[4]&=~(1UL<<4));             //R4  DB4=0
   delayMicroseconds(delayVCO); //  VCO stabilization delay
   WriteRegister32(regs[0]|=1UL<<21);               //R0  DB21=1
   /*digitalWrite(LE, HIGH);
   digitalWrite(LE, LOW);
   digitalWrite(LE, HIGH); */              
}

void hopUpdateADF5355()
{
   WriteRegister32(regsHop[10]);                       //R10
   WriteRegister32(regsHop[4]|=1UL<<4);                //R4  DB4=1
   WriteRegister32(regsHop[2]);                        //R2
   WriteRegister32(regsHop[1]);                        //R1
   WriteRegister32(regsHop[0]&=~(1UL<<21));            //R0  DB21=0   
   WriteRegister32(regsHop[4]&=~(1UL<<4));             //R4  DB4=0
   delayMicroseconds(delayVCO); //  VCO stabilization delay
   WriteRegister32(regsHop[0]|=1UL<<21);               //R0  DB21=1
   /*digitalWrite(LE, HIGH);
   digitalWrite(LE, LOW);
   digitalWrite(LE, HIGH);       */        
}

// Function to extract k bits from p position 
// and returns the extracted value as integer 
int bitExtracted(int number, int k, int p) 
{ 
    return (((1 << k) - 1) & (number >> (p - 1))); 
} 

void WriteRegister32(const uint32_t value)
{
  SPI.begin();
  SPI.beginTransaction(SPISettings(600000, MSBFIRST, SPI_MODE0));
  digitalWrite(LE, LOW);
  for (int i = 3; i >= 0; i--){          
    SPI.transfer((value >> 8 * i) & 0xFF); 
  }
  digitalWrite(LE, HIGH);
  SPI.endTransaction();
  SPI.end();
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
