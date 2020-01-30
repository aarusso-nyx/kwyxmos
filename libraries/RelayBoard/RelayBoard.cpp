/*
  RelayBoard Module
  By Gustavo Suim and Vitor Paganini
  data: 22/07/2013
*/

#include "Arduino.h"
#include "RelayBoard.h"

RelayBoard::RelayBoard(int data, int str, int clk, int numberboard)
{
  pinMode(clk,  OUTPUT);
  pinMode(str,  OUTPUT);
  pinMode(data, OUTPUT);
    	
  _clk  = clk;
  _str  = str; 
  _data = data;
  _numberboard = numberboard;
  
  //clear all outs[]
	outs[0]=0; outs[1]=0; outs[2]=0;outs[3]=0;outs[4]=0;outs[5]=0;outs[6]=0;outs[7]=0;outs[8]=0;outs[9]=0;
	outs[10]=0; outs[11]=0; outs[12]=0;outs[13]=0;outs[14]=0;outs[15]=0;outs[16]=0;outs[17]=0;outs[18]=0;outs[19]=0;
	outs[20]=0; outs[21]=0; outs[22]=0;outs[23]=0;outs[24]=0;outs[25]=0;outs[26]=0;outs[27]=0;outs[28]=0;outs[29]=0;
	outs[30]=0; outs[31]=0; outs[32]=0;outs[33]=0;outs[34]=0;outs[35]=0;outs[36]=0;outs[37]=0;outs[38]=0;outs[39]=0;
	
}

//----------------------------------------------------------------------
void RelayBoard::go()
{ int count;

 for(count = (_numberboard *8) - 1; count >=0; count--)
 {

		if(outs[count] == 1) digitalWrite(_data,1);
		if(outs[count] == 0) digitalWrite(_data,0);

		digitalWrite(_clk,1);
		delay(1);
		digitalWrite(_clk,0);

  }

	digitalWrite(_str,1);
	delay(1);
	digitalWrite(_str,0);
}

//----------------------------------------------------------------------
void RelayBoard::set(int board, int out, boolean status)
{
    int count;

	 outs[board*8 +  out] = status;
	
  	for(count = (_numberboard *8) - 1; count >=0; count--)
    {
      if(outs[count] == 1) digitalWrite(_data,1);
      if(outs[count] == 0) digitalWrite(_data,0);

    	digitalWrite(_clk,1);
    	delay(1);
    	digitalWrite(_clk,0);
      
  	} 

	  delay(10);
    digitalWrite(_str,1);
    delay(1);
    digitalWrite(_str,0);

}
//----------------------------------------------------------------------
void RelayBoard::load(int board, int out, boolean status)
{
  outs[board*8 +  out] = status;
}
