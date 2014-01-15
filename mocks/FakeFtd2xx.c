#include <stdio.h>
#include <stdarg.h>
#include "memory.h"
#include "../lib/ftd2xx.h"

#define SPY_FILE_PATH "/tmp/ftd2xx.spy"
#define NUM_OF_FAKE_DEVICES 2

typedef struct {
    char * message;
    int messageCounter;
    char serialNumber[16];
    char description[64];
    FILE * spyFileHandler;
} FakeDevice;

static FakeDevice fakeDeviceList[10];
static int fakeDeviceCount = 0;

static void FakeDevice_Create(void)
{
    if (fakeDeviceCount > NUM_OF_FAKE_DEVICES)
        return;

    char serialNumber[16];
    char filePath[255];
    FakeDevice fakeDevice;

    sprintf(serialNumber, "FTDX%02d", fakeDeviceCount);
    sprintf(filePath, "%s_%02d", SPY_FILE_PATH, fakeDeviceCount);

    fakeDevice.message = "Fake Message";
    fakeDevice.messageCounter = 0;
    strcpy(fakeDevice.serialNumber, serialNumber);
    strcpy(fakeDevice.description, "The devices description");
    fakeDevice.spyFileHandler = fopen(filePath, "w+");

    fakeDeviceList[fakeDeviceCount] = fakeDevice;
    fakeDeviceCount++;
}

static void writeToSpyFile(char * message)
{
    fwrite(message, strlen(message), 1, fakeDeviceList[0].spyFileHandler);
    fclose(fakeDeviceList[0].spyFileHandler);
}

/*
* To be used like printf, but prefixes the output
*/
static void printLog(char *fmt, ...)
{
    va_list ap; /* points to each unnamed arg in turn */
    va_start(ap, fmt);
    printf("__DeviceLog: ");
    vprintf(fmt, ap);
    va_end(ap); /* clean up when done */
}


FT_STATUS FT_Open(int deviceNumber, FT_HANDLE *pHandle)
{
    printLog("Device %d requested to open\n", deviceNumber);
    return FT_OK;
}

FT_STATUS FT_OpenEx(PVOID pArg1, DWORD Flags, FT_HANDLE *pHandle)
{
    if (fakeDeviceCount < 1)
        FakeDevice_Create();
    printLog("Device requested to openEx\n");
    printLog("Flag: %d\n", Flags);
    switch(Flags) {
        case FT_OPEN_BY_SERIAL_NUMBER:
            printLog("opened by serial number %s (%d devices faked)\n", (PCHAR)pArg1, fakeDeviceCount);
            int i;
            for(i=0; i<fakeDeviceCount; i++) {
                if (strncmp((PCHAR)pArg1, fakeDeviceList[i].serialNumber, 16) == 0) {
                    printLog("Serial number matches.\n");
                    return FT_OK;
                }
            }
            return FT_DEVICE_NOT_FOUND;
            break;
        case FT_OPEN_BY_DESCRIPTION:
            printLog("opened by description\n");
            break;
        case FT_OPEN_BY_LOCATION:
            printLog("opened by location\n");
            break;
        default:
            printLog("Flag not found!");
    }
    return FT_OK;
}

FT_STATUS FT_SetDataCharacteristics(FT_HANDLE ftHandle, UCHAR WordLength, UCHAR StopBits, UCHAR Parity)
{
    printLog("DataCharacteristics set!\n");
    return FT_OK;
}

FT_STATUS FT_SetBaudRate(FT_HANDLE ftHandle,ULONG BaudRate)
{
    printLog("new Baud rate: %d\n", BaudRate);
    return FT_OK;
}

FT_STATUS FT_SetEventNotification(FT_HANDLE ftHandle, DWORD Mask, PVOID Param)
{
    printLog("Event notification set.\n");
    return FT_OK;
}

