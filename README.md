
# Sudoku Game and Solver

A Windows-based Sudoku implementation featuring both a playable game and a solver utility.

## Features

### Sudoku Game (sudoku.cpp)
- Interactive 9x9 Sudoku grid
- Three difficulty levels: Easy, Medium, and Hard
- Real-time validation of moves
- Auto-complete functionality
- Visual feedback for incorrect entries
- Non-editable initial cells
- Puzzle completion detection

### Sudoku Solver (solver.cpp)
- Clean interface for manual puzzle input
- One-click solution using backtracking algorithm
- Visual solving process
- Supports any valid Sudoku puzzle

## Controls

### Game Mode
- Left-click to select a cell
- Number keys (1-9) to input values
- Backspace/Delete to clear a cell
- "Populate" button to generate new puzzle
- "Auto Complete" to solve current puzzle
- Difficulty dropdown to select puzzle complexity

### Solver Mode
- Left-click to select a cell
- Number keys (1-9) to input values
- Backspace/Delete to clear a cell
- "Solve" button to find solution

## Technical Details
- Written in C++ using Win32 API
- Grid size: 9x9
- Cell size: 50x50 pixels
- Uses backtracking algorithm for solving
