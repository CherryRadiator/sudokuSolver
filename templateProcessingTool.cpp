#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    // Путь к папке с исходными изображениями
    std::string input_folder = "templates_raw/";
    // Путь к папке, куда сохранять обработанные
    std::string output_folder = "templates_processed/";

    // Создаем выходную папку, если её нет
    if (!fs::exists(output_folder)) {
        fs::create_directory(output_folder);
    }

    // Обрабатываем каждое изображение от 0 до 9
    for (int i = 0; i <= 9; ++i) {
        std::string filename = "" + std::to_string(i) + ".png";
        std::string input_path = input_folder + filename;
        std::string output_path = output_folder + filename;

        // Загружаем изображение
        cv::Mat img = cv::imread(input_path, cv::IMREAD_COLOR);
        if (img.empty()) {
            std::cerr << "Не удалось загрузить изображение: " << input_path << std::endl;
            continue;
        }

        // Преобразуем в градации серого
        cv::Mat gray;
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

        // Бинаризуем с инверсией и автоматическим порогом Оцу
        cv::Mat binary;
        cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);

        // Сохраняем результат
        cv::imwrite(output_path, binary);
        std::cout << "Сохранено: " << output_path << std::endl;
    }

    std::cout << "Обработка завершена!" << std::endl;
    return 0;
}