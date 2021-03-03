#ifndef __ATADRV_H__
#define __ATADRV_H__

#define REG_DEVSEL 6        
#define REG_CYL_LO 4        
#define REG_CYL_HI 5        

#define ATADEV_PATAPI 0     
#define ATADEV_SATAPI 1
#define ATADEV_PATA 2
#define ATADEV_SATA 3
#define ATADEV_UNKNOWN 10

struct DEVICE{          
    int base;
    int dev_ctl;
};

int init_atadrv();

#endif
