
//ER-TFT4.58-1_Initial  

/* LCD  panel SPI interface define */
sbit	SPI_RES = P3^6;	 //reset
sbit	SPI_DI = P1^5;	//data input		 
sbit	SPI_CLK = P1^7;   //clock
sbit	SPI_CS = P3^4;  //chip select


//RGB+9b_SPI(rise)
 void SPI_SendData(unsigned char i)
{  
   unsigned char n;
   
   for(n=0; n<8; n++)			
   {       
			SPI_CLK=0;
			_nop_(); _nop_();_nop_();_nop_();
			SPI_DI=i&0x80;
			_nop_(); _nop_();_nop_();_nop_();
			SPI_CLK=1;
			i<<=1;
			_nop_(); _nop_();_nop_();_nop_();
	  
   }
}
void SPI_WriteComm(unsigned char i)
{
    SPI_CS=0;
	  	_nop_(); _nop_();_nop_();_nop_();
    SPI_DI=0 ;
	  	_nop_(); _nop_();_nop_();_nop_(); 
	SPI_CLK=0;
		_nop_(); _nop_();_nop_();_nop_();
	SPI_CLK=1;
		_nop_(); _nop_();_nop_();_nop_();
	SPI_SendData(i);
		
    SPI_CS=1;
}



void SPI_WriteData(unsigned char i)
{ 
    SPI_CS=0;
	  	_nop_(); _nop_();_nop_();_nop_();
    SPI_DI=1 ;
	  	_nop_(); _nop_();_nop_();_nop_(); 
	SPI_CLK=0;
		_nop_(); _nop_();_nop_();_nop_();
	SPI_CLK=1;
		_nop_(); _nop_();_nop_();_nop_();
	SPI_SendData(i);
		
    SPI_CS=1;
} 
 

/*
 //RGB+9b_SPI(fall)
 void SPI_SendData(unsigned char i)
{  
   unsigned char n;
   
   for(n=0; n<8; n++)			
   {       
			SPI_CLK=1;
			_nop_(); _nop_();_nop_();_nop_();
			SPI_DI=i&0x80;
			_nop_(); _nop_();_nop_();_nop_();
			SPI_CLK=0;
			i<<=1;
			_nop_(); _nop_();_nop_();_nop_();
	  
   }
}
void SPI_WriteComm(unsigned char i)
{
    SPI_CS=0;
	  	_nop_(); _nop_();_nop_();_nop_();
    SPI_DI=0 ;
	  	_nop_(); _nop_();_nop_();_nop_(); 
	SPI_CLK=1;
		_nop_(); _nop_();_nop_();_nop_();
	SPI_CLK=0;
		_nop_(); _nop_();_nop_();_nop_();
	SPI_SendData(i);
		
    SPI_CS=1;
}



void SPI_WriteData(unsigned char i)
{ 
    SPI_CS=0;
	  	_nop_(); _nop_();_nop_();_nop_();
    SPI_DI=1 ;
	  	_nop_(); _nop_();_nop_();_nop_(); 
	SPI_CLK=1;
		_nop_(); _nop_();_nop_();_nop_();
	SPI_CLK=0;
		_nop_(); _nop_();_nop_();_nop_();
	SPI_SendData(i);
		
    SPI_CS=1;
} 
 */

