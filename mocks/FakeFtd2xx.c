#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "memory.h"
#include "../lib/ftd2xx.h"

#define SPY_FILE_PATH "/tmp/ftd2xx.spy"
#define NUM_OF_FAKE_DEVICES 2


static void printLog(char *fmt, ...);

typedef struct {
    char * message;
    int messageCounter;
    char serialNumber[16];
    char description[64];
    long vendorId;
    long productId;
    long locationId;
    char spyFilePath[255];
} FakeDevice;

static long usb_ids[2][2] =
    {
        {123,456},
        {321,654}
    };

static long vidPidFilter[2] = {0,0};

static FakeDevice fakeDeviceList[NUM_OF_FAKE_DEVICES];
static int fakeDeviceCount = 0;

static void FakeDevice_Create(void)
{
    if (fakeDeviceCount > NUM_OF_FAKE_DEVICES-1)
        return;

    char serialNumber[16];
    char description[64];
    char filePath[255];
    FakeDevice fakeDevice;

    sprintf(serialNumber, "FTDX%02d", fakeDeviceCount);
    sprintf(description, "Description for %02d.", fakeDeviceCount);
    sprintf(filePath, "%s_%02d", SPY_FILE_PATH, fakeDeviceCount);

    fakeDevice.message = "Fake Message";
    fakeDevice.messageCounter = 0;
    fakeDevice.vendorId = usb_ids[fakeDeviceCount][0];
    fakeDevice.productId = usb_ids[fakeDeviceCount][1];
    fakeDevice.locationId = 4;
    strcpy(fakeDevice.serialNumber, serialNumber);
    strcpy(fakeDevice.description, description);
    strcpy(fakeDevice.spyFilePath, filePath);

    printLog("Device %d created!\n", fakeDeviceCount);

    fakeDeviceList[fakeDeviceCount] = fakeDevice;
    fakeDeviceCount++;
}

static void writeToSpyFile(int deviceIndex, char * message)
{
    FILE * handler = fopen(fakeDeviceList[deviceIndex].spyFilePath, "w+");
    fwrite(message, strlen(message), 1, handler);
    fclose(handler);
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
    if (fakeDeviceCount < 1)
        FakeDevice_Create();
    writeToSpyFile(deviceNumber, "FT_Open");
    *(long *)pHandle = deviceNumber;
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
                    writeToSpyFile(i, "FT_OpenEX");
                    *(long *)pHandle = i;
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
    // Reset counter so the whole test suite can pass.
    // (SetEventNotification is called when subscribed to a 'data'-event)
    fakeDeviceList[(int)(long *)ftHandle].messageCounter = 0;
    printLog("Event notification set.\n");
    return FT_OK;
}

FT_STATUS FT_GetQueueStatus(FT_HANDLE ftHandle, LPDWORD lpRxBytes)
{
    int deviceIndex = (int)(long *)ftHandle;
    *lpRxBytes = strlen(fakeDeviceList[deviceIndex].message) - fakeDeviceList[deviceIndex].messageCounter;
    //if (*lpRxBytes == 0)
    //    fakeDeviceList[deviceIndex].messageCounter = 0;
    printLog("Que Status requested for %d (position %d).\n",deviceIndex ,fakeDeviceList[deviceIndex].messageCounter);
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

FT_STATUS FT_Write(FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD dwBytesToWrite, LPDWORD lpBytesWritten)
{
    printLog("Received write request: %s %d\n",(char *)lpBuffer,dwBytesToWrite);
    char * outputFmt = "FT_Write(\"%s\")\n";
    char output[strlen(outputFmt)+dwBytesToWrite];
    sprintf(output, outputFmt, lpBuffer);
    writeToSpyFile((int)(long *)ftHandle, output);
    return FT_OK;
}

FT_STATUS FT_Close(FT_HANDLE ftHandle) {
    printLog("Close requested for %d.\n", (long *)ftHandle);
    writeToSpyFile((int)(long *)ftHandle, "FT_Close\n");
    return FT_OK;
}

FT_STATUS FT_Purge(FT_HANDLE ftHandle, DWORD dwMask) {
    printLog("RX and TX Buffer Purge requested.\n");
    return FT_OK;
}

FT_STATUS FT_CreateDeviceInfoList(LPDWORD lpdwNumDevs)
{
    *lpdwNumDevs = 0;
    int i;
    for(i=1; i <= NUM_OF_FAKE_DEVICES; i++) {
        // TODO: move check for vid/pid elsewhere
        if (vidPidFilter[0] == 123 && vidPidFilter[1] == 456 && i > 1)
            break;
        FakeDevice_Create();
        *lpdwNumDevs += 1;
    }
    printLog("Create DeviceInfoList requested, returning %d device(s).\n", *lpdwNumDevs);
    return FT_OK;
}

FT_STATUS FT_GetDeviceInfoList(FT_DEVICE_LIST_INFO_NODE *pDest, LPDWORD lpdwNumDevs)
{
    *lpdwNumDevs = 0;
    int i;
    for(i=0; i < fakeDeviceCount; i++) {
        if (vidPidFilter[0] == 123 && vidPidFilter[1] == 456 && i > 1)
            break;
        long id = (fakeDeviceList[i].vendorId << 16) + fakeDeviceList[i].productId;
        FT_HANDLE ftHandle = malloc(sizeof(FT_HANDLE));

        FT_DEVICE_LIST_INFO_NODE device = {
            0, //ULONG Flags;
            0, //ULONG Type;
            id, //ULONG ID;
            fakeDeviceList[i].locationId, //DWORD LocId;
            "my serial", //char SerialNumber[16];
            "my description", //char Description[64];
            ftHandle
        };
        pDest[i] = device;
        *lpdwNumDevs += 1;
    }
    printLog("Create GetDeviceInfoList requested, returning %d node(s)\n", *lpdwNumDevs);
    return FT_OK;
}

FT_STATUS FT_ListDevices(PVOID pArg1, PVOID pArg2, DWORD flags)
{
    printLog("Device List requested with %08X\n", flags);
    if(flags & FT_LIST_NUMBER_ONLY) {
        printLog("(only number).\n");
    }
    if(flags & FT_LIST_BY_INDEX) {
        printLog(" - list by index\n");

        DWORD deviceIndex = (DWORD)pArg1;
        if(flags & FT_OPEN_BY_SERIAL_NUMBER) {
            printLog(" - get serial number for %d, returning %s\n", deviceIndex, fakeDeviceList[deviceIndex].serialNumber);
            strncpy((char *)pArg2, fakeDeviceList[deviceIndex].serialNumber, strlen(pArg2));
        }
        if(flags & FT_OPEN_BY_DESCRIPTION) {
            printLog(" - get description for %d, returning %s\n", deviceIndex, fakeDeviceList[deviceIndex].description);
            strcpy(pArg2, fakeDeviceList[deviceIndex].description);
        }
        if(flags & FT_OPEN_BY_LOCATION) {
            printLog(" - get location for %d, Not implemented on Mac, returning 0\n", deviceIndex);
            *(DWORD *)pArg2 = 0;
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
    strcpy(lpSerialNumber,"8888");
    strcpy(lpDescription, "8888");
    pftHandle = 0;
    return FT_OK;
}

FT_STATUS FT_SetVIDPID(DWORD dwVID, DWORD dwPID)
{
    vidPidFilter[0] = dwVID;
    vidPidFilter[1] = dwPID;
    printLog("VIDPID set to vid: %d pid: %d\n", dwVID, dwPID);
    return FT_OK;
}