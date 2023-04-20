#include "stdint.h"
#include "stdio.h"
#include "STC15xx_config.h"



#if defined(STC15W20XS)
   //#include "STC15.h"
#endif
   #define RELAY P55
   #define POWER_IN_PIN P32
   #define POWER_LOST_DET P33
   #define INIT_PIN //Pin init not needed

   void _delay_ms(unsigned char ms)
{	
    // i,j selected for fosc 11.0592MHz, using oscilloscope
    // the stc-isp tool gives unusable values (perhaps for C51 vs sdcc?)
    // max 255 ms
    unsigned char i, j;
    do {
    	i = 4;
    	j = 200;
    	do
    	{
    		while (--j);
    	} while (--i);
    } while (--ms);
}


void setup(){
    INIT_PIN
    
    POWER_IN_PIN=LOW;
    POWER_LOST_DET=LOW;
    RELAY=LOW;
}

void main(){
    

    setup();
    while (1)
    {
     _delay_ms(10000);
        if(POWER_IN_PIN==1){
            RELAY=HIGH;
            _delay_ms(500);
            }
            
        else if(POWER_IN_PIN=0){
            RELAY=LOW;
            _delay_ms(500);
            }
        /*else if(POWER_LOST_DET=1){
                _nop_();
            
            }*/
        

    }
    
}