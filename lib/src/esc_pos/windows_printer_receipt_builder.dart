import 'dart:typed_data';
import 'package:windows_printer/src/esc_pos/windows_printer_esc_pos_generator.dart';

import '../windows_printer_enums.dart';
import '../windows_printer_models.dart';

/// Helper class for easy receipt generation
///
/// ⚠️ EMOJI WARNING: Thermal printers use ASCII/Extended ASCII character sets, 
/// not Unicode. Avoid emojis (🎉, ❤️, 🌟) as they won't print correctly.
class WPReceiptBuilder {
  final WPESCPOSGenerator _generator;
  
  WPReceiptBuilder({WPPaperSize wpPaperSize = WPPaperSize.mm80}) 
      : _generator = WPESCPOSGenerator(paperSize : wpPaperSize);
  
  /// Add header text
  WPReceiptBuilder header(String text) {
    _generator.text(
      text, 
      style: const WPTextStyle(
        align: WPTextAlign.center, 
        bold: true, 
        size: WPTextSize.doubleHeightWidth
      )
    );
    return this;
  }
  
  /// Add subtitle
  WPReceiptBuilder subtitle(String text) {
    _generator.text(
      text,
      style: const WPTextStyle(align: WPTextAlign.center)
    );
    return this;
  }
  
  /// Add separator line
  WPReceiptBuilder separator() {
    _generator.separator();
    return this;
  }
  
  /// Add regular text line
  WPReceiptBuilder line(String text, {WPTextStyle? style}) {
    _generator.text(text, style: style);
    return this;
  }
  
  /// Add item with price (left-aligned item, right-aligned price)
  WPReceiptBuilder item(String name, String price) {
    final paperWidth = _generator.paperSize.width ~/ 12; // Approximate chars
    final totalLength = name.length + price.length;
    final spacesNeeded = paperWidth - totalLength;
    final spaces = spacesNeeded > 0 ? ' ' * spacesNeeded : ' ';
    
    _generator.text('$name$spaces$price');
    return this;
  }
  
  /// Add total line
  WPReceiptBuilder totalAmt(String amount) {
    _generator.text(
      'TOTAL: $amount',
      style: const WPTextStyle(
        bold: true, 
        size: WPTextSize.doubleHeight, 
        align: WPTextAlign.center
      )
    );
    return this;
  }
  
  /// Add footer text
  WPReceiptBuilder footer(String text) {
    _generator.text(
      text,
      style: const WPTextStyle(align: WPTextAlign.center, bold: true)
    );
    return this;
  }
  
  /// Add blank lines
  WPReceiptBuilder blank([int lines = 1]) {
    _generator.feed(lines);
    return this;
  }
  
  /// Add barcode
  WPReceiptBuilder addBarcode(WPBarcodeType type, String data, {
    int height = 162,
    int width = 3,
    bool showText = true,
  }) {
    _generator.barcode(type, data, height: height, width: width, showText: showText);
    return this;
  }
  
  /// Add QR code
  WPReceiptBuilder addQRCode(String data, {int size = 6, int errorCorrection = 1}) {
    _generator.qrCode(data, size: size, errorCorrection: errorCorrection);
    return this;
  }
  
  /// Adds an image to the receipt.
  ///
  /// [imageData] should be a [Uint8List] containing the raw image bytes.
  /// You must also specify the [width] and [height] of the image in pixels.
  ///
  /// This method is typically used to add logos, QR codes, or any other
  /// graphical content to the receipt.
  ///
  /// To convert an image file (e.g., from assets or a file path) into a 
  /// [Uint8List], you can use the [`image`](https://pub.dev/packages/image) 
  /// package from pub.dev. For example:
  ///
  /// ```dart
  /// import 'package:image/image.dart' as img;
  ///
  /// final bytes = File('path_to_image.png').readAsBytesSync();
  /// final decoded = img.decodeImage(bytes);
  /// final resized = img.copyResize(decoded, width: 200, height: 100);
  /// final uint8List = Uint8List.fromList(img.encodePng(resized));
  ///
  /// receiptBuilder.addImage(uint8List, 200, 100);
  /// ```
  ///
  WPReceiptBuilder addImage(Uint8List imageData, int width, int height) {
    _generator.image(imageData, width, height);
    return this;
  }
  
  /// Cut the paper. Set [partial] to true for a partial cut (leaves a small tab connected),
  /// or false for a full cut (completely separates the receipt).
  WPReceiptBuilder cut({bool partial = true}) {
    _generator.cut(partial: partial);
    return this;
  }
  
