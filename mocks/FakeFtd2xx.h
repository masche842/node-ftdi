typedef struct {
    char message[65536];
    int messageCounter;
    char serialNumber[16];
    char description[64];
    long vendorId;
    long productId;
    long locationId;
    char spyFilePath[255];
} FakeDevice;

void printLog(char *fmt, ...);
void escapeEscapeSequences(char *string, char *result);

void FakeFtd2xx_setConfigFilePath(const char *);
int FakeFtd2xx_readConfigFile(char * result);
int FakeFtd2xx_parseConfigFile(char * json, FakeDevice * devices, int numberOfDevices);
int FakeFtd2xx_createDevices(FakeDevice * devices, int numberOfDevices);
void FakeFtd2xx_destroy();
