import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'windows_printer_platform_interface.dart';

/// An implementation of [WindowsPrinterPlatform] that uses method channels.
class MethodChannelWindowsPrinter extends WindowsPrinterPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('windows_printer');

  @override
  Future<List<String>> getAvailablePrinters() async {
    final List<Object?> result = await methodChannel.invokeMethod('getAvailablePrinters');
    return result.map((e) => e.toString()).toList();
  }

  @override
  Future<Map<String, dynamic>> getPrinterProperties(String printerName) async {
    final Map<Object?, Object?> result = await methodChannel.invokeMethod(
      'getPrinterProperties',
      {'printerName': printerName},
    );
    return _convertMap(result);
  }

  @override
  Future<Map<String, dynamic>> getPaperSizeDetails(String printerName) async {
    final Map<Object?, Object?> result = await methodChannel.invokeMethod(
      'getPaperSizeDetails',
      {'printerName': printerName},
    );
    return _convertMap(result);
  }

  @override
  Future<bool> setPaperSize(String printerName, int paperSizeId) async {
    final bool result = await methodChannel.invokeMethod(
      'setPaperSize',
      {
        'printerName': printerName,
        'paperSizeId': paperSizeId,
      },
    );
    return result;
  }

  @override
  Future<bool> printRawData({String? printerName, required Uint8List data}) async {
    final Map<String, dynamic> args = {
      'data': data,
    };
    
    if (printerName != null) {
      args['printerName'] = printerName;
    }
    
    final bool result = await methodChannel.invokeMethod('printRawData', args);
    return result;
  }

  @override
  Future<bool> printPdf({String? printerName, required Uint8List data, int copies = 1}) async {
    final Map<String, dynamic> args = {
      'data': data,
      'copies': copies,
    };
    
    if (printerName != null) {
      args['printerName'] = printerName;
    }
    
    final bool result = await methodChannel.invokeMethod('printPdf', args);
    return result;
  }

  @override
  Future<bool> setDefaultPrinter(String printerName) async {
    final bool result = await methodChannel.invokeMethod(
      'setDefaultPrinter',
      {'printerName': printerName},
    );
    return result;
  }

  @override
  Future<Map<String, dynamic>> getPrinterDeviceSettings(String printerName) async {
    final Map<Object?, Object?> result = await methodChannel.invokeMethod(
      'getPrinterDeviceSettings',
      {'printerName': printerName},
    );
    return _convertMap(result);
  }

  @override
  Future<bool> openPrinterProperties(String printerName) async {
    final bool result = await methodChannel.invokeMethod(
      'openPrinterProperties',
      {'printerName': printerName},
    );
    return result;
  }

  @override
  Future<bool> setPrinterQuality(String printerName, int quality) async {
    final bool result = await methodChannel.invokeMethod(
      'setPrinterQuality',
      {
        'printerName': printerName,
        'quality': quality,
      },
    );
    return result;
  }

  // Helper to convert from platform channel types to Dart types
  Map<String, dynamic> _convertMap(Map<Object?, Object?> map) {
    final result = <String, dynamic>{};
    for (final entry in map.entries) {
      final key = entry.key.toString();
      final value = entry.value;
      
      if (value is Map<Object?, Object?>) {
        result[key] = _convertMap(value);
      } else if (value is List<Object?>) {
        result[key] = _convertList(value);
      } else {
        result[key] = value;
      }
    }
    return result;
  }

  List<dynamic> _convertList(List<Object?> list) {
    final result = <dynamic>[];
    for (final item in list) {
      if (item is Map<Object?, Object?>) {
        result.add(_convertMap(item));
      } else if (item is List<Object?>) {
        result.add(_convertList(item));
      } else {
        result.add(item);
      }
    }
    return result;
  }
}
