#include "desktop_screenshot_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <atlimage.h>
#include <codecvt>
#include <map>

#include <cstdint>

#include <iostream>
#include <vector>
#include <fstream>

#include <memory>
#include <sstream>

namespace desktop_screenshot {

    HBITMAP CaptureScreen();

    std::vector<BYTE> Hbitmap2PNG(HBITMAP hbitmap);

// static
    void DesktopScreenshotPlugin::RegisterWithRegistrar(
            flutter::PluginRegistrarWindows *registrar) {
        auto channel =
                std::make_unique < flutter::MethodChannel < flutter::EncodableValue >> (
                        registrar->messenger(), "desktop_screenshot",
                                &flutter::StandardMethodCodec::GetInstance());

        auto plugin = std::make_unique<DesktopScreenshotPlugin>();

        channel->SetMethodCallHandler(
                [plugin_pointer = plugin.get()](const auto &call, auto result) {
                    plugin_pointer->HandleMethodCall(call, std::move(result));
                });

        registrar->AddPlugin(std::move(plugin));
    }

    DesktopScreenshotPlugin::DesktopScreenshotPlugin() {}

    DesktopScreenshotPlugin::~DesktopScreenshotPlugin() {}

    void DesktopScreenshotPlugin::HandleMethodCall(
            const flutter::MethodCall <flutter::EncodableValue> &method_call,
            std::unique_ptr <flutter::MethodResult<flutter::EncodableValue>> result) {
        if (method_call.method_name().compare("getPlatformVersion") == 0) {
            std::ostringstream version_stream;
            version_stream << "Windows ";
            if (IsWindows10OrGreater()) {
                version_stream << "10+";
            } else if (IsWindows8OrGreater()) {
                version_stream << "8";
            } else if (IsWindows7OrGreater()) {
                version_stream << "7";
            }
            result->Success(flutter::EncodableValue(version_stream.str()));
        } else if (method_call.method_name().compare("getScreenshot") == 0) {
            HBITMAP bitmap = CaptureScreen();
            if (bitmap) {
                std::vector<BYTE> pngBuf = Hbitmap2PNG(bitmap);
                result->Success(flutter::EncodableValue(pngBuf));
                pngBuf.clear();
                DeleteObject(bitmap);
            } else {
                result->Error("INVALID_IMAGE_DATA", "Failed to capture valid image data");
            }
        } else {
            result->NotImplemented();
        }
    }

    HBITMAP CaptureScreen() {
        // Отримуємо bounds віртуального екрану
        int left = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int top = GetSystemMetrics(SM_YVIRTUALSCREEN);
        int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

        // Створюємо DC для екрану
        HDC hdcScreen = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
        if (!hdcScreen) {
            return nullptr;
        }

        // Створюємо сумісний DC
        HDC hdcMemDC = CreateCompatibleDC(hdcScreen);
        if (!hdcMemDC) {
            DeleteDC(hdcScreen);
            return nullptr;
        }

        // Створюємо bitmap
        HBITMAP hbitmap = CreateCompatibleBitmap(hdcScreen, width, height);
        if (!hbitmap) {
            DeleteDC(hdcMemDC);
            DeleteDC(hdcScreen);
            return nullptr;
        }

        // Вибираємо bitmap
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMemDC, hbitmap);

        // Копіюємо екран
        BOOL success = BitBlt(
                hdcMemDC,
                0, 0,
                width, height,
                hdcScreen,
                left, top,
                SRCCOPY | CAPTUREBLT
        );

        // Очищаємо
        SelectObject(hdcMemDC, hOldBitmap);
        DeleteDC(hdcMemDC);
        DeleteDC(hdcScreen);

        if (!success) {
            DeleteObject(hbitmap);
            return nullptr;
        }

