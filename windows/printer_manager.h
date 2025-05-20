#ifndef FLUTTER_PLUGIN_PRINTER_MANAGER_H_
#define FLUTTER_PLUGIN_PRINTER_MANAGER_H_

#include <flutter/standard_method_codec.h>
#include <string>
#include <vector>

// Class that encapsulates all printer-related functionality
class PrinterManager {
public:
  /// Get list of available printers
  static flutter::EncodableList GetAvailablePrinters();

  /// Get printer properties
  static flutter::EncodableMap GetPrinterProperties(const std::string& printerName);
  
  /// Get paper size details
  static flutter::EncodableMap GetPaperSizeDetails(const std::string& printerName);
  
  // Set paper size
  static bool SetPaperSize(const std::string& printerName, int paperSizeId);
  
  /// Print raw data
  static bool PrintRawData(const std::string& printerName, const std::vector<uint8_t>& data);
  
  /// Print PDF data
  static bool PrintPdf(const std::string& printerName, const std::vector<uint8_t>& data, int copies = 1);
  
  /// Get printer device settings
  static flutter::EncodableMap GetPrinterDeviceSettings(const std::string& printerName);
  
  /// Set default printer
  static bool AssignDefaultPrinter(const std::string& printerName);
  
  /// Open printer properties dialog
  static bool OpenPrinterProperties(const std::string& printerName);
  
  /// Set printer quality
  static bool SetPrinterQuality(const std::string& printerName, int quality);

private:
  /// Helper function to convert wide string to UTF-8
  static std::string WideToUtf8(const wchar_t* wide_str);
  
  /// Helper function to convert UTF-8 to wide string
  static std::wstring Utf8ToWide(const std::string& utf8_str);
};

#endif // FLUTTER_PLUGIN_PRINTER_MANAGER_H_