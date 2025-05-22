#include "printer_manager.h"

#include <windows.h>
#include <winspool.h>
#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <sstream>
#include <string>

// Helper function to convert wide string to UTF-8
std::string PrinterManager::WideToUtf8(const wchar_t* wide_str) {
  if (wide_str == nullptr) return "";
  
  int size_needed = WideCharToMultiByte(CP_UTF8, 0, wide_str, -1, nullptr, 0, nullptr, nullptr);
  std::string utf8_str(size_needed, 0);
  WideCharToMultiByte(CP_UTF8, 0, wide_str, -1, &utf8_str[0], size_needed, nullptr, nullptr);
  
  // Remove null terminator
  if (!utf8_str.empty() && utf8_str.back() == 0) {
    utf8_str.pop_back();
  }
  return utf8_str;
}

// Helper function to convert UTF-8 to wide string
std::wstring PrinterManager::Utf8ToWide(const std::string& utf8_str) {
  if (utf8_str.empty()) return L"";
  
  int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, nullptr, 0);
  std::wstring wide_str(size_needed, 0);
  MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, &wide_str[0], size_needed);
  
  // Remove null terminator
  if (!wide_str.empty() && wide_str.back() == 0) {
    wide_str.pop_back();
  }
  return wide_str;
}

// GetAvailablePrinters Implementation
flutter::EncodableList PrinterManager::GetAvailablePrinters() {
  flutter::EncodableList printerList;
  
  DWORD needed = 0, returned = 0;
  EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 4, NULL, 0, &needed, &returned);
  
  if (needed > 0) {
    std::vector<BYTE> buffer(needed);
    PRINTER_INFO_4* printerInfo = reinterpret_cast<PRINTER_INFO_4*>(buffer.data());
    
    if (EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 4, buffer.data(), needed, &needed, &returned)) {
      for (DWORD i = 0; i < returned; i++) {
        std::string printerName = WideToUtf8(printerInfo[i].pPrinterName);
        printerList.push_back(flutter::EncodableValue(printerName));
      }
    }
  }
  
  return printerList;
}

