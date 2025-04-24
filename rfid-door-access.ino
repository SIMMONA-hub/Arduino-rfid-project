#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#define RST_PIN 9
#define SS_PIN 10
#define SERVO_PIN A0
#define BUZZER_PIN A1

MFRC522 rfid(SS_PIN, RST_PIN);
Servo doorServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};    
byte colPins[COLS] = {5, 4, 3, 2};    
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String password = "111";
String input = "";
String validUID = "2F14E2C1";  

 
void showSadAnimation() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wrong");

  for (int i = 0; i < 12; i++) {
    lcd.setCursor(i, 1);
    lcd.print(":(");
    delay(150);
    lcd.setCursor(i, 1);
    lcd.print("  ");
  }

  lcd.clear();
  lcd.print("Enter ID or Code");
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  doorServo.attach(SERVO_PIN);
  doorServo.write(0);

  pinMode(BUZZER_PIN, OUTPUT);
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Enter ID or Code");
}

void loop() {
  checkRFID();
  checkKeypad();
}

void checkRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String readUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) readUID += "0";
    readUID += String(rfid.uid.uidByte[i], HEX);
  }
  readUID.toUpperCase();

  Serial.print("Scanned UID: ");
  Serial.println(readUID);

  if (readUID == validUID) {
    lcd.clear();
    lcd.print("Open");
    playHappyMelody();
    openDoor();
  } else {
    showSadAnimation();      
    playSadMelody();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void checkKeypad() {
  char key = keypad.getKey();
  if (key && isDigit(key)) {
    input += key;
    lcd.clear();
    lcd.print("Code: " + input);

    if (input.length() == 3) {
      delay(500);

      if (input == password) {
        lcd.clear();
        lcd.print("Access Granted");
        playHappyMelody();
        openDoor();
      } 
      else if (input == "777") {
        lcd.clear();
        lcd.print("Happy Birthday!");
        playBirthdayMelody();  
        lcd.clear();
        lcd.print("Enter ID or Code");
      }
      else {
        showSadAnimation();    
        playSadMelody();
      }

      input = "";
    }
  }
}

void openDoor() {
  for (int pos = 0; pos <= 80; pos++) {
    doorServo.write(pos);
    delay(10);
  }

  delay(5000);

  for (int pos = 80; pos >= 0; pos--) {
    doorServo.write(pos);
    delay(10);
  }

  lcd.clear();
  lcd.print("Enter ID or Code");
}

void playHappyMelody() {
  tone(BUZZER_PIN, 1047, 100); delay(120);
  tone(BUZZER_PIN, 1319, 100); delay(120);
  tone(BUZZER_PIN, 1568, 100); delay(120);
  tone(BUZZER_PIN, 1760, 80);  delay(100);
  tone(BUZZER_PIN, 2093, 80);  delay(100);
  tone(BUZZER_PIN, 2349, 80);  delay(120);
  tone(BUZZER_PIN, 2637, 150); delay(200);
  noTone(BUZZER_PIN);
}

void playSadMelody() {
  tone(BUZZER_PIN, 330, 300); delay(350);
  tone(BUZZER_PIN, 262, 400); delay(450);
  tone(BUZZER_PIN, 196, 500); delay(550);
  noTone(BUZZER_PIN);
}

void playBirthdayMelody() {
  int melody[] = {
    262, 262, 294, 262, 349, 330,
    262, 262, 294, 262, 392, 349,
    262, 262, 523, 440, 349, 330, 294,
    466, 466, 440, 349, 392, 349
  };
  int duration[] = {
    300, 300, 600, 600, 600, 800,
    300, 300, 600, 600, 600, 800,
    300, 300, 600, 600, 600, 600, 1200,
    300, 300, 600, 600, 600, 800
  };
  for (int i = 0; i < 26; i++) {
    tone(BUZZER_PIN, melody[i], duration[i]);
    delay(duration[i] * 1.3);
    noTone(BUZZER_PIN);
  }
}
