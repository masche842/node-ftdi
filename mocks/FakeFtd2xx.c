#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include "memory.h"
#include "utils/jansson-2.5/src/jansson.h"
#include "../lib/ftd2xx.h"
#include "FakeFtd2xx.h"

#define SPY_FILE_PATH "/tmp/ftd2xx.spy"
#define NUM_OF_FAKE_DEVICES 4
#define DEFAULT_CONFIG_PATH "tests/support/fake_devices.json"
#define DEFAULT_CONFIG_PATH_ALT "test/support/fake_devices.json"

static long vidPidFilter[2] = {0,0};

static FakeDevice fakeDeviceList[NUM_OF_FAKE_DEVICES];
static int fakeDeviceListCount = 0;
char FakeFtd2xx_configFilePath[255];

int FakeFtd2xx_parseConfigFile(char * text, FakeDevice * fakeDevices, int maxDevices)
{
    int i;
    json_t *root;
    json_error_t error;
    root = json_loads(text, 0, &error);

    if(!json_is_array(root))
    {
        fprintf(stderr, "error: root is not an array\n");
        json_decref(root);
        return 1;
    }

    fakeDeviceListCount = 0;

    for(i = 0; i < json_array_size(root) && i < maxDevices; i++)
    {
        json_t *data, *serialNumber, *description, *vendorId, *productId,
               *locationId, *spyFilePath, *message, *communication;

        data = json_array_get(root, i);
        if(!json_is_object(data))
        {
            fprintf(stderr, "error: data for device %d is not an object\n", i + 1);
            json_decref(root);
            return 1;
        }
        serialNumber = json_object_get(data, "serialNumber");
        if(!json_is_string(serialNumber)) {
            fprintf(stderr, "error: can not read serialNumber from %d\n", i + 1);
            return 1;
        }
        description = json_object_get(data, "description");
        if(!json_is_string(description)) {
            fprintf(stderr, "error: can not read description from %d\n", i + 1);
            return 1;
        }
        message = json_object_get(data, "message");
        if(!json_is_string(message)) {
            fprintf(stderr, "error: can not read message from %d\n", i + 1);
            return 1;
        }
        vendorId = json_object_get(data, "vendorId");
        if(!json_is_number(vendorId)) {
            fprintf(stderr, "error: can not read vendorId from %d\n", i + 1);
            return 1;
        }
        productId = json_object_get(data, "productId");
        if(!json_is_number(productId)) {
            fprintf(stderr, "error: can not read productId from %d\n", i + 1);
            return 1;
        }
        locationId = json_object_get(data, "locationId");
        if(!json_is_number(locationId)) {
            fprintf(stderr, "error: can not read locationId from %d\n", i + 1);
            return 1;
        }
        spyFilePath = json_object_get(data, "spyFilePath");
        if(!json_is_string(spyFilePath)) {
            fprintf(stderr, "error: can not read spyFilePath from %d\n", i + 1);
            return 1;
        }
        communication = json_object_get(data, "communication");
        if(!json_is_object(communication)) {
            fprintf(stderr, "error: can not read communication from %d\n", i + 1);
            return 1;
        }

        strcpy(fakeDevices[i].serialNumber, json_string_value(serialNumber));
        strcpy(fakeDevices[i].description, json_string_value(description));
        strcpy(fakeDevices[i].message, json_string_value(message));

        fakeDevices[i].vendorId = json_number_value(vendorId);
        fakeDevices[i].productId = json_number_value(productId);
        fakeDevices[i].locationId = json_number_value(locationId);
        fakeDevices[i].communication = communication;

        strcpy(fakeDevices[i].spyFilePath, json_string_value(spyFilePath));

        fakeDevices[i].messageCounter = 0;

        fakeDeviceListCount += 1;
    }
    return 0;
}

const char * FakeFtd2xx_responseFromCommand(char * command, int deviceIndex)
{
    json_t * response;
    response = json_object_get(fakeDeviceList[deviceIndex].communication, command);
    if(!json_is_string(response))
        return 0;
    return json_string_value(response);
}