FT_STATUS FT_GetQueueStatus(FT_HANDLE ftHandle, LPDWORD lpRxBytes)
{
    *lpRxBytes = strlen(fakeDeviceList[0].message) - fakeDeviceList[0].messageCounter;
    printLog("Que Status requested.\n");
    return FT_OK;
}

FT_STATUS FT_Read(FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD dwBytesToRead, LPDWORD lpBytesReturned)
{
    printLog("%d Bytes requested to read. \n", dwBytesToRead);
    memcpy(lpBuffer, fakeDeviceList[0].message + fakeDeviceList[0].messageCounter, dwBytesToRead);
    *lpBytesReturned = dwBytesToRead;
    fakeDeviceList[0].messageCounter += dwBytesToRead;
    return FT_OK;
}

FT_STATUS FT_Close(FT_HANDLE ftHandle) {
    printLog("Close requested.\n");
    writeToSpyFile("FT_Close\n");
    return FT_OK;
}

FT_STATUS FT_Purge(FT_HANDLE ftHandle, DWORD dwMask) {
    printLog("RX and TX Buffer Purge requested.\n");
    return FT_OK;
}

FT_STATUS FT_CreateDeviceInfoList(LPDWORD lpdwNumDevs)
{
    int i;
    for(i=1; i<NUM_OF_FAKE_DEVICES; i++)
        FakeDevice_Create();
    printLog("Create DeviceInfoList requested.\n");
    *lpdwNumDevs = fakeDeviceCount;
    return FT_OK;
}

FT_STATUS FT_GetDeviceInfoList(FT_DEVICE_LIST_INFO_NODE *pDest, LPDWORD lpdwNumDevs)
{
    printLog("Create GetDeviceInfoList requested.\n");
    *lpdwNumDevs = fakeDeviceCount;
    return FT_OK;
}

FT_STATUS FT_ListDevices(PVOID pArg1, PVOID pArg2, DWORD flags)
{
    printLog("Device List requested with %08X (%08X)\n", flags, FT_LIST_BY_INDEX);
    if(flags & FT_LIST_NUMBER_ONLY) {
        printLog("(only number).\n");
    }
    if(flags & FT_LIST_BY_INDEX) {
        printLog("(list by index)\n");

        DWORD deviceIndex = (DWORD)pArg1;
        if(flags & FT_OPEN_BY_SERIAL_NUMBER) {
            printLog("here we go!\n");
            //DWORD deviceIndex = *devIndexPointer;
            printLog("(get serial number for %d)\n", deviceIndex);
            strcpy(pArg2, fakeDeviceList[deviceIndex].serialNumber);
        }
        if(flags & FT_OPEN_BY_DESCRIPTION) {
            printLog("(get description for %d)\n", deviceIndex);
            strcpy(pArg2, fakeDeviceList[deviceIndex].description);
        }
        if(flags & FT_OPEN_BY_LOCATION) {
            printLog("(get location for %d)\n", deviceIndex);
            strcpy(pArg2, "Not implemented on Mac");
        }

    } else if(flags & FT_LIST_ALL)
        printLog("(list all [not implemented])\n");

    return FT_OK;
}

FT_STATUS FT_GetDeviceInfoDetail(DWORD dwIndex, LPDWORD lpdwFlags,
    LPDWORD lpdwType, LPDWORD lpdwID, LPDWORD lpdwLocId, LPVOID lpSerialNumber,
    LPVOID lpDescription, FT_HANDLE *pftHandle)
{
    printLog("Device Info Detail requested.\n");
    lpdwFlags = 0;
    lpdwType = 0;
    lpdwID = 0;
    lpdwLocId = 0;
    strcpy(lpSerialNumber,"DC0089EX");
    strcpy(lpDescription, "I2C Converter");
    pftHandle = 0;
    return FT_OK;
}

FT_STATUS FT_SetVIDPID(DWORD dwVID, DWORD dwPID)
{
    printLog("VIDPID set to vid: %d pid: %d\n", dwVID, dwPID);
    return FT_OK;
}