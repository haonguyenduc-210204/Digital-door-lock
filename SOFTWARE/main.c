#include <16F887.h>
#fuses INTRC_IO, PUT, NOWDT, NOMCLR
#use delay(clock = 8MHz)
#include <TV_LCD_4BIT.c>

//================ SERVO ==================
#define SERVO_PIN PIN_C2
#define BUZZER_PIN PIN_A1

#define    _slcd_  10           //h?ng s? dùng cho kh? rung phím (debounce).
#define    KEY4x3_ROW_0    PIN_B0
#define    KEY4x3_ROW_1    PIN_B1
#define    KEY4x3_ROW_2    PIN_B2
#define    KEY4x3_ROW_3    PIN_B3
#define    KEY4x3_COL_0    PIN_B4
#define    KEY4x3_COL_1    PIN_B5
#define    KEY4x3_COL_2    PIN_B6

const char keymap[12] = {'1','4','7','*',
                         '2','5','8','0',
                         '3','6','9','#'};

unsigned int8 keyread() // Ki?m tra t?ng hàng c?a bàn phím.
{
     if (!INPUT(KEY4x3_ROW_0)) return 0;       // N?u hàng 0 ?ang ? m?c LOW ? phím thu?c hàng 0 ???c nh?n        
     else if (!INPUT(KEY4x3_ROW_1)) return 1;  // N?u hàng 1 LOW ? phím thu?c hàng 1 ???c nh?n             
     else if (!INPUT(KEY4x3_ROW_2)) return 2;  // N?u hàng 2 LOW ? phím thu?c hàng 2 ???c nh?n      
     else if (!INPUT(KEY4x3_ROW_3)) return 3; // N?u hàng 3 LOW ? phím thu?c hàng 3 ???c nh?n
     return 0xff; // Không có hàng nào LOW ? không có phím nào ???c nh?n
}
char key_4x3_dw()
{      
      unsigned int8 mp=0xff,tam;
      static int8 cot=0,dem=0;
      static int1 tt=0;
	  // mp  : mã phím tr? v? (0..11). 0xff ngh?a là không có phím.
	  // tam : giá tr? hàng ??c ???c t? keyread() (0..3 ho?c 0xff).
	  // cot : c?t hi?n ?ang ???c quét (0?c?t 1, 1?c?t 2, 2?c?t 3).
	  // dem : b? ??m ?? kh? rung phím.
	  // tt  : tr?ng thái phím (0: ch?a nh?n, 1: ?ang gi?).
      tam = keyread();// ??c xem hàng nào ?ang th?p (phím nh?n)
	  // ??????????????????????????????????????????????????????????????
	  // 1) PHÁT HI?N NH?N PHÍM L?N ??U (tam != 0xff và tt == 0)
	  // ??????????????????????????????????????????????????????????????
      if((tam!=0xff)&&(!tt))
      {
          dem++;// T?ng b? ??m kh? rung
          if(dem>=_slcd_)  // N?u gi? ?n ??nh ?? s? l?n ? ch?p nh?n phím nh?n
          {
               mp = tam + cot*4;  // Tính mã phím = hàng + c?t*4
               tt=1;    // Chuy?n sang tr?ng thái "?ang gi? phím"            
          }
       }
	    // ??????????????????????????????????????????????????????????????
	    // 2) KHÔNG CÓ PHÍM NH?N & CH?A TRONG TR?NG THÁI NH?N
	    //    ? QUÉT C?T TI?P THEO
	    // ??????????????????????????????????????????????????????????????
       else if((tam==0xff)&&(!tt))
       {
          dem=0;cot++;cot%=4;
          if(cot==0) {output_high(KEY4x3_COL_2); output_low(KEY4x3_COL_0);} // C?t 0 ???c kéo LOW
          if(cot==1) {output_high(KEY4x3_COL_0); output_low(KEY4x3_COL_1);} // C?t 1 LOW
          if(cot==2) {output_high(KEY4x3_COL_1); output_low(KEY4x3_COL_2);}// C?t 2 LOW
       }
	   
	   // ??????????????????????????????????????????????????????????????
	   // 3) PHÍM ?ANG ???C GI? & V?N NH?N (tam != 0xff và tt == 1)
	   // ??????????????????????????????????????????????????????????????
       else if((tam!=0xff)&&(tt))   dem=_slcd_;// Gi? b? ??m ? m?c max ?? ?ánh d?u phím v?n ?ang gi? 
       else if(tam==0xff)  
       { 
          dem++;// ??m th?i gian nh? phím  
          if(dem>=2*_slcd_){tt=0;dem=0;}  // Khi nh? ?? lâu ? reset tr?ng thái,// Cho phép nh?n phím m?i

       }
       return mp;// Tr? v? mã phím (ho?c 0xff n?u không có)
}