// GetPrinterProperties implementation
flutter::EncodableMap PrinterManager::GetPrinterProperties(const std::string& printerName) {
  flutter::EncodableMap properties;
  HANDLE hPrinter = NULL;
  PRINTER_INFO_2* pPrinterInfo = NULL;
  DWORD needed = 0;
  
  std::wstring widePrinterName = Utf8ToWide(printerName);
  
  // Check if this is the default printer
  WCHAR defaultPrinterName[256] = {0};
  DWORD defaultPrinterSize = sizeof(defaultPrinterName) / sizeof(WCHAR);
  BOOL isDefault = FALSE;
  
  if (GetDefaultPrinter(defaultPrinterName, &defaultPrinterSize)) {
    isDefault = (wcscmp(widePrinterName.c_str(), defaultPrinterName) == 0);
  }
  
  properties[flutter::EncodableValue("isDefault")] = flutter::EncodableValue(isDefault ? true : false);
  
  // Open printer
  if (!OpenPrinter(const_cast<LPWSTR>(widePrinterName.c_str()), &hPrinter, NULL)) {
    // Error info if we can't open the printer
    properties[flutter::EncodableValue("error")] = flutter::EncodableValue("Failed to open printer");
    return properties;
  }
  
  // Get the buffer size needed
  GetPrinter(hPrinter, 2, NULL, 0, &needed);
  if (needed > 0) {
    try {
      std::vector<BYTE> buffer(needed);
      pPrinterInfo = reinterpret_cast<PRINTER_INFO_2*>(buffer.data());
      
      // Get the printer information
      if (GetPrinter(hPrinter, 2, buffer.data(), needed, &needed)) {
        // Basic properties
        if (pPrinterInfo->pPrinterName)
          properties[flutter::EncodableValue("name")] = flutter::EncodableValue(WideToUtf8(pPrinterInfo->pPrinterName));
        
        if (pPrinterInfo->pServerName)
          properties[flutter::EncodableValue("serverName")] = flutter::EncodableValue(WideToUtf8(pPrinterInfo->pServerName));
        
        if (pPrinterInfo->pShareName)
          properties[flutter::EncodableValue("shareName")] = flutter::EncodableValue(WideToUtf8(pPrinterInfo->pShareName));
        
        if (pPrinterInfo->pPortName)
          properties[flutter::EncodableValue("portName")] = flutter::EncodableValue(WideToUtf8(pPrinterInfo->pPortName));
        
        if (pPrinterInfo->pDriverName)
          properties[flutter::EncodableValue("driverName")] = flutter::EncodableValue(WideToUtf8(pPrinterInfo->pDriverName));
        
        if (pPrinterInfo->pComment)
          properties[flutter::EncodableValue("comment")] = flutter::EncodableValue(WideToUtf8(pPrinterInfo->pComment));
        
        if (pPrinterInfo->pLocation)
          properties[flutter::EncodableValue("location")] = flutter::EncodableValue(WideToUtf8(pPrinterInfo->pLocation));
        
        if (pPrinterInfo->pSepFile)
          properties[flutter::EncodableValue("separatorFile")] = flutter::EncodableValue(WideToUtf8(pPrinterInfo->pSepFile));
        
        if (pPrinterInfo->pPrintProcessor)
          properties[flutter::EncodableValue("printProcessor")] = flutter::EncodableValue(WideToUtf8(pPrinterInfo->pPrintProcessor));
        
        if (pPrinterInfo->pDatatype)
          properties[flutter::EncodableValue("datatype")] = flutter::EncodableValue(WideToUtf8(pPrinterInfo->pDatatype));
        
        // Status properties
        properties[flutter::EncodableValue("status")] = flutter::EncodableValue(static_cast<int>(pPrinterInfo->Status));
        
        // Decode the status to human-readable formats
        flutter::EncodableList statusMessages;
        if (pPrinterInfo->Status == 0) {
          statusMessages.push_back(flutter::EncodableValue("Ready"));
        } else {
          if (pPrinterInfo->Status & PRINTER_STATUS_PAUSED)
            statusMessages.push_back(flutter::EncodableValue("Paused"));
          if (pPrinterInfo->Status & PRINTER_STATUS_ERROR)
            statusMessages.push_back(flutter::EncodableValue("Error"));
          if (pPrinterInfo->Status & PRINTER_STATUS_PENDING_DELETION)
            statusMessages.push_back(flutter::EncodableValue("Pending Deletion"));
          if (pPrinterInfo->Status & PRINTER_STATUS_PAPER_JAM)
            statusMessages.push_back(flutter::EncodableValue("Paper Jam"));
          if (pPrinterInfo->Status & PRINTER_STATUS_PAPER_OUT)
            statusMessages.push_back(flutter::EncodableValue("Paper Out"));
          if (pPrinterInfo->Status & PRINTER_STATUS_MANUAL_FEED)
            statusMessages.push_back(flutter::EncodableValue("Manual Feed"));
          if (pPrinterInfo->Status & PRINTER_STATUS_PAPER_PROBLEM)
            statusMessages.push_back(flutter::EncodableValue("Paper Problem"));
          if (pPrinterInfo->Status & PRINTER_STATUS_OFFLINE)
            statusMessages.push_back(flutter::EncodableValue("Offline"));
          if (pPrinterInfo->Status & PRINTER_STATUS_IO_ACTIVE)
            statusMessages.push_back(flutter::EncodableValue("I/O Active"));
          if (pPrinterInfo->Status & PRINTER_STATUS_BUSY)
            statusMessages.push_back(flutter::EncodableValue("Busy"));
          if (pPrinterInfo->Status & PRINTER_STATUS_PRINTING)
            statusMessages.push_back(flutter::EncodableValue("Printing"));
          if (pPrinterInfo->Status & PRINTER_STATUS_OUTPUT_BIN_FULL)
            statusMessages.push_back(flutter::EncodableValue("Output Bin Full"));
          if (pPrinterInfo->Status & PRINTER_STATUS_NOT_AVAILABLE)
            statusMessages.push_back(flutter::EncodableValue("Not Available"));
          if (pPrinterInfo->Status & PRINTER_STATUS_WAITING)
            statusMessages.push_back(flutter::EncodableValue("Waiting"));
          if (pPrinterInfo->Status & PRINTER_STATUS_PROCESSING)
            statusMessages.push_back(flutter::EncodableValue("Processing"));
          if (pPrinterInfo->Status & PRINTER_STATUS_INITIALIZING)
            statusMessages.push_back(flutter::EncodableValue("Initializing"));
          if (pPrinterInfo->Status & PRINTER_STATUS_WARMING_UP)
            statusMessages.push_back(flutter::EncodableValue("Warming Up"));
          if (pPrinterInfo->Status & PRINTER_STATUS_TONER_LOW)
            statusMessages.push_back(flutter::EncodableValue("Toner Low"));
          if (pPrinterInfo->Status & PRINTER_STATUS_NO_TONER)
            statusMessages.push_back(flutter::EncodableValue("No Toner"));
          if (pPrinterInfo->Status & PRINTER_STATUS_PAGE_PUNT)
            statusMessages.push_back(flutter::EncodableValue("Page Punt"));
          if (pPrinterInfo->Status & PRINTER_STATUS_USER_INTERVENTION)
            statusMessages.push_back(flutter::EncodableValue("User Intervention Required"));
          if (pPrinterInfo->Status & PRINTER_STATUS_OUT_OF_MEMORY)
            statusMessages.push_back(flutter::EncodableValue("Out of Memory"));
          if (pPrinterInfo->Status & PRINTER_STATUS_DOOR_OPEN)
            statusMessages.push_back(flutter::EncodableValue("Door Open"));
          if (pPrinterInfo->Status & PRINTER_STATUS_SERVER_UNKNOWN)
            statusMessages.push_back(flutter::EncodableValue("Server Unknown"));
          if (pPrinterInfo->Status & PRINTER_STATUS_POWER_SAVE)
            statusMessages.push_back(flutter::EncodableValue("Power Save"));
        }
        properties[flutter::EncodableValue("statusMessages")] = flutter::EncodableValue(statusMessages);
        
        // Numerical properties
        properties[flutter::EncodableValue("attributes")] = flutter::EncodableValue(static_cast<int>(pPrinterInfo->Attributes));
        properties[flutter::EncodableValue("priority")] = flutter::EncodableValue(static_cast<int>(pPrinterInfo->Priority));
        properties[flutter::EncodableValue("defaultPriority")] = flutter::EncodableValue(static_cast<int>(pPrinterInfo->DefaultPriority));
        properties[flutter::EncodableValue("startTime")] = flutter::EncodableValue(static_cast<int>(pPrinterInfo->StartTime));
        properties[flutter::EncodableValue("untilTime")] = flutter::EncodableValue(static_cast<int>(pPrinterInfo->UntilTime));
        properties[flutter::EncodableValue("jobs")] = flutter::EncodableValue(static_cast<int>(pPrinterInfo->cJobs));
        properties[flutter::EncodableValue("averagePPM")] = flutter::EncodableValue(static_cast<int>(pPrinterInfo->AveragePPM));
        
        // Decode printer attributes to readable format
        flutter::EncodableList attributesList;
        if (pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_QUEUED)
          attributesList.push_back(flutter::EncodableValue("Queued"));
        if (pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_DIRECT)
          attributesList.push_back(flutter::EncodableValue("Direct"));
        if (pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_DEFAULT)
          attributesList.push_back(flutter::EncodableValue("Default"));
        if (pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_SHARED)
          attributesList.push_back(flutter::EncodableValue("Shared"));
        if (pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_NETWORK)
          attributesList.push_back(flutter::EncodableValue("Network"));
        if (pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_HIDDEN)
          attributesList.push_back(flutter::EncodableValue("Hidden"));
        if (pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_LOCAL)
          attributesList.push_back(flutter::EncodableValue("Local"));
        if (pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_ENABLE_DEVQ)
          attributesList.push_back(flutter::EncodableValue("Enable DevQ"));
        if (pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS)
          attributesList.push_back(flutter::EncodableValue("Keep Printed Jobs"));
        if (pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST)
          attributesList.push_back(flutter::EncodableValue("Do Complete First"));
        if (pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE)
          attributesList.push_back(flutter::EncodableValue("Work Offline"));
        if (pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_ENABLE_BIDI)
          attributesList.push_back(flutter::EncodableValue("Enable BiDi"));
        if (pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_RAW_ONLY)
          attributesList.push_back(flutter::EncodableValue("Raw Only"));
        if (pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_PUBLISHED)
          attributesList.push_back(flutter::EncodableValue("Published"));
        properties[flutter::EncodableValue("attributesList")] = flutter::EncodableValue(attributesList);
        
        // Get additional information about printer capabilities
        DWORD caps_size = 0;
        DeviceCapabilities(pPrinterInfo->pPrinterName, pPrinterInfo->pPortName, DC_PAPERNAMES, NULL, NULL);
        caps_size = DeviceCapabilities(pPrinterInfo->pPrinterName, pPrinterInfo->pPortName, DC_PAPERSIZE, NULL, NULL);
        
        // Get paper sizes supported by printer
        if (caps_size > 0) {
          flutter::EncodableList paperSizes;
          std::vector<WCHAR> paperNames(64 * caps_size); // Each paper name can be up to 64 characters
          
          if (DeviceCapabilities(pPrinterInfo->pPrinterName, pPrinterInfo->pPortName, DC_PAPERNAMES, 
                                (LPWSTR)paperNames.data(), NULL) > 0) {
            for (DWORD i = 0; i < caps_size; i++) {
              std::wstring paperName(&paperNames[i * 64], 64);
              size_t nullPos = paperName.find(L'\0');
              if (nullPos != std::wstring::npos) {
                paperName = paperName.substr(0, nullPos);
              }
              paperSizes.push_back(flutter::EncodableValue(WideToUtf8(paperName.c_str())));
            }
            properties[flutter::EncodableValue("paperSizes")] = flutter::EncodableValue(paperSizes);
          }
        }
        
        // Get supported resolutions
        caps_size = DeviceCapabilities(pPrinterInfo->pPrinterName, pPrinterInfo->pPortName, DC_ENUMRESOLUTIONS, NULL, NULL);
        if (caps_size > 0) {
          flutter::EncodableList resolutions;
          std::vector<LONG> resolutionData(caps_size * 2); // Each resolution has X and Y values
          
          if (DeviceCapabilities(pPrinterInfo->pPrinterName, pPrinterInfo->pPortName, DC_ENUMRESOLUTIONS, 
                                (LPWSTR)resolutionData.data(), NULL) > 0) {
            for (DWORD i = 0; i < caps_size; i++) {
              flutter::EncodableMap resolution;
              resolution[flutter::EncodableValue("x")] = flutter::EncodableValue(static_cast<int>(resolutionData[i * 2]));
              resolution[flutter::EncodableValue("y")] = flutter::EncodableValue(static_cast<int>(resolutionData[i * 2 + 1]));
              resolutions.push_back(flutter::EncodableValue(resolution));
            }
            properties[flutter::EncodableValue("resolutions")] = flutter::EncodableValue(resolutions);
          }
        }
      }
    } catch (const std::exception&) {
      // Handle any exceptions that might occur
      properties[flutter::EncodableValue("error")] = flutter::EncodableValue("Exception while getting printer information");
    }
  }
  
  // Close the printer handle
  if (hPrinter) {
    ClosePrinter(hPrinter);
  }
  
  return properties;
}

