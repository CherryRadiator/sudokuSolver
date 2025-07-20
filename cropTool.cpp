#include <iostream>
#include <cstdlib>
#include <opencv2/opencv.hpp>

int getOffset(int index, int cell_size, int thick, int thin, int margin) {
    int thick_count = index / 3;
    int thin_count = index - thick_count;
    return thick + margin + index * cell_size + thick_count * thick + thin_count * thin;
}

int cell_size = 113;
int thick = 5;
int thin = 3;
int margin_left = 13;
int margin_top = 555;

int main() {
    // Путь, где будет сохранён исходный скриншот
    std::string tempScreenshotPath = "screen.png";

    // Путь для сохранения обрезанного изображения
    std::string outputImagePath = "cropped_screen.png";

    // Выполняем команду ADB для получения скриншота
    std::cout << "Делаем скриншот с помощью ADB..." << std::endl;
    std::string adbCommand = "adb exec-out screencap -p > " + tempScreenshotPath;
    int adbResult = std::system(adbCommand.c_str());

    if (adbResult != 0) {
        std::cerr << "Ошибка выполнения команды ADB!" << std::endl;
        return 1;
    }

    // Загружаем изображение с помощью OpenCV
    cv::Mat image = cv::imread(tempScreenshotPath, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Не удалось загрузить изображение: " << tempScreenshotPath << std::endl;
        return 1;
    }



    // Задаём область интереса (ROI)
    // толщина толстой линии 5px
    // толщина тонкой линии 3px
    // отступ слева 13px
    // отступ сверху 555px
    // ширина ячейки 113px
    
    // // working version
    // int width = 113;
    // // int heigh = width;
    // // int x = 18 + (width+4) * 0;
    // // int y = 560 + (heigh+4) * 0;

    // int thickLineThickness = 5;
    // int thinLineThickness = 3; 

    // // int testwidth = 9;
    // int heigh = width;
    // int x = 13 + thickLineThickness;
    // int y = 555 + thickLineThickness;

    int col = 2;
    int row = 2;
    int x = getOffset(col, cell_size, thick, thin, margin_left);
    int y = getOffset(row, cell_size, thick, thin, margin_top);
    cv::Rect roi(x, y, cell_size, cell_size);




    
    // Проверяем, чтобы ROI не выходил за границы изображения
    roi.width = std::min(roi.width, image.cols - roi.x);
    roi.height = std::min(roi.height, image.rows - roi.y);

    if (roi.width <= 0 || roi.height <= 0) {
        std::cerr << "Некорректная область обрезки (ROI)!" << std::endl;
        return 1;
    }

    // Обрезаем изображение
    cv::Mat croppedImage = image(roi);

    // Сохраняем обрезанное изображение
    std::cout << "Сохраняем обрезанное изображение в " << outputImagePath << std::endl;
    if (!cv::imwrite(outputImagePath, croppedImage)) {
        std::cerr << "Не удалось сохранить обрезанное изображение!" << std::endl;
        return 1;
    }

    std::cout << "Готово!" << std::endl;

    return 0;
}