void servo_dong()
{
   int8 m;
   for(m=0;m<=50;m++)  // L?p kho?ng 51 l?n ? duy trì tín hi?u ~1 giây ?? servo ch?c ch?n quay xong
   {                                            
      output_high(SERVO_PIN);       // B?t ??u t?o xung ?i?u khi?n: ??a chân servo lên m?c HIGH                        
      delay_ms (1);                 // Gi? m?c HIGH trong 1 ms ? xung r?ng 1ms          
      output_low(SERVO_PIN);        // K?t thúc xung: ??a chân xu?ng LOW
      delay_us (1000);              // Thêm 1 ms LOW (ph?n này k?t h?p v?i delay_ms(18) ?? t?o chu k? 20 ms)
      delay_ms (18);			    // T?ng th?i gian m?t chu k? PWM ? 1ms HIGH + 19ms LOW = 20ms (chu k? chu?n 50Hz)
   }         
}

void servo_mo()
{
   int8 m;
   for(m=0;m<=50;m++) 
   {                                            
      output_high(SERVO_PIN);                             
      delay_ms (1);    
      delay_us (1000); 
      output_low(SERVO_PIN);                      
      delay_ms (18);
   }         
}


//================ MAIN ==================
char mk[5] = {' ',' ',' ',' ',' '};//mã PIN ?ã l?u trong EEPROM
char str[5] = {' ',' ',' ',' ',' '};//mã ng??i dùng ?ang nh?p l?n 1
char strs[5] = {' ',' ',' ',' ',' '};//mã ng??i dùng nh?p l?n 2 ?? xác nh?n (confirm)
unsigned int8 j = 0, t = 0, epp = 0, sai = 0;
int1 s = 0, set = 0, clr = 0;
unsigned int8 mp;

void doc_ma_pin()
{
   mk[0] = read_eeprom(1); // ??c ký t? th? 1 c?a mã PIN t? EEPROM ??a ch? 1
   delay_ms(10);
   mk[1] = read_eeprom(2);// ??c ký t? th? 2 t? EEPROM ??a ch? 2
   delay_ms(10);
   mk[2] = read_eeprom(3);// ??c ký t? th? 3 t? EEPROM ??a ch? 3
   delay_ms(10);
   mk[3] = read_eeprom(4);// ??c ký t? th? 4 t? EEPROM ??a ch? 4
   delay_ms(10);
   mk[4] = read_eeprom(5);// ??c ký t? th? 5 t? EEPROM ??a ch? 5
   delay_ms(10);
}

void tone(unsigned int8 sl, unsigned int16 tg)
{
   unsigned int8 l;
   for(l = 0; l<sl; l++)// L?p l?i 'sl' l?n ? s? l?n kêu c?a buzzer
   {
      output_high(BUZZER_PIN);// B?t còi (xu?t m?c HIGH ra chân BUZZER)
      delay_ms(tg); // Gi? ti?ng kêu trong tg mili-giây
      output_low(BUZZER_PIN);// T?t còi (xu?t m?c LOW)
      delay_ms(tg); // Gi? im l?ng tg mili-giây r?i m?i l?p ti?p
   }
}

