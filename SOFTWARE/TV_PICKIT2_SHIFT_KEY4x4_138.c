#ifndef  __TV_PICKIT2_SHIFT_KEY4x4_C__
#define  __TV_PICKIT2_SHIFT_KEY4x4_C__
#include <TV_PICKIT2_SHIFT_KEY4x4_138.h>
unsigned int8 keyread()
{
     IF          (!INPUT(KEY4x4_ROW_0))    return 0;             
     ELSE    IF  (!INPUT(KEY4x4_ROW_1))    return 1;                 
     ELSE    IF  (!INPUT(KEY4x4_ROW_2))    return 2;              
     ELSE    IF  (!INPUT(KEY4x4_ROW_3))    return 3; 
     return 0xff;
}
unsigned int8 key_4x4_dw()
{      
      unsigned int8 mp=0xff,tam;
      static int8 cot=0,dem=0;
      static int1 tt=0;
      tam = keyread();
      if((tam!=0xff)&&(!tt))
      {
          dem++;
          if(dem>=_slcd_)
          {
               mp = tam + cot*4;
               tt=1;              
          }
       }
       else if((tam==0xff)&&(!tt))
       {
          dem=0;cot++;cot%=4;
          if(cot==0) {output_high(KEY4x4_COL_3);output_low(KEY4x4_COL_0);}
          if(cot==1) {output_high(KEY4x4_COL_0);output_low(KEY4x4_COL_1);}
          if(cot==2) {output_high(KEY4x4_COL_1);output_low(KEY4x4_COL_2);}
          if(cot==3) {output_high(KEY4x4_COL_2);output_low(KEY4x4_COL_3);} 
       }
       else if((tam!=0xff)&&(tt))   dem=_slcd_;   
       else if(tam==0xff)  
       { 
          dem++;
          if(dem>=2*_slcd_){tt=0;dem=0;}
       }
       return mp;
}
#endif
