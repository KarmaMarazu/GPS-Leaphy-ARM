/**
* @file Leapy_Arduino_Motor_Bepaling.ino
* @brief Deze file bevat de code voor het laten bewegen van de motoren. <BR>
* @author Twan Ton, Mika Dahlkamp, Thomas van Ooijen en Jasper Verduin.
* @date 16/10/2025.
*/


//Define snelheid van de linker motor.
#define DRAAI_L 220         
#define FAST_L 180          
#define SLOW_L 110          
#define STOP_L 0            

//Define snelheid van de rechter motor. 0.914 * snelheid_L voor motor verschil compensatie. 
#define DRAAI_R 201         
#define FAST_R 165          
#define SLOW_R 101          
#define STOP_R 0

//Declaratie van de pins voor het ontvangen van de data van het arm bordje
const byte Bit1 = A3;
const byte Bit2 = A2;
const byte Bit3 = A1;
const byte Bit4 = A0;

//Decaratie van de pins voor het bepalen van de snelheid van de motor
const byte PWML = 5;
const byte PWMR = 6;

//Decaratie van de pins voor het bepalen van de snelheid van de motor
const byte ML = 4;
const byte MR = 7;

//Ontvang data en encode het in hex
char Data_Ontvangen(void)
{
  char data = 0;

  if(analogRead(Bit1)>100) data |= 0b0001;
  else data &= 0b1110;
  if(analogRead(Bit2)>100) data |= 0b0010;
  else data &= 0b1101;
  if(analogRead(Bit3)>100) data |= 0b0100;
  else data &= 0b1011;
  if(analogRead(Bit4)>100) data |= 0b1000;
  else data &= 0b0111;

  //Debug data
  Serial.println(data, HEX);
  return data;
}

void Motor_Aansturen(char data)
{
  //Zet de snelheid van de corresponderende motor op 0
  analogWrite(PWML, 0);
  analogWrite(PWMR, 0);

  //Zet de richting van de corresponderende motor op high
  digitalWrite(MR, HIGH);
  digitalWrite(ML, HIGH);
  switch(data)
  {
    //Leaphy rijdt niet
    case 0x00:analogWrite(PWML, 0);
              analogWrite(PWMR, 0);
              break;

    //Leaphy rijdt naar vooren
    case 0x01:analogWrite(PWML, FAST_L);
              analogWrite(PWMR, FAST_R);
              break;

    //Leaphy rijdt langzaam naar rechts
    case 0x02:analogWrite(PWMR, SLOW_R);
              analogWrite(PWML, FAST_L);
              break;
            
    //Leaphy rijdt snel naar rechts
    case 0x03:analogWrite(PWMR, SLOW_R);     
              analogWrite(PWML, DRAAI_L);
              break;

    //Leaphy rijdt langzaam naar links
    case 0x04:analogWrite(PWMR, FAST_R);      
              analogWrite(PWML, SLOW_L);
              break;

    //Leaphy rijdt snel naar links
    case 0x05:analogWrite(PWMR, DRAAI_R);
              analogWrite(PWML, SLOW_L);
              break;

    //Leaphy staat stil
    default:  analogWrite(PWML, STOP_L);
              analogWrite(PWMR, STOP_R);
              Serial.println("fout");
              break;
  }
  
  //Debug data
  Serial.println(data, HEX);
}

//Setup code
void setup() 
{
  pinMode(Bit1, INPUT);
  pinMode(Bit2, INPUT);
  pinMode(Bit3, INPUT);
  pinMode(Bit4, INPUT);
  pinMode(PWML, OUTPUT);
  pinMode(PWMR, OUTPUT);
  pinMode(ML, OUTPUT);
  pinMode(MR, OUTPUT);
  Serial.begin(9600);
}

//Functies aan sturen
void loop() 
{
  char data = Data_Ontvangen();
  Motor_Aansturen(data);
}