void main()
{
    set_tris_b(0x0f); // RB0..RB3 là input (ROW), RB4..RB7 là output (COL)
    port_b_pullups(0xff);// B?t tr? kéo lên n?i RB0..RB7 ?? ??c phím ?n ??nh
    set_tris_a(0x00); // PORTA xu?t (buzzer,…)
    set_tris_c(0b00000000); // PORTC xu?t (servo,…)
    set_tris_d(0b00000001);// RD0 là input (nút reset PIN), RD1..RD7 output

    output_low(BUZZER_PIN);// T?t còi
    servo_dong();// ??a servo v? tr?ng thái ?óng khi kh?i ??ng
    delay_ms(500);// Ch? ?n ??nh

    lcd_setup();// Kh?i t?o LCD 4-bit
    delay_ms(1000); // Ch? LCD s?n sàng
    
    epp = read_eeprom(6);// ??c EEPROM xem ?ã t?ng l?u mã PIN hay ch?a
    if(epp != 9)  // N?u EEPROM[6] != 9 ? ch?a cài ??t mã PIN
    {
       set = 1;			// B?t ch? ?? cài ??t mã PIN m?i
       lcd_goto_xy(0,0);
       lcd_data("> CAI DAT MA PIN");
       lcd_goto_xy(0,1);
       lcd_data("> HAY NHAP 5 SO!");
    }
    else              // ?ã cài mã PIN ? ??c mã l?u s?n
    {
       doc_ma_pin();	 // L?y 5 ký t? PIN t? EEPROM ??a vào mk[]
       set = 0;			// Ch? ?? nh?p mã m? khóa
       lcd_goto_xy(0,0);
       lcd_data("HAY NHAP MA PIN ");
       lcd_goto_xy(0,1);
       lcd_data("DE MO KHOA CUA  ");
    }
    
    while(TRUE)			// Vòng l?p chính
    {
		  //================ RESET MÃ PIN (Nh?n gi? RD0 3 giây) ================
      if(input(PIN_D0) == 0)	// N?u nút reset ???c nh?n
      {
         delay_ms(20);
         if(input(PIN_D0) == 0)
         {
            delay_ms(3000);		 // Gi? 3 giây liên t?c
            if(input(PIN_D0) == 0)// N?u v?n còn gi? ? xác nh?n reset
            {
               lcd_goto_xy(0,0);
               lcd_data("KHOI PHUC MA PIN");
               lcd_goto_xy(0,1);
               lcd_data("DANG XU LY......");
               tone(8,50);		// Kêu báo
               int8 a;
               for(a = 0; a < 7; a++);
               {
                  write_eeprom(a,0xff);
               }
               delay_ms(1500);	 // Ch? hoàn t?t
               lcd_goto_xy(0,0);
               lcd_data("> CAI DAT MA PIN");
               lcd_goto_xy(0,1);
               lcd_data("> HAY NHAP 5 SO!");
               int8 w;
               for(w=0;w<5;w++) {str[w] = ' '; strs[w] = ' ';}// Xóa nh?p c?
               set = 1; s = 0; j = 0; t = 0; clr = 0; mp = 0xff;// Reset bi?n
            }
         }
      }
	  //========================= CH? ?? M? KHÓA ===========================
      if(set == 0)
      {
         mp = key_4x3_dw();  // ??c mã phím (0..11 ho?c 0xFF)
         if(mp!=0xff) // N?u phím ???c nh?n (mp ? 0xFF ngh?a là có phím h?p l?)
         {
         if(clr == 0){lcd_goto_xy(0,1); lcd_data("                "); clr = 1;}//Xóa dòng nh?p mã PIN trên LCD ?úng 1 l?n duy nh?t khi ng??i dùng b?t ??u nh?p phím m?i.
         char key = keymap[mp];// ??i mã phím ra ký t? th?c ('0'..'9', *, #)
            if(key >= '0' && key <= '9')// N?u là phím s?
            {
                if(j < 5)// V?n còn v? trí nh?p
                {
                    str[j] = key; // L?u s?
                    lcd_goto_xy(j,1);// Hi?n s?
                    lcd_data(str[j]);
                    delay_ms(500);
                    lcd_goto_xy(j,1);// Che l?i b?ng *
                    lcd_data('*'); 
                    j++;
                    if(j == 5) s = 1; // ?ã nh?p ?? 5 s? ? ki?m tra
                    
                     if(s == 1)
                     {
                         int1 correct = TRUE;
                         int8 k;
                         for(k=0;k<5;k++) // So sánh t?ng ký t?
                         {
                             if(str[k] != mk[k]) { correct = FALSE; break; }
                         }
         
                         lcd_goto_xy(0,0);
                         if(correct)  // ============ ?ÚNG MÃ PIN ============
                         {
                             sai = 0;
                             lcd_data("  DUNG MAT KHAU  ");
                             lcd_goto_xy(0,1);
                             lcd_data("=> CUA DA MO.....");
                             tone(1,500);
                             servo_mo();  // M? c?a
                             delay_ms(5000);// Gi? c?a m? 5 giây
                             servo_dong();    // ?óng c?a
                         }
                         else                // ============ SAI MÃ PIN ============
                         {
                             
                             lcd_data("SAI MAT KHAU !!!");
                             lcd_goto_xy(0,1);
                             lcd_data("HAY NHAP LAI....");
                             tone(3,80);
                             delay_ms(1500);
                             sai ++;
                             if(sai >= 3)  // Sai 3 l?n ? khóa t?m
                             {
                                 lcd_goto_xy(0,0);
                                 lcd_data(">> NHAP SAI QUA ");
                                 lcd_goto_xy(0,1);
                                 lcd_data("SO LAN QUY DINH ");
                                 delay_ms(1500);
                                 lcd_goto_xy(0,0);
                                 lcd_data(">> NHAP LAI SAU ");
                                 lcd_goto_xy(0,1);
                                 lcd_data("1 PHUT..........");
                                 tone(2000,30);// Ti?ng còi báo
                                 sai = 0;
                             }
                         }
						 // Reset ?? nh?p l?i
                         lcd_goto_xy(0,0);
                         lcd_data("HAY NHAP MA PIN ");
                         lcd_goto_xy(0,1);
                         lcd_data("DE MO KHOA CUA  ");
                         int8 h;
                         for(h=0;h<5;h++) {str[h] = ' ';}// Xóa d? li?u nh?p
                         clr = 0;
                         j = 0;
                         s = 0;
                     }
                }
                key = ' ';// Xóa ký t? t?m
            }
            else if(key == '*' || key == '#')// Xóa toàn b? khi nh?n * ho?c #
            {
               int8 b;
               for(b=0;b<5;b++) {str[b] = ' ';}
               lcd_goto_xy(0,1); 
               lcd_data("                ");
               j = 0;
            }
        }
         
      }
	     //========================= CH? ?? CÀI MÃ PIN ==========================
      else
      {
         mp = key_4x3_dw();
         if(mp!=0xff)
         {
         if(clr == 0){lcd_goto_xy(0,1); lcd_data("                "); clr = 1;}
         char keys = keymap[mp];
            if(keys >= '0' && keys <= '9')
            {
				   //===== Nh?p l?i l?n 2 (xác nh?n) =====
                if(s == 1)
                {
                   if(t < 5)
                   {
                       strs[t] = keys; // L?u ký t? confirm
                       lcd_goto_xy(t,1);
                       lcd_data(strs[t]);
                       delay_ms(500);
                       lcd_goto_xy(t,1);
                       lcd_data('*'); 
                       t++;
                       if(t == 5)  // Nh?p ?? ? ki?m tra
                       {
                            int1 cor = TRUE;
                            int8 kt;
                            for(kt=0;kt<5;kt++)
                            {
                                if(str[kt] != strs[kt]) { cor = FALSE; break; }
                            }
            
                            lcd_goto_xy(0,0);
                            if(cor) // Mã kh?p ? l?u EEPROM
                            {
                                tone(1,500);
                                lcd_data("  DUNG MAT KHAU  ");
                                lcd_goto_xy(0,1);
                                lcd_data("LUU MA PIN......."); 
                                delay_ms(500); 
                                write_eeprom(1,strs[0]);
                                delay_ms(10); 
                                write_eeprom(2,strs[1]);
                                delay_ms(10); 
                                write_eeprom(3,strs[2]);
                                delay_ms(10); 
                                write_eeprom(4,strs[3]);
                                delay_ms(10); 
                                write_eeprom(5,strs[4]);
                                delay_ms(10); 
                                write_eeprom(6,9);
                                delay_ms(10); 
              
                                int8 q;
                                for(q=0;q<5;q++) {str[q] = ' '; strs[q] = ' ';}
                                doc_ma_pin();  // ??c l?i mã m?i l?u
                                lcd_goto_xy(0,0);
                                lcd_data("HAY NHAP MA PIN ");
                                lcd_goto_xy(0,1);
                                lcd_data("DE MO KHOA CUA  ");
                                delay_ms(200); 
                                mp = 0xff;
                                s = 0;
                                t = 0;
                                j = 0;
                                clr = 0;
                                set = 0;
                                
                            }
                            else                     // Mã không kh?p
                            {
                                tone(3,80);
                                lcd_data("SAI MAT KHAU !!!");
                                lcd_goto_xy(0,1);
                                lcd_data("HAY NHAP LAI....");
                                delay_ms(1500);
                                t = 0;
                                int8 q;
                                for(q=0;q<5;q++) {strs[q] = ' ';}
                                mp = 0xff;
                                lcd_goto_xy(0,0);
                                lcd_data("XAC NHAN MA PIN:");
                                lcd_data("                ");
                                clr = 0;
                            }
                       }
                       keys = ' ';
                   }
                }
				    //===== Nh?p mã PIN l?n ??u =====
                else
                {
                   if(j < 5)
                   {
                       str[j] = keys;// L?u ký t? l?n 1
                       lcd_goto_xy(j,1);
                       lcd_data(str[j]);
                       delay_ms(500);
                       lcd_goto_xy(j,1);
                       lcd_data('*'); 
                       j++;
                       if(j == 5)    // ?? 5 s? ? sang b??c xác nh?n
                       {
                           s = 1;
                           tone(1,500);
                           lcd_goto_xy(0,0);
                           lcd_data("XAC NHAN MA PIN:");
                           lcd_goto_xy(0,1); 
                           lcd_data("                ");
                           j = 0;
                       }
                       keys = ' ';
                   }
                }
                
            }
            else if(keys == '*' || keys == '#')// Xóa h?t khi nh?n * ho?c #
            {
               j = 0;
               t = 0;
               int8 c;
               for(c=0;c<5;c++) {str[c] = ' '; strs[c] = ' ';}
               lcd_goto_xy(0,1); 
               lcd_data("                ");
            }
         }
      }
         
    }
}

