import 'dart:convert';
import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'package:windows_printer/windows_printer.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Windows Printer Demo',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: const PrinterDemoPage(),
    );
  }
}

class PrinterDemoPage extends StatefulWidget {
  const PrinterDemoPage({super.key});

  @override
  State<PrinterDemoPage> createState() => _PrinterDemoPageState();
}

class _PrinterDemoPageState extends State<PrinterDemoPage> {
  List<String> _printers = [];
  String? _selectedPrinter;
  Map<String, dynamic>? _printerProperties;
  Map<String, dynamic>? _paperSizeDetails;
  bool _loading = true;
  String _status = '';
  
  // Demo settings
  bool _isThermalPrinter = false;
  int _pdfCopies = 1;
  String _selectedFont = 'Courier New';
  int _fontSize = 12;

  @override
  void initState() {
    super.initState();
    _loadPrinters();
  }

  Future<void> _loadPrinters() async {
    setState(() {
      _loading = true;
      _status = 'Loading printers...';
    });

    try {
      final printers = await WindowsPrinter.getAvailablePrinters();
      setState(() {
        _printers = printers;
        _loading = false;
        _status = 'Found ${printers.length} printers';
      });
    } catch (e) {
      setState(() {
        _loading = false;
        _status = 'Error: $e';
      });
    }
  }

  Future<void> _selectPrinter(String? printer) async {
    if (printer == null) return;

    setState(() {
      _selectedPrinter = printer;
      _printerProperties = null;
      _paperSizeDetails = null;
      _loading = true;
      _status = 'Loading printer details...';
    });

    try {
      final properties = WindowsPrinter.getPrinterProperties(printer);
      final paperDetails = WindowsPrinter.getPaperSizeDetails(printer);
      final results = await Future.wait([properties, paperDetails]);

      setState(() {
        _printerProperties = results[0];
        _paperSizeDetails = results[1];
        _loading = false;
        _status = 'Printer details loaded';
        
        // Try to check if te printer is thermal printer
        _isThermalPrinter = _detectThermalPrinter(printer);
      });
    } catch (e) {
      setState(() {
        _loading = false;
        _status = 'Error: $e';
      });
    }
  }

  bool _detectThermalPrinter(String printerName) {
    final thermal = printerName.toLowerCase();
    return thermal.contains('thermal') || 
           thermal.contains('receipt') || 
           thermal.contains('pos') ||
           thermal.contains('tm-') ||
           thermal.contains('rp-');
  }

  Future<void> _setDefaultPrinter() async {
    if (_selectedPrinter == null) return;

    setState(() {
      _loading = true;
      _status = 'Setting default printer...';
    });

    try {
      final success = await WindowsPrinter.setDefaultPrinter(_selectedPrinter!);
      await _loadPrinters();
      
      setState(() {
        _loading = false;
        _status = success ? 'Default printer set successfully' : 'Failed to set default printer';
      });
    } catch (e) {
      setState(() {
        _loading = false;
        _status = 'Error: $e';
      });
    }
  }

  Future<void> _openPrinterProperties() async {
    if (_selectedPrinter == null) return;

    setState(() {
      _status = 'Opening printer properties...';
    });

    try {
      await WindowsPrinter.openPrinterProperties(_selectedPrinter!);
      setState(() {
        _status = 'Printer properties dialog opened';
      });
    } catch (e) {
      setState(() {
        _status = 'Error: $e';
      });
    }
  }

  // THERMAL PRINTER METHODS (using ESC/POS)
  Future<void> _printThermalReceipt() async {
    if (_selectedPrinter == null) return;

    setState(() {
      _loading = true;
      _status = 'Printing thermal receipt...';
    });

    try {
      final escPosData = await _generateThermalReceipt();
      final success = await WindowsPrinter.printRawData(
        printerName: _selectedPrinter!,
        data: Uint8List.fromList(escPosData),
        useRawDatatype: true, // ESSENTIAL for thermal printers
      );
      
      setState(() {
        _loading = false;
        _status = success ? 'Thermal receipt printed successfully' : 'Failed to print thermal receipt';
      });
    } catch (e) {
      setState(() {
        _loading = false;
        _status = 'Error: $e';
      });
    }
  }

