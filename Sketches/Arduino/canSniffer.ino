//modified code from https://github.com/adamtheone/canDrive/blob/main/01_canSniffer_Arduino/canSniffer/canSniffer.ino

#include <CAN.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
//------------------------------------------------------------------------------
// Settings
#define RANDOM_CAN 0
#define CAN_SPEED (500E3) // LOW=33E3, MID=95E3, HIGH=500E3 (for Vectra)
typedef struct
{
  long id;
  byte rtr;
  byte ide;
  byte dlc;
  byte dataArray[20];
} packet_t;

const char SEPARATOR = ',';
const char TERMINATOR = '\n';
const char RXBUF_LEN = 100;

float gpslat, gpslon;

TinyGPS gps;
SoftwareSerial sgps(4, 5); //changepins
SoftwareSerial sgsm(2, 3);

bool isPortieraAperta = false;
const int relay1 = 26;
const int relay2 = 27;
char cons[RXBUF_LEN];
//------------------------------------------------------------------------------
// Printing a packet to serial
void printHex(long num)
{
  if (num < 0x10)
  {
    Serial.print("0");
  }
  Serial.print(num, HEX);
}

void printPacket(packet_t *packet)
{
  // packet format (hex string): [ID],[RTR],[IDE],[DATABYTES 0..8B]\n
  // example: 014A,00,00,1A002B003C004D\n
  printHex(packet->id);
  Serial.print(SEPARATOR);
  printHex(packet->rtr);
  Serial.print(SEPARATOR);
  printHex(packet->ide);
  Serial.print(SEPARATOR);
  // DLC is determinded by number of data bytes, format: [00]
  for (int i = 0; i < packet->dlc; i++)
  {
    printHex(packet->dataArray[i]);
  }
  Serial.print(TERMINATOR);
}
//------------------------------------------------------------------------------
// CAN packet simulator
void CANsimulate(void)
{
  packet_t txPacket;

  long sampleIdList[] = {0x110, 0x18DAF111, 0x23A, 0x257, 0x412F1A1, 0x601, 0x18EA0C11};
  int idIndex = random(sizeof(sampleIdList) / sizeof(sampleIdList[0]));
  int sampleData[] = {0xA, 0x1B, 0x2C, 0x3D, 0x4E, 0x5F, 0xA0, 0xB1};

  txPacket.id = sampleIdList[idIndex];
  txPacket.ide = txPacket.id > 0x7FF ? 1 : 0;
  txPacket.rtr = 0; // random(2);
  txPacket.dlc = random(1, 9);

  for (int i = 0; i < txPacket.dlc; i++)
  {
    int changeByte = random(4);
    if (changeByte == 0)
    {
      sampleData[i] = random(256);
    }
    txPacket.dataArray[i] = sampleData[i];
  }

  printPacket(&txPacket);
}
//------------------------------------------------------------------------------
// CAN RX, TX
void onCANReceive(int packetSize)
{
  // received a CAN packet
  packet_t rxPacket;
  rxPacket.id = CAN.packetId();
  rxPacket.rtr = CAN.packetRtr() ? 1 : 0;
  rxPacket.ide = CAN.packetExtended() ? 1 : 0;
  rxPacket.dlc = CAN.packetDlc();
  byte i = 0;
  while (CAN.available())
  {
    rxPacket.dataArray[i++] = CAN.read();
    if (i >= (sizeof(rxPacket.dataArray) / (sizeof(rxPacket.dataArray[0]))))
    {
      break;
    }
  }
  printPacket(&rxPacket);
}

void sendPacketToCan(packet_t *packet)
{
  for (int retries = 10; retries > 0; retries--)
  {
    bool rtr = packet->rtr ? true : false;
    if (packet->ide)
    {
      CAN.beginExtendedPacket(packet->id, packet->dlc, rtr);
    }
    else
    {
      CAN.beginPacket(packet->id, packet->dlc, rtr);
    }
    CAN.write(packet->dataArray, packet->dlc);
    if (CAN.endPacket())
    {
      // success
      break;
    }
    else if (retries <= 1)
    {
      return;
    }
  }
}
//------------------------------------------------------------------------------
// Serial parser
char getNum(char c)
{
  if (c >= '0' && c <= '9')
  {
    return c - '0';
  }
  if (c >= 'a' && c <= 'f')
  {
    return c - 'a' + 10;
  }
  if (c >= 'A' && c <= 'F')
  {
    return c - 'A' + 10;
  }
  return 0;
}

