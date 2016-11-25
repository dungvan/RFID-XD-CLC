#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <RFID.h>

#define SS_PIN 53
#define RST_PIN 2

#define pulse_in 48
#define red_in 43
#define green_in 44

#define pulse_out 45
#define red_out 46
#define green_out 47

RFID rfid(SS_PIN, RST_PIN);

/*thay doi cau hinh led*/
void change_led_position();
/*doc the*/
void readCard();
/*mo-dong cong*/
void open_door(byte in, byte out);

byte red[10] = {3, 6, 9, 22, 25, 28, 31, 34, 37, 40};
byte green[10] = {4, 7, 10, 23, 26, 29, 32, 35, 38, 41};
byte pulse[] = {5, 8, 11, 24, 27, 30, 33, 36, 39, 42};

// LCD address and geometry and library initialization
const byte lcdAddr = 0x3F;  // Address of I2C backpack
const byte lcdCols = 16;    // Number of character in a row
const byte lcdRows = 2;     // Number of lines
//const byte lcdAddr = 0x3F;  // Address of I2C backpack
//const byte lcdCols = 20;    // Number of character in a row
//const byte lcdRows = 4;     // Number of lines

LiquidCrystal_I2C lcd(lcdAddr, lcdCols, lcdRows);

byte upper[8] {
  B00100,
  B01110,
  B11111,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};


byte lefter1[8] {
  B00000,
  B00000,
  B00000,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000
};

byte lefter2[8] {
  B01000,
  B01100,
  B01110,
  B11111,
  B01110,
  B01100,
  B01000,
  B00000
};
typedef struct id_value {
  unsigned int num0;
  unsigned int num1;
  unsigned int num2;
  unsigned int num3;
  unsigned int num4;
  byte checkExitsInList = 0;
};
id_value temp_value_in;
id_value temp_value_out;
id_value id_values[10];
byte numberOfCar = 0;
byte fullList = 0;

byte defaultNumberOfFreePosition = 5;
byte numberOfFreePosition1 = 0;
byte numberOfFreePosition2 = 0;
byte lastNumberOfFreePosition1 = 0;
byte lastNumberOfFreePosition2 = 0;

int save_id = 0;
byte in_door;
byte out_door;
byte temp_in_door = 1;
byte temp_out_door = 1;
// Setup variables:
int serNum0;
int serNum1;
int serNum2;
int serNum3;
int serNum4;

void setup()
{
  lcd.init();
  lcd.backlight();
  lcd.home();
  lcd.createChar(0, upper);
  lcd.createChar(1, lefter1);
  lcd.createChar(2, lefter2);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.write(0);
  lcd.setCursor(0, 1);
  lcd.write(1);
  lcd.setCursor(1, 1);
  lcd.write(2);
  Serial.begin(9600);
  pinMode(pulse_in, INPUT);
  pinMode(pulse_out, INPUT);
  pinMode(red_in, OUTPUT);
  pinMode(green_in, OUTPUT);
  pinMode(red_out, OUTPUT);
  pinMode(green_out, OUTPUT);
  byte index = 0;
  for (index; index < 10; index ++) {
    pinMode(red[index], OUTPUT);
    pinMode(green[index], OUTPUT);
    pinMode(pulse[index], INPUT);
  }
  SPI.begin();
  rfid.init();
}

void loop()
{
  lcdDisplay();
  change_led_position();
  if (rfid.isCard()) {
    if (!in_door) {
      readCard();
    }
  }
  else {
    if (save_id) {
      open_door(in_door, out_door);
    }
  }
  temp_in_door = in_door;
  temp_out_door = out_door;
  rfid.halt();
}

void readCard() {
  if (rfid.readCardSerial()) {
    if (fullList || (temp_value_in.num0 == rfid.serNum[0]
                     && temp_value_in.num1 == rfid.serNum[1]
                     && temp_value_in.num2 == rfid.serNum[2]
                     && temp_value_in.num3 == rfid.serNum[3]
                     && temp_value_in.num4 == rfid.serNum[4])) return;
    /* Hiển thị số ID của card */
    Serial.println(" ");
    Serial.println("Tim thay RFID Card");
    temp_value_in.num0 = rfid.serNum[0];
    temp_value_in.num1 = rfid.serNum[1];
    temp_value_in.num2 = rfid.serNum[2];
    temp_value_in.num3 = rfid.serNum[3];
    temp_value_in.num4 = rfid.serNum[4];
    temp_value_in.checkExitsInList = 1;
    saveIdToList();
    //Serial.println(" ");
    Serial.println("Ma Card:");
    Serial.print("Hex: ");
    Serial.print(rfid.serNum[0], HEX);
    Serial.print(", ");
    Serial.print(rfid.serNum[1], HEX);
    Serial.print(", ");
    Serial.print(rfid.serNum[2], HEX);
    Serial.print(", ");
    Serial.print(rfid.serNum[3], HEX);
    Serial.print(", ");
    Serial.print(rfid.serNum[4], HEX);
    Serial.println(" ");
    save_id = 1;
  }
}