// AssignDefaultPrinter Implementation
bool PrinterManager::AssignDefaultPrinter(const std::string& printerName) {
  std::wstring widePrinterName = Utf8ToWide(printerName);
  return SetDefaultPrinter(widePrinterName.c_str()) ? true : false;
}

// OpenPrinterProperties Implementation
bool PrinterManager::OpenPrinterProperties(const std::string& printerName) {
  std::wstring widePrinterName = Utf8ToWide(printerName);
  
  // Command to open the printer properties
  std::wstring command = L"rundll32.exe printui.dll,PrintUIEntry /p /n\"" + widePrinterName + L"\"";
  
  // Execute cmd
  HINSTANCE hInstance = ShellExecuteW(
    NULL,
    L"open",
    L"rundll32.exe",
    (L"printui.dll,PrintUIEntry /p /n\"" + widePrinterName + L"\"").c_str(),
    NULL,
    SW_SHOWNORMAL
  );
  
  // Check if the command was executed successfully
  return ((INT_PTR)hInstance > 32);
}

// GetPaperSizeDetails implementation
flutter::EncodableMap PrinterManager::GetPaperSizeDetails(const std::string& printerName) {
  flutter::EncodableMap paperDetails;
  std::wstring widePrinterName = Utf8ToWide(printerName);
  
  // Open printer
  HANDLE hPrinter = NULL;
  if (!OpenPrinter(const_cast<LPWSTR>(widePrinterName.c_str()), &hPrinter, NULL)) {
    paperDetails[flutter::EncodableValue("error")] = flutter::EncodableValue("Failed to open printer");
    return paperDetails;
  }
  
  // Get device mode to get current paper size
  LONG devModeSize = DocumentProperties(
    NULL, 
    hPrinter, 
    const_cast<LPWSTR>(widePrinterName.c_str()), 
    NULL, 
    NULL, 
    0
  );
  
  if (devModeSize > 0) {
    DEVMODE* pDevMode = (DEVMODE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, devModeSize);
    if (pDevMode) {
      LONG getResult = DocumentProperties(
        NULL, 
        hPrinter, 
        const_cast<LPWSTR>(widePrinterName.c_str()), 
        pDevMode, 
        NULL, 
        DM_OUT_BUFFER
      );
      
      if (getResult >= 0) {
        // Current paper size information
        if (pDevMode->dmFields & DM_PAPERSIZE) {
          paperDetails[flutter::EncodableValue("currentPaperSize")] = 
            flutter::EncodableValue(static_cast<int>(pDevMode->dmPaperSize));
        }
        
        if (pDevMode->dmFields & DM_PAPERLENGTH) {
          paperDetails[flutter::EncodableValue("currentPaperLength")] = 
            flutter::EncodableValue(static_cast<int>(pDevMode->dmPaperLength));
        }
        
        if (pDevMode->dmFields & DM_PAPERWIDTH) {
          paperDetails[flutter::EncodableValue("currentPaperWidth")] = 
            flutter::EncodableValue(static_cast<int>(pDevMode->dmPaperWidth));
        }
        
        HDC hDC = CreateDC(L"WINSPOOL", widePrinterName.c_str(), NULL, NULL);
        if (hDC) {
          // Get number of available paper sizes
          int numPapers = DeviceCapabilities(
            const_cast<LPWSTR>(widePrinterName.c_str()), 
            NULL, 
            DC_PAPERS, 
            NULL, 
            NULL
          );
          
          if (numPapers > 0) {
            // Allocate memory for paper data
            std::vector<WORD> paperIDs(numPapers);
            std::vector<WCHAR> paperNames(numPapers * 64); // 64 chars max per name
            std::vector<POINT> paperSizes(numPapers);
            
            // Get paper IDs
            DeviceCapabilities(
              const_cast<LPWSTR>(widePrinterName.c_str()), 
              NULL, 
              DC_PAPERS, 
              (LPWSTR)paperIDs.data(), 
              NULL
            );
            
            // Get paper names
            DeviceCapabilities(
              const_cast<LPWSTR>(widePrinterName.c_str()), 
              NULL, 
              DC_PAPERNAMES, 
              (LPWSTR)paperNames.data(), 
              NULL
            );
            
            // Get paper dimensions
            DeviceCapabilities(
              const_cast<LPWSTR>(widePrinterName.c_str()), 
              NULL, 
              DC_PAPERSIZE, 
              (LPWSTR)paperSizes.data(), 
              NULL
            );
            
            // Create a list of available paper sizes with details
            flutter::EncodableList availablePapers;
            for (int i = 0; i < numPapers; i++) {
              // Get the paper name
              std::wstring paperName(&paperNames[i * 64], 64);
              size_t nullPos = paperName.find(L'\0');
              if (nullPos != std::wstring::npos) {
                paperName = paperName.substr(0, nullPos);
              }
              
              // Create a map for this paper size
              flutter::EncodableMap paperInfo;
              paperInfo[flutter::EncodableValue("id")] = flutter::EncodableValue(static_cast<int>(paperIDs[i]));
              paperInfo[flutter::EncodableValue("name")] = flutter::EncodableValue(WideToUtf8(paperName.c_str()));
              
              // Add width and height in 1/10 mm units
              if (i < static_cast<int>(paperSizes.size())) {
                paperInfo[flutter::EncodableValue("width")] = flutter::EncodableValue(static_cast<int>(paperSizes[i].x));
                paperInfo[flutter::EncodableValue("height")] = flutter::EncodableValue(static_cast<int>(paperSizes[i].y));
              }
              
              // Check if this is the current paper size
              if (pDevMode->dmFields & DM_PAPERSIZE && paperIDs[i] == pDevMode->dmPaperSize) {
                paperInfo[flutter::EncodableValue("isSelected")] = flutter::EncodableValue(true);
                // Add the current paper name to the top level
                paperDetails[flutter::EncodableValue("currentPaperName")] = 
                  flutter::EncodableValue(WideToUtf8(paperName.c_str()));
              } else {
                paperInfo[flutter::EncodableValue("isSelected")] = flutter::EncodableValue(false);
              }
              
              availablePapers.push_back(flutter::EncodableValue(paperInfo));
            }
            
            // Add the list to the result
            paperDetails[flutter::EncodableValue("availablePaperSizes")] = 
              flutter::EncodableValue(availablePapers);
          }
          
          // Delete the device context
          DeleteDC(hDC);
        }
      }
      
      // Free device mode memory
      HeapFree(GetProcessHeap(), 0, pDevMode);
    }
  }
  
  // Close printer
  ClosePrinter(hPrinter);
  
  return paperDetails;
}