char *strToHex(char *str, byte *hexArray, byte *len)
{
  byte *ptr = hexArray;
  char *idx;
  for (idx = str; *idx != SEPARATOR && *idx != TERMINATOR; ++idx, ++ptr)
  {
    *ptr = (getNum(*idx++) << 4) + getNum(*idx);
  }
  *len = ptr - hexArray;
  return idx;
}
//////////////////////////////////////////////////////////////////////////////// GPS+GSM Module
void checkLocate()
{

  sgsm.listen();
  if (sgsm.available() > 0)
  {
    String c = sgsm.readString();
    c.trim();
    if (c.indexOf("GET-GPS") >= 0)
    {
      sgps.listen();
      while (sgps.available())
      {
        int coord = sgps.read();
        if (gps.encode(coord))
        {
          gps.f_get_position(&gpslat, &gpslon);
        }
      }
      sgsm.print("\r");
      delay(1000);
      sgsm.print("AT+CMGF=1\r");
      delay(1000);
      /*Replace XXXXXXXXXX to 10 digit mobile number &
        ZZ to 2 digit country code*/
      sgsm.print("AT+CMGS=\"+ZZXXXXXXXXXX\"\r");
      delay(1000);
      // The text of the message to be sent.
      sgsm.print("Latitude :");
      sgsm.println(gpslat, 6);
      sgsm.print("Longitude:");
      sgsm.println(gpslon, 6);
      sgsm.print("http://maps.google.com/maps?q=");
      sgsm.print(gpslat, 6);
      sgsm.print(",");
      sgsm.println(gpslon, 6);
      delay(1000);
      sgsm.write(0x1A);
      delay(1000);
    }
    delay(100);
  }
}
//////////////////////////////////////////////////////////////////////////////// more stuff

void moreStuff(char *cons)
{

  if (strcmp(cons, "light_on") == 0)
  {
    Serial.println("on");
    // apre portiera
    // analogRead(A0) qualcosa che mi dice se la portiera è aperta o chiusa
    // if(analogRead(A0) > 100))
    digitalWrite(relay1, LOW);
    delay(100);
    digitalWrite(relay1, HIGH);
    isPortieraAperta = true;
  }
  else if (strcmp(cons, "light_off") == 0)
  {
    Serial.println("off");
    // chiude portiera
    digitalWrite(relay1, HIGH);
    isPortieraAperta = false;
  }
  //checkLocate(); to enable when I buy the gps and gsm module
}
////////////////////////////////////////////////////////////////////////////////
void rxParse(char *buf, int len)
{
  packet_t rxPacket;
  char *ptr = buf;
  // All elements have to have leading zero!

  // ID
  byte idTempArray[8], tempLen;
  ptr = strToHex(ptr, idTempArray, &tempLen);
  rxPacket.id = 0;
  for (int i = 0; i < tempLen; i++)
  {
    rxPacket.id |= (long)idTempArray[i] << ((tempLen - i - 1) * 8);
  }

  // RTR
  ptr = strToHex(ptr + 1, &rxPacket.rtr, &tempLen);

  // IDE
  ptr = strToHex(ptr + 1, &rxPacket.ide, &tempLen);

  // DATA
  ptr = strToHex(ptr + 1, rxPacket.dataArray, &rxPacket.dlc);
  // Rimuovi gli spazi bianchi all'inizio della stringa
  cons = buf;

  int start = 0;
  while (isspace(cons[start]))
  {
    start++;
  }

  // Rimuovi gli spazi bianchi alla fine della stringa
  int end = strlen(cons) - 1;
  while (end > start && isspace(cons[end]))
  {
    end--;
  }
  cons[end + 1] = '\0'; // Termina la stringa dopo l'ultimo carattere significativo

  moreStuff(cons);

#if RANDOM_CAN == 1
  // echo back
  printPacket(&rxPacket);
#else
  sendPacketToCan(&rxPacket);
#endif
}

void RXcallback(void)
{
  static int rxPtr = 0;
  static char rxBuf[RXBUF_LEN];

  while (Serial.available() > 0)
  {
    if (rxPtr >= RXBUF_LEN)
    {
      rxPtr = 0;
    }
    char c = Serial.read();
    rxBuf[rxPtr++] = c;
    if (c == TERMINATOR)
    {
      rxParse(rxBuf, rxPtr);
      rxPtr = 0;
    }
  }
}

//------------------------------------------------------------------------------
// Setup
void setup()
{
  Serial.begin(250000);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  sgsm.begin(9600);
  sgps.begin(9600);
  /*
  if(analogRead(A0) > 100)
    isPortieraAperta = true;
  else
    isPortieraAperta = false;
  */
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

#if RANDOM_CAN == 1
  randomSeed(12345);
  Serial.println("randomCAN Started");
#else
  if (!CAN.begin(CAN_SPEED))
  {
    Serial.println("Starting CAN failed!");
    while (1)
      ;
  }
  // register the receive callback
  CAN.onReceive(onCANReceive);
  Serial.println("CAN RX TX Started");
#endif
}
//------------------------------------------------------------------------------
// Main
void loop()
{
  RXcallback();
#if RANDOM_CAN == 1
  CANsimulate();
  delay(100);
#endif
}
