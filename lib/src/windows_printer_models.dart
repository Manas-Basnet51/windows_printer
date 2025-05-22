import 'windows_printer_enums.dart';

/// Text styling options
class WPTextStyle {
  final bool bold;
  final bool italic;
  final bool underline;
  final bool doubleStrike;
  final WPTextAlign align;
  final WPTextSize size;
  final bool invert;
  
  const WPTextStyle({
    this.bold = false,
    this.italic = false,
    this.underline = false,
    this.doubleStrike = false,
    this.align = WPTextAlign.left,
    this.size = WPTextSize.normal,
    this.invert = false,
  });
}

class PaperSizeInfo {
  static const Map<WPPaperSize, Map<String, int>> specs = {
    WPPaperSize.mm58: {
      'width': 384,
      'normalChars': 32,
      'doubleWidthChars': 16,
      'recommendedMaxHeader': 14,
    },
    WPPaperSize.mm80: {
      'width': 576,
      'normalChars': 48,
      'doubleWidthChars': 24,
      'recommendedMaxHeader': 20,
    },
  };
  
  static int getMaxChars(WPPaperSize size, WPTextSize textSize) {
    final spec = specs[size]!;
    switch (textSize) {
      case WPTextSize.normal:
        return spec['normalChars']!;
      case WPTextSize.doubleWidth:
      case WPTextSize.doubleHeightWidth:
        return spec['doubleWidthChars']!;
      case WPTextSize.doubleHeight:
        return spec['normalChars']!;
    }
  }
}