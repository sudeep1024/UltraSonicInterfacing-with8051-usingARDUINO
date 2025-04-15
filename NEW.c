#include<reg51.h>

#define LCD_ADDR 0x4E // I2C LCD address for PCF8574 (0x27 << 1)

sbit SDA = P1^0;
sbit SCL = P1^1;

sbit trig = P3^2;
sbit echo = P3^3;

// ---------- Delay Functions ----------
void delay_us(unsigned int t) {
  while(t--);
}

void delay_ms(unsigned int t) {
  unsigned int i, j;
  for(i = 0; i < t; i++)
    for(j = 0; j < 1275; j++);
}

// ---------- I2C Functions ----------
void i2c_start() {
  SDA = 1; SCL = 1; delay_us(5);
  SDA = 0; delay_us(5);
  SCL = 0;
}

void i2c_stop() {
  SDA = 0; SCL = 1; delay_us(5);
  SDA = 1; delay_us(5);
}

bit i2c_write(unsigned char dat) {
  unsigned char i;
  bit ack;
  for(i = 0; i < 8; i++) {
    SDA = (dat & 0x80);
    SCL = 1; delay_us(5);
    SCL = 0; delay_us(5);
    dat <<= 1;
  }
  SDA = 1;
  SCL = 1; delay_us(5);
  ack = SDA;
  SCL = 0;
  return ack;
}

// ---------- LCD I2C Functions ----------
void lcd_i2c_cmd(unsigned char cmd) {
  unsigned char upper = cmd & 0xF0;
  unsigned char lower = (cmd << 4) & 0xF0;

  i2c_start();
  i2c_write(LCD_ADDR);
  i2c_write(upper | 0x0C);
  i2c_write(upper | 0x08);
  i2c_write(lower | 0x0C);
  i2c_write(lower | 0x08);
  i2c_stop();
  delay_us(50);
}

void lcd_i2c_data(unsigned char dat) {
  unsigned char upper = dat & 0xF0;
  unsigned char lower = (dat << 4) & 0xF0;

  i2c_start();
  i2c_write(LCD_ADDR);
  i2c_write(upper | 0x0D);
  i2c_write(upper | 0x09);
  i2c_write(lower | 0x0D);
  i2c_write(lower | 0x09);
  i2c_stop();
  delay_us(50);
}

void lcd_i2c_init() {
  delay_ms(50);
  lcd_i2c_cmd(0x33);
  lcd_i2c_cmd(0x32);
  lcd_i2c_cmd(0x28);
  lcd_i2c_cmd(0x0C);
  lcd_i2c_cmd(0x06);
  lcd_i2c_cmd(0x01);
  delay_ms(5);
}

void lcd_i2c_print(char *str) {
  while(*str)
    lcd_i2c_data(*str++);
}

// ---------- Ultrasonic Sensor Functions ----------
void send_trigger() {
  trig = 0;
  delay_us(2);
  trig = 1;
  delay_us(15);
  trig = 0;
}

unsigned int get_distance() {
  unsigned int time;

  send_trigger();

  // Wait for echo to go HIGH
  while(!echo);

  // Start Timer1 (16-bit mode)
  TMOD &= 0x0F;      // Clear Timer1 bits
  TMOD |= 0x10;      // Timer1 in Mode 1
  TH1 = 0;
  TL1 = 0;
  TR1 = 1;           // Start Timer1

  while(echo);       // Wait until echo goes LOW

  TR1 = 0;           // Stop Timer1

  time = (TH1 << 8) | TL1;

  // Convert to distance in cm (based on 11.0592 MHz clock and 1 machine cycle = 1.085 us)
  return (time * 1.085) / 58;
}

// ---------- Main ----------
void main() {
  unsigned int dist;

  lcd_i2c_init();
  lcd_i2c_cmd(0x80);
  lcd_i2c_print("Distance:");

  while(1) {
    dist = get_distance();

    lcd_i2c_cmd(0xC0);
    lcd_i2c_data((dist/100) + '0');
    lcd_i2c_data(((dist/10)%10) + '0');
    lcd_i2c_data((dist%10) + '0');
    lcd_i2c_print(" cm    ");
    delay_ms(200);
  }
}