  Future<List<int>> _generateThermalReceipt() async {
    final receipt = WPReceiptBuilder(wpPaperSize: WPPaperSize.mm58)
      
      .header('RECEIPT')
      .line('Coffee & More Shop', style: const WPTextStyle(
        align: WPTextAlign.center, 
        bold: true,
        size: WPTextSize.doubleHeight
      ))
      .line('123 Main Street, City', style: const WPTextStyle(
        align: WPTextAlign.center,
        italic: true
      ))
      .line('Tel: (555) 123-4567', style: const WPTextStyle(
        align: WPTextAlign.center,
        underline: true
      ))
      .separator()
      
      .line('Receipt #: R001234', style: const WPTextStyle(align: WPTextAlign.left))
      .line('Date: ${DateTime.now().toString().substring(0, 16)}', style: const WPTextStyle(align: WPTextAlign.left))
      .line('Cashier: John D.', style: const WPTextStyle(align: WPTextAlign.right))
      .separator()
      
      .line('ITEMS', style: const WPTextStyle(
        bold: true, 
        align: WPTextAlign.center,
        size: WPTextSize.doubleWidth,
        underline: true
      ))
      .blank()
      
      .item('Espresso', '\$2.50')
      .line('  * Premium blend', style: const WPTextStyle(italic: true))
      .item('Cappuccino', '\$3.75')
      .line('  * Large size', style: const WPTextStyle(italic: true, align: WPTextAlign.right))
      .line('Croissant', style: const WPTextStyle(bold: true))
      .line('  Fresh baked          \$2.25', style: const WPTextStyle(italic: true))
      .line('Muffin (Blueberry)', style: const WPTextStyle(underline: true))
      .line('  Special of the day   \$3.00')
      .item('Orange Juice', '\$2.75')
      .blank()
      
      .separator()
      .line('Subtotal:            \$14.25', style: const WPTextStyle(align: WPTextAlign.left))
      .line('Tax (8.5%):           \$1.21', style: const WPTextStyle(align: WPTextAlign.left))
      .line('Discount:            -\$0.50', style: const WPTextStyle(
        align: WPTextAlign.left,
        italic: true
      ))
      .separator()

      .line('TOTAL: \$14.96', style: const WPTextStyle(
        bold: true,
        size: WPTextSize.doubleHeightWidth,
        align: WPTextAlign.center,
        underline: true
      ))
      .separator()
      
      .line('PAYMENT', style: const WPTextStyle(
        bold: true, 
        align: WPTextAlign.center,
        size: WPTextSize.doubleHeight
      ))
      .line('Card Payment:        \$14.96', style: const WPTextStyle(bold: true))
      .line('Card Type:', style: const WPTextStyle(italic: true))
      .line('  VISA ****1234', style: const WPTextStyle(align: WPTextAlign.center))
      .line('Auth Code: A12345', style: const WPTextStyle(
        align: WPTextAlign.center,
        underline: true
      ))
      .separator()
      
      .line('Customer: Sarah M.', style: const WPTextStyle(align: WPTextAlign.left))
      .line('Points Earned: 15', style: const WPTextStyle(
        align: WPTextAlign.center,
        italic: true
      ))
      .line('Total Points: 245', style: const WPTextStyle(
        align: WPTextAlign.right,
        bold: true
      ))
      .separator()
      
      .line('SPECIAL OFFERS', style: const WPTextStyle(
        align: WPTextAlign.center, 
        bold: true,
        size: WPTextSize.doubleWidth,
        invert: true 
      ))
      .line('Buy 5 coffees, get 1 FREE!', style: const WPTextStyle(
        align: WPTextAlign.center,
        italic: true,
        underline: true
      ))
      .line('Valid until: Dec 31, 2025', style: const WPTextStyle(
        align: WPTextAlign.center,
        size: WPTextSize.normal
      ))
      .separator()
      
      .line('Scan for digital receipt:', style: const WPTextStyle(
        align: WPTextAlign.center,
        italic: true
      ))
      .addQRCode('https://manasbasnet.com')
      .blank()
      
      .line('Thank you for your visit!', style: const WPTextStyle(
        align: WPTextAlign.center, 
        bold: true,
        size: WPTextSize.doubleHeight
      ))
      .line('Please come again soon', style: const WPTextStyle(
        align: WPTextAlign.center,
        italic: true
      ))
      .line('Follow us @coffeeshop', style: const WPTextStyle(
        align: WPTextAlign.center,
        underline: true
      ))
      .separator()
      
      .line('RETURN POLICY:', style: const WPTextStyle(
        bold: true,
        underline: true,
        size: WPTextSize.doubleWidth
      ))
      .line('Returns accepted within 7 days', style: const WPTextStyle(italic: true))
      .line('with receipt and original', style: const WPTextStyle(italic: true))
      .line('packaging.', style: const WPTextStyle(italic: true))
      .blank()
      
      .line('FORMATTING DEMO COMPLETE', style: const WPTextStyle(
        bold: true,
        align: WPTextAlign.center,
        size: WPTextSize.doubleHeightWidth,
        invert: true
      ))
      .blank()
      
      // Hardware actions
      // .drawer() // Open cash drawer
      // .beep()   // Confirmation beep
      // .cut()  // Uncomment to auto-cut paper
      
      .build();
      
    return receipt;
  }

