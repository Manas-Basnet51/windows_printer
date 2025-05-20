#include "include/windows_printer/windows_printer_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "windows_printer_plugin.h"

void WindowsPrinterPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  windows_printer::WindowsPrinterPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
