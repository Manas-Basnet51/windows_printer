#include "printer_manager.h"

#include <windows.h>
#include <winspool.h>
#include <string>
#include <vector>
#include <optional>
#include <fstream>

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

// Get list of available printers
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

// Get printer properties
flutter::EncodableMap PrinterManager::GetPrinterProperties(const std::string& printerName) {
  flutter::EncodableMap properties;
  HANDLE hPrinter = NULL;
  PRINTER_INFO_2* pPrinterInfo = NULL;
  DWORD needed = 0;
  
  // Convert printer name to wide string
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
    // Return error info if we can't open the printer
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

// Function to get printer device settings
flutter::EncodableMap PrinterManager::GetPrinterDeviceSettings(const std::string& printerName) {
  flutter::EncodableMap deviceSettings;
  HANDLE hPrinter = NULL;
  
  // Convert printer name to wide string
  std::wstring widePrinterName = Utf8ToWide(printerName);
  
  // Open printer
  if (!OpenPrinter(const_cast<LPWSTR>(widePrinterName.c_str()), &hPrinter, NULL)) {
    deviceSettings[flutter::EncodableValue("error")] = flutter::EncodableValue("Failed to open printer");
    return deviceSettings;
  }
  
  // Get the printer information
  DWORD needed = 0;
  GetPrinter(hPrinter, 2, NULL, 0, &needed);
  
  if (needed > 0) {
    std::vector<BYTE> buffer(needed);
    
    if (GetPrinter(hPrinter, 2, buffer.data(), needed, &needed)) {
      // Get the device mode
      LONG result = DocumentProperties(
        NULL, 
        hPrinter, 
        const_cast<LPWSTR>(widePrinterName.c_str()),
        NULL, 
        NULL, 
        0
      );
      
      if (result > 0) {
        // Allocate memory for the DEVMODE structure
        DEVMODE* pDevMode = (DEVMODE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, result);
        
        if (pDevMode) {
          // Get the current device settings
          DocumentProperties(
            NULL, 
            hPrinter, 
            const_cast<LPWSTR>(widePrinterName.c_str()),
            pDevMode, 
            NULL, 
            DM_OUT_BUFFER
          );
          
          // Extract basic device settings
          if (pDevMode->dmFields & DM_PAPERSIZE) {
            deviceSettings[flutter::EncodableValue("paperSize")] = 
              flutter::EncodableValue(static_cast<int>(pDevMode->dmPaperSize));
          }
          
          if (pDevMode->dmFields & DM_PAPERWIDTH) {
            deviceSettings[flutter::EncodableValue("paperWidth")] = 
              flutter::EncodableValue(static_cast<int>(pDevMode->dmPaperWidth));
          }
          
          if (pDevMode->dmFields & DM_PAPERLENGTH) {
            deviceSettings[flutter::EncodableValue("paperLength")] = 
              flutter::EncodableValue(static_cast<int>(pDevMode->dmPaperLength));
          }
          
          if (pDevMode->dmFields & DM_ORIENTATION) {
            deviceSettings[flutter::EncodableValue("orientation")] = 
              flutter::EncodableValue(static_cast<int>(pDevMode->dmOrientation));
            
            // Convert orientation to string
            if (pDevMode->dmOrientation == DMORIENT_PORTRAIT) {
              deviceSettings[flutter::EncodableValue("orientationName")] = 
                flutter::EncodableValue("Portrait");
            } else if (pDevMode->dmOrientation == DMORIENT_LANDSCAPE) {
              deviceSettings[flutter::EncodableValue("orientationName")] = 
                flutter::EncodableValue("Landscape");
            }
          }
          
          if (pDevMode->dmFields & DM_COPIES) {
            deviceSettings[flutter::EncodableValue("copies")] = 
              flutter::EncodableValue(static_cast<int>(pDevMode->dmCopies));
          }
          
          if (pDevMode->dmFields & DM_DEFAULTSOURCE) {
            deviceSettings[flutter::EncodableValue("defaultSource")] = 
              flutter::EncodableValue(static_cast<int>(pDevMode->dmDefaultSource));
            
            // Convert paper source to string
            std::string sourceName = "Unknown";
            switch (pDevMode->dmDefaultSource) {
              case DMBIN_UPPER:
                sourceName = "Upper Tray";
                break;
              case DMBIN_LOWER:
                sourceName = "Lower Tray";
                break;
              case DMBIN_MIDDLE:
                sourceName = "Middle Tray";
                break;
              case DMBIN_MANUAL:
                sourceName = "Manual Feed";
                break;
              case DMBIN_ENVELOPE:
                sourceName = "Envelope Feed";
                break;
              case DMBIN_ENVMANUAL:
                sourceName = "Envelope Manual Feed";
                break;
              case DMBIN_AUTO:
                sourceName = "Auto Select";
                break;
              case DMBIN_TRACTOR:
                sourceName = "Tractor Feed";
                break;
              case DMBIN_SMALLFMT:
                sourceName = "Small Format";
                break;
              case DMBIN_LARGEFMT:
                sourceName = "Large Format";
                break;
              case DMBIN_LARGECAPACITY:
                sourceName = "Large Capacity";
                break;
              case DMBIN_CASSETTE:
                sourceName = "Cassette";
                break;
              case DMBIN_FORMSOURCE:
                sourceName = "Form Source";
                break;
            }
            deviceSettings[flutter::EncodableValue("defaultSourceName")] = 
              flutter::EncodableValue(sourceName);
          }
          
          if (pDevMode->dmFields & DM_PRINTQUALITY) {
            deviceSettings[flutter::EncodableValue("printQuality")] = 
              flutter::EncodableValue(static_cast<int>(pDevMode->dmPrintQuality));
            
            // Convert print quality to string
            std::string qualityName = "Unknown";
            switch (pDevMode->dmPrintQuality) {
              case DMRES_DRAFT:
                qualityName = "Draft";
                break;
              case DMRES_LOW:
                qualityName = "Low";
                break;
              case DMRES_MEDIUM:
                qualityName = "Medium";
                break;
              case DMRES_HIGH:
                qualityName = "High";
                break;
              default:
                if (pDevMode->dmPrintQuality > 0) {
                  qualityName = std::to_string(pDevMode->dmPrintQuality) + " DPI";
                }
            }
            deviceSettings[flutter::EncodableValue("printQualityName")] = 
              flutter::EncodableValue(qualityName);
          }
          
          if (pDevMode->dmFields & DM_DUPLEX) {
            deviceSettings[flutter::EncodableValue("duplex")] = 
              flutter::EncodableValue(static_cast<int>(pDevMode->dmDuplex));
            
            // Convert duplex to string
            std::string duplexName = "Unknown";
            switch (pDevMode->dmDuplex) {
              case DMDUP_SIMPLEX:
                duplexName = "Single-sided";
                break;
              case DMDUP_VERTICAL:
                duplexName = "Duplex (Flip on Long Edge)";
                break;
              case DMDUP_HORIZONTAL:
                duplexName = "Duplex (Flip on Short Edge)";
                break;
            }
            deviceSettings[flutter::EncodableValue("duplexName")] = 
              flutter::EncodableValue(duplexName);
          }
          
          if (pDevMode->dmFields & DM_COLLATE) {
            deviceSettings[flutter::EncodableValue("collate")] = 
              flutter::EncodableValue(static_cast<int>(pDevMode->dmCollate));
            
            // Convert collate to string
            std::string collateName = "Unknown";
            switch (pDevMode->dmCollate) {
              case DMCOLLATE_FALSE:
                collateName = "Uncollated";
                break;
              case DMCOLLATE_TRUE:
                collateName = "Collated";
                break;
            }
            deviceSettings[flutter::EncodableValue("collateName")] = 
              flutter::EncodableValue(collateName);
          }
          
          // Free the allocated memory
          HeapFree(GetProcessHeap(), 0, pDevMode);
        }
      }
    }
  }
  
  // Close the printer handle
  ClosePrinter(hPrinter);
  
  return deviceSettings;
}

// Set default printer
bool PrinterManager::AssignDefaultPrinter(const std::string& printerName) {
  std::wstring widePrinterName = Utf8ToWide(printerName);
  return SetDefaultPrinter(widePrinterName.c_str()) ? true : false;
}

// Open printer properties dialog
bool PrinterManager::OpenPrinterProperties(const std::string& printerName) {
  std::wstring widePrinterName = Utf8ToWide(printerName);
  
  // Build the command to open the printer properties
  std::wstring command = L"rundll32.exe printui.dll,PrintUIEntry /p /n\"" + widePrinterName + L"\"";
  
  // Execute the command
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

// Set printer quality
bool PrinterManager::SetPrinterQuality(const std::string& printerName, int quality) {
  // Convert printer name to wide string
  std::wstring widePrinterName = Utf8ToWide(printerName);
  
  // Open printer
  HANDLE hPrinter = NULL;
  if (!OpenPrinter(const_cast<LPWSTR>(widePrinterName.c_str()), &hPrinter, NULL)) {
    return false;
  }
  
  // Get the current device mode size
  LONG devModeSize = DocumentProperties(
    NULL, 
    hPrinter, 
    const_cast<LPWSTR>(widePrinterName.c_str()), 
    NULL, 
    NULL, 
    0
  );
  
  if (devModeSize <= 0) {
    ClosePrinter(hPrinter);
    return false;
  }
  
  // Allocate memory for the device mode
  DEVMODE* pDevMode = (DEVMODE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, devModeSize);
  if (!pDevMode) {
    ClosePrinter(hPrinter);
    return false;
  }
  
  // Get the current settings
  LONG getResult = DocumentProperties(
    NULL, 
    hPrinter, 
    const_cast<LPWSTR>(widePrinterName.c_str()), 
    pDevMode, 
    NULL, 
    DM_OUT_BUFFER
  );
  
  if (getResult < 0) {
    HeapFree(GetProcessHeap(), 0, pDevMode);
    ClosePrinter(hPrinter);
    return false;
  }
  
  // Modify the quality setting
  pDevMode->dmPrintQuality = static_cast<short>(quality);
  pDevMode->dmFields |= DM_PRINTQUALITY;
  
  // Update the printer settings
  LONG setResult = DocumentProperties(
    NULL, 
    hPrinter, 
    const_cast<LPWSTR>(widePrinterName.c_str()), 
    pDevMode, 
    pDevMode, 
    DM_IN_BUFFER | DM_OUT_BUFFER
  );
  
  // Free resources
  HeapFree(GetProcessHeap(), 0, pDevMode);
  ClosePrinter(hPrinter);
  
  return (setResult >= 0);
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
        // Get current paper size information
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
        
        // For getting paper information, use a device context
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

// SetPaperSize implementation
bool PrinterManager::SetPaperSize(const std::string& printerName, int paperSizeId) {

  // Convert printer name to wide string
  std::wstring widePrinterName = Utf8ToWide(printerName);
  
  // Try multiple methods to set the paper size
  bool success = false;
  
  // Method 1: Use DocumentProperties and SetPrinter together
  HANDLE hPrinter = NULL;
  if (OpenPrinter(const_cast<LPWSTR>(widePrinterName.c_str()), &hPrinter, NULL)) {
    // Get printer info to determine which level to use for SetPrinter
    DWORD needed = 0;
    GetPrinter(hPrinter, 2, NULL, 0, &needed);
    
    if (needed > 0) {
      std::vector<BYTE> buffer(needed);
      PRINTER_INFO_2* pPrinterInfo = reinterpret_cast<PRINTER_INFO_2*>(buffer.data());
      
      if (GetPrinter(hPrinter, 2, buffer.data(), needed, &needed)) {
        // Get the device mode size
        LONG devModeSize = DocumentProperties(
          NULL,
          hPrinter,
          const_cast<LPWSTR>(widePrinterName.c_str()),
          NULL,
          NULL,
          0
        );
        
        if (devModeSize > 0) {
          // Allocate memory for the device mode
          DEVMODE* pDevMode = (DEVMODE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, devModeSize);
          if (pDevMode) {
            // Get the current settings
            LONG getResult = DocumentProperties(
              NULL,
              hPrinter,
              const_cast<LPWSTR>(widePrinterName.c_str()),
              pDevMode,
              NULL,
              DM_OUT_BUFFER
            );
            
            if (getResult >= 0) {
              // Modify the paper size
              pDevMode->dmPaperSize = static_cast<short>(paperSizeId);
              pDevMode->dmFields |= DM_PAPERSIZE;
              
              // Update the settings
              LONG setResult = DocumentProperties(
                NULL,
                hPrinter,
                const_cast<LPWSTR>(widePrinterName.c_str()),
                pDevMode,
                pDevMode,
                DM_IN_BUFFER | DM_OUT_BUFFER
              );
              
              if (setResult >= 0) {
                // Now set this as the default DevMode for the printer
                pPrinterInfo->pDevMode = pDevMode;
                SetPrinter(hPrinter, 2, (LPBYTE)pPrinterInfo, 0);
                success = true;
              }
            }
            
            // Free the device mode memory
            HeapFree(GetProcessHeap(), 0, pDevMode);
          }
        }
      }
    }
    
    // Close the printer handle
    ClosePrinter(hPrinter);
  }
  
  // Method 2: Use the DC approach if method 1 failed
  if (!success) {
    HDC hDC = CreateDC(L"WINSPOOL", widePrinterName.c_str(), NULL, NULL);
    if (hDC) {
      // Get the current DEVMODE
      DEVMODE dm = {0};
      dm.dmSize = sizeof(DEVMODE);
      
      if (EnumForms(hDC, 1, NULL, 0, NULL, NULL)) {
        // Set paper size
        dm.dmPaperSize = static_cast<short>(paperSizeId);
        dm.dmFields = DM_PAPERSIZE;
        
        // Apply the changes
        HDC newDC = ResetDC(hDC, &dm);
        if (newDC != NULL) {
          success = true;
        }
      }
      
      // Delete the DC
      DeleteDC(hDC);
    }
  }
  
  // Method 3: If all else fails, use the rundll32 approach to modify printer properties
  if (!success) {
    // Format the command with the appropriate parameters to set paper size
    std::wstring command = L"rundll32.exe printui.dll,PrintUIEntry /Xs /n\"" + 
                          widePrinterName + L"\" paper=" + std::to_wstring(paperSizeId);
    
    // Execute the command
    STARTUPINFOW si = {0};
    si.cb = sizeof(STARTUPINFOW);
    PROCESS_INFORMATION pi = {0};
    
    if (CreateProcessW(
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
    )) {
      // Wait for the process to finish
      WaitForSingleObject(pi.hProcess, 5000); // Wait up to 5 seconds
      
      // Close process and thread handles
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      
      success = true;
    }
  }
  
  return success;
}

// PrintRawData implementation
bool PrinterManager::PrintRawData(const std::string& printerName, const std::vector<uint8_t>& data) {
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
  
  // Convert printer name to wide string
  std::wstring widePrinterName = Utf8ToWide(actualPrinterName);
  
  // Open the printer
  HANDLE hPrinter = NULL;
  if (!OpenPrinter(const_cast<LPWSTR>(widePrinterName.c_str()), &hPrinter, NULL)) {
    return false;
  }
  
  // Get printer information to determine if it's a raw-capable printer
  DWORD needed = 0;
  GetPrinter(hPrinter, 2, NULL, 0, &needed);
  bool isPosRawPrinter = false;
  
  if (needed > 0) {
    std::vector<BYTE> buffer(needed);
    PRINTER_INFO_2* pPrinterInfo = reinterpret_cast<PRINTER_INFO_2*>(buffer.data());
    
    if (GetPrinter(hPrinter, 2, buffer.data(), needed, &needed)) {
      // Check if printer supports RAW format (most receipt printers do)
      if (pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_RAW_ONLY) {
        isPosRawPrinter = true;
      }
    }
  }
  
  // Start a print job
  DOC_INFO_1 docInfo = {0};
  docInfo.pDocName = L"Raw Print Job";
  docInfo.pOutputFile = NULL;
  docInfo.pDatatype = isPosRawPrinter ? L"RAW" : L"TEXT";
  
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
  
  // Convert printer name and temp file to wide strings
  std::wstring widePrinterName = Utf8ToWide(actualPrinterName);
  std::wstring wideTempFile = Utf8ToWide(tempPdfFile);
  
  // Build the command to print the PDF silently
  std::wstring command = L"rundll32.exe mshtml.dll,PrintHTML \"" + wideTempFile + L"\" \"" + widePrinterName + L"\"";
  
  // Execute the command
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
