# Windows Printer

A Flutter plugin for managing and interacting with printers on Windows platforms.

## Features

- Get a list of available printers
- Set a default printer
- Get detailed printer properties
- Retrieve paper size information
- Print raw data (useful for receipt printers)
- Print PDF documents
- Open printer properties dialog

## Requirements

- Windows platform
- Flutter 2.0.0 or higher
- Windows 10 or higher (recommended)

## Installation

Add this to your package's `pubspec.yaml` file:

```yaml
dependencies:
  windows_printer: ^0.1.0
```

## Usage

Import the package:
```dart
import 'package:windows_printer/windows_printer.dart';
```

## Simple Example:
```dart
// Get available printers
List<String> printers = await WindowsPrinter.getAvailablePrinters();

// Print to default printer
if (printers.isNotEmpty) {
  // Get printer properties
  Map<String, dynamic> properties = await WindowsPrinter.getPrinterProperties(printers[0]);
  
  // Print a PDF file
  final file = File('document.pdf');
  final bytes = await file.readAsBytes();
  await WindowsPrinter.printPdf(data: bytes);
}
```

## API Reference

### 1. Method: WindowsPrinter.getAvailablePrinters()
Returns a list of available printer names.

```dart
static Future<List<String>> printers = await WindowsPrinter.getAvailablePrinters();
```

### 2. Method: WindowsPrinter.setDefaultPrinter(String printerName)
Sets the specified printer as the default system printer.

```dart
static Future<bool> success = await WindowsPrinter.setDefaultPrinter("Printer Name");
``` 

### 3. Method: WindowsPrinter.getPrinterProperties(String printerName)
Returns detailed properties of the specified printer.

```dart
static Future<Map<String, dynamic>> properties = await WindowsPrinter.getPrinterProperties("Printer Name");
``` 

### 4. Method: WindowsPrinter.getPaperSizeDetails(String printerName)
Returns paper size information for the specified printer.

```dart
static Future<Map<String, dynamic>> paperDetails = await WindowsPrinter.getPaperSizeDetails("Printer Name");
``` 

### 5. Method: WindowsPrinter.printRawData({String? printerName, required Uint8List data})
Sends raw data to a printer. Useful for receipt/thermal printers.

```dart
static Future<bool> success = await WindowsPrinter.printRawData(
  printerName: "Printer Name", // Optional, uses default printer if null
  data: Uint8List.fromList([...]), // Byte data to print
);
``` 

### 6. Method: WindowsPrinter.printPdf({String? printerName, required Uint8List data, int copies = 1})
Prints PDF data.

```dart
static Future<bool> success = await WindowsPrinter.printPdf(
  printerName: "Printer Name", // Optional, uses default printer if null
  data: Uint8List.fromList([...]), // PDF data
  copies: 2, // Number of copies to print
);
```

### 7. Method: WindowsPrinter.openPrinterProperties(String printerName)
Opens the Windows printer properties dialog for the specified printer.

```dart
static Future<bool> success = await WindowsPrinter.openPrinterProperties("Printer Name");
``` 

## Troubleshooting
Common Issues

__Plugin not loading on Windows__

- Ensure you're running on a Windows platform
- Verify Flutter's Windows support is properly configured


__No printers found__

- Verify that printers are properly installed on your Windows system
- Check that your app has necessary permissions

## License
This project is licensed under the MIT License - see the LICENSE file for details.