void ST7701S_Initial(void)
{

	SPI_RES=1;
	delay_ms(10);
	SPI_RES=0;
	delay_ms(100);
	SPI_RES=1;
	delay_ms(10);  


	//ST7701S+AUO4.58

	SPI_WriteComm (0xFF);     
	SPI_WriteData (0x77); 
	SPI_WriteData (0x01);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x13);   
	
	SPI_WriteComm (0xEF);     
	SPI_WriteData (0x08);   
	
	SPI_WriteComm (0xFF);     
	SPI_WriteData (0x77);   
	SPI_WriteData (0x01);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x10);   
	
	SPI_WriteComm (0xC0);     
	SPI_WriteData (0x77);   
	SPI_WriteData (0x00);   
	
	SPI_WriteComm (0xC1);     
	SPI_WriteData (0x09);   
	SPI_WriteData (0x08);   
	
	SPI_WriteComm (0xC2);//inv     
	SPI_WriteData (0x37);   
	SPI_WriteData (0x02);  
	
	SPI_WriteComm (0xC3); //????    
	SPI_WriteData (0x80);
	SPI_WriteData (0x05);
	SPI_WriteData (0x0d);	 
	
	SPI_WriteComm (0xCC);     
	SPI_WriteData (0x10);   
	
	SPI_WriteComm (0xB0);     
	SPI_WriteData (0x40);   
	SPI_WriteData (0x14);   
	SPI_WriteData (0x59);   
	SPI_WriteData (0x10);   
	SPI_WriteData (0x12);   
	SPI_WriteData (0x08);   
	SPI_WriteData (0x03);   
	SPI_WriteData (0x09);   
	SPI_WriteData (0x05);   
	SPI_WriteData (0x1E);   
	SPI_WriteData (0x05);   
	SPI_WriteData (0x14);   
	SPI_WriteData (0x10);   
	SPI_WriteData (0x68);   
	SPI_WriteData (0x33);   
	SPI_WriteData (0x15);   
	
	SPI_WriteComm (0xB1);     
	SPI_WriteData (0x40);   
	SPI_WriteData (0x08);   
	SPI_WriteData (0x53);   
	SPI_WriteData (0x09);   
	SPI_WriteData (0x11);   
	SPI_WriteData (0x09);   
	SPI_WriteData (0x02);   
	SPI_WriteData (0x07);   
	SPI_WriteData (0x09);   
	SPI_WriteData (0x1A);   
	SPI_WriteData (0x04);   
	SPI_WriteData (0x12);   
	SPI_WriteData (0x12);   
	SPI_WriteData (0x64);   
	SPI_WriteData (0x29);   
	SPI_WriteData (0x29);   
	
	SPI_WriteComm (0xFF);     
	SPI_WriteData (0x77);   
	SPI_WriteData (0x01);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x11);   
	
	SPI_WriteComm (0xB0);     
	SPI_WriteData (0x6D);  //6D 
	
	SPI_WriteComm (0xB1);   //vcom  
	SPI_WriteData (0x1D);   
	
	SPI_WriteComm (0xB2);     
	SPI_WriteData (0x87);   
	
	SPI_WriteComm (0xB3);     
	SPI_WriteData (0x80);   
	
	SPI_WriteComm (0xB5);     
	SPI_WriteData (0x49);   
	
	SPI_WriteComm (0xB7);     
	SPI_WriteData (0x85);   
	
	SPI_WriteComm (0xB8);     
	SPI_WriteData (0x20);   
	
	SPI_WriteComm (0xC1);     
	SPI_WriteData (0x78);   
	
	SPI_WriteComm (0xC2);     
	SPI_WriteData (0x78);   
	
	SPI_WriteComm (0xD0);     
	SPI_WriteData (0x88);   
	
	SPI_WriteComm (0xE0);     
	SPI_WriteData (0x00);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x02);   
	
	SPI_WriteComm (0xE1);     
	SPI_WriteData (0x02);   
	SPI_WriteData (0x8C);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x03);   
	SPI_WriteData (0x8C);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x33);   
	SPI_WriteData (0x33);   
	
	SPI_WriteComm (0xE2);     
	SPI_WriteData (0x33);   
	SPI_WriteData (0x33);   
	SPI_WriteData (0x33);   
	SPI_WriteData (0x33);   
	SPI_WriteData (0xC9);   
	SPI_WriteData (0x3C);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0xCA);   
	SPI_WriteData (0x3C);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x00);   
	
	SPI_WriteComm (0xE3);     
	SPI_WriteData (0x00);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x33);   
	SPI_WriteData (0x33);   
	
	SPI_WriteComm (0xE4);     
	SPI_WriteData (0x44);   
	SPI_WriteData (0x44);   
	
	SPI_WriteComm (0xE5);     
	SPI_WriteData (0x05);   
	SPI_WriteData (0xCD);   
	SPI_WriteData (0x82);   
	SPI_WriteData (0x82);   
	SPI_WriteData (0x01);   
	SPI_WriteData (0xC9);   
	SPI_WriteData (0x82);   
	SPI_WriteData (0x82);   
	SPI_WriteData (0x07);   
	SPI_WriteData (0xCF);   
	SPI_WriteData (0x82);   
	SPI_WriteData (0x82);   
	SPI_WriteData (0x03);   
	SPI_WriteData (0xCB);   
	SPI_WriteData (0x82);   
	SPI_WriteData (0x82);   
	
	SPI_WriteComm (0xE6);     
	SPI_WriteData (0x00);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x33);   
	SPI_WriteData (0x33);   
	
	SPI_WriteComm (0xE7);     
	SPI_WriteData (0x44);   
	SPI_WriteData (0x44);   
	
	SPI_WriteComm (0xE8);     
	SPI_WriteData (0x06);   
	SPI_WriteData (0xCE);   
	SPI_WriteData (0x82);   
	SPI_WriteData (0x82);   
	SPI_WriteData (0x02);   
	SPI_WriteData (0xCA);   
	SPI_WriteData (0x82);   
	SPI_WriteData (0x82);   
	SPI_WriteData (0x08);   
	SPI_WriteData (0xD0);   
	SPI_WriteData (0x82);   
	SPI_WriteData (0x82);   
	SPI_WriteData (0x04);   
	SPI_WriteData (0xCC);   
	SPI_WriteData (0x82);   
	SPI_WriteData (0x82);   
	
	SPI_WriteComm (0xEB);     
	SPI_WriteData (0x08);   
	SPI_WriteData (0x01);   
	SPI_WriteData (0xE4);   
	SPI_WriteData (0xE4);   
	SPI_WriteData (0x88);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x40);   
	
	SPI_WriteComm (0xEC);     
	SPI_WriteData (0x00);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x00);   
	
	SPI_WriteComm (0xED);     
	SPI_WriteData (0xFF);   
	SPI_WriteData (0xF0);   
	SPI_WriteData (0x07);   
	SPI_WriteData (0x65);   
	SPI_WriteData (0x4F);   
	SPI_WriteData (0xFC);   
	SPI_WriteData (0xC2);   
	SPI_WriteData (0x2F);   
	SPI_WriteData (0xF2);   
	SPI_WriteData (0x2C);   
	SPI_WriteData (0xCF);   
	SPI_WriteData (0xF4);   
	SPI_WriteData (0x56);   
	SPI_WriteData (0x70);   
	SPI_WriteData (0x0F);   
	SPI_WriteData (0xFF);   
	
	SPI_WriteComm (0xEF);     
	SPI_WriteData (0x10);   
	SPI_WriteData (0x0D);   
	SPI_WriteData (0x04);   
	SPI_WriteData (0x08);   
	SPI_WriteData (0x3F);   
	SPI_WriteData (0x1F);   
	
	SPI_WriteComm (0xFF);     
	SPI_WriteData (0x77);   
	SPI_WriteData (0x01);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x00);   
	SPI_WriteData (0x00);   
	
	SPI_WriteComm (0x11);     
	delay_ms(120);                
	
	SPI_WriteComm (0x35);     
	SPI_WriteData (0x00);   
	
	SPI_WriteComm (0x3A);     
	SPI_WriteData (0x66);   
	
	SPI_WriteComm (0x29); 

}


