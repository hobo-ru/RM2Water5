#include <stdlib.h>
#include "water5.h"
#include "defines.h"


uint8_t battery_read_measure();
uint8_t CompVersion();

W5_TAGS_TO_SAVE W5_Tags;

uint8_t  SendBuf[8];
uint8_t  LaterSendBuf[8];
uint8_t  LaterTimer = 0;

uint8_t SendFlag;

uint8_t Seconds = 0;
uint8_t Minutes = 0;
uint8_t Hours = 0;
uint8_t Days = 0;
uint8_t Days4month = 0;

uint16_t HourMas0[24], HourMas1[24];
uint32_t DayMas0[7], DayMas1[7];

uint8_t sendweekflag = 0;
uint8_t sendmonthflag = 0;

uint32_t  PC_PrevHour0, PC_PrevHour1;
uint32_t  PC_PrevDay0, PC_PrevDay1;

uint32_t  PC_PrevMinute0, PC_PrevMinute1;

uint8_t   MaxFlow0 = 0;
uint8_t   MaxFlow1 = 0;


uint8_t  Calibrate;

uint16_t  Switch_i;

uint8_t Voltage = 0xA0;
uint8_t Temperature = 20;
uint8_t Every_15min_flag = 1;

int8_t  PowLev = 0;

uint32_t DaysAfterDepas = 59;

uint8_t depassivation_active = 0;

uint8_t waitsomeminutes = 0;



void ReadAllParams()
{

  TD_FLASH_Read((void*)(&W5_Tags), sizeof(W5_Tags));

}

void WriteAllParams()
{
  
  TD_FLASH_Write((void*)(&W5_Tags), sizeof(W5_Tags));
  
}

void ClearAllParams()
{
  memset((void*)&W5_Tags, 0, sizeof(W5_TAGS_TO_SAVE));
  WriteAllParams();
}

uint8_t log10_DIV()
{
    switch(DIV)
    {
    case 1:
      return    1<<6; 
    case 10:
      return    2<<6;
    case 100:
      return    3<<6;
    }
    return 0;
}

void Water5Init(uint8_t clear)
{
               
        if(clear)  ClearAllParams();
        else ReadAllParams();

        for(int i = 0; i < 24; i++) HourMas0[i] = HourMas1[i] = 0;
        for(int i = 0; i < 7; i++) DayMas0[i] = DayMas1[i] = 0;
        
        PC_PrevMinute0 = PC_PrevHour0 = PC_PrevDay0 = W5_Tags.PulseCounter0;
        PC_PrevMinute1 = PC_PrevHour1 = PC_PrevDay1 = W5_Tags.PulseCounter1;
        
        *((uint32_t *)(&SendBuf[1])) = W5_Tags.PulseCounter0/DIV;
        *((uint32_t *)(&LaterSendBuf[1])) = W5_Tags.PulseCounter1/DIV;        

        SendBuf[0] = 0x41;
        LaterSendBuf[0] = 0x49;    
        SendBuf[5] = LaterSendBuf[5] = HARDWARE_REV;
        SendBuf[6] = LaterSendBuf[6] = log10_DIV() + SOFTWARE_REV;
        SendBuf[7] = LaterSendBuf[7] = CompVersion();
        SendFlag = 3;
}


void PCSendDayData(uint8_t chan, uint8_t later)
{

    uint8_t i;
    uint32_t Hours2BitsLo, Hours2BitsHi;
    uint16_t maxHour;
    uint16_t* HourMas;
    uint8_t*  buf;

    Hours2BitsLo = 0;
    Hours2BitsHi = 0;

    maxHour = 1;

    WriteAllParams();

    HourMas = chan?HourMas1:HourMas0;
    buf =  later?LaterSendBuf:SendBuf;

    for(i=0;i!=24;i++) if(HourMas[i] > maxHour) maxHour = HourMas[i];

    for(i=0;i!=24;i++)
    {
        if(HourMas[i])
        {
            if(i<16) Hours2BitsLo+=(((uint32_t)(HourMas[i])*3*99/maxHour/100)+1)<<(i*2);
            else Hours2BitsHi+=(((uint32_t)(HourMas[i])*3*99/maxHour/100)+1)<<((i-16)*2);
        }

    }
    *((uint16_t*)(&buf[0])) = ((((chan?W5_Tags.PulseCounter1:W5_Tags.PulseCounter0)/DIV)&0xFFFF)<<1)&0xfffe;
    *((uint32_t*)(&buf[2])) = Hours2BitsLo;
    *((uint16_t*)(&buf[6])) = Hours2BitsHi&0xFFFF;

     if(later) 
     {
       LaterTimer = later&0x7F;
       if(later&0x80) SendFlag |= 2;
     }
     else SendFlag |= 1;

}


