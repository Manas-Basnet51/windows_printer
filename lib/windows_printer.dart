
import 'dart:typed_data';

import 'windows_printer_platform_interface.dart';

class WindowsPrinter {
  /// Get the list of available printers
  static Future<List<String>> getAvailablePrinters() {
    return WindowsPrinterPlatform.instance.getAvailablePrinters();
  }

  /// Get detailed properties of a printer
  static Future<Map<String, dynamic>> getPrinterProperties(String printerName) {
    return WindowsPrinterPlatform.instance.getPrinterProperties(printerName);
  }

  /// Get detailed paper size information for a printer
  static Future<Map<String, dynamic>> getPaperSizeDetails(String printerName) {
    return WindowsPrinterPlatform.instance.getPaperSizeDetails(printerName);
  }

  /// Set paper size for a printer
  // static Future<bool> setPaperSize(String printerName, int paperSizeId) {
  //   return WindowsPrinterPlatform.instance.setPaperSize(printerName, paperSizeId);
  // }

  /// Print raw data (useful for receipt/thermal printers)
  static Future<bool> printRawData({
    String? printerName,
    required Uint8List data,
  }) {
    return WindowsPrinterPlatform.instance.printRawData(
      printerName: printerName,
      data: data,
    );
  }

  /// Print PDF data
  static Future<bool> printPdf({
    String? printerName,
    required Uint8List data,
    int copies = 1,
  }) {
    return WindowsPrinterPlatform.instance.printPdf(
      printerName: printerName,
      data: data,
      copies: copies,
    );
  }

  /// Set default printer
  static Future<bool> setDefaultPrinter(String printerName) {
    return WindowsPrinterPlatform.instance.setDefaultPrinter(printerName);
  }

  /// Get printer device settings
  // static Future<Map<String, dynamic>> getPrinterDeviceSettings(String printerName) {
  //   return WindowsPrinterPlatform.instance.getPrinterDeviceSettings(printerName);
  // }

  /// Open printer properties dialog
  static Future<bool> openPrinterProperties(String printerName) {
    return WindowsPrinterPlatform.instance.openPrinterProperties(printerName);
  }

  /// Set printer quality
  // static Future<bool> setPrinterQuality(String printerName, int quality) {
  //   return WindowsPrinterPlatform.instance.setPrinterQuality(printerName, quality);
  // }
}
