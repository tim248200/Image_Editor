#include <iostream>
#include <vector>
#include <windows.h>
#include <commdlg.h>
#include <cstdint>
#include <gdiplus.h>

// Подключаем системную библиотеку GDI+
using namespace Gdiplus;

int main() {
    std::setlocale(LC_ALL, "Russian");

    // Инициализация графического движка Windows (GDI+)
    ULONG_PTR gdiplusToken;
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // 1. Графическое окно теперь ВИДИТ ВСЕ КАРТИНКИ
    wchar_t fileBuffer[MAX_PATH] = {0};
    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = fileBuffer;
    ofn.nMaxFile = sizeof(fileBuffer);
    // Настраиваем фильтр на любые форматы изображений
    ofn.lpstrFilter = L"Все изображения (*.jpg;*.jpeg;*.png;*.bmp)\0*.jpg;*.jpeg;*.png;*.bmp\0";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;

    std::cout << "Открывается окно... Теперь видны PNG, JPG, JPEG, BMP файлы!\n";
    if (!GetOpenFileNameW(&ofn)) {
        std::cout << "Выбор отменен.\n";
        GdiplusShutdown(gdiplusToken);
        return 0;
    }

    // 2. Открываем ЛЮБУЮ картинку (JPG, PNG или BMP)
    Bitmap* bitmap = Bitmap::FromFile(fileBuffer);
    if (!bitmap || bitmap->GetLastStatus() != Ok) {
        std::cout << "Ошибка открытия файла.\n";
        delete bitmap;
        GdiplusShutdown(gdiplusToken);
        return 0;
    }

    int width = bitmap->GetWidth();
    int height = bitmap->GetHeight();

    // Блокируем пиксели в памяти для быстрой работы
    Rect rect(0, 0, width, height);
    BitmapData bitmapData;
    bitmap->LockBits(&rect, ImageLockModeRead | ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData);

    // Получаем прямой указатель на массив байт пикселей
    uint8_t* pixels = reinterpret_cast<uint8_t*>(bitmapData.Scan0);

    // 3. РАБОТА С КАЖДЫМ ПИКСЕЛЕМ (2 цикла for для создания негатива)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // каждый пиксель занимает ровно 4 байта
            int pixelIndex = (y * width + x) * 4;

            // Инвертируем цвета (Негатив)
            pixels[pixelIndex]     = 255 - pixels[pixelIndex];     // Синий (B)
            pixels[pixelIndex + 1] = 255 - pixels[pixelIndex + 1]; // Зеленый (G)
            pixels[pixelIndex + 2] = 255 - pixels[pixelIndex + 2]; // Красный (R)
            // pixels[pixelIndex + 3] - это Альфа-канал (прозрачность)
        }
    }

    // Разблокируем пиксели после обработки
    bitmap->UnlockBits(&bitmapData);

    // 4. Сохраняем результат в формате BMP рядом с оригиналом
    std::wstring inputPath(fileBuffer);
    std::wstring outputPath = inputPath.substr(0, inputPath.find_last_of(L'.')) + L"_negative.bmp";

    CLSID bmpClsid;
    // Получаем системный идентификатор для сохранения в BMP
    CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &bmpClsid);

    if (bitmap->Save(outputPath.c_str(), &bmpClsid, NULL) == Ok) {
        std::cout << "Успешно! Обработанный файл сохранен рядом с оригиналом в формате _negative.bmp\n";
    } else {
        std::cout << "Ошибка при сохранении файла.\n";
    }

    // Очищаем ресурсы
    delete bitmap;
    GdiplusShutdown(gdiplusToken);
    return 0;
}