#ifdef SENDEVERYONEHOUR
void PCSendHourData(uint8_t chan, uint8_t later)
{
    uint8_t*  buf;
 
     buf =  later?LaterSendBuf:SendBuf;

     *((uint32_t*)(&buf[1])) = (chan?W5_Tags.PulseCounter1:W5_Tags.PulseCounter0)/DIV;
     buf[0] = chan?0x99:0x91;
     buf[5] = buf[6] = buf[7] = 0xaa;
     
     if(later) 
     {
       LaterTimer = later&0x7F;
       if(later&0x80) SendFlag |= 2;
     }
     else SendFlag |= 1;
}
#endif


uint8_t GetVoltageOrTemp(uint8_t val)
{
    if(val) return Voltage;
    return Temperature;
}


void PCSendWeekData(uint8_t chan, uint8_t later)
{

    uint8_t i;

    uint32_t   maxDay;
    uint32_t   DayBits;
    uint32_t* DayMas;
    uint8_t*  buf;

    DayMas = chan?DayMas1:DayMas0;
    buf =  later?LaterSendBuf:SendBuf;

    *((uint32_t*)(&buf[1])) = (chan?W5_Tags.PulseCounter1:W5_Tags.PulseCounter0)/DIV;

    buf[7] = GetVoltageOrTemp(1);

    DayBits = 0;
    maxDay=1;
    for(i=0;i!=7;i++) if(DayMas[i] > maxDay) maxDay = DayMas[i];
    for(i=0;i!=7;i++) DayBits+=(((uint32_t)(DayMas[i])*8*99/maxDay/100))<<(i*3);

    DayBits<<=3;
    DayBits|=buf[4]&0x7;
    *((uint16_t*)(&buf[4])) = DayBits&0xFFFF;
    buf[6] = DayBits>>16;
    buf[0] = chan?0x7B:0x73;
    if(later) 
    {
       LaterTimer = later&0x7F;
       if(later&0x80) SendFlag |= 2;
    }
    else SendFlag |= 1;

}

void PCSendExtInfo(uint8_t chan, uint8_t later)
{

     uint8_t*  buf;

     buf =  later?LaterSendBuf:SendBuf;

     buf[0] = chan?0x89:0x81;;
     buf[1] = HARDWARE_REV;
     buf[2] = log10_DIV() + SOFTWARE_REV;
     buf[3] = CompVersion();
     buf[4] = GetVoltageOrTemp(0);
     buf[5] = GetVoltageOrTemp(1);
#ifdef  DIN
     buf[6] = PowLev*3 + 8;
#endif
#ifdef  TIFFANY
     buf[6] = PowLev*3 + 2;
#endif
#ifdef  TIFFANY_14dBm
     buf[6] = PowLev*3 + 5;
#endif
#ifdef  TIFFANY_17dBm
     buf[6] = PowLev*3 + 8;
#endif

     buf[7] = chan?MaxFlow1:MaxFlow0;

     if(later)
     {
       LaterTimer = later&0x7F;
       if(later&0x80) SendFlag |= 2;
     }
     else SendFlag |= 1;
}


void PCSendInfoData(uint8_t chan, uint8_t later)
{

     uint8_t*  buf;

#ifdef EXTENDED_INFO     
     static uint8_t sendextinfo_timer = 5;
     
     if(!chan) sendextinfo_timer++;
     
     if(!(sendextinfo_timer%6))
     {
        PCSendExtInfo(chan, later);
        return;
     }  
#endif     
     buf =  later?LaterSendBuf:SendBuf;

     *((uint32_t*)(&buf[1])) = (chan?W5_Tags.PulseCounter1:W5_Tags.PulseCounter0)/DIV;
     *((uint16_t*)(&buf[5])) = W5_Tags.MesNum;
     buf[0] = chan?0x69:0x61;
     buf[7] = PowLev;
     buf[7] <<= 6;
     buf[7] += (chan?MaxFlow1:MaxFlow0)&0x3F;
     
     if(later) 
     {
       LaterTimer = later&0x7F;
       if(later&0x80) SendFlag |= 2;
     }
     else SendFlag |= 1;
}


