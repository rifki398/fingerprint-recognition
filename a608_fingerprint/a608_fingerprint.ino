#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()
{
  Serial.begin(9600);
  while (!Serial); // tunggu serial monitor (hanya untuk board tertentu)

  Serial.println("Mengecek koneksi sensor fingerprint...");

  finger.begin(57600); // pastikan sesuai dengan baudrate sensor
  if (finger.verifyPassword()) {
    Serial.println("Sensor fingerprint terdeteksi.");
  } else {
    Serial.println("Sensor fingerprint tidak ditemukan :(");
    while (1); // berhenti di sini
  }
}

void loop()
{
  Serial.println("Silakan letakkan jari...");
  int id = getFingerprintID();

  if (id != -1) {
    Serial.print("Sidik jari dikenali! ID: ");
    Serial.println(id);
    // Tambahkan aksi jika jari dikenali, misalnya:
    // digitalWrite(RELAY_PIN, HIGH);
  } else {
    Serial.println("Sidik jari tidak dikenali.");
  }

  delay(2000); // jeda agar tidak terus-terusan membaca
}

int getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerSearch();
  if (p != FINGERPRINT_OK) return -1;

  // Jika berhasil ditemukan
  return finger.fingerID;
}
