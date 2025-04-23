#include <windows.h>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <functional>

const int cellSize = 50;                    // Size of each cell
const int gridSize = 9;                     // 9x9 Sudoku grid
const int windowSize = cellSize * gridSize; // Total window size

std::vector<std::vector<int>> sudokuGrid(gridSize, std::vector<int>(gridSize, 0));        // Sudoku grid
std::vector<std::vector<bool>> isEditable(gridSize, std::vector<bool>(gridSize, false));  // Editable cells
std::vector<std::vector<bool>> isIncorrect(gridSize, std::vector<bool>(gridSize, false)); // Incorrect cells
int selectedRow = -1, selectedCol = -1;                                                   // Currently selected cell
int difficultyLevel = 0; // 0 = Easy, 1 = Medium, 2 = Hard

HWND populateButton, easyRadio, mediumRadio, hardRadio;

void drawSudokuGrid(HDC hdc)
{
    // Draw numbers and highlight cells
    for (int row = 0; row < gridSize; ++row)
    {
        for (int col = 0; col < gridSize; ++col)
        {
            RECT rect = {col * cellSize, row * cellSize, (col + 1) * cellSize, (row + 1) * cellSize};

            // Highlight non-editable cells with a light gray background
            if (!isEditable[row][col])
            {
                HBRUSH brush = CreateSolidBrush(RGB(220, 220, 220)); // Light gray
                FillRect(hdc, &rect, brush);
                DeleteObject(brush);
            }

            if (sudokuGrid[row][col] != 0)
            {
                std::wstring number = std::to_wstring(sudokuGrid[row][col]);
                SetTextColor(hdc, isIncorrect[row][col] ? RGB(255, 0, 0) : RGB(0, 0, 0)); // Red for incorrect, black otherwise
                SetBkMode(hdc, TRANSPARENT);
                DrawTextW(hdc, number.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }
    }

    // Draw vertical and horizontal lines
    for (int i = 0; i <= gridSize; ++i)
    {
        HPEN pen = CreatePen(PS_SOLID, (i % 3 == 0) ? 2 : 1, (i % 3 == 0) ? RGB(0, 0, 0) : RGB(192, 192, 192));
        SelectObject(hdc, pen);
        MoveToEx(hdc, i * cellSize, 0, NULL);
        LineTo(hdc, i * cellSize, windowSize);
        MoveToEx(hdc, 0, i * cellSize, NULL);
        LineTo(hdc, windowSize, i * cellSize);
        DeleteObject(pen);
    }

    // Highlight selected cell
    if (selectedRow != -1 && selectedCol != -1)
    {
        RECT rect = {selectedCol * cellSize, selectedRow * cellSize, (selectedCol + 1) * cellSize, (selectedRow + 1) * cellSize};
        FrameRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
    }
}

void populateSudokuGrid()
{
    sudokuGrid = std::vector<std::vector<int>>(gridSize, std::vector<int>(gridSize, 0));
    isEditable = std::vector<std::vector<bool>>(gridSize, std::vector<bool>(gridSize, false));
    isIncorrect = std::vector<std::vector<bool>>(gridSize, std::vector<bool>(gridSize, false));

    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<int> baseRow(gridSize);
    std::iota(baseRow.begin(), baseRow.end(), 1);
    std::shuffle(baseRow.begin(), baseRow.end(), gen);

    for (int row = 0; row < gridSize; ++row)
    {
        for (int col = 0; col < gridSize; ++col)
        {
            sudokuGrid[row][col] = baseRow[(col + row * 3 + row / 3) % gridSize];
        }
    }

    int cellsToRemove;
    if (difficultyLevel == 0) // Easy
        cellsToRemove = 30;
    else if (difficultyLevel == 1) // Medium
        cellsToRemove = 45;
    else // Hard
        cellsToRemove = 70;

    for (int i = 0; i < cellsToRemove; ++i)
    {
        int row = gen() % gridSize;
        int col = gen() % gridSize;
        sudokuGrid[row][col] = 0;
        isEditable[row][col] = true;
    }
}

bool isValidEntry(int row, int col, int value)
{
    for (int i = 0; i < gridSize; ++i)
    {
        if (sudokuGrid[row][i] == value || sudokuGrid[i][col] == value)
        {
            return false;
        }
    }
    int boxRow = (row / 3) * 3, boxCol = (col / 3) * 3;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            if (sudokuGrid[boxRow + i][boxCol + j] == value)
            {
                return false;
            }
        }
    }
    return true;
}

bool isPuzzleSolved()
{
    for (int row = 0; row < gridSize; ++row)
    {
        for (int col = 0; col < gridSize; ++col)
        {
            int val = sudokuGrid[row][col];
            if (val == 0)
                return false;

            sudokuGrid[row][col] = 0;
            if (!isValidEntry(row, col, val))
            {
                sudokuGrid[row][col] = val;
                return false;
            }
            sudokuGrid[row][col] = val;
        }
    }
    return true;
}