// PrintRawData implementation
bool PrinterManager::PrintRawData(const std::string& printerName, 
                                 const std::vector<uint8_t>& data, 
                                 bool useRawDatatype) {
  std::string actualPrinterName = printerName;
  
  // If no printer name provided, use the default printer
  if (actualPrinterName.empty()) {
    WCHAR defaultPrinterName[256] = {0};
    DWORD defaultPrinterSize = sizeof(defaultPrinterName) / sizeof(WCHAR);
    if (GetDefaultPrinter(defaultPrinterName, &defaultPrinterSize)) {
      actualPrinterName = WideToUtf8(defaultPrinterName);
    } else {
      return false; // No default printer found
    }
  }
  
  std::wstring widePrinterName = Utf8ToWide(actualPrinterName);
  
  // Open printer
  HANDLE hPrinter = NULL;
  if (!OpenPrinter(const_cast<LPWSTR>(widePrinterName.c_str()), &hPrinter, NULL)) {
    return false;
  }
  
  // Get printer information to determine if it's a raw-capable printer
  DWORD needed = 0;
  GetPrinter(hPrinter, 2, NULL, 0, &needed);
  
  // Start a print job with user-controlled data typ
  DOC_INFO_1 docInfo = {0};
  docInfo.pDocName = L"Raw Print Job";
  docInfo.pOutputFile = NULL;
  docInfo.pDatatype = useRawDatatype ? L"RAW" : L"TEXT";
  
  DWORD jobId = StartDocPrinter(hPrinter, 1, (LPBYTE)&docInfo);
  if (jobId == 0) {
    ClosePrinter(hPrinter);
    return false;
  }
  
  // Start a page
  if (!StartPagePrinter(hPrinter)) {
    EndDocPrinter(hPrinter);
    ClosePrinter(hPrinter);
    return false;
  }
  
  // Write the data to the printer
  DWORD bytesWritten = 0;
  bool success = WritePrinter(hPrinter, const_cast<BYTE*>(data.data()), static_cast<DWORD>(data.size()), &bytesWritten);
  
  // End the page and document
  EndPagePrinter(hPrinter);
  EndDocPrinter(hPrinter);
  ClosePrinter(hPrinter);
  
  return (success && bytesWritten == data.size());
}