void PCSendComm(uint8_t comm, uint8_t chan, uint8_t later)
{
     uint8_t*  buf;

     buf =  later?LaterSendBuf:SendBuf;

     *((uint32_t*)(&buf[1])) =  (chan?W5_Tags.PulseCounter1:W5_Tags.PulseCounter0)/DIV;
     buf[5] = comm;
     buf[6] = GetVoltageOrTemp(0);
     buf[7] = GetVoltageOrTemp(1);
     buf[0] = chan?0x59:0x51;
     if(later) 
     {
       LaterTimer = later&0x7F;
       if(later&0x80) SendFlag |= 2;
     }
     else SendFlag |= 1;
}

void PCSendDepass()
{
    SendBuf[0] = 0xE3;
    SendBuf[1] = HARDWARE_REV;
    SendBuf[2] = log10_DIV() + SOFTWARE_REV;;
    SendBuf[3] = CompVersion();
    SendBuf[4] = GetVoltageOrTemp(0);
    SendBuf[5] = GetVoltageOrTemp(1);
#ifdef  DIN
    SendBuf[6] = PowLev*3 + 8;
#endif
#ifdef  TIFFANY
    SendBuf[6] = PowLev*3 + 2;
#endif
#ifdef  TIFFANY_14dBm
    SendBuf[6] = PowLev*3 + 5;
#endif
#ifdef  TIFFANY_17dBm
    SendBuf[6] = PowLev*3 + 8;
#endif
    SendBuf[7] = MaxFlow0;
    SendFlag |= 1;
}


#ifdef SIMPLE_ONESEC_TESTER
void PCSendSimpleIterator()
{
    static uint32_t iter = 0;
    SendBuf[0] = 0xEE;
    SendBuf[1] = 0;
    SendBuf[2] = 0;
    SendBuf[3] = 0;
    SendBuf[4] = (iter>>24)&0xff;
    SendBuf[5] = (iter>>16)&0xff;
    SendBuf[6] = (iter>>8)&0xff;
    SendBuf[7] = (iter>>0)&0xff;
    SendFlag |= 1;
    iter++;
}
#endif

bool Water5Loop()
{   
   
  if(Every_15min_flag)
  {
    Every_15min_flag = 0;
    Temperature = TD_MEASURE_Temperature();
  }
  
  if(SendFlag&1)
    {
        if(W5_Tags.Protocol) UNBsend(SendBuf, 8, serial);
        else UNB_ProtocolC_Send(SendBuf, serial);
        Voltage = battery_read_measure();
        SendFlag &= 0xfe;
        W5_Tags.MesNum++;
        return 1;
        
    }
    if(SendFlag&2)
    {

#ifdef TWOCHANNELS
        if(W5_Tags.Protocol) UNBsend(LaterSendBuf, 8, serial);
        else UNB_ProtocolC_Send(LaterSendBuf, serial);
        Voltage = battery_read_measure();
        W5_Tags.MesNum++;
        SendFlag &= 0xfd;
        return 1;
#else
        SendFlag &= 0xfd;
#endif

    }
    return 0;
}



