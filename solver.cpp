#include <windows.h>
#include <vector>
#include <string>
#include <functional>

const int cellSize = 50;                    // Size of each cell
const int gridSize = 9;                     // 9x9 Sudoku grid
const int windowSize = cellSize * gridSize; // Total window size

std::vector<std::vector<int>> sudokuGrid(gridSize, std::vector<int>(gridSize, 0));        // Sudoku grid
std::vector<std::vector<bool>> isEditable(gridSize, std::vector<bool>(gridSize, true));   // All cells are editable
int selectedRow = -1, selectedCol = -1;                                                   // Currently selected cell

void drawSudokuGrid(HDC hdc)
{
    // Draw numbers and highlight cells
    for (int row = 0; row < gridSize; ++row)
    {
        for (int col = 0; col < gridSize; ++col)
        {
            RECT rect = {col * cellSize, row * cellSize, (col + 1) * cellSize, (row + 1) * cellSize};

            if (sudokuGrid[row][col] != 0)
            {
                std::wstring number = std::to_wstring(sudokuGrid[row][col]);
                SetTextColor(hdc, RGB(0, 0, 0)); // Black text
                SetBkMode(hdc, TRANSPARENT);
                DrawTextW(hdc, number.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }
    }

    // Draw vertical and horizontal lines
    for (int i = 0; i <= gridSize; ++i)
    {
        HPEN pen = CreatePen(PS_SOLID, (i % 3 == 0) ? 2 : 1, RGB(0, 0, 0));
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
    static HWND autoCompleteButton;

    switch (uMsg)
    {
    case WM_CREATE:
        autoCompleteButton = CreateWindowW(L"BUTTON", L"Solve", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                           windowSize + 20, 20, 100, 30, hwnd, (HMENU)1, NULL, NULL);
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == 1) // Solve button
        {
            autoComplete(hwnd);
        }
        return 0;

    case WM_LBUTTONDOWN:
    {
        int x = LOWORD(lParam) / cellSize;
        int y = HIWORD(lParam) / cellSize;
        if (x >= 0 && x < gridSize && y >= 0 && y < gridSize)
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
        if (selectedRow != -1 && selectedCol != -1)
        {
            int key = wParam - '0';
            if (key >= 1 && key <= 9)
            {
                sudokuGrid[selectedRow][selectedCol] = key;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (wParam == VK_BACK || wParam == VK_DELETE)
            {
                sudokuGrid[selectedRow][selectedCol] = 0;
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
        L"Sudoku Solver",
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