// PrintPdf implementation
bool PrinterManager::PrintPdf(const std::string& printerName, const std::vector<uint8_t>& data, int copies) {
  std::string actualPrinterName = printerName;
  
  // If no printer name provided, use the default printer
  if (actualPrinterName.empty()) {
    WCHAR defaultPrinterName[256] = {0};
    DWORD defaultPrinterSize = sizeof(defaultPrinterName) / sizeof(WCHAR);
    if (GetDefaultPrinter(defaultPrinterName, &defaultPrinterSize)) {
      actualPrinterName = WideToUtf8(defaultPrinterName);
    } else {
      return false; // No default printer found
    }
  }
  
  // For PDF printing, we need to save it to a temporary file
  // and use ShellExecute to print it silently
  char tempPath[MAX_PATH];
  char tempFileName[MAX_PATH];
  
  GetTempPathA(MAX_PATH, tempPath);
  GetTempFileNameA(tempPath, "pdf", 0, tempFileName);
  
  // Create a temporary file with .pdf extension (for proper handling)
  std::string tempPdfFile = std::string(tempFileName) + ".pdf";
  
  // Write PDF data to the temporary file
  std::ofstream outFile(tempPdfFile, std::ios::binary);
  if (!outFile) {
    return false;
  }
  
  outFile.write(reinterpret_cast<const char*>(data.data()), data.size());
  outFile.close();
  
  std::wstring widePrinterName = Utf8ToWide(actualPrinterName);
  std::wstring wideTempFile = Utf8ToWide(tempPdfFile);
  
  // Command to print the PDF silently
  std::wstring command = L"rundll32.exe mshtml.dll,PrintHTML \"" + wideTempFile + L"\" \"" + widePrinterName + L"\"";
  
  // Execute cmd
  STARTUPINFOW si = {0};
  si.cb = sizeof(STARTUPINFOW);
  si.dwFlags = STARTF_USESHOWWINDOW;
  si.wShowWindow = SW_HIDE; // Hide the window
  
  PROCESS_INFORMATION pi = {0};
  
  bool success = CreateProcessW(
      NULL,                          // No module name (use command line)
      const_cast<LPWSTR>(command.c_str()), // Command line
      NULL,                          // Process handle not inheritable
      NULL,                          // Thread handle not inheritable
      FALSE,                         // Set handle inheritance to FALSE
      0,                             // No creation flags
      NULL,                          // Use parent's environment block
      NULL,                          // Use parent's starting directory 
      &si,                           // Pointer to STARTUPINFO structure
      &pi                            // Pointer to PROCESS_INFORMATION structure
  );
  
  if (success) {
    // Wait for the process to finish (5 seconds max)
    WaitForSingleObject(pi.hProcess, 5000);
    
    // Close handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    // Delete the temporary file (or try to)
    try {
      std::remove(tempPdfFile.c_str());
    } catch (...) {
      // Ignore errors deleting temp file
    }
    
    return true;
  } else {
    // Alternative approach if the first one failed
    HINSTANCE hInstance = ShellExecuteW(
      NULL,
      L"print",
      wideTempFile.c_str(),
      NULL,
      NULL,
      SW_HIDE
    );
    
    bool altSuccess = ((INT_PTR)hInstance > 32);
    
    // Try to delete the temp file even if printing failed
    try {
      std::remove(tempPdfFile.c_str());
    } catch (...) {
      // Ignore errors deleting temp file
    }
    
    return altSuccess;
  }
}

