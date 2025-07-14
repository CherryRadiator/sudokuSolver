#include <iostream>
#include <vector>
#include <string>
#include <sstream>

/**
 * SudokuSolver class - Encapsulates all Sudoku solving logic
 * 
 * Data Structure:
 * - board: 9x9 2D vector of integers (0 = empty, 1-9 = filled)
 * - solutionCount: tracks number of solutions found
 * 
 * Design Philosophy:
 * - Single responsibility: handles input, validation, solving, and output
 * - Extensible: easy to add optimizations like constraint propagation
 * - Clear separation of concerns with well-defined methods
 */
class SudokuSolver {
private:
    // 9x9 board representation: board[row][col] = digit (0 for empty)
    std::vector<std::vector<int>> board;
    
    // Counter for tracking total solutions found
    int solutionCount;
    
    /**
     * Validates if placing 'num' at position (row, col) is legal
     * Checks three Sudoku constraints:
     * 1. Row constraint: no duplicate in the same row
     * 2. Column constraint: no duplicate in the same column  
     * 3. 3x3 box constraint: no duplicate in the same 3x3 sub-grid
     * 
     * @param row: target row (0-8)
     * @param col: target column (0-8)
     * @param num: digit to place (1-9)
     * @return true if placement is valid, false otherwise
     */
    bool isValidPlacement(int row, int col, int num) {
        // Check row constraint: scan entire row for duplicates
        for (int c = 0; c < 9; c++) {
            if (board[row][c] == num) {
                return false; // Found duplicate in row
            }
        }
        
        // Check column constraint: scan entire column for duplicates
        for (int r = 0; r < 9; r++) {
            if (board[r][col] == num) {
                return false; // Found duplicate in column
            }
        }
        
        // Check 3x3 box constraint
        // Calculate top-left corner of the 3x3 box containing (row, col)
        int boxStartRow = (row / 3) * 3; // Integer division gives box boundary
        int boxStartCol = (col / 3) * 3;
        
        // Scan the entire 3x3 box for duplicates
        for (int r = boxStartRow; r < boxStartRow + 3; r++) {
            for (int c = boxStartCol; c < boxStartCol + 3; c++) {
                if (board[r][c] == num) {
                    return false; // Found duplicate in 3x3 box
                }
            }
        }
        
        return true; // All constraints satisfied
    }
    
    /**
     * Finds the next empty cell in the board using row-major order
     * This is where future optimizations can be added:
     * - Most Constrained Variable (MCV): choose cell with fewest possibilities
     * - Degree heuristic: choose cell that constrains the most other cells
     * 
     * @param row: reference to store found row
     * @param col: reference to store found column
     * @return true if empty cell found, false if board is complete
     */
    bool findEmptyCell(int& row, int& col) {
        // Scan board in row-major order (left-to-right, top-to-bottom)
        for (row = 0; row < 9; row++) {
            for (col = 0; col < 9; col++) {
                if (board[row][col] == 0) {
                    return true; // Found empty cell
                }
            }
        }
        return false; // No empty cells = puzzle complete
    }
    
    /**
     * Core backtracking algorithm - recursive function that explores all possibilities
     * 
     * Algorithm steps:
     * 1. Find next empty cell
     * 2. If no empty cell exists, puzzle is solved (base case)
     * 3. Try digits 1-9 in the empty cell
     * 4. For each valid digit:
     *    a. Place digit in cell
     *    b. Recursively solve remaining puzzle
     *    c. Backtrack by removing digit (try next possibility)
     * 
     * This explores the entire solution space systematically
     */
    void solveBacktrack() {
        int row, col;
        
        // Base case: no empty cells means puzzle is solved
        if (!findEmptyCell(row, col)) {
            solutionCount++;
            std::cout << "Solution " << solutionCount << ":\n";
            printBoard();
            std::cout << "\n";
            return; // Backtrack to find more solutions
        }
        
        // Try each digit 1-9 in the empty cell
        for (int num = 1; num <= 9; num++) {
            // Check if this digit placement is valid
            if (isValidPlacement(row, col, num)) {
                // Make the move: place digit in cell
                board[row][col] = num;
                
                // Recursively solve with this digit placed
                solveBacktrack();
                
                // Backtrack: undo the move to try next possibility
                // This is crucial - without this, we'd only find one solution
                board[row][col] = 0;
            }
        }
        // When loop ends, we've tried all digits - backtrack to previous level
    }
    
