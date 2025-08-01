#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <string>
#include <vector>
#include <cctype>
#include <fstream>
#include <iostream>

class SudokuGUI
{
private:
  // GUI components
  Fl_Window *window;
  Fl_Choice *grid[9][9]; // 9x9 grid of combo boxes
  Fl_Button *write_button;
  Fl_Button *solve_button;
  Fl_Button *clear_button; // Кнопка очистки
  Fl_Button *scan_button;

  // Data storage
  int sudoku_board[9][9];    // Current state of the board
  bool original_cells[9][9]; // Track which cells were originally filled

  // Constants for layout
  static const int CELL_SIZE = 40;
  static const int CELL_MARGIN = 2;
  static const int GRID_START_X = 20;
  static const int GRID_START_Y = 50; // Move grid down for title
  static const int WINDOW_WIDTH = 450;
  static const int WINDOW_HEIGHT = 550;

public:
  SudokuGUI()
  {
    // Initialize the board and tracking arrays
    for (int i = 0; i < 9; i++)
    {
      for (int j = 0; j < 9; j++)
      {
        sudoku_board[i][j] = 0;
        original_cells[i][j] = false;
      }
    }

    create_window();
    create_grid();
    create_solve_button();
  }

  ~SudokuGUI()
  {
    delete window;
  }

  void show()
  {
    window->show();
  }

private:
  /**
   * Creates the main window for the application
   */
  void create_window()
  {
    window = new Fl_Window(WINDOW_WIDTH, WINDOW_HEIGHT, "Sudoku Solver");
    window->color(FL_WHITE);
    // Add a title label above the grid
    Fl_Box *title = new Fl_Box(GRID_START_X, 10, WINDOW_WIDTH - 2 * GRID_START_X, 30, "Sudoku Solver");
    title->labelfont(FL_BOLD);
    title->labelsize(24);
    title->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
  }

  /**
   * Creates the 9x9 grid of combo boxes
   * Each cell is positioned to form a proper Sudoku grid layout
   */
  void create_grid()
  {
    Fl_Color block_colors[2] = {FL_WHITE, fl_rgb_color(230, 230, 230)}; // white and light gray
    for (int row = 0; row < 9; row++)
    {
      for (int col = 0; col < 9; col++)
      {
        // Increase extra spacing for 3x3 block separation
        int x = GRID_START_X + col * (CELL_SIZE + CELL_MARGIN) + (col / 3) * 12;
        int y = GRID_START_Y + row * (CELL_SIZE + CELL_MARGIN) + (row / 3) * 12;

        grid[row][col] = new Fl_Choice(x, y, CELL_SIZE, CELL_SIZE);
        grid[row][col]->textsize(18);

        // Add choices "0" (empty), "1", ..., "9"
        grid[row][col]->add("0");
        for (int n = 1; n <= 9; n++)
        {
          char buf[2];
          sprintf(buf, "%d", n);
          grid[row][col]->add(buf);
        }
        grid[row][col]->value(0); // Default to "0" (empty)
        // Alternate block background color
        int block_color_idx = ((row / 3) + (col / 3)) % 2;
        grid[row][col]->color(block_colors[block_color_idx]);
        grid[row][col]->textcolor(FL_BLACK);
      }
    }
  }

  /**
   * Creates the solve button below the grid
   */
  void create_solve_button()
  {
    int button_y = GRID_START_Y + 9 * (CELL_SIZE + CELL_MARGIN) + 4 * 5 + 20;
    int button_x = (WINDOW_WIDTH - 120) / 2; // Center button
    solve_button = new Fl_Button(button_x, button_y, 120, 30, "Solve");
    solve_button->callback(solve_callback, this);

    // Кнопка очистки справа от Solve
    clear_button = new Fl_Button(button_x + 140, button_y, 120, 30, "Clear");
    clear_button->callback(clear_callback, this);

    write_button = new Fl_Button(button_x - 140, button_y, 120, 30, "Write");
    write_button->callback(write_callback, this);

    scan_button = new Fl_Button(button_x, button_y + 40, 120, 30, "Scan");
    scan_button->callback(scan_callback, this);
  }