int FakeFtd2xx_readConfigFile(char * result)
{
    FILE *fp;
    int fileSize;

    if(strlen(FakeFtd2xx_configFilePath) <= 0) {
        int status = 1;
        status = FakeFtd2xx_setConfigFilePath(DEFAULT_CONFIG_PATH);
        if (0 != status) {
            status = FakeFtd2xx_setConfigFilePath(DEFAULT_CONFIG_PATH_ALT);
            if (0 != status) {
                return status;
            }
        }
    }
    printLog("Fake devices config file: %s\n", FakeFtd2xx_configFilePath);
    fp = fopen(FakeFtd2xx_configFilePath, "r+");
    if(!fp)
        return 1;
    // obtain file size:
    fseek(fp , 0 , SEEK_END);
    fileSize = ftell(fp);
    rewind(fp);
    fread(result, 1, fileSize, fp);
    fclose(fp);
    result[fileSize] = '\0';
    return 0;
}

int FakeFtd2xx_createDevices(FakeDevice * devices, int numberOfDevices)
{
    int status = 0;
    char rawJson[24000];
    status |= FakeFtd2xx_readConfigFile(rawJson);
    status |= FakeFtd2xx_parseConfigFile(rawJson, devices, numberOfDevices);

    return status;
}

int FakeFtd2xx_setConfigFilePath(const char * path)
{
    struct stat buffer;
    strncpy(FakeFtd2xx_configFilePath, path, 255);
    return stat(FakeFtd2xx_configFilePath, &buffer);
}

int FakeFtd2xx_numberOfDevices()
{
    return fakeDeviceListCount;
}

void FakeFtd2xx_destroy()
{
    free(fakeDeviceList);
}

static void writeToSpyFile(int deviceIndex, char * message)
{
    FILE * handler = fopen(fakeDeviceList[deviceIndex].spyFilePath, "a");
    if(handler)
        fwrite(message, strlen(message), 1, handler);
    fclose(handler);
}

/*
* To be used like printf, but prefixes the output
*/
void printLog(char *fmt, ...)
{
    va_list ap; /* points to each unnamed arg in turn */
    va_start(ap, fmt);
    printf("__DeviceLog: ");
    vprintf(fmt, ap);
    va_end(ap); /* clean up when done */
}

static void scheduleResponse(char * buffer, int deviceIndex)
{
    const char * response = FakeFtd2xx_responseFromCommand(buffer, deviceIndex);
    if(response) {
        strncpy(fakeDeviceList[deviceIndex].message, response, 256);
        fakeDeviceList[deviceIndex].messageCounter = 0;
    } else {
        printLog("Command not found in list for %s\n", fakeDeviceList[deviceIndex].serialNumber);
    }
}

FT_STATUS FT_Open(int deviceNumber, FT_HANDLE *pHandle)
{
    if (fakeDeviceListCount < 1)
        FakeFtd2xx_createDevices(fakeDeviceList, NUM_OF_FAKE_DEVICES);
    writeToSpyFile(deviceNumber, "FT_Open\n");
    *(long *)pHandle = deviceNumber;
    return FT_OK;
}