        return hbitmap;
    }

    std::vector<BYTE> Hbitmap2PNG(HBITMAP hbitmap) {
        std::vector<BYTE> buf;
        if (hbitmap != NULL) {
            IStream* stream = NULL;
            CreateStreamOnHGlobal(0, TRUE, &stream);
            CImage image;
            ULARGE_INTEGER liSize;

            // screenshot to png and save to stream
            image.Attach(hbitmap);
            image.Save(stream, Gdiplus::ImageFormatPNG);
            IStream_Size(stream, &liSize);
            DWORD len = liSize.LowPart;
            IStream_Reset(stream);
            buf.resize(len);
            IStream_Read(stream, &buf[0], len);
            stream->Release();
        }
        return buf;
    }

}  // namespace desktop_screenshot
//#include "desktop_screenshot_plugin.h"
//
//// This must be included before many other Windows headers.
//#include <windows.h>
//
//// For getPlatformVersion; remove unless needed for your plugin implementation.
//#include <VersionHelpers.h>
//
//#include <flutter/method_channel.h>
//#include <flutter/plugin_registrar_windows.h>
//#include <flutter/standard_method_codec.h>
//
//#include <atlimage.h>
//#include <codecvt>
//#include <map>
//
//#include <cstdint>
//
//#include <iostream>
//#include <vector>
//#include <fstream>
//
//#include <memory>
//#include <sstream>
//
//namespace desktop_screenshot {
//
//    HBITMAP CaptureScreen();
//
//    std::vector<BYTE> Hbitmap2PNG(HBITMAP hbitmap);
//
//// static
//    void DesktopScreenshotPlugin::RegisterWithRegistrar(
//            flutter::PluginRegistrarWindows *registrar) {
//        auto channel =
//                std::make_unique < flutter::MethodChannel < flutter::EncodableValue >> (
//                        registrar->messenger(), "desktop_screenshot",
//                                &flutter::StandardMethodCodec::GetInstance());
//
//        auto plugin = std::make_unique<DesktopScreenshotPlugin>();
//
//        channel->SetMethodCallHandler(
//                [plugin_pointer = plugin.get()](const auto &call, auto result) {
//                    plugin_pointer->HandleMethodCall(call, std::move(result));
//                });
//
//        registrar->AddPlugin(std::move(plugin));
//    }
//
//    DesktopScreenshotPlugin::DesktopScreenshotPlugin() {}
//
//    DesktopScreenshotPlugin::~DesktopScreenshotPlugin() {}
//
//    void DesktopScreenshotPlugin::HandleMethodCall(
//            const flutter::MethodCall <flutter::EncodableValue> &method_call,
//            std::unique_ptr <flutter::MethodResult<flutter::EncodableValue>> result) {
//        if (method_call.method_name().compare("getPlatformVersion") == 0) {
//            std::ostringstream version_stream;
//            version_stream << "Windows ";
//            if (IsWindows10OrGreater()) {
//                version_stream << "10+";
//            } else if (IsWindows8OrGreater()) {
//                version_stream << "8";
//            } else if (IsWindows7OrGreater()) {
//                version_stream << "7";
//            }
//            result->Success(flutter::EncodableValue(version_stream.str()));
//        } else if (method_call.method_name().compare("getScreenshot") == 0) {
//            HBITMAP bitmap = CaptureScreen();
//            if (bitmap) {
//                std::vector<BYTE> pngBuf = Hbitmap2PNG(bitmap);
//                result->Success(flutter::EncodableValue(pngBuf));
//                pngBuf.clear();
//                DeleteObject(bitmap);
//            } else {
//                result->Error("INVALID_IMAGE_DATA", "Failed to capture valid image data");
//            }
//        } else {
//            result->NotImplemented();
//        }
//    }
//
//    HBITMAP CaptureScreen() {
//        // Отримуємо координати віртуального екрану (всі монітори разом)
//        int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
//        int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
//        int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
//        int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
//
//        HDC hdcScreen = GetDC(NULL);
//        HDC hdcMemDC = CreateCompatibleDC(hdcScreen);
//
//        if (!hdcMemDC) {
//            ReleaseDC(NULL, hdcScreen);
//            return nullptr;
//        }
//
//        // Створюємо bitmap з розміром всього віртуального екрану
//        HBITMAP hbitmap = CreateCompatibleBitmap(hdcScreen, width, height);
//
//        if (!hbitmap) {
//            DeleteDC(hdcMemDC);
//            ReleaseDC(NULL, hdcScreen);
//            return nullptr;
//        }
//
//        // Вибираємо bitmap в memory DC
//        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMemDC, hbitmap);
//
//        // Копіюємо весь віртуальний екран (всі монітори)
//        if (!BitBlt(hdcMemDC, 0, 0, width, height, hdcScreen, x, y, SRCCOPY)) {
//            SelectObject(hdcMemDC, hOldBitmap);
//            DeleteObject(hbitmap);
//            DeleteDC(hdcMemDC);
//            ReleaseDC(NULL, hdcScreen);
//            return nullptr;
//        }
//
//        // Відновлюємо старий bitmap і очищаємо ресурси
//        SelectObject(hdcMemDC, hOldBitmap);
//        DeleteDC(hdcMemDC);
//        ReleaseDC(NULL, hdcScreen);
//
//        return hbitmap;
//    }
//
//    std::vector<BYTE> Hbitmap2PNG(HBITMAP hbitmap) {
//        std::vector<BYTE> buf;
//        if (hbitmap != NULL) {
//            IStream* stream = NULL;
//            CreateStreamOnHGlobal(0, TRUE, &stream);
//            CImage image;
//            ULARGE_INTEGER liSize;
//
//            // screenshot to png and save to stream
//            image.Attach(hbitmap);
//            image.Save(stream, Gdiplus::ImageFormatPNG);
//            IStream_Size(stream, &liSize);
//            DWORD len = liSize.LowPart;
//            IStream_Reset(stream);
//            buf.resize(len);
//            IStream_Read(stream, &buf[0], len);
//            stream->Release();
//        }
//        return buf;
//    }
//
//}  // namespace desktop_screenshot