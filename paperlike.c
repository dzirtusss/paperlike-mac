#include <stdio.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/i2c/IOI2CInterface.h>
#include <ApplicationServices/ApplicationServices.h>
#include <IOKit/graphics/IOGraphicsLib.h>

#define VERBOSE true
#define DASUNG_VENDOR_ID 4707

typedef struct SignalData {
  union _SignalDataTypeHead {
    unsigned char value;

    struct _Head {
      unsigned char uSignalDataType : 4;
      unsigned char Reserved : 4;
    } Head;
  } SignalDataTypeHead;

  union _SignalDataType {
    unsigned char value;

    struct _CleanMonitor {
      unsigned char uCleanMonitorSoft : 1;
      unsigned char uCleanMonitorHard : 1;
      unsigned char Reserved : 6;
    } CleanMonitor;
  } SignalDataType;
}__attribute__((packed)) SignalData;

int getDSFramebuffer() {
  if (VERBOSE) printf("--- getDSFramebuffer\n");

  io_iterator_t iter;
  io_service_t serv, foundService = 0;

  kern_return_t err = IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching(IOFRAMEBUFFER_CONFORMSTO), &iter);
  if (err != KERN_SUCCESS) return 0;

  while((serv = IOIteratorNext(iter)) != MACH_PORT_NULL) {
    io_name_t name;

    IORegistryEntryGetName(serv, name);

    CFDictionaryRef info = IODisplayCreateInfoDictionary(serv, kIODisplayOnlyPreferredName);

    CFIndex vendorID = 0;
    CFNumberRef vendorIDRef;
    CFDictionaryGetValueIfPresent(info, CFSTR(kDisplayVendorID), (const void**)&vendorIDRef) &&
      CFNumberGetValue(vendorIDRef, kCFNumberCFIndexType, &vendorID);

    CFRelease(info);

    if (vendorID == DASUNG_VENDOR_ID) { foundService = serv; break; }
  }

  if (!foundService) printf("Can't find Dasung framebuffer\n");
  if (VERBOSE && foundService) printf("Dasung framebuffer: %i\n", foundService);

  return foundService;
}

int sendDDCWrite(int framebuffer, unsigned char data1, unsigned char data2) {
  if (VERBOSE) printf("--- sendDDCWrite\n");

  kern_return_t result;
  IOI2CConnectRef connect;
  IOItemCount busCount;

  result = IOFBGetI2CInterfaceCount(framebuffer, &busCount);
  if (result != KERN_SUCCESS) { printf("No buses"); return 0; }

  for(IOOptionBits bus = 0; bus < busCount; bus++) {
    io_service_t interface;

    result = IOFBCopyI2CInterfaceForBus(framebuffer, bus, &interface);
    if (result != KERN_SUCCESS) { printf("no copy"); return 0; }

    result = IOI2CInterfaceOpen(interface, kNilOptions, &connect);
    if (result != KERN_SUCCESS) { printf("Not opened %i", result); return 0; }
  }

  if (VERBOSE) printf("Opened connection\n");

  IOI2CRequest request;
  UInt8 data[128];
  memset((void *)&data, 0, 128);

  request.commFlags = 0;
  request.sendAddress = 0x6E;
  request.sendTransactionType = kIOI2CSimpleTransactionType;
  request.sendBuffer= (vm_address_t) &data[0];
  request.sendBytes = 7;
  request.replyTransactionType = kIOI2CNoTransactionType;
  request.replyBytes = 0;

  data[0] = 0x51; // magic header
  data[1] = 0x84; // 0x80 + buffer size
  // buffer start
  data[2] = 0x03; // DDCCI_COMMAND_WRITE
  data[3] = 0x08; // VCPCODE
  data[4] = data1;
  data[5] = data2;
  // buffer end
  data[6] = 0x6E ^ data[0] ^ data[1] ^ data[2] ^ data[3] ^ data[4] ^ data[5];

  if (VERBOSE) {
    printf("DDCCI Command (%i):", request.sendBytes);
    for(int i=0; i<request.sendBytes; i++) { printf(" %02x", data[i]); }
    printf("\n");
  }

  result = IOI2CSendRequest(connect, kNilOptions, &request);
  if (result != KERN_SUCCESS) { printf("Send error %i %i\n", request.result, result); }

  IOI2CInterfaceClose(connect, kNilOptions );

  return 0;
};

int sendClean(int framebuffer) {
  if (VERBOSE) printf("--- sendClean\n");

  SignalData SD;
  memset((void *)&SD, 0, sizeof(SignalData));

  SD.SignalDataTypeHead.Head.uSignalDataType = 6; // DS_PACKET_TYPE_CLEANMONITOR;
  SD.SignalDataType.CleanMonitor.uCleanMonitorHard = 1;
  // SD.SignalDataType.CleanMonitor.uCleanMonitorSoft = 1;

  if (VERBOSE) printf("Dasung SignalData (2): %02x %02x\n", SD.SignalDataTypeHead.value, SD.SignalDataType.value);

  return sendDDCWrite(framebuffer, SD.SignalDataTypeHead.value, SD.SignalDataType.value);
}

int main(int argc, char *argv[]) {
  if (argc == 2 && strcmp("clean", argv[1]) == 0) {
    int framebuffer = getDSFramebuffer();
    if (framebuffer) sendClean(framebuffer);
  } else {
    printf("\nPaperlike is alternative command line tool for Dasung Paperlike eInk display ");
    printf("for MacOS (https://github.com/dzirtusss/paperlike-mac). ");
    printf("Usage: 'paperlike clean'\n\n");
  }
  return 0;
}
