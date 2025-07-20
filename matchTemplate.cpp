#include <iostream>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <fstream>

int getOffset(int index, int cell_size, int thick, int thin, int margin) {
    int thick_count = index / 3;
    int thin_count = index - thick_count;
    return thick + margin + index * cell_size + thick_count * thick + thin_count * thin;
}

int cell_size = 113;
int thick = 5;
int thin = 3;
int margin_left = 13;
// int margin_top = 555; для одиночки
int margin_top = 567; // для мультиплеера 

int main() {
  std::string sudokuGridRawPath = "./sudoku_grid_raw/";
  std::string sudokuGridProcessedPath = "./sudoku_grid/";
  std::string screenRawPath = "screen.png";
  
  std::string inputPath = screenRawPath;
  
  std::cout << "Making screenshot..." << std::endl;
  std::string adbCommand = "adb exec-out screencap -p > " + screenRawPath;
  int adbResult = std::system(adbCommand.c_str());
  if (adbResult != 0) {
    std::cerr << "Failed executing adb command" << std::endl;
    return 1;
  }
  
  cv::Mat image = cv::imread(inputPath, cv::IMREAD_COLOR);
  if (image.empty()) {
    std::cerr << "Failed loading image " << inputPath << std::endl; 
    return 1;
  }
  
  for (int row = 0; row < 9; ++row) {
    for (int column = 0; column < 9; ++column) {
      std::string fileName = "" + std::to_string(row) + '_' + std::to_string(column) + ".png";
      std::string outputPath = sudokuGridRawPath + fileName;
          
    int x = getOffset(column, cell_size, thick, thin, margin_left);
    int y = getOffset(row, cell_size, thick, thin, margin_top);

      cv::Rect roi(x, y, cell_size, cell_size);
    
      roi.width = std::min(roi.width, image.cols - roi.x);
      roi.height = std::min(roi.height, image.rows - roi.y);

      if (roi.width <= 0 || roi.height <= 0) {
          std::cerr << "Некорректная область обрезки (ROI)!" << std::endl;
          return 1;
      }

      cv::Mat croppedImage = image(roi);

      std::cout << "Saving croped image to " << outputPath << std::endl;
      if (!cv::imwrite(outputPath, croppedImage)) {
        std::cerr << "Failed saving image!" << std::endl;
        return 1;
      }

      std::cout << "Cell number " << column << "_" << row << " croped" << std::endl;

    }
  }
  
  //=========================================================================================

  for (int row = 0; row < 9; ++row) {
    for (int column = 0; column < 9; ++column) {
      std::string filename = "" + std::to_string(row) + "_" + std::to_string(column) + ".png";
      std::string inputPath = sudokuGridRawPath + filename;
      std::string outputPath = sudokuGridProcessedPath + filename;

      cv::Mat img = cv::imread(inputPath, cv::IMREAD_COLOR);
      if (img.empty()) {
        std::cerr << "Failed loading image: " << inputPath << std::endl;
        continue;
      }

      cv::Mat gray;
      cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

      cv::Mat binary;
      cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);

      cv::imwrite(outputPath, binary);
      std::cout << "Сохранено: " << outputPath << std::endl;
      }
    }

    std::vector<cv::Mat> templates(10);
    for (int i = 0; i <= 9; ++i) {
      templates[i] = cv::imread("templates_processed/" + std::to_string(i) + ".png", cv::IMREAD_GRAYSCALE);
    }

    int sudoku[9][9];  // 2D массив для хранения результата

    for (int row = 0; row < 9; ++row) {
      for (int column = 0; column < 9; ++column) {
        std::string cellPath = sudokuGridProcessedPath + std::to_string(row) + "_" + std::to_string(column) + ".png";
        cv::Mat cell = cv::imread(cellPath, cv::IMREAD_GRAYSCALE);

        if (cell.empty()) {
          std::cerr << "Не удалось загрузить ячейку: " << cellPath << std::endl;
          sudoku[row][column] = 0;  // если ошибка — пусть будет 0
          continue;
        }

        int bestDigit = 0;
        double bestScore = -1;

        for (int i = 1; i <= 9; ++i) {
          cv::Mat result;
          cv::matchTemplate(cell, templates[i], result, cv::TM_CCOEFF_NORMED);
          double minVal, maxVal;
          cv::minMaxLoc(result, &minVal, &maxVal);

          if (maxVal > bestScore) {
            bestScore = maxVal;
            bestDigit = i;
          }
        }

        // Порог уверенности — цифра только если уверенность высокая
        if (bestScore > 0.9) {
          sudoku[row][column] = bestDigit;
        } else {
          sudoku[row][column] = 0;
        }

        std::cout << "cell " << row << "," << column << " => " << sudoku[row][column]
                  << " (score: " << bestScore << ")" << std::endl;
      }
    }

    std::fstream file("./sudoku.txt");

    for (int row = 0; row < 9; ++row) {
      for (int column = 0; column < 9; ++column) {
        file << sudoku[row][column];
      }
      file << "\n";
    }

    file.close();

    return 0;
}