void open_door(byte in, byte out) {
  if (in) {
    Serial.print("----- mo cong vao");
    numberOfCar++;
  }
  else Serial.print("----- khong mo cong vao");
}

void change_led_position() {
  byte index = 0;
  numberOfFreePosition1 = numberOfFreePosition2 = 0;
  while (index < 10) {
    if (digitalRead(pulse[index])) {
      if (index < 5) numberOfFreePosition2++;
      else numberOfFreePosition1++;
      digitalWrite(red[index], LOW);
      digitalWrite(green[index], HIGH);
    } else {
      digitalWrite(red[index], HIGH);
      digitalWrite(green[index], LOW);
    }
    index++;
  }
  in_door = digitalRead(pulse_in);
  out_door = digitalRead(pulse_out);
  if (in_door) {
    digitalWrite(red_in, LOW);
    digitalWrite(green_in, HIGH);
  } else {
    if (save_id) save_id = 0;
    digitalWrite(red_in, HIGH);
    digitalWrite(green_in, LOW);
  }
  if (out_door) {
    digitalWrite(red_out, LOW);
    digitalWrite(green_out, HIGH);
  } else {
    digitalWrite(red_out, HIGH);
    digitalWrite(green_out, LOW);
  }
}
void saveIdToList() {
  byte index = 0;
  for (index; index < 10; index++) {
    if (!id_values[index].checkExitsInList) {
      id_values[index].num0 = temp_value_in.num0;
      id_values[index].num1 = temp_value_in.num1;
      id_values[index].num2 = temp_value_in.num2;
      id_values[index].num3 = temp_value_in.num3;
      id_values[index].num4 = temp_value_in.num4;
      id_values[index].checkExitsInList = temp_value_in.checkExitsInList;
      numberOfCar++;
      if (index == 9) fullList = 1;
      return;
    }
  }
  fullList = 1;
}
int checkIdFromList() {
  byte index;
  for (index; index < 10; index++) {
    if (id_values[index].checkExitsInList
        && id_values[index].num0 == temp_value_out.num0
        && id_values[index].num1 == temp_value_out.num1
        && id_values[index].num2 == temp_value_out.num2
        && id_values[index].num3 == temp_value_out.num3
        && id_values[index].num4 == temp_value_out.num4) {
      return index;
    }
  }
  return -1;
}
void removeFromList(byte index) {
  if (index >= 0) {
    id_values[index].num0 = id_values[index].num1 = id_values[index].num2 = id_values[index].num3 = id_values[index].num4 = id_values[index].checkExitsInList = 0;
    fullList = 0;
    return;
  }
  lcd.setCursor(0, 0);
  lcd.print("id not found!");
  delay(1000);
}
void lcdDisplay() {
  byte up = defaultNumberOfFreePosition + numberOfFreePosition1;
  lcdDisplayCharArray(up, lastNumberOfFreePosition1, 2, 0);
  lastNumberOfFreePosition1 = up;
  byte left = defaultNumberOfFreePosition + numberOfFreePosition2;
  lcdDisplayCharArray(left, lastNumberOfFreePosition2, 3, 1);
  lastNumberOfFreePosition2 = left;
}
void lcdDisplayCharArray(byte direct, byte lastNumberOfFreePosition, int cursor_column, int cursor_row) {
  if (lastNumberOfFreePosition != direct) {
    char c[2] = {0x30, 0x30};
    byte index = 0;
    while (direct / 10) {
      c[index] += direct % 10;
      direct /= 10;
      index++;
    }
    c[index] += direct;
    lcd.printstr(c, cursor_column, cursor_row);
    char d[10] = {0x43, 0x20};
    lcd.printstr(d, cursor_column += 2, cursor_row);
    d[0] = 0x4F; d[1] = 0x48;
    lcd.printstr(d, cursor_column += 2, cursor_row);
    d[0] = 0x54; d[1] = 0x20;
    lcd.printstr(d, cursor_column += 2, cursor_row);
    d[0] = 0x4F; d[1] = 0x52;
    lcd.printstr(d, cursor_column += 2, cursor_row);
    d[0] = 0x47; d[1] = 0x4E;
    lcd.printstr(d, cursor_column += 2, cursor_row);
//    lcd.printstr(*(d+2), cursor_column + 2, cursor_row);
  }
}

