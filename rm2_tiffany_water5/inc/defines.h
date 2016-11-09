

#define UL_CENTER_FREQ  868800000

#define SIMPLE_ONESEC_TESTER

#define POWER_METER

//#define WATER_METER

//#define SENDEVERYONEHOUR

//#define TIFFANY
//#define TIFFANY_14dBm
//#define TIFFANY_17dBm
#define DIN



#ifdef DIN
#define HARDWARE_REV 8  //OldDin
#else
#define HARDWARE_REV 5  //Tiffany
#endif


#define EXTENDED_INFO

#ifdef POWER_METER

#define DIV 100
//#define SOFTWARE_REV 16+1
#define SOFTWARE_REV 16+2 //+ сообщение о депассивации

//#define SOFTWARE_REV 16 - с ошибкой итератора

#endif

#ifdef WATER_METER
#define TWOCHANNELS
//#define DIV 1
#define DIV 100
//#define SOFTWARE_REV 0+1
#define SOFTWARE_REV 0+2       //+ сообщение о депассивации
//#define SOFTWARE_REV 0 - с ошибкой итератора

#endif