  // REGULAR PRINTER METHODS (using GDI)
  Future<void> _printRichTextDocument() async {
    if (_selectedPrinter == null) return;

    setState(() {
      _loading = true;
      _status = 'Printing rich text document...';
    });

    try {
      final content = _createRichTextContent();
      final success = await WindowsPrinter.printRichTextDocument(
        printerName: _selectedPrinter!,
        content: content,
        fontName: _selectedFont,
        fontSize: _fontSize,
      );
      
      setState(() {
        _loading = false;
        _status = success ? 'Rich text document printed successfully' : 'Failed to print rich text';
      });
    } catch (e) {
      setState(() {
        _loading = false;
        _status = 'Error: $e';
      });
    }
  }

  String _createRichTextContent() {
    return '''##REGULAR PRINTER DEMO##
Rich Text Document with GDI Rendering
Font: $_selectedFont, Size: $_fontSize pt
=====================================

**FORMATTING EXAMPLES:**
- **Bold text example**
- *Italic text example*  
- ##Large header text##
- ***Bold and italic combined***
- Regular normal text

**RECEIPT SIMULATION:**
Date: ${DateTime.now().toString().substring(0, 19)}
-------------------------------------
Coffee                         \$3.50
*Special* Sandwich             \$7.25
Donut                          \$2.00
-------------------------------------
**Total:                      \$12.75**

*Thank you for your business!*
**Please visit again soon**

This document demonstrates rich text formatting
using markup symbols for regular printers.''';
  }

  // PDF PRINTING (works with both printer types)
  Future<void> _printPdfSample() async {
    if (_selectedPrinter == null) return;

    setState(() {
      _loading = true;
      _status = 'Printing PDF sample...';
    });

    try {
      // Note: In real app, you'd load actual PDF data
      // This is just a demonstration of the method signature
      final pdfData = _generateSamplePdfData();
      
      final success = await WindowsPrinter.printPdf(
        printerName: _selectedPrinter!,
        data: pdfData,
        copies: _pdfCopies,
      );
      
      setState(() {
        _loading = false;
        _status = success ? 'PDF printed successfully ($_pdfCopies copies)' : 'Failed to print PDF';
      });
    } catch (e) {
      setState(() {
        _loading = false;
        _status = 'Error: $e';
      });
    }
  }

  Uint8List _generateSamplePdfData() {
    // In a real application, you would load actual PDF file data
    // For demo purposes, we'll use placeholder data
    final sampleText = 'Sample PDF content for demonstration';
    return Uint8List.fromList(utf8.encode(sampleText));
  }

