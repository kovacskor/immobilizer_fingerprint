#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <EEPROM.h>

#define EEPROM_SIZE 12


SoftwareSerial mySerial(D2, D3);
const int buzzerPin = D8;
const int relePin = D7;

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()
{
  Serial.begin(115200);
  pinMode(buzzerPin, OUTPUT);
  pinMode(relePin, OUTPUT);

  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
}



uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
//      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
    tone(buzzerPin,6000, 500);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    tone(buzzerPin,3000, 500);
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}



uint8_t checkFingerprint() {
  uint8_t p = finger.getImage();

  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
//      Serial.println("No finger detected");
      return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return 0;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return 0;
    default:
      Serial.println("Unknown error");
      return 0;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return 0;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return 0;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return 0;
    default:
      Serial.println("Unknown error");
      return 0;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
    tone(buzzerPin,6000, 500);
    Serial.print("Found ID #"); Serial.print(finger.fingerID);
    Serial.print(" with confidence of "); Serial.println(finger.confidence);
    return 1;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
      return 0;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    tone(buzzerPin,3000, 500);
      return 0;
  } else {
    Serial.println("Unknown error");
  }

  // found a match!

  return 0;
}

bool FingerOk = false;
bool Rablas = false;
unsigned long ido_Rablas = 0;

void loop()                     // run over and over again
{
  unsigned long ido_millis = millis();
  // Ha elvették az áramot és benne van az EEPROM-ban a riasztás akkor léptesse meg az időt és zárjon azonnal
  int readId;
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0, readId);
  EEPROM.end();
  if (readId == 1){
    ido_millis = 120000;      
  }

  if (checkFingerprint() ==1){
    if (FingerOk == true and Rablas == false){
      Rablas = true;
      ido_Rablas = ido_millis;
      Serial.println("Rablas be. ----------------");
    }else{
      Rablas = false;
      ido_Rablas = 0;
      Serial.println("Rablas ki. ----------------");
    }

    Serial.println("Found. --------------------");
    FingerOk = true;

  }

  if ( Rablas == true){
      tone(buzzerPin,2500, 20);
  }

  if ( (ido_millis - ido_Rablas > 60000) and Rablas == true){
      tone(buzzerPin,1500, 20);
      digitalWrite(relePin, HIGH);
      EEPROM.begin(EEPROM_SIZE);
      EEPROM.get(0, readId);
      if (readId !=1){
        EEPROM.put(0, 1);  
        EEPROM.commit();
        Serial.println("EEPROM write 1");
      }
      EEPROM.end();
  }else{

    if ( (ido_millis > 30000) and FingerOk == false){
        tone(buzzerPin,2000, 20);
    }
    if ( (ido_millis > 60000) and FingerOk == false){
        tone(buzzerPin,1000, 20);
        digitalWrite(relePin, HIGH);
        EEPROM.begin(EEPROM_SIZE);
        EEPROM.get(0, readId);
        if (readId !=1){
          EEPROM.put(0, 1);  
          EEPROM.commit();
          Serial.println("EEPROM write 1");
        }
        EEPROM.end();
    }else{
        digitalWrite(relePin, LOW);
        EEPROM.begin(EEPROM_SIZE);
        EEPROM.get(0, readId);
        if (readId !=0){
          EEPROM.put(0, 0);  
          EEPROM.commit();
          Serial.println("EEPROM write 0");
        }
        EEPROM.commit();
        EEPROM.end();
    }
  }
  delay(50);            //don't ned to run this at full speed.
}