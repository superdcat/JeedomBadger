

#include <EtherCard.h>
#include <Wiegand.h>

// reader and Jeedom settings

// Jeedom server IP
#define JEEDOM_IP "192.168.0.39"
// Number of reader in networks ( 0 to 255 )
#define READER_NUMBER 1
// Number of second to forget the entered code
#define FORGET_SECOND 30

// End Of Settings

//wiring  green D0 - 3
//        white D1 - 4


byte Ethernet::buffer[600];
static uint32_t timer;
static byte mymac[] = { 0x42,0x41,0x44,0x47,0x45,0x00 };


WIEGAND wg;
String Code;
byte CodeLen;
unsigned long lastEntered = millis();


static void my_callback (byte status, word off, word len) {
  
  Ethernet::buffer[off+300] = 0;
  

}

static void sendtoJeedom (char * cmd,char * value) {
  char url[100]; 
 
  sprintf(url,"name=BADGER%d&ip=%d.%d.%d.%d&cmd=%s&value=%s",mymac[5],ether.myip[0],ether.myip[1],ether.myip[2],ether.myip[3],cmd,value);   // prepare the GET, all variables in one string
 
  ether.browseUrl(PSTR("/plugins/badger/core/api/jeebadger.php?"), url, JEEDOM_IP, my_callback); // send it to the server
}

void setup () {
  mymac[5] =  READER_NUMBER;

  ether.begin(sizeof Ethernet::buffer, mymac,8);
  ether.dhcpSetup();

  ether.parseIp(ether.hisip, JEEDOM_IP);

  wg.begin();
  Code= "";
  CodeLen=0;
}

void loop () {
  
  ether.packetLoop(ether.packetReceive());

if(wg.available())
  {
    int type = wg.getWiegandType();

    if ( CodeLen >24 )
    {
          Code = "";
          CodeLen=0;    
    }
  
    if ( type == 4  || type == 8 ) //En mode digicode on r�cup�re les touches une par une
    {
    
	  if(CodeLen > 0 && millis() - lastEntered > FORGET_SECOND*1000) //On oublie, la derni�re sasie est trop vieille
      {
            Serial.println("Init code by time");
            Code = "";
            CodeLen=0;
      }
      lastEntered = millis();
      
      if ( wg.getCode() == 13 )
      {
        sendtoJeedom("pin",Code.c_str());
        Code = "";
        CodeLen=0;
      }
      else
      {
        if( wg.getCode() == 27 )
        {
           Code = "";
           CodeLen=0;
       }
        else
          Code.concat( String(wg.getCode(),HEX));
		  CodeLen++;
      }
    }
    else
    {
      Code = String(wg.getCode(),HEX);
      sendtoJeedom("tag",Code.c_str());
      Code="";
      CodeLen=0;
    } 

  }
  
}