  /**
   * Static callback function for the solve button
   */
  static void solve_callback(Fl_Widget *widget, void *data)
  {
    SudokuGUI *gui = (SudokuGUI *)data;
    gui->solve_sudoku();
  }

  static void clear_callback(Fl_Widget *widget, void *data)
  {
    SudokuGUI *gui = (SudokuGUI *)data;
    gui->clear_board();
  }

  static void write_callback(Fl_Widget *widget, void *data) {
    SudokuGUI *gui = (SudokuGUI *)data;
    gui->write_grid();
  }

  static void scan_callback(Fl_Widget *widget, void *data) {
      int result = std::system("./matchTemplate"); // или ./extract_digits
      if (result != 0) {
          std::cerr << "Error: OCR script failed with code " << result << std::endl;
      }
  }

  /**
   * Main solving function that coordinates the entire process
   */
  void solve_sudoku()
  {
    // Step 1: Read current values from GUI into internal board
    read_board_from_gui();

    // Step 2: Validate the current board state
    if (!is_valid_board())
    {
      fl_alert("Invalid Sudoku configuration! Please check your input.");
      return;
    }

    // Step 3: Mark which cells are originally filled
    mark_original_cells();

    // Step 4: Solve using backtracking algorithm
    if (solve_backtracking(0, 0))
    {
      // Step 5: Update GUI with solution and apply colors
      update_gui_with_solution();
      fl_message("Sudoku solved successfully!");
    }
    else
    {
      fl_alert("No solution exists for this Sudoku puzzle!");
    }
  }

  /**
   * Reads all values from the GUI combo boxes into the internal board
   */
  void read_board_from_gui()
  {
    for (int i = 0; i < 9; i++)
    {
      for (int j = 0; j < 9; j++)
      {
        int idx = grid[i][j]->value();
        // idx 0 is "0" (empty), idx 1 is "1", ..., idx 9 is "9"
        sudoku_board[i][j] = idx; // 0 means empty
      }
    }
  }

  /**
   * Validates that the current board configuration is legal
   * Checks for duplicate numbers in rows, columns, and 3x3 boxes
   */
  bool is_valid_board()
  {
    // Check rows
    for (int row = 0; row < 9; row++)
    {
      bool used[10] = {false}; // Track digits 1-9
      for (int col = 0; col < 9; col++)
      {
        int num = sudoku_board[row][col];
        if (num != 0)
        {
          if (used[num])
            return false; // Duplicate found
          used[num] = true;
        }
      }
    }

    // Check columns
    for (int col = 0; col < 9; col++)
    {
      bool used[10] = {false};
      for (int row = 0; row < 9; row++)
      {
        int num = sudoku_board[row][col];
        if (num != 0)
        {
          if (used[num])
            return false;
          used[num] = true;
        }
      }
    }

    // Check 3x3 boxes
    for (int box_row = 0; box_row < 3; box_row++)
    {
      for (int box_col = 0; box_col < 3; box_col++)
      {
        bool used[10] = {false};
        for (int row = box_row * 3; row < box_row * 3 + 3; row++)
        {
          for (int col = box_col * 3; col < box_col * 3 + 3; col++)
          {
            int num = sudoku_board[row][col];
            if (num != 0)
            {
              if (used[num])
                return false;
              used[num] = true;
            }
          }
        }
      }
    }

    return true;
  }

  /**
   * Records which cells were originally filled by the user
   */
  void mark_original_cells()
  {
    for (int i = 0; i < 9; i++)
    {
      for (int j = 0; j < 9; j++)
      {
        original_cells[i][j] = (sudoku_board[i][j] != 0);
      }
    }
  }