  /// Open the cash drawer.
  /// Wherever this method is placed in the chain, it will trigger the drawer to open 
  /// at that specific point during the printing process.
  WPReceiptBuilder drawer({int pin = 0, int onTime = 60, int offTime = 120}) {
    _generator.openDrawer(pin: pin, onTime: onTime, offTime: offTime);
    return this;
  }
  
  /// Beep
  WPReceiptBuilder beep({int count = 1, int duration = 3}) {
    _generator.beep(count: count, duration: duration);
    return this;
  }
  
  /// Add raw ESC/POS commands
  WPReceiptBuilder raw(List<int> bytes) {
    _generator.raw(bytes);
    return this;
  }
  
  /// Create a complete receipt template
  WPReceiptBuilder createReceipt({
    required String storeName,
    required String storeAddress,
    required String phone,
    required List<ReceiptItem> items,
    required String total,
    String? customerName,
    String? paymentMethod,
    bool openDrawer = false,
    bool cutPaper = true,
    bool addBeep = false,
  }) {
    header(storeName);
    subtitle(storeAddress);
    line(phone, style: const WPTextStyle(align: WPTextAlign.center));
    separator();
    
    line('Receipt #: ${DateTime.now().millisecondsSinceEpoch}');
    line('Date: ${DateTime.now().toString().substring(0, 16)}');
    if (customerName != null) {
      line('Customer: $customerName');
    }
    separator();
    
    line('ITEMS', style: const WPTextStyle(
      bold: true, 
      align: WPTextAlign.center,
      size: WPTextSize.doubleWidth
    ));
    blank();
    
    for (final receiptItem in items) {
      item(receiptItem.name, receiptItem.price);
      if (receiptItem.description != null) {
        line('  ${receiptItem.description}', style: const WPTextStyle(italic: true));
      }
    }
    
    separator();
    totalAmt(total);
    separator();
    
    // Payment info
    if (paymentMethod != null) {
      line('Payment: $paymentMethod', style: const WPTextStyle(bold: true));
    }
    
    blank();
    footer('Thank you for your business!');
    line('Please visit again soon', style: const WPTextStyle(
      align: WPTextAlign.center,
      italic: true
    ));
    
    blank(3);
    
    if (addBeep) beep();
    if (openDrawer) drawer();
    if (cutPaper) cut();
    
    return this;
  }
  
  /// Use this to test your printer
  WPReceiptBuilder createTestReceipt() {
    header('TEST RECEIPT');
    subtitle('ESC/POS Feature Demo');
    line('Generated: ${DateTime.now()}', style: const WPTextStyle(align: WPTextAlign.center));
    separator();
    
    line('TEXT FORMATTING TEST:', style: const WPTextStyle(bold: true, underline: true));
    line('Normal text');
    line('Bold text', style: const WPTextStyle(bold: true));
    line('Underlined text', style: const WPTextStyle(underline: true));
    line('Large text', style: const WPTextStyle(size: WPTextSize.doubleHeight));
    line('Double width', style: const WPTextStyle(size: WPTextSize.doubleWidth));
    line('Double H&W', style: const WPTextStyle(size: WPTextSize.doubleHeightWidth));
    line('Inverted text', style: const WPTextStyle(invert: true));
    
    separator();
    
    line('ALIGNMENT TEST:', style: const WPTextStyle(bold: true));
    line('Left aligned', style: const WPTextStyle(align: WPTextAlign.left));
    line('Center aligned', style: const WPTextStyle(align: WPTextAlign.center));
    line('Right aligned', style: const WPTextStyle(align: WPTextAlign.right));
    
    separator();
    
    line('ITEMS TEST:', style: const WPTextStyle(bold: true));
    item('Coffee', '\$3.50');
    item('Sandwich', '\$7.25');
    item('Cake slice', '\$4.00');
    separator();
    totalAmt('\$14.75');
    
    separator();
    
    line('QR CODE TEST:', style: const WPTextStyle(bold: true, align: WPTextAlign.center));
    addQRCode('https://github.com/Manas-Basnet51/windows_printer');
    
    blank();
    
    line('BARCODE TEST:', style: const WPTextStyle(bold: true, align: WPTextAlign.center));
    addBarcode(WPBarcodeType.code128, '123456789012');
    
    blank(2);
    
    line('HARDWARE TEST:', style: const WPTextStyle(bold: true, align: WPTextAlign.center));
    line('Testing beep...');
    beep(count: 2);
    
    blank();
    line('Opening drawer...');
    drawer();
    
    blank();
    footer('Test completed successfully!');
    line('All features demonstrated', style: const WPTextStyle(
      align: WPTextAlign.center,
      italic: true
    ));
    
    blank(3);
    cut();
    
    return this;
  }
  
  /// Get the generated bytes
  List<int> build() => _generator.getBytes();
}
