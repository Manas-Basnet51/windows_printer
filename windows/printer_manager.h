#ifndef FLUTTER_PLUGIN_PRINTER_MANAGER_H_
#define FLUTTER_PLUGIN_PRINTER_MANAGER_H_

#include <flutter/standard_method_codec.h>
#include <string>
#include <vector>
#include <windows.h>

// Class that encapsulates all printer-related functionality
class PrinterManager {
public:
  /// Get list of available printers
  static flutter::EncodableList GetAvailablePrinters();

  /// Get printer properties
  static flutter::EncodableMap GetPrinterProperties(const std::string& printerName);
  
  /// Get paper size details
  static flutter::EncodableMap GetPaperSizeDetails(const std::string& printerName);
  
  /// Print raw data(useful for receipt/thermal printers)
  static bool PrintRawData(const std::string& printerName, 
                          const std::vector<uint8_t>& data, 
                          bool useRawDatatype = true);
  
  /// Print PDF data
  static bool PrintPdf(const std::string& printerName, const std::vector<uint8_t>& data, int copies = 1);
  
  
  /// Set as default printer
  static bool AssignDefaultPrinter(const std::string& printerName);
  
  /// Open printer properties dialog screen
  static bool OpenPrinterProperties(const std::string& printerName);

  /// Print rich text (use symbols *, #, **,....)
  static bool PrintRichTextDocument(const std::string& printerName, 
                                    const std::string& richTextContent,
                                    const std::string& defaultFontName = "Courier New",
                                    int defaultFontSize = 12);

private:
  /// Helper function to convert wide string to UTF-8
  static std::string WideToUtf8(const wchar_t* wide_str);
  
  /// Helper function to convert UTF-8 to wide string
  static std::wstring Utf8ToWide(const std::string& utf8_str);

  /// Helper function to parse and print a line with formatting
  static void ParseAndPrintLine(HDC hDC, const std::string& line, int x, int y,
                               HFONT normalFont, HFONT boldFont, HFONT italicFont, 
                               HFONT boldItalicFont, HFONT largeFont, int* lineHeight);
};

#endif // FLUTTER_PLUGIN_PRINTER_MANAGER_H_