import 'dart:typed_data';

import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'windows_printer_method_channel.dart';

abstract class WindowsPrinterPlatform extends PlatformInterface {
  /// Constructs a WindowsPrinterPlatform.
  WindowsPrinterPlatform() : super(token: _token);

  static final Object _token = Object();

  static WindowsPrinterPlatform _instance = MethodChannelWindowsPrinter();

  /// The default instance of [WindowsPrinterPlatform] to use.
  ///
  /// Defaults to [MethodChannelWindowsPrinter].
  static WindowsPrinterPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [WindowsPrinterPlatform] when
  /// they register themselves.
  static set instance(WindowsPrinterPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  /// Get the list of available printers
  Future<List<String>> getAvailablePrinters();

  /// Get detailed properties of a printer
  Future<Map<String, dynamic>> getPrinterProperties(String printerName);

  /// Get detailed paper size information for a printer
  Future<Map<String, dynamic>> getPaperSizeDetails(String printerName);

  /// Print raw data with datatype control
  Future<bool> printRawData({
    String? printerName,
    required Uint8List data,
    bool useRawDatatype = true, // true=RAW (thermal), false=TEXT (regular)
  });

  /// Print PDF data
  Future<bool> printPdf({
    String? printerName,
    required Uint8List data,
    int copies = 1,
  });

  /// Set default printer
  Future<bool> setDefaultPrinter(String printerName);

  /// Open printer properties dialog
  Future<bool> openPrinterProperties(String printerName);

  /// Print rich text document with inline formatting
  Future<bool> printRichTextDocument({
    required String printerName,
    required String content,
    String fontName = 'Courier New',
    int fontSize = 12,
  });
}