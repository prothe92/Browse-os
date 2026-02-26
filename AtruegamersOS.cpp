#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <iomanip>
#include <chrono>
#include <thread>
#include <random>
#include <deque>

using namespace std;

// Platform-specific headers for terminal manipulation
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

// --- Simulated Environment ---
class Environment {
public:
    map<string, string> files;
    string currentUser = "goodgamer";
};

// --- Command and help structures ---
struct CommandInfo {
    string name;
    string description;
    string usage;
};

map<string, CommandInfo> commandHelp = {
    {"help", {"help [command]", "Displays information about available commands.", "Usage: help [command]"}},
    {"ls", {"ls", "Lists files in the simulated system.", "Usage: ls"}},
    {"create", {"create <filename> \"<content>\"", "Creates a new file with specified content.", "Usage: create <filename> \"<content>\""}},
    {"read", {"read <filename>", "Reads and displays the content of a file.", "Usage: read <filename>"}},
    {"del", {"del <filename>", "Deletes a file from the simulated system.", "Usage: del <filename>"}},
    {"whoami", {"whoami", "Displays the current active user.", "Usage: whoami"}},
    {"pong", {"pong", "Starts a game of text-based Pong.", "Usage: pong"}},
    {"snake", {"snake", "Starts a game of text-based Snake.", "Usage: snake"}},
    {"exit", {"exit", "Shuts down SwitchOS.", "Usage: exit"}}
};

// --- Command Parser ---
vector<string> parseCommand(const string& commandLine) {
    vector<string> tokens;
    string currentToken;
    bool inQuotes = false;

    for (char c : commandLine) {
        if (c == '"' && !inQuotes) {
            inQuotes = true;
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        } else if (c == '"' && inQuotes) {
            inQuotes = false;
            tokens.push_back(currentToken);
            currentToken.clear();
        } else if (c == ' ' && !inQuotes) {
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        } else {
            currentToken += c;
        }
    }
    if (!currentToken.empty()) {
        tokens.push_back(currentToken);
    }
    return tokens;
}

// --- Terminal Control (Platform-specific) ---
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void setCursorPosition(int x, int y) {
#ifdef _WIN32
    COORD coord = {(short)x, (short)y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
    cout << "\033[" << y << ";" << x << "H";
#endif
}

void setRawMode() {
#ifndef _WIN32
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
#endif
}

void restoreTerminal() {
#ifndef _WIN32
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
#endif
}

int getch_nonblock() {
#ifdef _WIN32
    if (_kbhit()) {
        return _getch();
    }
    return -1;
#else
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    timeval timeout = {0, 0};
    if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &timeout) > 0) {
        return getchar();
    }
    return -1;
#endif
}