void  Water5OneSec()    //run it every 1 sec
{
 #ifndef SIMPLE_ONESEC_TESTER
  
    if(!GPIO_PinInGet(SB1_PORT , SB1_PIN)&&(Switch_i!=60)&&!waitsomeminutes)
    {
        if(Switch_i%2) 
        {
          RM2_LED(ON);
          Calibrate = 0;
        }
        else RM2_LED(OFF);
        Switch_i++;
    }
    else
    {
        if(!(DaysAfterDepas%60)&&(Voltage < 0x80))
        {
            if(!depassivation_active)
            {
                depassivation_active = 1;
                PCSendDepass();
            }
        }
        if(depassivation_active || (Voltage < 60)) RM2_LED(ON);
  
        if(Switch_i)
        {
            if(W5_Tags.initcal == 0)
            {
                W5_Tags.initcal = 1;
                WriteAllParams();
                Switch_i = 66;
            }
            switch(Switch_i/2)
            {
            case 5: //cal
            case 33: //init cal
                if(Switch_i/2 == 5) Calibrate = 10;
                PC_PrevMinute0 = PC_PrevHour0 = PC_PrevDay0 = W5_Tags.PulseCounter0;
                PC_PrevMinute1 = PC_PrevHour1 = PC_PrevDay1 = W5_Tags.PulseCounter1;
                Seconds = 0;//random() % 60;;
                Minutes = 0;//random() % 60;;
                Hours = 0;//12 + random() % 11;;
                MaxFlow0 = MaxFlow1 = 0;
                for(int i = 0; i < 24; i++) HourMas0[i] = HourMas1[i] = 0;
                for(int i = 0; i < 7; i++) DayMas0[i] = DayMas1[i] = 0;
                WriteAllParams();
                PCSendComm(Switch_i/2,0,0);
                PCSendComm(Switch_i/2,1,0x80);
                break;
 /*           case 8:
                ClearAllParams();
                SCB->AIRCR = 0x05FA0004;
                break;*/
            case 9:
                depassivation_active = 1;
                PCSendDepass();
                break;
            case 21:
                if(W5_Tags.Protocol) W5_Tags.Protocol = 0;
                else W5_Tags.Protocol = 1;
                WriteAllParams();
                break;
            case 30:
                WriteAllParams();
                PCSendComm(30,0,0);
                waitsomeminutes = 120;
                break;
            case 0:
                break;
            default:
                Calibrate = 10;
                WriteAllParams();
                PCSendComm(Switch_i/2,0,0);
                PCSendComm(Switch_i/2,1,0x80);
                break;
            }
            Switch_i = 0;
            RM2_LED(OFF);
        }
    }

#endif
      
    if(++Seconds == 60)
    {

        Seconds = 0;

        if((W5_Tags.PulseCounter0 - PC_PrevMinute0) > MaxFlow0) MaxFlow0 = W5_Tags.PulseCounter0 - PC_PrevMinute0;
        if((W5_Tags.PulseCounter1 - PC_PrevMinute1) > MaxFlow1) MaxFlow1 = W5_Tags.PulseCounter1 - PC_PrevMinute1;
        PC_PrevMinute0 = W5_Tags.PulseCounter0;
        PC_PrevMinute1 = W5_Tags.PulseCounter1;
        
        if(LaterTimer)
        {
            if(--LaterTimer == 0) SendFlag |= 2;
        }

        if(Calibrate&&!SendFlag)
        {
            Calibrate--; 
            PCSendInfoData(0,0);
            PCSendInfoData(1,0x80);           
        }
        if(!(Minutes%15))
        {
              Every_15min_flag = 1;
//////////////for DEMO - stend///////////////////////
//            PCSendInfoData(0,0);
//            PCSendInfoData(1,0x80);
////////////////////////////////////////////////
        }
        
#ifdef SIMPLE_ONESEC_TESTER
              PCSendSimpleIterator();
#endif     
        if(sendweekflag)
        {
            sendweekflag = 0;
            PCSendWeekData(0,0);
            PCSendWeekData(1,0x80);
        }
        else if(sendmonthflag)
        {
            sendmonthflag = 0;
            PCSendInfoData(0,0);
            PCSendInfoData(1,0x80);
        }
        if(waitsomeminutes) waitsomeminutes--;
        if(++Minutes == 60)
        {
          Minutes = 0;
          RM2_LED(OFF);
          HourMas0[Hours] = W5_Tags.PulseCounter0 - PC_PrevHour0;
          PC_PrevHour0 = W5_Tags.PulseCounter0;

          HourMas1[Hours] = W5_Tags.PulseCounter1 - PC_PrevHour1;
          PC_PrevHour1 = W5_Tags.PulseCounter1;

          if((!(W5_Tags.poehali&0x1))&&(HourMas0[Hours]>2))
          {
            W5_Tags.poehali |= 1;
            PCSendInfoData(0,0);
          }
          if((!(W5_Tags.poehali&0x2))&&(HourMas1[Hours]>2))
          {
            W5_Tags.poehali |= 2;
            PCSendInfoData(1,0x80);
          }
          if(++Hours == 24)
          {
            Hours = 0;
            DaysAfterDepas++;
            if(depassivation_active)
            {
                    depassivation_active = 0;
                    WriteAllParams();
                    Calibrate = 6;
                    RM2_LED(OFF);
            }
            DayMas0[Days] = W5_Tags.PulseCounter0 - PC_PrevDay0;
            DayMas1[Days] = W5_Tags.PulseCounter1 - PC_PrevDay1;
            PC_PrevDay0 = W5_Tags.PulseCounter0;
            PC_PrevDay1 = W5_Tags.PulseCounter1;
#ifdef TWOCHANNELS
            if(W5_Tags.Protocol)
            {
                PCSendDayData(0,0);
                PCSendDayData(1,1);
            }
            else
            {
          
                PCSendDayData(0,0x80*((UNB_GetCiter()%2) != 0));
                PCSendDayData(1,0x80*((UNB_GetCiter()%2) == 0));
            }
#else
            PCSendDayData(0,0);
#endif
           
            if(++Days == 7) {Days = 0; sendweekflag = 2;}
            if(++Days4month == 30) {Days4month = 0; sendmonthflag = 1;}
          }
          else
          {
#ifdef SENDEVERYONEHOUR
            PCSendHourData(0,0);
            PCSendHourData(1,30);
#endif            
          }
        }
    }
}

