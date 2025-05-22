#include "windows_printer_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>
#include "printer_manager.h"

namespace windows_printer {

// static
void WindowsPrinterPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "windows_printer",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<WindowsPrinterPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

WindowsPrinterPlugin::WindowsPrinterPlugin() {}

WindowsPrinterPlugin::~WindowsPrinterPlugin() {}

void WindowsPrinterPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  // Handle all printer operations using the PrinterManager
  if (method_call.method_name().compare("getAvailablePrinters") == 0) {
    result->Success(flutter::EncodableValue(PrinterManager::GetAvailablePrinters()));
  } else if (method_call.method_name().compare("getPrinterProperties") == 0) {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGUMENTS", "Expected map arguments");
      return;
    }
    
    auto nameIter = arguments->find(flutter::EncodableValue("printerName"));
    if (nameIter == arguments->end() || !std::holds_alternative<std::string>(nameIter->second)) {
      result->Error("INVALID_PRINTER_NAME", "Printer name must be provided as a string");
      return;
    }
    
    std::string printerName = std::get<std::string>(nameIter->second);
    result->Success(flutter::EncodableValue(PrinterManager::GetPrinterProperties(printerName)));
  } else if (method_call.method_name().compare("getPaperSizeDetails") == 0) {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGUMENTS", "Expected map arguments");
      return;
    }
    
    auto nameIter = arguments->find(flutter::EncodableValue("printerName"));
    if (nameIter == arguments->end() || !std::holds_alternative<std::string>(nameIter->second)) {
      result->Error("INVALID_PRINTER_NAME", "Printer name must be provided as a string");
      return;
    }
    
    std::string printerName = std::get<std::string>(nameIter->second);
    result->Success(flutter::EncodableValue(PrinterManager::GetPaperSizeDetails(printerName)));
  } else if (method_call.method_name().compare("printPdf") == 0) {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGUMENTS", "Expected map arguments");
      return;
    }
    
    // Get printer name (optional - uses default printer if not provided)
    std::string printerName;
    auto nameIter = arguments->find(flutter::EncodableValue("printerName"));
    if (nameIter != arguments->end() && std::holds_alternative<std::string>(nameIter->second)) {
      printerName = std::get<std::string>(nameIter->second);
    }
    
    // Get the data to print
    auto dataIter = arguments->find(flutter::EncodableValue("data"));
    if (dataIter == arguments->end() || !std::holds_alternative<std::vector<uint8_t>>(dataIter->second)) {
      result->Error("INVALID_DATA", "PDF data must be provided as a Uint8List");
      return;
    }
    const std::vector<uint8_t>& pdfData = std::get<std::vector<uint8_t>>(dataIter->second);
    
    // Get options (optional)
    int copies = 1;
    auto copiesIter = arguments->find(flutter::EncodableValue("copies"));
    if (copiesIter != arguments->end() && std::holds_alternative<int>(copiesIter->second)) {
      copies = std::get<int>(copiesIter->second);
      if (copies < 1) copies = 1;
    }
    
    bool success = PrinterManager::PrintPdf(printerName, pdfData, copies);
    if (success) {
      result->Success(flutter::EncodableValue(true));
    } else {
      result->Error("PRINT_FAILED", "Failed to print PDF");
    }
  } else if (method_call.method_name().compare("printRawData") == 0) {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGUMENTS", "Expected map arguments");
      return;
    }
    
    auto nameIter = arguments->find(flutter::EncodableValue("printerName"));
    std::string printerName = "";
    if (nameIter != arguments->end() && std::holds_alternative<std::string>(nameIter->second)) {
      printerName = std::get<std::string>(nameIter->second);
    }
    
    auto dataIter = arguments->find(flutter::EncodableValue("data"));
    if (dataIter == arguments->end() || !std::holds_alternative<std::vector<uint8_t>>(dataIter->second)) {
      result->Error("INVALID_DATA", "Data must be provided as Uint8List");
      return;
    }
    std::vector<uint8_t> data = std::get<std::vector<uint8_t>>(dataIter->second);
    
    // Get optional useRawDatatype parameter (default: true)
    bool useRawDatatype = true;
    auto rawIter = arguments->find(flutter::EncodableValue("useRawDatatype"));
    if (rawIter != arguments->end() && std::holds_alternative<bool>(rawIter->second)) {
      useRawDatatype = std::get<bool>(rawIter->second);
    }
    
    bool success = PrinterManager::PrintRawData(printerName, data, useRawDatatype);

    if (success) {
      result->Success(flutter::EncodableValue(true));
    } else {
      result->Error("PRINT_RAW_DATA_FAILED", "Failed to print raw data");
    }
  } else if (method_call.method_name().compare("setDefaultPrinter") == 0) {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGUMENTS", "Expected map arguments");
      return;
    }
    
    auto nameIter = arguments->find(flutter::EncodableValue("printerName"));
    if (nameIter == arguments->end() || !std::holds_alternative<std::string>(nameIter->second)) {
      result->Error("INVALID_PRINTER_NAME", "Printer name must be provided as a string");
      return;
    }
    
    std::string printerName = std::get<std::string>(nameIter->second);
    bool success = PrinterManager::AssignDefaultPrinter(printerName);
    
    if (success) {
      result->Success(flutter::EncodableValue(true));
    } else {
      result->Error("SET_DEFAULT_FAILED", "Failed to set default printer");
    }
  } else if (method_call.method_name().compare("openPrinterProperties") == 0) {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGUMENTS", "Expected map arguments");
      return;
    }
    
    auto nameIter = arguments->find(flutter::EncodableValue("printerName"));
    if (nameIter == arguments->end() || !std::holds_alternative<std::string>(nameIter->second)) {
      result->Error("INVALID_PRINTER_NAME", "Printer name must be provided as a string");
      return;
    }
    
    std::string printerName = std::get<std::string>(nameIter->second);
    bool success = PrinterManager::OpenPrinterProperties(printerName);
    
    if (success) {
      result->Success(flutter::EncodableValue(true));
    } else {
      result->Error("OPEN_PRINTER_PROPERTIES_FAILED", "Failed to open printer properties");
    }
  } else if (method_call.method_name().compare("printRichTextDocument") == 0) {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGUMENTS", "Expected map arguments");
      return;
    }
    
    // Extract required parameters
    auto nameIter = arguments->find(flutter::EncodableValue("printerName"));
    if (nameIter == arguments->end() || !std::holds_alternative<std::string>(nameIter->second)) {
      result->Error("INVALID_PRINTER_NAME", "Printer name must be provided as a string");
      return;
    }
    std::string printerName = std::get<std::string>(nameIter->second);
    
    auto contentIter = arguments->find(flutter::EncodableValue("content"));
    if (contentIter == arguments->end() || !std::holds_alternative<std::string>(contentIter->second)) {
      result->Error("INVALID_CONTENT", "Content must be provided as a string");
      return;
    }
    std::string content = std::get<std::string>(contentIter->second);
    
    // Extract optional parameters with defaults
    std::string fontName = "Courier New";
    auto fontNameIter = arguments->find(flutter::EncodableValue("fontName"));
    if (fontNameIter != arguments->end() && std::holds_alternative<std::string>(fontNameIter->second)) {
      fontName = std::get<std::string>(fontNameIter->second);
    }
    
    int fontSize = 12;
    auto fontSizeIter = arguments->find(flutter::EncodableValue("fontSize"));
    if (fontSizeIter != arguments->end() && std::holds_alternative<int>(fontSizeIter->second)) {
      fontSize = std::get<int>(fontSizeIter->second);
    }
    
    bool success = PrinterManager::PrintRichTextDocument(printerName, content, fontName, fontSize);

    if (success) {
      result->Success(flutter::EncodableValue(true));
    } else {
      result->Error("PRINT_RICH_TEXT_FAILED", "Failed to print rich text document");
    }
  } else {
    result->NotImplemented();
  }
}

}  // namespace windows_printerwindows_printe