  /**
   * Backtracking algorithm to solve the Sudoku puzzle
   * @param row Current row being processed
   * @param col Current column being processed
   * @return true if solution found, false otherwise
   */
  bool solve_backtracking(int row, int col)
  {
    // Base case: if we've processed all rows, solution is complete
    if (row == 9)
    {
      return true;
    }

    // Move to next row when we reach the end of current row
    if (col == 9)
    {
      return solve_backtracking(row + 1, 0);
    }

    // If cell is already filled, move to next cell
    if (sudoku_board[row][col] != 0)
    {
      return solve_backtracking(row, col + 1);
    }

    // Try digits 1-9 for empty cell
    for (int num = 1; num <= 9; num++)
    {
      if (is_safe(row, col, num))
      {
        sudoku_board[row][col] = num; // Place number

        // Recursively solve remaining cells
        if (solve_backtracking(row, col + 1))
        {
          return true; // Solution found
        }

        // Backtrack: remove number and try next
        sudoku_board[row][col] = 0;
      }
    }

    return false; // No solution found
  }

  /**
   * Checks if it's safe to place a number at given position
   * @param row Row index
   * @param col Column index
   * @param num Number to place (1-9)
   * @return true if placement is valid, false otherwise
   */
  bool is_safe(int row, int col, int num)
  {
    // Check row for conflicts
    for (int j = 0; j < 9; j++)
    {
      if (sudoku_board[row][j] == num)
      {
        return false;
      }
    }

    // Check column for conflicts
    for (int i = 0; i < 9; i++)
    {
      if (sudoku_board[i][col] == num)
      {
        return false;
      }
    }

    // Check 3x3 box for conflicts
    int box_start_row = (row / 3) * 3;
    int box_start_col = (col / 3) * 3;
    for (int i = box_start_row; i < box_start_row + 3; i++)
    {
      for (int j = box_start_col; j < box_start_col + 3; j++)
      {
        if (sudoku_board[i][j] == num)
        {
          return false;
        }
      }
    }

    return true;
  }

  /**
   * Updates the GUI with the solved puzzle and applies color coding
   * Original cells remain default color, solver-filled cells become green
   */
  void update_gui_with_solution()
  {
    for (int i = 0; i < 9; i++)
    {
      for (int j = 0; j < 9; j++)
      {
        // Set combo box value
        grid[i][j]->value(sudoku_board[i][j]); // 0 for "0", 1 for "1", etc.

        // Apply color coding
        if (original_cells[i][j])
        {
          // Original user input - keep default appearance
          grid[i][j]->color(FL_WHITE);
          grid[i][j]->textcolor(FL_BLACK);
        }
        else
        {
          // Solver-filled cell - make it green
          grid[i][j]->color(FL_GREEN);
          grid[i][j]->textcolor(FL_BLACK);
        }
      }
    }

    // Refresh the display
    window->redraw();
  }

  void clear_board()
  {
    for (int i = 0; i < 9; i++)
    {
      for (int j = 0; j < 9; j++)
      {
        grid[i][j]->value(0); // "0" (пусто)
        grid[i][j]->color(FL_WHITE);
        grid[i][j]->textcolor(FL_BLACK);
        sudoku_board[i][j] = 0;
        original_cells[i][j] = false;
      }
    }
    window->redraw();
  }

  void write_grid() {
    int row = 0;
    int column = 0;
    int sudoku[9][9];

    std::fstream file("./sudoku.txt");

    for (int row = 0; row < 9; ++row) {
        std::string line;
        if (!std::getline(file, line)) {
            std::cerr << "Error reading line " << row+1 << std::endl;
            return;
        }
        for (int column = 0; column < 9; ++column) {
          char c = line[column];
          sudoku[row][column] = c - '0';
          grid[row][column]->value(sudoku[row][column]);
        }
    }


    file.close();

  }
};

int main() {
  SudokuGUI sudoku_gui;
  sudoku_gui.show();

  return Fl::run();
}