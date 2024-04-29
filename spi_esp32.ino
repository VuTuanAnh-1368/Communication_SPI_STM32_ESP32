const int PIN_SCK = 18;  // SCK pin
const int PIN_MOSI = 23; // MOSI pin
const int PIN_SS = 5;    // SS pin (Slave Select)

volatile bool received = false;
char message[128]; // Buffer for the message
volatile int messageIndex = 0;

void setup() {
  Serial.begin(115200);

  pinMode(PIN_SS, INPUT_PULLUP);
  pinMode(PIN_MOSI, INPUT);
  pinMode(PIN_SCK, INPUT);
  
  attachInterrupt(digitalPinToInterrupt(PIN_SS), onSSFall, CHANGE);
}

void loop() {
  if (received) {
    Serial.print("Message received: ");
    Serial.println("message");
    received = false;
    messageIndex = 0; // Reset the index
    memset(message, 0, sizeof(message)); // Clear the buffer
  }
}

void onSSFall() {
  // Read data only if SS line is LOW
  if (digitalRead(PIN_SS) == LOW) {
    message[messageIndex++] = bitBangSPIRead();
    if (messageIndex >= sizeof(message) - 1) {
      // Prevent buffer overflow
      received = true;
    }
  } else {
    // If SS line goes HIGH, the message is complete
    received = true;
  }
}

byte bitBangSPIRead() {
  byte data = 0;
  // Wait for SCK to be HIGH to ensure correct timing (CPHA = 0)
  while(digitalRead(PIN_SCK) == LOW);
  // Read 8 bits of data
  for (int i = 0; i < 8; i++) {
    data <<= 1;
    if (digitalRead(PIN_MOSI)) {
      data |= 1;
    }
    // Wait for the next clock edge
    while(digitalRead(PIN_SCK) == HIGH);
    while(digitalRead(PIN_SCK) == LOW);
  }
  return data;
}
