#include <RelayBoard.h>

#define date    6
#define strobe  3
#define clock   2
#define numberboards 2

RelayBoard relay(date, strobe, clock, numberboards);

int i;
int j;

void setup()
{
  delay(1000);
}

void loop()
{

  for(i=0; i<=7; i++)
  {
    relay.set(0,i,1);
    delay(50);
    relay.set(0,i,0);
    delay(50);
  }

  for(i=7; i>=0; i--)
  {
    relay.set(0,i,1);
    delay(50);
    relay.set(0,i,0);
    delay(50);
  }

  for(i=0,j=7;i<=3;i++,j--)
  {
    relay.load(0,i,1);
    relay.load(0,j,1);
    relay.go();
    delay(50);
  }

  for(i=3,j=4;i>=0;i--,j++)
  {
    relay.load(0,i,0);
    relay.load(0,j,0);
    relay.go();
    delay(50);
  }

  for(i=0;i<=1;i++)
  {
    for(j=0;j<=1;j++)
    {
      relay.load(0,0,j);
      relay.load(0,1,j);
      relay.load(0,6,j);
      relay.load(0,7,j);
      relay.go();
      delay(500);
    }
  }

  for(i=0;i<=1;i++)
  {
    for(j=0;j<=1;j++)
    {
      relay.load(0,2,j);
      relay.load(0,3,j);
      relay.load(0,4,j);
      relay.load(0,5,j);
      relay.go();
      delay(500);
    }
  }

  for(i=0; i<=7; i++)
  {
    relay.load(0,i,0);
  }

  relay.go();
  delay(500);

  for(i=0; i<=7; i++)
  {
    relay.load(0,i,1);
  }

  relay.go();
  delay(500);

  for(i=0; i<=7; i++)
  {
    relay.set(1,i,1);
    delay(50);
    relay.set(1,i,0);
    delay(50);
  }

  for(i=7; i>=0; i--)
  {
    relay.set(1,i,1);
    delay(50);
    relay.set(1,i,0);
    delay(50);
  }

  for(i=0,j=7;i<=3;i++,j--)
  {
    relay.load(1,i,1);
    relay.load(1,j,1);
    relay.go();
    delay(50);
  }

  for(i=3,j=4;i>=0;i--,j++)
  {
    relay.load(1,i,0);
    relay.load(1,j,0);
    relay.go();
    delay(50);
  }

  for(i=0;i<=1;i++)
  {
    for(j=0;j<=1;j++)
    {
      relay.load(1,0,j);
      relay.load(1,1,j);
      relay.load(1,6,j);
      relay.load(1,7,j);
      relay.go();
      delay(500);
    }
  }

  for(i=0;i<=1;i++)
  {
    for(j=0;j<=1;j++)
    {
      relay.load(1,2,j);
      relay.load(1,3,j);
      relay.load(1,4,j);
      relay.load(1,5,j);
      relay.go();
      delay(500);
    }
  }

  for(i=0; i<=7; i++)
  {
    relay.load(1,i,0);
  }
  
  relay.go();
  delay(500);
  
  for(i=0; i<=7; i++)
  {
    relay.load(1,i,1);
  }
  
  relay.go();
  delay(1000);

  for(i=0; i<=7; i++)
  {
    relay.load(1,i,0);
    relay.load(0,i,0);
  }
  
  relay.go();

}

