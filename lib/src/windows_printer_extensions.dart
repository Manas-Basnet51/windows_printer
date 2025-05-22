/// ⚠️ WARNING: This file is not in use as it has problems and is still in development
/// 
/// This extension is temporarily disabled due to issues and i will probably work in this in future
/// Do not use these methods in production until further testing is completed.

import '../windows_printer.dart';

/// Extension methods for WPReceiptBuilder with safety measures
extension SafeReceiptBuilder on WPReceiptBuilder {
  
  /// Add header with automatic text truncation for paper width
  WPReceiptBuilder safeHeader(String text, int maxChars) {
    if (text.length <= maxChars) {
      return header(text);
    } else {
      final words = text.split(' ');
      String currentLine = '';
      
      for (final word in words) {
        if ((currentLine + word).length <= maxChars) {
          currentLine += (currentLine.isEmpty ? '' : ' ') + word;
        } else {
          if (currentLine.isNotEmpty) {
            line(currentLine, style: const WPTextStyle(
              align: WPTextAlign.center, 
              bold: true, 
              size: WPTextSize.doubleHeightWidth
            ));
          }
          currentLine = word;
        }
      }
      
      if (currentLine.isNotEmpty) {
        line(currentLine, style: const WPTextStyle(
          align: WPTextAlign.center, 
          bold: true, 
          size: WPTextSize.doubleHeightWidth
        ));
      }
      
      return this;
    }
  }
  
  /// Add subtitle with automatic text wrapping
  WPReceiptBuilder safeSubtitle(String text, int maxChars) {
    if (text.length <= maxChars) {
      return subtitle(text);
    } else {
      final lines = _wrapText(text, maxChars);
      for (final line in lines) {
        subtitle(line);
      }
      return this;
    }
  }
  
  /// Add item with automatic spacing calculation
  WPReceiptBuilder safeItem(String name, String price, int maxChars) {
    final totalLength = name.length + price.length;
    
    if (totalLength <= maxChars - 2) {
      return item(name, price);
    } else {
      final maxNameLength = maxChars - price.length - 3;
      final truncatedName = name.length > maxNameLength 
          ? '${name.substring(0, maxNameLength - 3)}...'
          : name;
      return item(truncatedName, price);
    }
  }
  
  /// Add total with automatic sizing based on length
  WPReceiptBuilder safeTotal(String amount, int maxChars) {
    final totalText = 'TOTAL: $amount';
    
    if (totalText.length <= maxChars) {
      return total(amount);
    } else {
      line(totalText, style: const WPTextStyle(
        bold: true,
        size: WPTextSize.normal,
        align: WPTextAlign.center
      ));
      return this;
    }
  }
  
  /// Helper method to wrap text into multiple lines
  List<String> _wrapText(String text, int maxChars) {
    final words = text.split(' ');
    final lines = <String>[];
    String currentLine = '';
    
    for (final word in words) {
      if ((currentLine + word).length <= maxChars) {
        currentLine += (currentLine.isEmpty ? '' : ' ') + word;
      } else {
        if (currentLine.isNotEmpty) {
          lines.add(currentLine);
        }
        currentLine = word;
      }
    }
    
    if (currentLine.isNotEmpty) {
      lines.add(currentLine);
    }
    
    return lines;
  }
}