  // TEXT MODE DEMONSTRATION (for comparison)
  Future<void> _printTextModeComparison() async {
    if (_selectedPrinter == null) return;

    setState(() {
      _loading = true;
      _status = 'Printing text mode comparison...';
    });

    try {
      final textContent = 'TEXT MODE COMPARISON\n'
          'Date: ${DateTime.now()}\n'
          'Printer: $_selectedPrinter\n'
          '-------------------------\n'
          'This uses TEXT datatype\n'
          'Notice the difference in quality\n'
          'compared to other methods.\n'
          '-------------------------\n\n\n';

      final success = await WindowsPrinter.printRawData(
        printerName: _selectedPrinter!,
        data: Uint8List.fromList(utf8.encode(textContent)),
        useRawDatatype: false, // Use TEXT mode for comparison
      );
      
      setState(() {
        _loading = false;
        _status = success ? 'Text mode sample printed' : 'Failed to print text mode sample';
      });
    } catch (e) {
      setState(() {
        _loading = false;
        _status = 'Error: $e';
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Windows Printer Complete Demo'),
        actions: [
          IconButton(
            icon: const Icon(Icons.refresh),
            onPressed: _loadPrinters,
          ),
        ],
      ),
      body: _loading
          ? const Center(child: CircularProgressIndicator())
          : SingleChildScrollView(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  // Status Section
                  Container(
                    width: double.infinity,
                    padding: const EdgeInsets.all(16.0),
                    color: Colors.grey[100],
                    child: Text(
                      'Status: $_status',
                      style: Theme.of(context).textTheme.bodyLarge,
                    ),
                  ),
                  
                  // Printer Selection Section
                  Padding(
                    padding: const EdgeInsets.all(16.0),
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Text('Printer Selection', style: Theme.of(context).textTheme.titleLarge),
                        const SizedBox(height: 8),
                        Row(
                          children: [
                            Expanded(
                              child: DropdownButton<String>(
                                isExpanded: true,
                                hint: const Text('Select a printer'),
                                value: _selectedPrinter,
                                items: _printers.map((printer) {
                                  return DropdownMenuItem<String>(
                                    value: printer,
                                    child: Text(printer),
                                  );
                                }).toList(),
                                onChanged: _selectPrinter,
                              ),
                            ),
                            const SizedBox(width: 16),
                            ElevatedButton(
                              onPressed: _selectedPrinter == null ? null : _setDefaultPrinter,
                              child: const Text('Set Default'),
                            ),
                            const SizedBox(width: 8),
                            ElevatedButton(
                              onPressed: _selectedPrinter == null ? null : _openPrinterProperties,
                              child: const Text('Properties'),
                            ),
                          ],
                        ),
                        if (_selectedPrinter != null) ...[
                          const SizedBox(height: 16),
                          Row(
                            children: [
                              Checkbox(
                                value: _isThermalPrinter,
                                onChanged: (value) => setState(() => _isThermalPrinter = value ?? false),
                              ),
                              const Text('This is a thermal/receipt printer'),
                            ],
                          ),
                        ],
                      ],
                    ),
                  ),

                  const Divider(),

                  // Print Methods Section
                  if (_selectedPrinter != null) ...[
                    Padding(
                      padding: const EdgeInsets.all(16.0),
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          Text('Print Methods Demo', style: Theme.of(context).textTheme.titleLarge),
                          const SizedBox(height: 16),

                          // Thermal Printer Section
                          if (_isThermalPrinter) ...[
                            Container(
                              width: double.infinity,
                              padding: const EdgeInsets.all(12),
                              decoration: BoxDecoration(
                                color: Colors.green[50],
                                border: Border.all(color: Colors.green),
                                borderRadius: BorderRadius.circular(8),
                              ),
                              child: Column(
                                crossAxisAlignment: CrossAxisAlignment.start,
                                children: [
                                  Text('🖨️ THERMAL PRINTER METHODS', 
                                       style: TextStyle(fontWeight: FontWeight.bold, color: Colors.green[800])),
                                  const SizedBox(height: 8),
                                  const Text('Use esc_pos_utils for programmatic control of thermal printers'),
                                  const SizedBox(height: 12),
                                  Wrap(
                                    spacing: 8,
                                    runSpacing: 8,
                                    children: [
                                      ElevatedButton.icon(
                                        icon: const Icon(Icons.receipt),
                                        label: const Text('Print ESC/POS Receipt'),
                                        onPressed: _printThermalReceipt,
                                        style: ElevatedButton.styleFrom(backgroundColor: Colors.green),
                                      ),
                                    ],
                                  ),
                                ],
                              ),
                            ),
                            const SizedBox(height: 16),
                          ],

                          // Regular Printer Section
                          if (!_isThermalPrinter) ...[
                            Container(
                              width: double.infinity,
                              padding: const EdgeInsets.all(12),
                              decoration: BoxDecoration(
                                color: Colors.blue[50],
                                border: Border.all(color: Colors.blue),
                                borderRadius: BorderRadius.circular(8),
                              ),
                              child: Column(
                                crossAxisAlignment: CrossAxisAlignment.start,
                                children: [
                                  Text('🖨️ REGULAR PRINTER METHODS', 
                                       style: TextStyle(fontWeight: FontWeight.bold, color: Colors.blue[800])),
                                  const SizedBox(height: 8),
                                  const Text('High-quality formatted output for office printers'),
                                  const SizedBox(height: 12),
                                  
                                  // Font Settings
                                  Row(
                                    children: [
                                      Expanded(
                                        child: DropdownButton<String>(
                                          value: _selectedFont,
                                          items: ['Courier New', 'Arial', 'Times New Roman', 'Verdana']
                                              .map((font) => DropdownMenuItem(value: font, child: Text(font)))
                                              .toList(),
                                          onChanged: (font) => setState(() => _selectedFont = font!),
                                        ),
                                      ),
                                      const SizedBox(width: 16),
                                      Text('Size: $_fontSize'),
                                      Slider(
                                        value: _fontSize.toDouble(),
                                        min: 8,
                                        max: 24,
                                        divisions: 16,
                                        onChanged: (value) => setState(() => _fontSize = value.round()),
                                      ),
                                    ],
                                  ),
                                  
                                  const SizedBox(height: 8),
                                  ElevatedButton.icon(
                                    icon: const Icon(Icons.text_fields),
                                    label: const Text('Print Rich Text (GDI)'),
                                    onPressed: _printRichTextDocument,
                                    style: ElevatedButton.styleFrom(backgroundColor: Colors.blue),
                                  ),
                                ],
                              ),
                            ),
                            const SizedBox(height: 16),
                          ],

                          // Universal Methods Section
                          Container(
                            width: double.infinity,
                            padding: const EdgeInsets.all(12),
                            decoration: BoxDecoration(
                              color: Colors.orange[50],
                              border: Border.all(color: Colors.orange),
                              borderRadius: BorderRadius.circular(8),
                            ),
                            child: Column(
                              crossAxisAlignment: CrossAxisAlignment.start,
                              children: [
                                Text('🖨️ UNIVERSAL METHODS', 
                                     style: TextStyle(fontWeight: FontWeight.bold, color: Colors.orange[800])),
                                const SizedBox(height: 8),
                                const Text('Works with both thermal and regular printers'),
                                const SizedBox(height: 12),
                                
                                Wrap(
                                  spacing: 8,
                                  runSpacing: 8,
                                  children: [
                                    ElevatedButton.icon(
                                      icon: const Icon(Icons.picture_as_pdf),
                                      label: Text('Print PDF ($_pdfCopies copies)'),
                                      onPressed: _printPdfSample,
                                      style: ElevatedButton.styleFrom(backgroundColor: Colors.orange),
                                    ),
                                    ElevatedButton.icon(
                                      icon: const Icon(Icons.text_snippet),
                                      label: const Text('Print Text Mode'),
                                      onPressed: _printTextModeComparison,
                                      style: ElevatedButton.styleFrom(backgroundColor: Colors.grey),
                                    ),
                                  ],
                                ),
                                
                                const SizedBox(height: 8),
                                Row(
                                  children: [
                                    const Text('PDF Copies: '),
                                    Slider(
                                      value: _pdfCopies.toDouble(),
                                      min: 1,
                                      max: 5,
                                      divisions: 4,
                                      label: _pdfCopies.toString(),
                                      onChanged: (value) => setState(() => _pdfCopies = value.round()),
                                    ),
                                  ],
                                ),
                              ],
                            ),
                          ),
                        ],
                      ),
                    ),

                    const Divider(),

                    // Method Usage Guide
                    Padding(
                      padding: const EdgeInsets.all(16.0),
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          Text('📚 Usage Guide', style: Theme.of(context).textTheme.titleLarge),
                          const SizedBox(height: 8),
                          Card(
                            child: Padding(
                              padding: const EdgeInsets.all(12.0),
                              child: Column(
                                crossAxisAlignment: CrossAxisAlignment.start,
                                children: [
                                  const Text('✅ FOR THERMAL PRINTERS:', style: TextStyle(fontWeight: FontWeight.bold, color: Colors.green)),
                                  const Text('• Use printRawData(useRawDatatype: true) with ESC/POS commands'),
                                  const Text('• Use esc_pos_utils package for programmatic control'),
                                  const Text('• Supports: bold, alignment, cutting, cash drawer, beeping'),
                                  const SizedBox(height: 8),
                                  
                                  const Text('✅ FOR REGULAR PRINTERS:', style: TextStyle(fontWeight: FontWeight.bold, color: Colors.blue)),
                                  const Text('• Use printRichTextDocument() for formatted text'),
                                  const Text('• Use printPdf() for PDF documents'),
                                  const Text('• Supports: fonts, sizes, bold, italic, large text'),
                                  const SizedBox(height: 8),
                                  
                                  const Text('❌ AVOID:', style: TextStyle(fontWeight: FontWeight.bold, color: Colors.red)),
                                  const Text('• printRawData() on regular printers (poor quality)'),
                                  const Text('• printRichTextDocument() on thermal printers (unreliable)'),
                                ],
                              ),
                            ),
                          ),
                        ],
                      ),
                    ),

                    // Printer Details Section
                    if (_printerProperties != null) ...[
                      const Divider(),
                      Padding(
                        padding: const EdgeInsets.all(16.0),
                        child: Row(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            // Printer Properties
                            Expanded(
                              child: Column(
                                crossAxisAlignment: CrossAxisAlignment.start,
                                children: [
                                  Text('Printer Properties', style: Theme.of(context).textTheme.titleMedium),
                                  const SizedBox(height: 8),
                                  SizedBox(
                                    height: 200,
                                    child: ListView(
                                      children: _printerProperties!.entries.map((entry) {
                                        return Card(
                                          child: Padding(
                                            padding: const EdgeInsets.all(8.0),
                                            child: Column(
                                              crossAxisAlignment: CrossAxisAlignment.start,
                                              children: [
                                                Text(entry.key, style: const TextStyle(fontWeight: FontWeight.bold)),
                                                Text(entry.value.toString()),
                                              ],
                                            ),
                                          ),
                                        );
                                      }).toList(),
                                    ),
                                  ),
                                ],
                              ),
                            ),
                            
                            const SizedBox(width: 16),
                            
                            // Paper Sizes
                            if (_paperSizeDetails != null)
                              Expanded(
                                child: Column(
                                  crossAxisAlignment: CrossAxisAlignment.start,
                                  children: [
                                    Text('Paper Sizes', style: Theme.of(context).textTheme.titleMedium),
                                    const SizedBox(height: 8),
                                    Text('Current: ${_paperSizeDetails!["currentPaperName"] ?? "Unknown"}',
                                         style: const TextStyle(fontWeight: FontWeight.bold)),
                                    const SizedBox(height: 8),
                                    SizedBox(
                                      height: 200,
                                      child: ListView.builder(
                                        itemCount: (_paperSizeDetails!["availablePaperSizes"] as List?)?.length ?? 0,
                                        itemBuilder: (context, index) {
                                          final paperSize = (_paperSizeDetails!["availablePaperSizes"] as List)[index];
                                          final bool isSelected = paperSize["isSelected"] == true;
                                          
                                          return Card(
                                            color: isSelected ? Colors.blue.shade100 : null,
                                            child: ListTile(
                                              dense: true,
                                              title: Text(paperSize["name"]),
                                              subtitle: Text('${paperSize["width"]}x${paperSize["height"]}'),
                                            ),
                                          );
                                        },
                                      ),
                                    ),
                                  ],
                                ),
                              ),
                          ],
                        ),
                      ),
                    ],
                  ],
                ],
              ),
            ),
    );
  }
}
