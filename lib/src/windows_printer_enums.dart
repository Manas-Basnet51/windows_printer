/// Paper sizes for thermal printers
enum WPPaperSize {
  mm58(384),   // 58mm paper width in pixels
  mm80(576);   // 80mm paper width in pixels
  
  const WPPaperSize(this.width);
  final int width;
}

/// Text alignment options
enum WPTextAlign {
  left(0),
  center(1),
  right(2);
  
  const WPTextAlign(this.value);
  final int value;
}

/// Text size options
enum WPTextSize {
  normal(0x00),
  doubleHeight(0x10),
  doubleWidth(0x20),
  doubleHeightWidth(0x30);
  
  const WPTextSize(this.value);
  final int value;
}

/// Barcode types
enum WPBarcodeType {
  upca(0),
  upce(1),
  ean13(2),
  ean8(3),
  code39(4),
  itf(5),
  codabar(6),
  code93(7),
  code128(8);
  
  const WPBarcodeType(this.value);
  final int value;
}

