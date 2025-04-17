#include <windows.h>
#include <stdio.h>


#define TRANSLATOR_DEVICE 0x8000
#define IOCTL_TRANSLATOR_TRANSLATE_ADDRESS \
    CTL_CODE(TRANSLATOR_DEVICE, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _lINPUT {
    ULONGLONG vAddress;  // Virtual address to translate
} lINPUT, * PlINPUT;

typedef struct _lOUTPUT {
    ULONGLONG pAddress;  // Translated physical address
} lOUTPUT, * PlOUTPUT;

int main() {
    int s = 3;

    HANDLE hDevice = CreateFile(
        L"\\\\.\\Translator",          // Symbolic link name
        GENERIC_READ | GENERIC_WRITE,  // Access rights
        0,                            // No sharing
        NULL,                         // Default security
        OPEN_EXISTING,                // Open existing device
        FILE_ATTRIBUTE_NORMAL,        // Normal file
        NULL                          // No template
    );

    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("Failed to open device. Error: %d\n", GetLastError());
        return 1;
    }

    // Prepare input/output buffers
    lINPUT input = { 0 };
    lOUTPUT output = { 0 };

    printf("Enter virtual address (hex): ");

    input.vAddress = (ULONGLONG)&s;

    DWORD bytesReturned = 0;

    // Send IOCTL request
    BOOL success = DeviceIoControl(
        hDevice,                          // Handle to device
        IOCTL_TRANSLATOR_TRANSLATE_ADDRESS, // IOCTL code
        &input,                           // Input buffer
        sizeof(input),                     // Input buffer size
        &output,                          // Output buffer
        sizeof(output),                    // Output buffer size
        &bytesReturned,                    // Bytes returned
        NULL                              // Overlapped (not used)
    );

    if (success && bytesReturned == sizeof(output)) {
        printf("Physical Address: 0x%llx\n", output.pAddress);
    }
    else {
        printf("IOCTL failed. Error: %d\n", GetLastError());
    }

    CloseHandle(hDevice);
    return 0;
}