    /**
     * Prints the current board state in simple 9x9 format
     * No fancy formatting - just digits with spaces
     */
    void printBoard() {
        for (int row = 0; row < 9; row++) {
            for (int col = 0; col < 9; col++) {
                std::cout << board[row][col];
                if (col < 8) std::cout << " "; // Space between digits
            }
            std::cout << "\n";
        }
    }

public:
    /**
     * Constructor: initializes empty 9x9 board and solution counter
     */
    SudokuSolver() : board(9, std::vector<int>(9, 0)), solutionCount(0) {}
    
    /**
     * Reads and validates Sudoku puzzle input from console
     * 
     * Input format: 9 lines, each containing 9 digits (0-9)
     * 0 represents empty cell, 1-9 represent filled cells
     * 
     * Validation checks:
     * - Exactly 9 lines
     * - Each line has exactly 9 characters
     * - All characters are digits 0-9
     * 
     * @return true if input is valid, false otherwise
     */
    bool readInput() {
        std::string line;
        
        std::cout << "Enter Sudoku puzzle (9 lines of 9 digits each, 0 for empty):\n";
        
        // Read exactly 9 lines
        for (int row = 0; row < 9; row++) {
            if (!std::getline(std::cin, line)) {
                std::cerr << "Error: Could not read line " << (row + 1) << "\n";
                return false;
            }
            
            // Validate line length
            if (line.length() != 9) {
                std::cerr << "Error: Line " << (row + 1) << " must contain exactly 9 digits\n";
                return false;
            }
            
            // Parse and validate each character
            for (int col = 0; col < 9; col++) {
                char ch = line[col];
                
                // Check if character is a digit
                if (ch < '0' || ch > '9') {
                    std::cerr << "Error: Invalid character '" << ch << "' at position " 
                              << (row + 1) << "," << (col + 1) << ". Only digits 0-9 allowed.\n";
                    return false;
                }
                
                // Convert char to int and store in board
                board[row][col] = ch - '0'; // ASCII conversion
            }
        }
        
        return true; // All validation passed
    }
    
    /**
     * Validates that the initial puzzle state is legal
     * Checks if any pre-filled digits violate Sudoku rules
     * 
     * @return true if puzzle is valid, false if contradictions exist
     */
    bool validateInitialState() {
        // Check each filled cell to ensure it doesn't violate constraints
        for (int row = 0; row < 9; row++) {
            for (int col = 0; col < 9; col++) {
                if (board[row][col] != 0) { // Only check filled cells
                    int num = board[row][col];
                    
                    // Temporarily remove the digit to test validity
                    board[row][col] = 0;
                    
                    // Check if placing this digit would be valid
                    if (!isValidPlacement(row, col, num)) {
                        board[row][col] = num; // Restore for error reporting
                        std::cerr << "Error: Invalid initial state - digit " << num 
                                  << " at position " << (row + 1) << "," << (col + 1) 
                                  << " violates Sudoku rules.\n";
                        return false;
                    }
                    
                    // Restore the digit
                    board[row][col] = num;
                }
            }
        }
        
        return true; // No contradictions found
    }
    
    /**
     * Main solving method - coordinates the entire solving process
     * 
     * Process:
     * 1. Read and validate input
     * 2. Check initial puzzle state for contradictions
     * 3. Solve using backtracking
     * 4. Report results
     */
    void solve() {
        // Step 1: Read puzzle input
        if (!readInput()) {
            std::cerr << "Failed to read input. Exiting.\n";
            return;
        }
        
        // Step 2: Validate initial puzzle state
        if (!validateInitialState()) {
            std::cerr << "Invalid puzzle. Exiting.\n";
            return;
        }
        
        std::cout << "\nSolving puzzle...\n\n";
        
        // Step 3: Solve using backtracking
        solveBacktrack();
        
        // Step 4: Report results
        if (solutionCount == 0) {
            std::cout << "No solutions found. The puzzle may be invalid.\n";
        } else {
            std::cout << "Total solutions found: " << solutionCount << "\n";
        }
    }
};

/**
 * Main function - entry point of the program
 * Creates SudokuSolver instance and starts solving process
 */
int main() {
    SudokuSolver solver;
    solver.solve();
    return 0;
}