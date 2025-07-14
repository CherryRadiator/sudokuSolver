#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <string>
#include <vector>
#include <cctype>

class SudokuSolverGUI {
private:
    // GUI components
    Fl_Window* window;
    Fl_Input* grid[9][9];  // 9x9 grid of input fields
    Fl_Button* solve_button;
    
    // Data storage
    int sudoku_board[9][9];     // Current state of the board
    bool original_cells[9][9];  // Track which cells were originally filled
    
    // Constants for layout
    static const int CELL_SIZE = 40;
    static const int CELL_MARGIN = 2;
    static const int GRID_START_X = 20;
    static const int GRID_START_Y = 20;
    static const int WINDOW_WIDTH = 500;
    static const int WINDOW_HEIGHT = 500;

public:
    SudokuSolverGUI() {
        // Initialize the board and tracking arrays
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                sudoku_board[i][j] = 0;
                original_cells[i][j] = false;
            }
        }
        
        create_window();
        create_grid();
        create_solve_button();
    }
    
    ~SudokuSolverGUI() {
        delete window;
    }
    
    void show() {
        window->show();
    }

private:
    /**
     * Creates the main window for the application
     */
    void create_window() {
        window = new Fl_Window(WINDOW_WIDTH, WINDOW_HEIGHT, "Sudoku Solver");
        window->color(FL_WHITE);
    }
    
    /**
     * Creates the 9x9 grid of input fields
     * Each cell is positioned to form a proper Sudoku grid layout
     */
    void create_grid() {
        for (int row = 0; row < 9; row++) {
            for (int col = 0; col < 9; col++) {
                // Calculate position with extra spacing for 3x3 block separation
                int x = GRID_START_X + col * (CELL_SIZE + CELL_MARGIN) + 
                       (col / 3) * 5;  // Extra space every 3 columns
                int y = GRID_START_Y + row * (CELL_SIZE + CELL_MARGIN) + 
                       (row / 3) * 5;  // Extra space every 3 rows
                
                // Create input field for this cell
                grid[row][col] = new Fl_Input(x, y, CELL_SIZE, CELL_SIZE);
                grid[row][col]->textsize(18);
                grid[row][col]->maximum_size(1);  // Only allow single character
                grid[row][col]->color(FL_WHITE);
                grid[row][col]->textcolor(FL_BLACK);
                
                // Set callback for input validation
                grid[row][col]->callback(input_callback, this);
                grid[row][col]->when(FL_WHEN_CHANGED);
            }
        }
    }
    
    /**
     * Creates the solve button below the grid
     */
    void create_solve_button() {
        int button_y = GRID_START_Y + 9 * (CELL_SIZE + CELL_MARGIN) + 4 * 5 + 20;
        solve_button = new Fl_Button(GRID_START_X, button_y, 120, 30, "Solve");
        solve_button->callback(solve_callback, this);
    }
    
    /**
     * Static callback function for input validation
     * Ensures only digits 1-9 are entered in cells
     */
    static void input_callback(Fl_Widget* widget, void* data) {
        Fl_Input* input = (Fl_Input*)widget;
        SudokuSolverGUI* gui = (SudokuSolverGUI*)data;
        
        const char* text = input->value();
        if (text && strlen(text) > 0) {
            char c = text[0];
            // Allow only digits 1-9, reject everything else
            if (c < '1' || c > '9') {
                input->value("");  // Clear invalid input
            }
        }
    }
    
    /**
     * Static callback function for the solve button
     */
    static void solve_callback(Fl_Widget* widget, void* data) {
        SudokuSolverGUI* gui = (SudokuSolverGUI*)data;
        gui->solve_sudoku();
    }
    
    /**
     * Main solving function that coordinates the entire process
     */
    void solve_sudoku() {
        // Step 1: Read current values from GUI into internal board
        read_board_from_gui();
        
        // Step 2: Validate the current board state
        if (!is_valid_board()) {
            fl_alert("Invalid Sudoku configuration! Please check your input.");
            return;
        }
        
        // Step 3: Mark which cells are originally filled
        mark_original_cells();
        
        // Step 4: Solve using backtracking algorithm
        if (solve_backtracking(0, 0)) {
            // Step 5: Update GUI with solution and apply colors
            update_gui_with_solution();
            fl_message("Sudoku solved successfully!");
        } else {
            fl_alert("No solution exists for this Sudoku puzzle!");
        }
    }
    
    /**
     * Reads all values from the GUI input fields into the internal board
     */
    void read_board_from_gui() {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                const char* text = grid[i][j]->value();
                if (text && strlen(text) > 0 && text[0] >= '1' && text[0] <= '9') {
                    sudoku_board[i][j] = text[0] - '0';  // Convert char to int
                } else {
                    sudoku_board[i][j] = 0;  // Empty cell
                }
            }
        }
    }
    
    /**
     * Validates that the current board configuration is legal
     * Checks for duplicate numbers in rows, columns, and 3x3 boxes
     */
    bool is_valid_board() {
        // Check rows
        for (int row = 0; row < 9; row++) {
            bool used[10] = {false};  // Track digits 1-9
            for (int col = 0; col < 9; col++) {
                int num = sudoku_board[row][col];
                if (num != 0) {
                    if (used[num]) return false;  // Duplicate found
                    used[num] = true;
                }
            }
        }
        
        // Check columns
        for (int col = 0; col < 9; col++) {
            bool used[10] = {false};
            for (int row = 0; row < 9; row++) {
                int num = sudoku_board[row][col];
                if (num != 0) {
                    if (used[num]) return false;
                    used[num] = true;
                }
            }
        }
        
        // Check 3x3 boxes
        for (int box_row = 0; box_row < 3; box_row++) {
            for (int box_col = 0; box_col < 3; box_col++) {
                bool used[10] = {false};
                for (int row = box_row * 3; row < box_row * 3 + 3; row++) {
                    for (int col = box_col * 3; col < box_col * 3 + 3; col++) {
                        int num = sudoku_board[row][col];
                        if (num != 0) {
                            if (used[num]) return false;
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
    void mark_original_cells() {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
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
    bool solve_backtracking(int row, int col) {
        // Base case: if we've processed all rows, solution is complete
        if (row == 9) {
            return true;
        }
        
        // Move to next row when we reach the end of current row
        if (col == 9) {
            return solve_backtracking(row + 1, 0);
        }
        
        // If cell is already filled, move to next cell
        if (sudoku_board[row][col] != 0) {
            return solve_backtracking(row, col + 1);
        }
        
        // Try digits 1-9 for empty cell
        for (int num = 1; num <= 9; num++) {
            if (is_safe(row, col, num)) {
                sudoku_board[row][col] = num;  // Place number
                
                // Recursively solve remaining cells
                if (solve_backtracking(row, col + 1)) {
                    return true;  // Solution found
                }
                
                // Backtrack: remove number and try next
                sudoku_board[row][col] = 0;
            }
        }
        
        return false;  // No solution found
    }
    
    /**
     * Checks if it's safe to place a number at given position
     * @param row Row index
     * @param col Column index
     * @param num Number to place (1-9)
     * @return true if placement is valid, false otherwise
     */
    bool is_safe(int row, int col, int num) {
        // Check row for conflicts
        for (int j = 0; j < 9; j++) {
            if (sudoku_board[row][j] == num) {
                return false;
            }
        }
        
        // Check column for conflicts
        for (int i = 0; i < 9; i++) {
            if (sudoku_board[i][col] == num) {
                return false;
            }
        }
        
        // Check 3x3 box for conflicts
        int box_start_row = (row / 3) * 3;
        int box_start_col = (col / 3) * 3;
        for (int i = box_start_row; i < box_start_row + 3; i++) {
            for (int j = box_start_col; j < box_start_col + 3; j++) {
                if (sudoku_board[i][j] == num) {
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
    void update_gui_with_solution() {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                // Convert number to string for display
                char buffer[2];
                sprintf(buffer, "%d", sudoku_board[i][j]);
                grid[i][j]->value(buffer);
                
                // Apply color coding
                if (original_cells[i][j]) {
                    // Original user input - keep default appearance
                    grid[i][j]->color(FL_WHITE);
                    grid[i][j]->textcolor(FL_BLACK);
                } else {
                    // Solver-filled cell - make it green
                    grid[i][j]->color(FL_GREEN);
                    grid[i][j]->textcolor(FL_BLACK);
                }
            }
        }
        
        // Refresh the display
        window->redraw();
    }
};

/**
 * Main function - entry point of the application
 */
int main() {
    // Create and show the GUI
    SudokuSolverGUI sudoku_gui;
    sudoku_gui.show();
    
    // Start the FLTK event loop
    return Fl::run();
}

/*
USAGE INSTRUCTIONS:
===================

1. Compilation:
   g++ -o sudoku_solver sudoku_solver.cpp `fltk-config --cxxflags --ldflags`

2. Running the Application:
   ./sudoku_solver

3. How to Use:
   - The application displays a 9x9 grid of input fields
   - Enter digits 1-9 in cells where you have clues
   - Leave cells empty (or they'll be treated as 0) for unknowns
   - Click "Solve" to solve the puzzle
   - Original numbers stay white, solved numbers appear in green
   - The app validates input and shows error messages for invalid configurations

4. Features:
   - Input validation (only accepts digits 1-9)
   - Sudoku rule validation before solving
   - Backtracking algorithm for solving
   - Visual feedback with color coding
   - Error handling for invalid/unsolvable puzzles

5. Example Input:
   Try entering a few numbers in different cells and click solve to see it work.
   The solver will fill in all empty cells while respecting Sudoku rules.
*/