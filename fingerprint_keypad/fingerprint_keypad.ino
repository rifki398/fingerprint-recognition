#include <Keypad.h>
#include <Adafruit_Fingerprint.h>

// LED & Switch
#define RED 4
#define YELLOW 5
#define GREEN 6
#define SWITCH 7
#define BUZZ 8

Adafruit_Fingerprint finger(&Serial1);

void beepShort(int times = 2), beepLong();
bool deleteFingerprint(int id);
void checkFingerprint();
void adminBlink(int times = 3);

// Keypad
const byte col = 4;
const byte row = 4;
byte rowpin[row] = {44,46,48,50};
byte colpin[col] = {36,38,40,42};

char keys[row][col] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

Keypad myKeypad = Keypad(makeKeymap(keys), rowpin, colpin, row, col);

// Variable
String idInput = "";
char pressedKey;
bool editMode = false;

bool current_adminNotif = false;
bool adminNotif;

void setup() {
  Serial.begin(9600);
  finger.begin(57600);
  
  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BUZZ,OUTPUT);
  pinMode(SWITCH, INPUT_PULLUP);
  digitalWrite(BUZZ,HIGH);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor found.");
  } else {
    Serial.println("Sensor not found :(");
    while (1);
  }

  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
}

void loop() {
  pressedKey = myKeypad.getKey();
  editMode = digitalRead(SWITCH);
  adminNotif = editMode;
  Serial.println("Please scan your finger...");
  
  while(editMode){
    digitalWrite(RED,HIGH);
    digitalWrite(YELLOW,HIGH);
    digitalWrite(GREEN,HIGH);
    if(adminNotif != current_adminNotif){
      Serial.println("------------ Admin Mode ------------");
      beepShort(3);
      current_adminNotif = adminNotif;
    }
    pressedKey = myKeypad.getKey();
    KeypadFunc(pressedKey);
    editMode = digitalRead(SWITCH);
  }
  
  checkFingerprint();
  
  current_adminNotif = false;
  digitalWrite(BUZZ, HIGH);
  digitalWrite(RED,LOW);
  digitalWrite(YELLOW,HIGH);
  digitalWrite(GREEN,LOW);
  delay(200);
}

