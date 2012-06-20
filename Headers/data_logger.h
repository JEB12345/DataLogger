
//******************************************************************************
// Function Prototypes
//******************************************************************************

#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H
#include "MDD File System/SD-SPI.h"
#include "GenericTypeDefs.h"
#include "MDD File System/FSIO.h"
#include "uart2.h"

char Init_Data_Logger(int id);

void Service_Spi(FSFILE *fo);

#endif


