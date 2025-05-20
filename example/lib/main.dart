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
      home: const PrinterPage(),
    );
  }
}

class PrinterPage extends StatefulWidget {
  const PrinterPage({super.key});

  @override
  State<PrinterPage> createState() => _PrinterPageState();
}

class _PrinterPageState extends State<PrinterPage> {
  List<String> _printers = [];
  String? _selectedPrinter;
  Map<String, dynamic>? _printerProperties;
  Map<String, dynamic>? _paperSizeDetails;
  bool _loading = true;
  String _status = '';

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
      // Get printer properties and paper size details in parallel
      final properties = WindowsPrinter.getPrinterProperties(printer);
      final paperDetails = WindowsPrinter.getPaperSizeDetails(printer);

      // Wait for both operations to complete
      final results = await Future.wait([properties, paperDetails]);

      setState(() {
        _printerProperties = results[0];
        _paperSizeDetails = results[1];
        _loading = false;
        _status = 'Printer details loaded';
      });
    } catch (e) {
      setState(() {
        _loading = false;
        _status = 'Error: $e';
      });
    }
  }

  Future<void> _setPaperSize(int paperSizeId) async {
    if (_selectedPrinter == null) return;

    setState(() {
      _loading = true;
      _status = 'Setting paper size...';
    });

    try {
      final success = await WindowsPrinter.setPaperSize(_selectedPrinter!, paperSizeId);
      
      // Reload paper size details
      final paperDetails = await WindowsPrinter.getPaperSizeDetails(_selectedPrinter!);
      
      setState(() {
        _paperSizeDetails = paperDetails;
        _loading = false;
        _status = success ? 'Paper size set successfully' : 'Failed to set paper size';
      });
    } catch (e) {
      setState(() {
        _loading = false;
        _status = 'Error: $e';
      });
    }
  }

  Future<void> _setDefaultPrinter() async {
    if (_selectedPrinter == null) return;

    setState(() {
      _loading = true;
      _status = 'Setting default printer...';
    });

    try {
      final success = await WindowsPrinter.setDefaultPrinter(_selectedPrinter!);
      
      // Reload printer list to update default status
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
    } catch (e) {
      setState(() {
        _status = 'Error: $e';
      });
    }
  }

  Future<void> _printTestPage() async {
    if (_selectedPrinter == null) return;

    setState(() {
      _loading = true;
      _status = 'Printing test page...';
    });

    try {
      // Generate a simple test receipt
      final testContent = 'Test Print\n'
          'Date: ${DateTime.now()}\n'
          'Printer: $_selectedPrinter\n'
          '-------------------------\n'
          'Hello, World!\n'
          '-------------------------\n\n\n\n';
      
      final Uint8List data = Uint8List.fromList(utf8.encode(testContent));
      
      final success = await WindowsPrinter.printRawData(
        printerName: _selectedPrinter,
        data: data,
      );
      
      setState(() {
        _loading = false;
        _status = success ? 'Test page printed successfully' : 'Failed to print test page';
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
        title: const Text('Windows Printer Demo'),
        actions: [
          IconButton(
            icon: const Icon(Icons.refresh),
            onPressed: _loadPrinters,
          ),
        ],
      ),
      body: _loading
          ? const Center(child: CircularProgressIndicator())
          : Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Padding(
                  padding: const EdgeInsets.all(16.0),
                  child: Text(
                    'Status: $_status',
                    style: Theme.of(context).textTheme.bodyLarge,
                  ),
                ),
                const Divider(),
                Padding(
                  padding: const EdgeInsets.all(16.0),
                  child: Row(
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
                        child: const Text('Set as Default'),
                      ),
                      const SizedBox(width: 16),
                      ElevatedButton(
                        onPressed: _selectedPrinter == null ? null : _openPrinterProperties,
                        child: const Text('Properties'),
                      ),
                      const SizedBox(width: 16),
                      ElevatedButton(
                        onPressed: _selectedPrinter == null ? null : _printTestPage,
                        child: const Text('Print Test'),
                      ),
                    ],
                  ),
                ),
                if (_printerProperties != null) ...[
                  const Divider(),
                  Expanded(
                    child: Row(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        // Printer properties
                        Expanded(
                          child: Padding(
                            padding: const EdgeInsets.all(16.0),
                            child: Column(
                              crossAxisAlignment: CrossAxisAlignment.start,
                              children: [
                                Text(
                                  'Printer Properties',
                                  style: Theme.of(context).textTheme.titleLarge,
                                ),
                                const SizedBox(height: 8),
                                Expanded(
                                  child: ListView(
                                    children: _printerProperties!.entries.map((entry) {
                                      return Card(
                                        child: Padding(
                                          padding: const EdgeInsets.all(8.0),
                                          child: Column(
                                            crossAxisAlignment: CrossAxisAlignment.start,
                                            children: [
                                              Text(
                                                entry.key,
                                                style: Theme.of(context).textTheme.titleMedium,
                                              ),
                                              const SizedBox(height: 4),
                                              Text(
                                                entry.value.toString(),
                                                style: Theme.of(context).textTheme.bodyMedium,
                                              ),
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
                        ),
                        // Paper size details
                        if (_paperSizeDetails != null)
                          Expanded(
                            child: Padding(
                              padding: const EdgeInsets.all(16.0),
                              child: Column(
                                crossAxisAlignment: CrossAxisAlignment.start,
                                children: [
                                  Text(
                                    'Paper Sizes',
                                    style: Theme.of(context).textTheme.titleLarge,
                                  ),
                                  const SizedBox(height: 8),
                                  Text(
                                    'Current: ${_paperSizeDetails!["currentPaperName"] ?? "Unknown"}',
                                    style: Theme.of(context).textTheme.titleMedium,
                                  ),
                                  const SizedBox(height: 8),
                                  Expanded(
                                    child: ListView.builder(
                                      itemCount: (_paperSizeDetails!["availablePaperSizes"] as List?)?.length ?? 0,
                                      itemBuilder: (context, index) {
                                        final paperSize = (_paperSizeDetails!["availablePaperSizes"] as List)[index] as Map<String, dynamic>;
                                        final bool isSelected = paperSize["isSelected"] == true;
                                        
                                        return Card(
                                          color: isSelected ? Colors.blue.shade100 : null,
                                          child: ListTile(
                                            title: Text(paperSize["name"] as String),
                                            subtitle: Text(
                                              'ID: ${paperSize["id"]}, '
                                              'Width: ${paperSize["width"]}, '
                                              'Height: ${paperSize["height"]}',
                                            ),
                                            trailing: ElevatedButton(
                                              onPressed: isSelected
                                                  ? null
                                                  : () => _setPaperSize(paperSize["id"] as int),
                                              child: const Text('Set'),
                                            ),
                                          ),
                                        );
                                      },
                                    ),
                                  ),
                                ],
                              ),
                            ),
                          ),
                      ],
                    ),
                  ),
                ],
              ],
            ),
    );
  }
}