// --- Text-based Pong Game ---
void runPongGame() {
    // Game settings
    const int width = 60;
    const int height = 20;
    int scoreLeft = 0;
    int scoreRight = 0;
    bool gameRunning = true;

    // Ball
    float ballX, ballY, ballDirX, ballDirY;
    
    // Paddles
    int paddleLeftY, paddleRightY;
    const int paddleLength = 4;
    
    auto resetBall = [&]() {
        ballX = width / 2;
        ballY = height / 2;
        ballDirX = (rand() % 2 == 0) ? 1.0f : -1.0f;
        ballDirY = (rand() % 3 - 1); // -1, 0, or 1
    };
    
    auto setupGame = [&]() {
        paddleLeftY = height / 2 - paddleLength / 2;
        paddleRightY = height / 2 - paddleLength / 2;
        resetBall();
    };
    
    setupGame();
    
    clearScreen();
    setRawMode();

    while (gameRunning) {
        // --- Input ---
        int key = getch_nonblock();
        if (key != -1) {
            if (key == 'w' && paddleLeftY > 1) paddleLeftY--;
            if (key == 's' && paddleLeftY + paddleLength < height - 1) paddleLeftY++;
            if (key == 'q') gameRunning = false;
        }

        // --- Update ---
        ballX += ballDirX;
        ballY += ballDirY;

        // Ball collision with walls
        if (ballY <= 0 || ballY >= height - 1) {
            ballDirY = -ballDirY;
        }

        // Ball collision with paddles
        if ((ballX <= 2 && ballY >= paddleLeftY && ballY <= paddleLeftY + paddleLength) ||
            (ballX >= width - 3 && ballY >= paddleRightY && ballY <= paddleRightY + paddleLength)) {
            ballDirX = -ballDirX;
        }

        // Scoring
        if (ballX < 0) {
            scoreRight++;
            resetBall();
        }
        if (ballX > width - 1) {
            scoreLeft++;
            resetBall();
        }

        // Simple AI for right paddle
        if (ballY < paddleRightY + paddleLength / 2 && paddleRightY > 1) {
            paddleRightY--;
        } else if (ballY > paddleRightY + paddleLength / 2 && paddleRightY + paddleLength < height - 1) {
            paddleRightY++;
        }

        // --- Draw ---
        clearScreen();
        for (int i = 0; i < width; ++i) cout << "#";
        cout << "\n";

        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                if (j == 0 || j == width - 1) cout << "#";
                else if (j == 2 && i >= paddleLeftY && i < paddleLeftY + paddleLength) cout << "|";
                else if (j == width - 3 && i >= paddleRightY && i < paddleRightY + paddleLength) cout << "|";
                else if (j == static_cast<int>(ballX) && i == static_cast<int>(ballY)) cout << "O";
                else cout << " ";
            }
            cout << "\n";
        }
        
        for (int i = 0; i < width; ++i) cout << "#";
        cout << "\n";

        cout << "Score: " << scoreLeft << " | " << scoreRight << "\n";
        cout << "Press 'q' to quit.\n";
        setCursorPosition(0, 0);

        // --- Frame delay ---
        this_thread::sleep_for(chrono::milliseconds(50));
    }

    restoreTerminal();
    clearScreen();
}