// ==================================================================================================
void KeypadFunc(char pressedKey){
  if(pressedKey == 'A'){
    // Register a fingerprint
    Serial.println("[A] -- Please fill id (0-127)! Press 'D' if you are done! ");
    adminBlink(1);
    Serial.print("[A]");
    int ID = fillID().toInt();
    bool regist_stat = enrollFingerprint(ID);
    Serial.print("[A] -- ID to register: ");
    Serial.print(ID);
    if(regist_stat){
      Serial.println(" success!");
    }else{
      Serial.println(" failed!");
    }
    adminBlink(3);
    finger.getTemplateCount();
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
    
  }else if(pressedKey == 'C'){
    // Clear all fingerprint data
    finger.emptyDatabase();
    Serial.println("[C] -- All data cleared");
    adminBlink(3);
    finger.getTemplateCount();
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
    
  }else if(pressedKey == 'B'){
    Serial.println("[B] -- Please fill id (0-127)! Press 'D' if you are done! ");
    adminBlink(1);
    Serial.print("[B]");
    int ID = fillID().toInt();
    bool del_stat = deleteFingerprint(ID);
    Serial.print("[B] -- ID "); 
    Serial.print(ID); 
    if(del_stat){
      Serial.println(" cleared");
    }else{
      Serial.println(" not cleared");
    }
    adminBlink(3);
    finger.getTemplateCount();
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
}

String fillID(){
  String id = "";
  char key;
  Serial.print("[fillID] -- ID: ");
  while(true){
    key = myKeypad.getKey();
    if (key){
      if (key == 'D') {
        break;
      }else{
        id += key;
        Serial.print(key);
      }
    }
  }
  Serial.println(" ");
  Serial.print("\t Done typing ID: "); Serial.println(id);
  return id;
}


bool enrollFingerprint(int id) {
  digitalWrite(YELLOW, LOW);
  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);
  int p;
  bool stat;

  delay(100);
  Serial.println("\t Place your finger on the sensor...");
  adminBlink(1);
  digitalWrite(YELLOW, HIGH);
  while ((p = finger.getImage()) != FINGERPRINT_OK);

  digitalWrite(YELLOW, LOW);
  if (p != FINGERPRINT_OK) {
    Serial.println("\t Image capture failed");
    digitalWrite(RED, HIGH); delay(1000); digitalWrite(RED, LOW);
    adminBlink(1);
    stat = false;
    return stat;
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("\t Image to template failed");
    digitalWrite(RED, HIGH); delay(1000); digitalWrite(RED, LOW);
    adminBlink(1);
    stat = false;
    return stat;
  }

  Serial.println("\t Remove finger");
  adminBlink(1);
  delay(2000);

  Serial.println("\t Place same finger again...");
  adminBlink(1);
  while ((p = finger.getImage()) != FINGERPRINT_OK);

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("\t Second image failed");
    digitalWrite(RED, HIGH); delay(1000); digitalWrite(RED, LOW);
    adminBlink(1);
    stat = false;
    return stat;
  }

  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("\t Template creation failed");
    digitalWrite(RED, HIGH); delay(1000); digitalWrite(RED, LOW);
    adminBlink(1);
    stat = false;
    return stat;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("\t Fingerprint enrolled successfully!");
    digitalWrite(GREEN, HIGH); delay(1000); digitalWrite(GREEN, LOW);
    stat = true;
  } else {
    Serial.println("\t Failed to store fingerprint.");
    digitalWrite(RED, HIGH); delay(1000); digitalWrite(RED, LOW);
    stat = false;
  }
  adminBlink(1);
  return stat;
}

void beepShort(int times = 2) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZ, LOW);
    delay(150);
    digitalWrite(BUZZ, HIGH);
    delay(150);
  }
}

void beepLong() {
  digitalWrite(BUZZ, LOW);
  delay(1000);
  digitalWrite(BUZZ, HIGH);
}
bool deleteFingerprint(int id) {
  uint8_t p;
  
  p = finger.loadModel(id);
  if (p != FINGERPRINT_OK) {
    Serial.println("\t ID not found, can't delete.");
    adminBlink(1);
    return false;
  }

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("\t Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("\t Communication error");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("\t Could not delete in that location");
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("\t Error writing to flash");
  } else {
    Serial.print("\t Unknown error: 0x"); Serial.println(p, HEX);
  }
  adminBlink(1);
  return true;
}


void checkFingerprint() {
  int p = finger.getImage();
  if (p != FINGERPRINT_OK) {
    return;  // Tidak ada jari di sensor
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    return;  // Gagal konversi gambar ke template
  }

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("Found ID: ");
    Serial.println(finger.fingerID);
    digitalWrite(GREEN, HIGH);
    digitalWrite(YELLOW, LOW);
    digitalWrite(RED, LOW);
    beepShort();
  } else {
    Serial.println("Fingerprint not recognized.");
    digitalWrite(GREEN, LOW);
    digitalWrite(YELLOW, LOW);
    digitalWrite(RED, HIGH);
    beepLong();
  }

  delay(1000);  // Biar LED menyala sebentar
  digitalWrite(GREEN, LOW);
  digitalWrite(RED, LOW);
}

void adminBlink(int times = 3){
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZ, LOW);
    digitalWrite(GREEN, HIGH);
    digitalWrite(YELLOW, HIGH);
    digitalWrite(RED, HIGH);
    delay(150);
    digitalWrite(BUZZ, HIGH);
    digitalWrite(GREEN, LOW);
    digitalWrite(YELLOW, LOW);
    digitalWrite(RED, LOW);
    delay(150);
  }
}
