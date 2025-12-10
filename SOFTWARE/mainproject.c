#include <16F887.h>
#fuses INTRC_IO, PUT, NOWDT, NOMCLR
#use delay(clock = 8MHz)
#include <TV_LCD_4BIT.c>

//================ SERVO ==================
#define SERVO_PIN PIN_C2
#define BUZZER_PIN PIN_A1

#define    _slcd_  10           //hằng số dùng cho khử rung phím (debounce).
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

unsigned int8 keyread() // Kiểm tra từng hàng của bàn phím.
{
     if (!INPUT(KEY4x3_ROW_0)) return 0;       // Nếu hàng 0 đang ở mức LOW → phím thuộc hàng 0 được nhấn        
     else if (!INPUT(KEY4x3_ROW_1)) return 1;  // Nếu hàng 1 LOW → phím thuộc hàng 1 được nhấn             
     else if (!INPUT(KEY4x3_ROW_2)) return 2;  // Nếu hàng 2 LOW → phím thuộc hàng 2 được nhấn      
     else if (!INPUT(KEY4x3_ROW_3)) return 3; // Nếu hàng 3 LOW → phím thuộc hàng 3 được nhấn
     return 0xff; // Không có hàng nào LOW → không có phím nào được nhấn
}
char key_4x3_dw()
{      
      unsigned int8 mp=0xff,tam;
      static int8 cot=0,dem=0;
      static int1 tt=0;
	  // mp  : mã phím trả về (0..11). 0xff nghĩa là không có phím.
	  // tam : giá trị hàng đọc được từ keyread() (0..3 hoặc 0xff).
	  // cot : cột hiện đang được quét (0→cột 1, 1→cột 2, 2→cột 3).
	  // dem : bộ đếm để khử rung phím.
	  // tt  : trạng thái phím (0: chưa nhấn, 1: đang giữ).
      tam = keyread();// Đọc xem hàng nào đang thấp (phím nhấn)
	  // ──────────────────────────────────────────────────────────────
	  // 1) PHÁT HIỆN NHẤN PHÍM LẦN ĐẦU (tam != 0xff và tt == 0)
	  // ──────────────────────────────────────────────────────────────
      if((tam!=0xff)&&(!tt))
      {
          dem++;// Tăng bộ đếm khử rung
          if(dem>=_slcd_)  // Nếu giữ ổn định đủ số lần → chấp nhận phím nhấn
          {
               mp = tam + cot*4;  // Tính mã phím = hàng + cột*4
               tt=1;    // Chuyển sang trạng thái "đang giữ phím"            
          }
       }
	    // ──────────────────────────────────────────────────────────────
	    // 2) KHÔNG CÓ PHÍM NHẤN & CHƯA TRONG TRẠNG THÁI NHẤN
	    //    → QUÉT CỘT TIẾP THEO
	    // ──────────────────────────────────────────────────────────────
       else if((tam==0xff)&&(!tt))
       {
          dem=0;cot++;cot%=4;
          if(cot==0) {output_high(KEY4x3_COL_2); output_low(KEY4x3_COL_0);} // Cột 0 được kéo LOW
          if(cot==1) {output_high(KEY4x3_COL_0); output_low(KEY4x3_COL_1);} // Cột 1 LOW
          if(cot==2) {output_high(KEY4x3_COL_1); output_low(KEY4x3_COL_2);}// Cột 2 LOW
       }
	   
	   // ──────────────────────────────────────────────────────────────
	   // 3) PHÍM ĐANG ĐƯỢC GIỮ & VẪN NHẤN (tam != 0xff và tt == 1)
	   // ──────────────────────────────────────────────────────────────
       else if((tam!=0xff)&&(tt))   dem=_slcd_;// Giữ bộ đếm ở mức max để đánh dấu phím vẫn đang giữ 
       else if(tam==0xff)  
       { 
          dem++;// Đếm thời gian nhả phím  
          if(dem>=2*_slcd_){tt=0;dem=0;}  // Khi nhả đủ lâu → reset trạng thái,// Cho phép nhận phím mới

       }
       return mp;// Trả về mã phím (hoặc 0xff nếu không có)
}

