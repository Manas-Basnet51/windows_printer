import 'dart:typed_data';
import 'src/windows_printer_platform_interface.dart';
import 'windows_printer.dart';

// Export all models, enums, and utilities
export 'src/windows_printer_models.dart';
export 'src/windows_printer_enums.dart';
export 'src/esc_pos/windows_printer_esc_pos_generator.dart';
export 'src/esc_pos/windows_printer_receipt_builder.dart';

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

  /// Sends raw binary data directly to printer with datatype control
  /// 
  /// **Parameters:**
  /// - [useRawDatatype]: 
  ///   - `true` (default): RAW mode - for thermal/receipt printers with ESC/POS commands
  ///   - `false`: TEXT mode - for regular printers with plain text (poor quality)
  /// 
  /// **Usage Guide:**
  /// - ✅ Thermal printers: `useRawDatatype: true` (default)
  /// - ❌ Regular printers: Use `printRichTextDocument()` instead for better quality
  /// 
  /// **FIXED FEATURES:**
  /// - Hardware controls now work: cut, beep, cash drawer
  /// - Barcode printing now works with proper ESC/POS sequences
  /// - Image printing now supported with monochrome bitmap conversion
  /// - QR codes enhanced with error correction and size options
  static Future<bool> printRawData({
    String? printerName, // null = use default printer
    required Uint8List data,
    bool useRawDatatype = true, // true=RAW (thermal), false=TEXT (regular)
  }) {
    return WindowsPrinterPlatform.instance.printRawData(
      printerName: printerName,
      data: data,
      useRawDatatype: useRawDatatype,
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

  /// Sets the specified printer as system default (affects all applications)
  static Future<bool> setDefaultPrinter(String printerName) {
    return WindowsPrinterPlatform.instance.setDefaultPrinter(printerName);
  }

  /// Opens Windows printer properties dialog for user configuration
  static Future<bool> openPrinterProperties(String printerName) {
    return WindowsPrinterPlatform.instance.openPrinterProperties(printerName);
  }

  /// Print a rich text document with inline formatting markup
  /// 
  /// Supports the following markup:
  /// - `**bold text**` for bold formatting
  /// - `*italic text*` for italic formatting  
  /// - `##large text##` for large header text
  /// - Regular text for normal formatting
  /// 
  /// ⚠️ **EXPERIMENTAL**: Best suited for regular printers, not thermal printers.
  /// 
  /// **Printer Type Guide:**
  /// - ✅ **Regular printers**: Use this method for formatted text
  /// - ❌ **Thermal printers**: Use `printRawData(useRawDatatype: true)` with ESC/POS commands instead
  /// 
  /// Example:
  /// ```dart
  /// // For regular printers
  /// final content = '''##RECEIPT##
  /// Date: 2025-05-22
  /// 
  /// **Item                   Price**
  /// Coffee                   \$3.50
  /// *Special* Sandwich       \$7.25
  /// 
  /// **Total: \$10.75**''';
  /// 
  /// await WindowsPrinter.printRichTextDocument(
  ///   printerName: 'HP LaserJet',
  ///   content: content,
  /// );
  /// ```
  static Future<bool> printRichTextDocument({
    required String printerName,
    required String content,
    String fontName = 'Courier New',
    int fontSize = 12,
  }) {
    return WindowsPrinterPlatform.instance.printRichTextDocument(
      printerName: printerName,
      content: content,
      fontName: fontName,
      fontSize: fontSize,
    );
  }

  /// Quick thermal receipt printing helper
  /// 
  /// **NEW**: Simplified method for quick thermal printing with fixed ESC/POS
  /// 
  /// Example:
  /// ```dart
  /// await WindowsPrinter.printThermalReceipt(
  ///   printerName: 'Thermal Printer',
  ///   storeName: 'My Store',
  ///   items: [
  ///     ReceiptItem(name: 'Coffee', price: '\$3.50'),
  ///     ReceiptItem(name: 'Muffin', price: '\$2.75'),
  ///   ],
  ///   total: '\$6.25',
  ///   openDrawer: true,
  /// );
  /// ```
  static Future<bool> printThermalReceipt({
    String? printerName,
    required String storeName,
    String? storeAddress,
    String? phone,
    required List<ReceiptItem> items,
    required String total,
    String? customerName,
    String? paymentMethod,
    bool openDrawer = false,
    bool cutPaper = true,
    bool addBeep = false,
    WPPaperSize paperSize = WPPaperSize.mm80,
  }) async {
    final receipt = WPReceiptBuilder(wpPaperSize: paperSize)
      .createReceipt(
        storeName: storeName,
        storeAddress: storeAddress ?? '',
        phone: phone ?? '',
        items: items,
        total: total,
        customerName: customerName,
        paymentMethod: paymentMethod,
        openDrawer: openDrawer,
        cutPaper: cutPaper,
        addBeep: addBeep,
      )
      .build();

    return printRawData(
      printerName: printerName,
      data: Uint8List.fromList(receipt),
      useRawDatatype: true,
    );
  }

  /// Test hardware features (beep, drawer, cut)
  static Future<bool> testThermalHardware({
    String? printerName,
    bool testBeep = true,
    bool testDrawer = true,
    bool testCut = true,
  }) async {
    final generator = WPESCPOSGenerator();
    
    generator.text('HARDWARE TEST', style: const WPTextStyle(
      align: WPTextAlign.center,
      bold: true,
      size: WPTextSize.doubleHeightWidth
    ));
    
    generator.feed();
    
    if (testBeep) {
      generator.text('Testing beep...');
      generator.beep(count: 2, duration: 3);
      generator.feed();
    }
    
    if (testDrawer) {
      generator.text('Testing cash drawer...');
      generator.openDrawer();
      generator.feed();
    }
    
    generator.text('Hardware test completed!', style: const WPTextStyle(
      align: WPTextAlign.center,
      bold: true
    ));
    
    generator.feed(3);
    
    if (testCut) {
      generator.cut();
    }

    return printRawData(
      printerName: printerName,
      data: Uint8List.fromList(generator.getBytes()),
      useRawDatatype: true,
    );
  }
}