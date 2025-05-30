## [0.2.1] - 2025-05-26

### Fixed
* Hardware control operations (cash drawer, paper cutting, beeping) now function correctly.
* Barcode generation now works as expected.
* Image printing issues resolved.
* Improved ESC/POS QR code generation for better compatibility and visual clarity.

### Added
* `createTestReceipt()` method in `WPReceiptBuilder` to test printer
* `printTestPage()` method in `WPESCPOSGenerator` to test printer

## 0.2.0

### Added
* Thermal/Receipt printer support with ESC/POS implementation
* `WPESCPOSGenerator` class for low-level ESC/POS command generation
* `WPReceiptBuilder` class with fluent API for easy receipt creation
* `printRichTextDocument()` method for formatted text on regular printers
* `useRawDatatype` parameter to `printRawData()` for explicit datatype control
* Paper size support: `WPPaperSize.mm58` and `WPPaperSize.mm80`
* QR code generation for receipts
* Hardware control: cash drawer opening, paper cutting, beeping
* Text formatting options: bold, italic, underline, alignment, sizing

### Fixed
* Native-side configuration issues in `printRawData()` method
* Improved font handling in rich text rendering

### Known Issues
* Barcode generation not working properly (under investigation)
* Image printing not working properly (under investigation)

### Removed (Internal APIs)
* `SetPaperSize()` 
* `GetPrinterDeviceSettings()`
* `SetPrinterQuality()`

## 0.1.0

### Added
* Getting available printers
* Getting printer properties
* Getting paper size details 
* Printing raw data
* Printing PDF documents
* Setting default printer
* Opening printer properties dialog