// --- Text-based Snake Game ---
void runSnakeGame() {
    const int width = 60;
    const int height = 20;
    int score = 0;
    bool gameRunning = true;

    // Snake
    deque<pair<int, int>> snake;
    int headX = width / 2;
    int headY = height / 2;
    int dirX = 1;
    int dirY = 0;

    // Food
    pair<int, int> food;

    auto spawnFood = [&]() {
        food.first = rand() % (width - 2) + 1;
        food.second = rand() % (height - 2) + 1;
    };

    auto setupGame = [&]() {
        snake.clear();
        snake.push_front({headX, headY});
        spawnFood();
    };

    setupGame();
    clearScreen();
    setRawMode();

    while (gameRunning) {
        // --- Input ---
        int key = getch_nonblock();
        if (key != -1) {
            switch (key) {
                case 'w':
                case 72: // Up arrow (Windows)
                    if (dirY == 0) { dirX = 0; dirY = -1; }
                    break;
                case 's':
                case 80: // Down arrow (Windows)
                    if (dirY == 0) { dirX = 0; dirY = 1; }
                    break;
                case 'a':
                case 75: // Left arrow (Windows)
                    if (dirX == 0) { dirX = -1; dirY = 0; }
                    break;
                case 'd':
                case 77: // Right arrow (Windows)
                    if (dirX == 0) { dirX = 1; dirY = 0; }
                    break;
                case 'q':
                    gameRunning = false;
                    break;
            }
        }
        
        // --- Update ---
        headX += dirX;
        headY += dirY;

        // Check for collisions with walls
        if (headX <= 0 || headX >= width - 1 || headY <= 0 || headY >= height - 1) {
            gameRunning = false;
        }

        // Check for collisions with self
        for (const auto& segment : snake) {
            if (segment.first == headX && segment.second == headY) {
                gameRunning = false;
            }
        }

        // Check for eating food
        if (headX == food.first && headY == food.second) {
            score += 10;
            snake.push_front({headX, headY});
            spawnFood();
        } else {
            snake.push_front({headX, headY});
            snake.pop_back();
        }

        // --- Draw ---
        clearScreen();
        setCursorPosition(0, 0);

        for (int i = 0; i < width; ++i) cout << "#";
        cout << "\n";

        for (int i = 1; i < height - 1; ++i) {
            cout << "#";
            for (int j = 1; j < width - 1; ++j) {
                bool isSnake = false;
                if (headX == j && headY == i) {
                    cout << "O";
                    isSnake = true;
                } else {
                    for (const auto& segment : snake) {
                        if (segment.first == j && segment.second == i) {
                            cout << "o";
                            isSnake = true;
                            break;
                        }
                    }
                }
                if (!isSnake) {
                    if (j == food.first && i == food.second) {
                        cout << "*";
                    } else {
                        cout << " ";
                    }
                }
            }
            cout << "#\n";
        }
        
        for (int i = 0; i < width; ++i) cout << "#";
        cout << "\n";

        cout << "Score: " << score << "\n";
        cout << "Press 'q' to quit. Use WASD to move.\n";

        // --- Frame delay ---
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    restoreTerminal();
    clearScreen();
    cout << "Game Over! Final Score: " << score << "\n";
}

// --- Command Execution ---
void executeCommand(const vector<string>& tokens, Environment& env) {
    if (tokens.empty()) {
        return;
    }

    const string& command = tokens[0];

    if (command == "help") {
        if (tokens.size() == 1) {
            cout << "SwitchOS commands:\n";
            for (const auto& pair : commandHelp) {
                cout << left << setw(15) << pair.second.name << " - " << pair.second.description << "\n";
            }
        } else {
            const string& cmd = tokens[1];
            if (commandHelp.count(cmd)) {
                const auto& info = commandHelp.at(cmd);
                cout << info.description << "\n";
                cout << info.usage << "\n";
            } else {
                cout << "Error: No help entry for command '" << cmd << "'.\n";
            }
        }
    }
    else if (command == "ls") {
        cout << "Files in the system:\n";
        if (env.files.empty()) {
            cout << "  (none)\n";
        } else {
            for (const auto& pair : env.files) {
                cout << "  - " << pair.first << "\n";
            }
        }
    }
    else if (command == "create") {
        if (tokens.size() >= 3) {
            const string& filename = tokens[1];
            const string& content = tokens[2];
            if (env.files.find(filename) == env.files.end()) {
                env.files[filename] = content;
                cout << "File '" << filename << "' created.\n";
            } else {
                cout << "Error: File '" << filename << "' already exists.\n";
            }
        } else {
            cout << "Usage: create <filename> \"<content>\"\n";
        }
    }
    else if (command == "read") {
        if (tokens.size() == 2) {
            const string& filename = tokens[1];
            if (env.files.find(filename) != env.files.end()) {
                cout << "--- " << filename << " ---\n";
                cout << env.files.at(filename) << "\n";
                cout << "--------------------\n";
            } else {
                cout << "Error: File '" << filename << "' not found.\n";
            }
        } else {
            cout << "Usage: read <filename>\n";
        }
    }
    else if (command == "del") {
        if (tokens.size() == 2) {
            const string& filename = tokens[1];
            if (env.files.erase(filename)) {
                cout << "File '" << filename << "' deleted.\n";
            } else {
                cout << "Error: File '" << filename << "' not found.\n";
            }
        } else {
            cout << "Usage: del <filename>\n";
        }
    }
    else if (command == "whoami") {
        cout << "Current user: " << env.currentUser << "\n";
    }
    else if (command == "pong") {
        runPongGame();
    }
    else if (command == "snake") {
        runSnakeGame();
    }
    else if (command == "exit") {
        exit(0);
    }
    else {
        cout << "Error: Command '" << command << "' not found.\n";
    }
}

int main() {
    Environment env;
    string commandLine;

    cout << "Welcome to SwitchOS!\n";
    cout << "Type 'help' for a list of commands.\n";

    while (true) {
        cout << env.currentUser << "@SwitchOS: ";
        getline(cin, commandLine);
        vector<string> tokens = parseCommand(commandLine);
        executeCommand(tokens, env);
    }

    return 0;
}