// PrintRichTextDocument Implementation
bool PrinterManager::PrintRichTextDocument(const std::string& printerName, 
                                          const std::string& richTextContent,
                                          const std::string& defaultFontName,
                                          int defaultFontSize) {
  std::string actualPrinterName = printerName;
  
  if (actualPrinterName.empty()) {
    WCHAR defaultPrinterName[256] = {0};
    DWORD defaultPrinterSize = sizeof(defaultPrinterName) / sizeof(WCHAR);
    if (GetDefaultPrinter(defaultPrinterName, &defaultPrinterSize)) {
      actualPrinterName = WideToUtf8(defaultPrinterName);
    } else {
      return false;
    }
  }
  
  std::wstring widePrinterName = Utf8ToWide(actualPrinterName);
  
  // Use GDI for better font control
  HDC hDC = CreateDC(L"WINSPOOL", widePrinterName.c_str(), NULL, NULL);
  if (!hDC) return false;
  
  DOCINFO di = {0};
  di.cbSize = sizeof(DOCINFO);
  di.lpszDocName = L"Rich Text Document";
  
  if (StartDoc(hDC, &di) <= 0) {
    DeleteDC(hDC);
    return false;
  }
  
  if (StartPage(hDC) <= 0) {
    EndDoc(hDC);
    DeleteDC(hDC);
    return false;
  }
  
  // Create different fonts
  std::wstring wideFontName = Utf8ToWide(defaultFontName);
  
  HFONT normalFont = CreateFont(
    -MulDiv(defaultFontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72),
    0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
    ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_MODERN, wideFontName.c_str());
    
  HFONT boldFont = CreateFont(
    -MulDiv(defaultFontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72),
    0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
    ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_MODERN, wideFontName.c_str());
    
  HFONT italicFont = CreateFont(
    -MulDiv(defaultFontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72),
    0, 0, 0, FW_NORMAL, TRUE, FALSE, FALSE,
    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
    ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_MODERN, wideFontName.c_str());
    
  HFONT boldItalicFont = CreateFont(
    -MulDiv(defaultFontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72),
    0, 0, 0, FW_BOLD, TRUE, FALSE, FALSE,
    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
    ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_MODERN, wideFontName.c_str());
    
  HFONT largeFont = CreateFont(
    -MulDiv(defaultFontSize + 6, GetDeviceCaps(hDC, LOGPIXELSY), 72),
    0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
    ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_MODERN, wideFontName.c_str());

  SetTextColor(hDC, RGB(0, 0, 0));
  SetBkMode(hDC, TRANSPARENT);
  
  int yPos = 50;
  int baseLineHeight = MulDiv(defaultFontSize + 2, GetDeviceCaps(hDC, LOGPIXELSY), 72);
  
  // parse rich text content 
  std::stringstream ss(richTextContent);
  std::string line;
  
  while (std::getline(ss, line)) {
    int currentLineHeight = baseLineHeight;
    ParseAndPrintLine(hDC, line, 50, yPos, normalFont, boldFont, italicFont, 
                     boldItalicFont, largeFont, &currentLineHeight);
    yPos += currentLineHeight;
  }
  
  DeleteObject(normalFont);
  DeleteObject(boldFont);
  DeleteObject(italicFont);
  DeleteObject(boldItalicFont);
  DeleteObject(largeFont);
  
  EndPage(hDC);
  EndDoc(hDC);
  DeleteDC(hDC);
  
  return true;
}