void servo_dong()
{
   int8 m;
   for(m=0;m<=50;m++)  // Lặp khoảng 51 lần → duy trì tín hiệu ~1 giây để servo chắc chắn quay xong
   {                                            
      output_high(SERVO_PIN);       // Bắt đầu tạo xung điều khiển: đưa chân servo lên mức HIGH                        
      delay_ms (1);                 // Giữ mức HIGH trong 1 ms → xung rộng 1ms          
      output_low(SERVO_PIN);        // Kết thúc xung: đưa chân xuống LOW
      delay_us (1000);              // Thêm 1 ms LOW (phần này kết hợp với delay_ms(18) để tạo chu kỳ 20 ms)
      delay_ms (18);			    // Tổng thời gian một chu kỳ PWM ≈ 1ms HIGH + 19ms LOW = 20ms (chu kỳ chuẩn 50Hz)
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
char mk[5] = {' ',' ',' ',' ',' '};//mã PIN đã lưu trong EEPROM
char str[5] = {' ',' ',' ',' ',' '};//mã người dùng đang nhập lần 1
char strs[5] = {' ',' ',' ',' ',' '};//mã người dùng nhập lần 2 để xác nhận (confirm)
unsigned int8 j = 0, t = 0, epp = 0, sai = 0;
int1 s = 0, set = 0, clr = 0;
unsigned int8 mp;

void doc_ma_pin()
{
   mk[0] = read_eeprom(1); // Đọc ký tự thứ 1 của mã PIN từ EEPROM địa chỉ 1
   delay_ms(10);
   mk[1] = read_eeprom(2);// Đọc ký tự thứ 2 từ EEPROM địa chỉ 2
   delay_ms(10);
   mk[2] = read_eeprom(3);// Đọc ký tự thứ 3 từ EEPROM địa chỉ 3
   delay_ms(10);
   mk[3] = read_eeprom(4);// Đọc ký tự thứ 4 từ EEPROM địa chỉ 4
   delay_ms(10);
   mk[4] = read_eeprom(5);// Đọc ký tự thứ 5 từ EEPROM địa chỉ 5
   delay_ms(10);
}

void tone(unsigned int8 sl, unsigned int16 tg)
{
   unsigned int8 l;
   for(l = 0; l<sl; l++)// Lặp lại 'sl' lần → số lần kêu của buzzer
   {
      output_high(BUZZER_PIN);// Bật còi (xuất mức HIGH ra chân BUZZER)
      delay_ms(tg); // Giữ tiếng kêu trong tg mili-giây
      output_low(BUZZER_PIN);// Tắt còi (xuất mức LOW)
      delay_ms(tg); // Giữ im lặng tg mili-giây rồi mới lặp tiếp
   }
}

void main()
{
    set_tris_b(0x0f); // RB0..RB3 là input (ROW), RB4..RB7 là output (COL)
    port_b_pullups(0xff);// Bật trở kéo lên nội RB0..RB7 để đọc phím ổn định
    set_tris_a(0x00); // PORTA xuất (buzzer,…)
    set_tris_c(0b00000000); // PORTC xuất (servo,…)
    set_tris_d(0b00000001);// RD0 là input (nút reset PIN), RD1..RD7 output

    output_low(BUZZER_PIN);// Tắt còi
    servo_dong();// Đưa servo về trạng thái đóng khi khởi động
    delay_ms(500);// Chờ ổn định

    lcd_setup();// Khởi tạo LCD 4-bit
    delay_ms(1000); // Chờ LCD sẵn sàng
    
    epp = read_eeprom(6);// Đọc EEPROM xem đã từng lưu mã PIN hay chưa
    if(epp != 9)  // Nếu EEPROM[6] != 9 → chưa cài đặt mã PIN
    {
       set = 1;			// Bật chế độ cài đặt mã PIN mới
       lcd_goto_xy(0,0);
       lcd_data("> CAI DAT MA PIN");
       lcd_goto_xy(0,1);
       lcd_data("> HAY NHAP 5 SO!");
    }
    else              // Đã cài mã PIN → đọc mã lưu sẵn
    {
       doc_ma_pin();	 // Lấy 5 ký tự PIN từ EEPROM đưa vào mk[]
       set = 0;			// Chế độ nhập mã mở khóa
       lcd_goto_xy(0,0);
       lcd_data("HAY NHAP MA PIN ");
       lcd_goto_xy(0,1);
       lcd_data("DE MO KHOA CUA  ");
    }
    
    while(TRUE)			// Vòng lặp chính
    {
		  //================ RESET MÃ PIN (Nhấn giữ RD0 3 giây) ================
      if(input(PIN_D0) == 0)	// Nếu nút reset được nhấn
      {
         delay_ms(20);
         if(input(PIN_D0) == 0)
         {
            delay_ms(3000);		 // Giữ 3 giây liên tục
            if(input(PIN_D0) == 0)// Nếu vẫn còn giữ → xác nhận reset
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
               delay_ms(1500);	 // Chờ hoàn tất
               lcd_goto_xy(0,0);
               lcd_data("> CAI DAT MA PIN");
               lcd_goto_xy(0,1);
               lcd_data("> HAY NHAP 5 SO!");
               int8 w;
               for(w=0;w<5;w++) {str[w] = ' '; strs[w] = ' ';}// Xóa nhập cũ
               set = 1; s = 0; j = 0; t = 0; clr = 0; mp = 0xff;// Reset biến
            }
         }
      }
	  //========================= CHẾ ĐỘ MỞ KHÓA ===========================
      if(set == 0)
      {
         mp = key_4x3_dw();  // Đọc mã phím (0..11 hoặc 0xFF)
         if(mp!=0xff) // Nếu phím được nhấn (mp ≠ 0xFF nghĩa là có phím hợp lệ)
         {
         if(clr == 0){lcd_goto_xy(0,1); lcd_data("                "); clr = 1;}//Xóa dòng nhập mã PIN trên LCD đúng 1 lần duy nhất khi người dùng bắt đầu nhập phím mới.
         char key = keymap[mp];// Đổi mã phím ra ký tự thực ('0'..'9', *, #)
            if(key >= '0' && key <= '9')// Nếu là phím số
            {
                if(j < 5)// Vẫn còn vị trí nhập
                {
                    str[j] = key; // Lưu số
                    lcd_goto_xy(j,1);// Hiện số
                    lcd_data(str[j]);
                    delay_ms(500);
                    lcd_goto_xy(j,1);// Che lại bằng *
                    lcd_data('*'); 
                    j++;
                    if(j == 5) s = 1; // Đã nhập đủ 5 số → kiểm tra
                    
                     if(s == 1)
                     {
                         int1 correct = TRUE;
                         int8 k;
                         for(k=0;k<5;k++) // So sánh từng ký tự
                         {
                             if(str[k] != mk[k]) { correct = FALSE; break; }
                         }
         
                         lcd_goto_xy(0,0);
                         if(correct)  // ============ ĐÚNG MÃ PIN ============
                         {
                             sai = 0;
                             lcd_data("  DUNG MAT KHAU  ");
                             lcd_goto_xy(0,1);
                             lcd_data("=> CUA DA MO.....");
                             tone(1,500);
                             servo_mo();  // Mở cửa
                             delay_ms(5000);// Giữ cửa mở 5 giây
                             servo_dong();    // Đóng cửa
                         }
                         else                // ============ SAI MÃ PIN ============
                         {
                             
                             lcd_data("SAI MAT KHAU !!!");
                             lcd_goto_xy(0,1);
                             lcd_data("HAY NHAP LAI....");
                             tone(3,80);
                             delay_ms(1500);
                             sai ++;
                             if(sai >= 3)  // Sai 3 lần → khóa tạm
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
                                 tone(2000,30);// Tiếng còi báo
                                 sai = 0;
                             }
                         }
						 // Reset để nhập lại
                         lcd_goto_xy(0,0);
                         lcd_data("HAY NHAP MA PIN ");
                         lcd_goto_xy(0,1);
                         lcd_data("DE MO KHOA CUA  ");
                         int8 h;
                         for(h=0;h<5;h++) {str[h] = ' ';}// Xóa dữ liệu nhập
                         clr = 0;
                         j = 0;
                         s = 0;
                     }
                }
                key = ' ';// Xóa ký tự tạm
            }
            else if(key == '*' || key == '#')// Xóa toàn bộ khi nhấn * hoặc #
            {
               int8 b;
               for(b=0;b<5;b++) {str[b] = ' ';}
               lcd_goto_xy(0,1); 
               lcd_data("                ");
               j = 0;
            }
        }
         
      }
	     //========================= CHẾ ĐỘ CÀI MÃ PIN ==========================
      else
      {
         mp = key_4x3_dw();
         if(mp!=0xff)
         {
         if(clr == 0){lcd_goto_xy(0,1); lcd_data("                "); clr = 1;}
         char keys = keymap[mp];
            if(keys >= '0' && keys <= '9')
            {
				   //===== Nhập lại lần 2 (xác nhận) =====
                if(s == 1)
                {
                   if(t < 5)
                   {
                       strs[t] = keys; // Lưu ký tự confirm
                       lcd_goto_xy(t,1);
                       lcd_data(strs[t]);
                       delay_ms(500);
                       lcd_goto_xy(t,1);
                       lcd_data('*'); 
                       t++;
                       if(t == 5)  // Nhập đủ → kiểm tra
                       {
                            int1 cor = TRUE;
                            int8 kt;
                            for(kt=0;kt<5;kt++)
                            {
                                if(str[kt] != strs[kt]) { cor = FALSE; break; }
                            }
            
                            lcd_goto_xy(0,0);
                            if(cor) // Mã khớp → lưu EEPROM
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
                                doc_ma_pin();  // Đọc lại mã mới lưu
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
                            else                     // Mã không khớp
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
				    //===== Nhập mã PIN lần đầu =====
                else
                {
                   if(j < 5)
                   {
                       str[j] = keys;// Lưu ký tự lần 1
                       lcd_goto_xy(j,1);
                       lcd_data(str[j]);
                       delay_ms(500);
                       lcd_goto_xy(j,1);
                       lcd_data('*'); 
                       j++;
                       if(j == 5)    // Đủ 5 số → sang bước xác nhận
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
            else if(keys == '*' || keys == '#')// Xóa hết khi nhấn * hoặc #
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

