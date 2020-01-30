/*
  RelayBoard Module
  By Gustavo Suim and Vitor Paganini
  Data: 22/07/2013
*/

#ifndef RelayBoard_h
#define RelayBoard_h

#include "Arduino.h"

class RelayBoard
{
  public:

    RelayBoard(int data, int str, int clk, int numberboard);

    void set(int board, int out, boolean status);
    void load(int board, int out, boolean status);
    void go();

  private:
    

    int _clk;
    int _str;
    int _data;
    int _numberboard;

    int outs[40];
    
 };


#endif
