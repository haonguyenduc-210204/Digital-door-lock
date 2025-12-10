#define lcd_rs   pin_d2
#define lcd_e    pin_d3
#define lcd_d4   pin_d4
#define lcd_d5   pin_d5
#define lcd_d6   pin_d6
#define lcd_d7   pin_d7

void lcd_write_4bit(int8 d)
{
   output_bit(lcd_d4, bit_test(d,0));
   output_bit(lcd_d5, bit_test(d,1));
   output_bit(lcd_d6, bit_test(d,2));
   output_bit(lcd_d7, bit_test(d,3));
   output_high(lcd_e);
   output_low(lcd_e);
   delay_us(20);
}

void lcd_comand(int8 d)
{
   output_low(lcd_rs);
   lcd_write_4bit(d>>4);
   lcd_write_4bit(d);
}

void lcd_data(int8 d)
{
   output_high(lcd_rs);
   lcd_write_4bit(d>>4);
   lcd_write_4bit(d);
}

void lcd_setup()
{
   lcd_comand(0x32); delay_ms(5);
   lcd_comand(0x2C); delay_ms(5);
   lcd_comand(0x0c);
   lcd_comand(0x01); delay_ms(2);
}

void lcd_goto_xy(int8 y, int8 x)
{
   const unsigned int8 dc[]={0x80,0xc0,0x94,0xd4};
   lcd_comand(dc[x]+y);
}


