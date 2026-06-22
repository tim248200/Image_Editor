#include <iostream>
#include <windows.h>
#include <commdlg.h>
#include <cstdint>
#include <gdiplus.h>

using namespace Gdiplus;

int main() {
    std::setlocale(LC_ALL, "Russian");

    // Запуск GDI+
    ULONG_PTR gdiplusToken;
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Диалог выбора файла
    wchar_t fileBuffer[MAX_PATH] = {0};
    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = fileBuffer;
    ofn.nMaxFile = sizeof(fileBuffer);
    ofn.lpstrFilter = L"Изображения\0*.jpg;*.jpeg;*.png;*.bmp\0";
    ofn.Flags = OFN_FILEMUSTEXIST;

    std::cout << "Выберите изображение...\n";
    if (!GetOpenFileNameW(&ofn)) {
        std::cout << "Отмена.\n";
        GdiplusShutdown(gdiplusToken);
        return 0;
    }

    // Загрузка картинки
    Bitmap* bitmap = Bitmap::FromFile(fileBuffer);
    if (!bitmap || bitmap->GetLastStatus() != Ok) {
        std::cout << "Ошибка загрузки.\n";
        delete bitmap;
        GdiplusShutdown(gdiplusToken);
        return 0;
    }

    int width = bitmap->GetWidth();
    int height = bitmap->GetHeight();

    // Блокировка пикселей
    Rect rect(0, 0, width, height);
    BitmapData bitmapData;
    bitmap->LockBits(&rect, ImageLockModeRead | ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData);

    uint8_t* pixels = (uint8_t*)bitmapData.Scan0;

    // Негатив (простой перебор всех пикселей)
    for (int i = 0; i < width * height; i++) {
        int pos = i * 4;
        pixels[pos]     = 255 - pixels[pos];     // B
        pixels[pos + 1] = 255 - pixels[pos + 1]; // G
        pixels[pos + 2] = 255 - pixels[pos + 2]; // R
    }

    bitmap->UnlockBits(&bitmapData);

    // Сохранение
    std::wstring path(fileBuffer);
    std::wstring newPath = path.substr(0, path.find_last_of(L'.')) + L"_negative.bmp";

    CLSID bmpClsid;
    CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &bmpClsid);

    if (bitmap->Save(newPath.c_str(), &bmpClsid, NULL) == Ok)
        std::cout << "Готово! Файл сохранён.\n";
    else
        std::cout << "Ошибка сохранения.\n";

    delete bitmap;
    GdiplusShutdown(gdiplusToken);
    return 0;
}