FT_STATUS FT_OpenEx(PVOID pArg1, DWORD Flags, FT_HANDLE *pHandle)
{
    if (fakeDeviceListCount < 1) {
        FakeFtd2xx_createDevices(fakeDeviceList, NUM_OF_FAKE_DEVICES);
    }
    printLog("Device requested to openEx\n");
    printLog("Flag: %d\n", Flags);
    switch(Flags) {
        case FT_OPEN_BY_SERIAL_NUMBER:
            printLog("trying to open by serial number %s (%d devices faked)\n", (PCHAR)pArg1, fakeDeviceListCount);
            int i;
            for(i=0; i<fakeDeviceListCount; i++) {
                if (strncmp((PCHAR)pArg1, fakeDeviceList[i].serialNumber, 16) == 0) {
                    printLog("Serial number matches.\n");
                    writeToSpyFile(0, "FT_OpenEX\n");
                    *(long *)pHandle = i;
                    return FT_OK;
                }
            }
            *(long *)pHandle = 99;
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
    // Uncomment for infinetely repeated message
    //if (*lpRxBytes == 0)
    //    fakeDeviceList[deviceIndex].messageCounter = 0;
    //printLog("Queue Status requested for %d (position %d, length %d).\n",
    //    deviceIndex, fakeDeviceList[deviceIndex].messageCounter, fakeDeviceList[deviceIndex].messageCounter);
    sleep(2);
    return FT_OK;
}

FT_STATUS FT_Read(FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD dwBytesToRead, LPDWORD lpBytesReturned)
{
    int deviceIndex = (int)(long *)ftHandle;
    printLog("%d Bytes requested to read. \n", dwBytesToRead);
    memcpy(lpBuffer, fakeDeviceList[deviceIndex].message + fakeDeviceList[deviceIndex].messageCounter, dwBytesToRead);
    *((char *)lpBuffer+dwBytesToRead) = '\0';
    *lpBytesReturned = strlen((char *)lpBuffer);
    fakeDeviceList[deviceIndex].messageCounter += dwBytesToRead;
    return FT_OK;
}

FT_STATUS FT_Write(FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD dwBytesToWrite, LPDWORD lpBytesWritten)
{
    char * outputFmt = "FT_Write(\"%s\")\n";

    char trimmedBuffer[dwBytesToWrite+1];
    strncpy(trimmedBuffer, (char *)lpBuffer, dwBytesToWrite);
    trimmedBuffer[dwBytesToWrite] = '\0';

    char escapedBuffer[dwBytesToWrite*2];
    escapeEscapeSequences(trimmedBuffer, escapedBuffer);

    printLog("length of string: %d of %d\n", strlen(escapedBuffer), dwBytesToWrite);

    char output[strlen(outputFmt)+strlen(escapedBuffer)];
    sprintf(output, outputFmt, escapedBuffer);

    printLog("Received write request: %s %d\n", escapedBuffer, dwBytesToWrite);
    writeToSpyFile((int)(long *)ftHandle, output);
    scheduleResponse(escapedBuffer, (int)(long *)ftHandle);
    *lpBytesWritten = dwBytesToWrite;

    return FT_OK;
}

FT_STATUS FT_Close(FT_HANDLE ftHandle) {
    printLog("Close requested for %d.\n", (long *)ftHandle);
    if((int)(long *)ftHandle > fakeDeviceListCount-1) {
        return FT_INVALID_HANDLE;
    } else {
        writeToSpyFile((int)(long *)ftHandle, "FT_Close\n");
        return FT_OK;
    }
}

FT_STATUS FT_Purge(FT_HANDLE ftHandle, DWORD dwMask) {
    printLog("RX and TX Buffer Purge requested.\n");
    return FT_OK;
}

FT_STATUS FT_CreateDeviceInfoList(LPDWORD lpdwNumDevs)
{
    *lpdwNumDevs = 0;
    FT_STATUS status;

    char json[10240];
    FakeFtd2xx_readConfigFile(json);
    status = FakeFtd2xx_parseConfigFile(json, fakeDeviceList, NUM_OF_FAKE_DEVICES);

    if(status)
        return 99;
    int i;
    for(i=0; i < NUM_OF_FAKE_DEVICES; i++) {
        if(strlen(fakeDeviceList[i].serialNumber) == 0)
            break;
        *lpdwNumDevs = i+1;
    }
    fakeDeviceListCount = *lpdwNumDevs;

    printLog("FT_CreateDeviceInfoList requested, returning %d device(s).\n", *lpdwNumDevs);
    return FT_OK;
}

FT_STATUS FT_GetDeviceInfoList(FT_DEVICE_LIST_INFO_NODE *pDest, LPDWORD lpdwNumDevs)
{
    *lpdwNumDevs = 0;
    int i;
    for(i=0; i < fakeDeviceListCount; i++) {
        long id = (fakeDeviceList[i].vendorId << 16) + fakeDeviceList[i].productId;
        FT_HANDLE ftHandle = malloc(sizeof(FT_HANDLE));

        FT_DEVICE_LIST_INFO_NODE device = {
            0, //ULONG Flags;
            0, //ULONG Type;
            id, //ULONG ID;
            fakeDeviceList[i].locationId, //DWORD LocId;
            "", //char SerialNumber[16];
            "", //char Description[64];
            ftHandle
        };
        strncpy(device.SerialNumber, fakeDeviceList[i].serialNumber, 16);
        strncpy(device.Description, fakeDeviceList[i].description, 64);
        pDest[i] = device;
        *lpdwNumDevs += 1;
    }
    printLog("FT_GetDeviceInfoList requested, returning %d node(s)\n", *lpdwNumDevs);
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


// Helper functions

void escapeEscapeSequences(char *string, char *result)
{
    char * p;
    *result = '\0';
    for (p = string; *p; p++) {
        switch(*p) {
           case '\n':
                *result++ = '\\';
                *result = 'n';
                break;
            case '\r':
                *result++ = '\\';
                *result = 'r';
                break;
            case '\t':
                *result++ = '\\';
                *result = 't';
                break;
            default:
                *result = *p;
        }
        result++;
    }
    *result = '\0';
}