void autoComplete(HWND hwnd)
{
    std::function<bool(int, int)> solve = [&](int row, int col) -> bool {
        if (row == gridSize) // If we've reached the end of the grid
            return true;

        int nextRow = (col == gridSize - 1) ? row + 1 : row;
        int nextCol = (col == gridSize - 1) ? 0 : col + 1;

        if (sudokuGrid[row][col] != 0) // Skip non-empty cells
            return solve(nextRow, nextCol);

        for (int num = 1; num <= gridSize; ++num)
        {


            if (isValidEntry(row, col, num))
            {
                sudokuGrid[row][col] = num;
                InvalidateRect(hwnd, NULL, TRUE); // Redraw the grid
                UpdateWindow(hwnd); // Ensure the grid updates immediately

                if (solve(nextRow, nextCol))
                    return true;
                
                sudokuGrid[row][col] = 0; // Backtrack
            }

        }
        return false;
    };

    solve(0, 0);


}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND difficultyDropdown, autoCompleteButton;

    switch (uMsg)
    {
        case WM_CREATE:
        populateButton = CreateWindowW(L"BUTTON", L"Populate", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                       windowSize + 20, 20, 100, 30, hwnd, (HMENU)1, NULL, NULL);
    
        autoCompleteButton = CreateWindowW(L"BUTTON", L"Auto Complete", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                           windowSize + 20, 60, 100, 30, hwnd, (HMENU)3, NULL, NULL);
    
        difficultyDropdown = CreateWindowW(L"COMBOBOX", NULL, WS_TABSTOP | WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
                                           windowSize + 20, 100, 100, 100, hwnd, (HMENU)2, NULL, NULL);
    
        // Add options to the dropdown
        SendMessageW(difficultyDropdown, CB_ADDSTRING, 0, (LPARAM)L"Easy");
        SendMessageW(difficultyDropdown, CB_ADDSTRING, 0, (LPARAM)L"Medium");
        SendMessageW(difficultyDropdown, CB_ADDSTRING, 0, (LPARAM)L"Hard");
    
        // Set default selection to "Easy"
        SendMessageW(difficultyDropdown, CB_SETCURSEL, 0, 0);
        return 0;
    
    case WM_TIMER:
        if (wParam == 1) // Timer for celebration
        {
            KillTimer(hwnd, 1); // Stop the timer
        }
        return 0;
    case WM_COMMAND:
        if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == 2) // Difficulty dropdown
        {
            int selectedIndex = SendMessageW(difficultyDropdown, CB_GETCURSEL, 0, 0);
            difficultyLevel = selectedIndex; // 0 = Easy, 1 = Medium, 2 = Hard
        }
        else if (LOWORD(wParam) == 1) // Populate button
        {
            populateSudokuGrid();
            InvalidateRect(hwnd, NULL, TRUE);
        }
        else if (LOWORD(wParam) == 3) // Auto Complete button
        {
            autoComplete(hwnd);
            if (isPuzzleSolved())
            {
                MessageBoxW(hwnd, L"Congratulations! You solved the puzzle!", L"Puzzle Solved", MB_OK | MB_ICONINFORMATION);
                SetTimer(hwnd, 1, 100, NULL); // Start the celebration timer
            }
        }
        return 0;

    case WM_LBUTTONDOWN:
    {
        int x = LOWORD(lParam) / cellSize;
        int y = HIWORD(lParam) / cellSize;
        if (x >= 0 && x < gridSize && y >= 0 && y < gridSize && isEditable[y][x])
        {
            selectedRow = y;
            selectedCol = x;
            SetFocus(hwnd);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }

    case WM_KEYDOWN:
    {
        if (selectedRow != -1 && selectedCol != -1 && isEditable[selectedRow][selectedCol])
        {
            int key = wParam - '0';
            if (key >= 1 && key <= 9)
            {
                if (isValidEntry(selectedRow, selectedCol, key))
                {
                    sudokuGrid[selectedRow][selectedCol] = key;
                    isIncorrect[selectedRow][selectedCol] = false;
                }
                else
                {
                    sudokuGrid[selectedRow][selectedCol] = key;
                    isIncorrect[selectedRow][selectedCol] = true;
                }
                InvalidateRect(hwnd, NULL, TRUE);

                if (isPuzzleSolved())
                {
                    MessageBoxW(hwnd, L"Congratulations! You solved the puzzle!", L"Puzzle Solved", MB_OK | MB_ICONINFORMATION);
                }
            }
            else if (wParam == VK_BACK || wParam == VK_DELETE)
            {
                sudokuGrid[selectedRow][selectedCol] = 0;
                isIncorrect[selectedRow][selectedCol] = false;
                InvalidateRect(hwnd, NULL, TRUE);
            }
        }
        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        drawSudokuGrid(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"SudokuGridWindow";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Sudoku Grid",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, windowSize + 150, windowSize + 80,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