// ParseAndPrintLine Implementation
void PrinterManager::ParseAndPrintLine(HDC hDC, const std::string& line, int x, int y,
                                       HFONT normalFont, HFONT boldFont, HFONT italicFont, 
                                       HFONT boldItalicFont, HFONT largeFont, int* lineHeight) {
  int currentX = x;
  std::string currentText = "";
  HFONT currentFont = normalFont;
  bool isBold = false;
  bool isItalic = false;
  bool isLarge = false;
  int maxHeight = *lineHeight;
  
  for (size_t i = 0; i < line.length(); ++i) {
    // Check for bold markers (**)
    if (i + 1 < line.length() && line.substr(i, 2) == "**") {
      // Print current text before changing format
      if (!currentText.empty()) {
        SelectObject(hDC, currentFont);
        std::wstring wideText = Utf8ToWide(currentText);
        TextOut(hDC, currentX, y, wideText.c_str(), static_cast<int>(wideText.length()));
        
        SIZE textSize;
        GetTextExtentPoint32(hDC, wideText.c_str(), static_cast<int>(wideText.length()), &textSize);
        currentX += textSize.cx;
        currentText = "";
      }
      
      // Toggle bold
      isBold = !isBold;
      
      // Update font based on current state
      if (isLarge) {
        currentFont = largeFont; // Large font is always bold
      } else if (isBold && isItalic) {
        currentFont = boldItalicFont;
      } else if (isBold) {
        currentFont = boldFont;
      } else if (isItalic) {
        currentFont = italicFont;
      } else {
        currentFont = normalFont;
      }
      
      i++; // Skip second *
      continue;
    }
    
    // Check for italic markers (*)
    if (line.substr(i, 1) == "*" && (i == 0 || line[i-1] != '*') && 
        (i + 1 >= line.length() || line[i+1] != '*')) {
      // Print current text before changing format
      if (!currentText.empty()) {
        SelectObject(hDC, currentFont);
        std::wstring wideText = Utf8ToWide(currentText);
        TextOut(hDC, currentX, y, wideText.c_str(), static_cast<int>(wideText.length()));
        
        SIZE textSize;
        GetTextExtentPoint32(hDC, wideText.c_str(), static_cast<int>(wideText.length()), &textSize);
        currentX += textSize.cx;
        currentText = "";
      }
      
      // Toggle italic
      isItalic = !isItalic;
      
      // Update font based on current state
      if (isLarge) {
        currentFont = largeFont; // Large font is always bold
      } else if (isBold && isItalic) {
        currentFont = boldItalicFont;
      } else if (isBold) {
        currentFont = boldFont;
      } else if (isItalic) {
        currentFont = italicFont;
      } else {
        currentFont = normalFont;
      }
      
      continue;
    }
    
    // Check for large text markers (##)
    if (i + 1 < line.length() && line.substr(i, 2) == "##") {
      // Print current text before changing format
      if (!currentText.empty()) {
        SelectObject(hDC, currentFont);
        std::wstring wideText = Utf8ToWide(currentText);
        TextOut(hDC, currentX, y, wideText.c_str(), static_cast<int>(wideText.length()));
        
        SIZE textSize;
        GetTextExtentPoint32(hDC, wideText.c_str(), static_cast<int>(wideText.length()), &textSize);
        currentX += textSize.cx;
        currentText = "";
      }
      
      // Toggle large
      isLarge = !isLarge;
      currentFont = isLarge ? largeFont : (isBold ? boldFont : (isItalic ? italicFont : normalFont));
      
      // Update line height for large text
      if (isLarge) {
        maxHeight = static_cast<int>(maxHeight * 1.5);
      }
      
      i++; // Skip second #
      continue;
    }
    
    // Regular character
    currentText += line[i];
  }
  
  // Print remaining text
  if (!currentText.empty()) {
    SelectObject(hDC, currentFont);
    std::wstring wideText = Utf8ToWide(currentText);
    TextOut(hDC, currentX, y, wideText.c_str(), static_cast<int>(wideText.length()));
  }
  
  *lineHeight = maxHeight;
}