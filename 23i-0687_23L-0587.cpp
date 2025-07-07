#include <SFML/Graphics.hpp>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include<iostream>
using namespace std;
using namespace sf;

// Game states
enum GameState {
    MENU,
    TOSS_RESULT,
    PLAYING,
    WIN_SCREEN,
    LOAD_RESULT
};

// Define valid connections based on the exact board pattern
bool validConnections[5][5][5][5];

void initializeConnections() {
    // Initialize all connections as false
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            for (int k = 0; k < 5; k++) {
                for (int l = 0; l < 5; l++) {
                    validConnections[i][j][k][l] = false;
                }
            }
        }
    }

    // Horizontal connections (all rows)
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 4; col++) {
            validConnections[col][row][col + 1][row] = true;
            validConnections[col + 1][row][col][row] = true;
        }
    }

    // Vertical connections (all columns)
    for (int col = 0; col < 5; col++) {
        for (int row = 0; row < 4; row++) {
            validConnections[col][row][col][row + 1] = true;
            validConnections[col][row + 1][col][row] = true;
        }
    }

    // Diagonal connections based on the exact board pattern
    // Star pattern around (1,1)
    validConnections[0][0][1][1] = true; validConnections[1][1][0][0] = true;
    validConnections[2][0][1][1] = true; validConnections[1][1][2][0] = true;
    validConnections[0][2][1][1] = true; validConnections[1][1][0][2] = true;
    validConnections[2][2][1][1] = true; validConnections[1][1][2][2] = true;

    // Star pattern around (3,1)
    validConnections[2][0][3][1] = true; validConnections[3][1][2][0] = true;
    validConnections[4][0][3][1] = true; validConnections[3][1][4][0] = true;
    validConnections[2][2][3][1] = true; validConnections[3][1][2][2] = true;
    validConnections[4][2][3][1] = true; validConnections[3][1][4][2] = true;

    // Star pattern around (1,3)
    validConnections[0][2][1][3] = true; validConnections[1][3][0][2] = true;
    validConnections[2][2][1][3] = true; validConnections[1][3][2][2] = true;
    validConnections[0][4][1][3] = true; validConnections[1][3][0][4] = true;
    validConnections[2][4][1][3] = true; validConnections[1][3][2][4] = true;

    // Star pattern around (3,3)
    validConnections[2][2][3][3] = true; validConnections[3][3][2][2] = true;
    validConnections[4][2][3][3] = true; validConnections[3][3][4][2] = true;
    validConnections[2][4][3][3] = true; validConnections[3][3][2][4] = true;
    validConnections[4][4][3][3] = true; validConnections[3][3][4][4] = true;

    // Center star pattern around (2,2) - connects to all 8 surrounding positions
    validConnections[1][1][2][2] = true; validConnections[2][2][1][1] = true;
    validConnections[2][1][2][2] = true; validConnections[2][2][2][1] = true;
    validConnections[3][1][2][2] = true; validConnections[2][2][3][1] = true;
    validConnections[1][2][2][2] = true; validConnections[2][2][1][2] = true;
    validConnections[3][2][2][2] = true; validConnections[2][2][3][2] = true;
    validConnections[1][3][2][2] = true; validConnections[2][2][1][3] = true;
    validConnections[2][3][2][2] = true; validConnections[2][2][2][3] = true;
    validConnections[3][3][2][2] = true; validConnections[2][2][3][3] = true;
}

void drawBoardLines(RenderWindow& window, int boxSize) {
    // Draw all the lines based on valid connections
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            for (int k = 0; k < 5; k++) {
                for (int l = 0; l < 5; l++) {
                    if (validConnections[i][j][k][l] && (i < k || (i == k && j < l))) {
                        // Calculate center positions of grid squares
                        float x1 = i * boxSize + boxSize / 2.0f;
                        float y1 = j * boxSize + boxSize / 2.0f;
                        float x2 = k * boxSize + boxSize / 2.0f;
                        float y2 = l * boxSize + boxSize / 2.0f;

                        // Create and draw line
                        sf::Vertex line[] = {
                            sf::Vertex(sf::Vector2f(x1, y1), Color::White),
                            sf::Vertex(sf::Vector2f(x2, y2), Color::White)
                        };
                        window.draw(line, 2, sf::Lines);
                    }
                }
            }
        }
    }
}

bool isValidConnection(int x1, int y1, int x2, int y2) {
    if (x1 < 0 || x1 >= 5 || y1 < 0 || y1 >= 5 ||
        x2 < 0 || x2 >= 5 || y2 < 0 || y2 >= 5) {
        return false;
    }
    return validConnections[x1][y1][x2][y2];
}

bool isValidJumpMove(int x1, int y1, int x2, int y2, CircleShape circles[][5], Color playerColor) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    // Must be exactly 2 steps away
    if ((abs(dx) == 2 && dy == 0) || (dx == 0 && abs(dy) == 2) || (abs(dx) == 2 && abs(dy) == 2)) {
        int midX = (x1 + x2) / 2;
        int midY = (y1 + y2) / 2;

        // Check if both segments of the jump are valid connections
        if (isValidConnection(x1, y1, midX, midY) && isValidConnection(midX, midY, x2, y2)) {
            Color midColor = circles[midX][midY].getFillColor();
            return (midColor != Color::Transparent && midColor != playerColor);
        }
    }

    return false;
}

Vector2i getJumpedPosition(int x1, int y1, int x2, int y2) {
    return Vector2i((x1 + x2) / 2, (y1 + y2) / 2);
}

void drawFullyCenteredText(RenderWindow& window, const std::string& text, float y, int characterSize, const Color& color, int windowWidth) {
    Font font;
    font.loadFromFile("SwanseaBoldItalic-p3Dv.ttf");

    Text message;
    if (font.getInfo().family != "") {
        message.setFont(font);
    }
    message.setString(text);
    message.setCharacterSize(characterSize);
    message.setFillColor(color);

    // Calculate center position horizontally
    FloatRect textBounds = message.getLocalBounds();
    message.setPosition((windowWidth - textBounds.width) / 2.0f, y);

    window.draw(message);
}

void drawRightAlignedText(RenderWindow& window, const std::string& text, float y, int characterSize, const Color& color, int windowWidth) {
    Font font;
    font.loadFromFile("SwanseaBoldItalic-p3Dv.ttf");

    Text message;
    if (font.getInfo().family != "") {
        message.setFont(font);
    }
    message.setString(text);
    message.setCharacterSize(characterSize);
    message.setFillColor(color);

    // Position on right side
    message.setPosition(windowWidth / 2.0f + 50, y);

    window.draw(message);
}

bool toss() {
    return rand() % 2 == 0;
}

// Count remaining pieces of each color
void countPieces(CircleShape circles[][5], int& greenCount, int& redCount) {
    greenCount = 0;
    redCount = 0;

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            Color color = circles[i][j].getFillColor();
            if (color == Color::Green) {
                greenCount++;
            }
            else if (color == Color::Red) {
                redCount++;
            }
        }
    }
}

bool savegame(bool playerTurn, const CircleShape circles[][5]) {
    ofstream outFile("game_info.txt");
    if (!outFile.is_open()) {
        return false;
    }

    outFile << (playerTurn ? 1 : 0) << endl;

    for (int j = 0; j < 5; ++j) {
        for (int i = 0; i < 5; ++i) {
            Color color = circles[i][j].getFillColor();
            if (color == Color::Red) {
                outFile << "1 ";
            }
            else if (color == Color::Green) {
                outFile << "2 ";
            }
            else {
                outFile << "0 ";
            }
        }
        outFile << endl;
    }

    outFile.close();
    return true;
}

bool loadgame(bool& playerTurn, CircleShape circles[][5]) {
    ifstream inFile("game_info.txt");
    if (!inFile.is_open()) {
        return false;
    }

    int turn;
    inFile >> turn;
    playerTurn = (turn == 1);

    const int boxSize = 200;

    for (int j = 0; j < 5; ++j) {
        for (int i = 0; i < 5; ++i) {
            int colorinfo;
            inFile >> colorinfo;

            circles[i][j].setRadius(60);
            circles[i][j].setPosition(i * boxSize + (boxSize - 2 * circles[i][j].getRadius()) / 2,
                j * boxSize + (boxSize - 2 * circles[i][j].getRadius()) / 2);

            if (colorinfo == 1) {
                circles[i][j].setFillColor(Color::Red);
            }
            else if (colorinfo == 2) {
                circles[i][j].setFillColor(Color::Green);
            }
            else {
                circles[i][j].setFillColor(Color::Transparent);
            }
        }
    }

    inFile.close();
    return true;
}

void initializeBoard(CircleShape circles[][5]) {
    const int boardSize = 5;
    const int boxSize = 200;

    // Initialize all circles
    for (int i = 0; i < boardSize; ++i) {
        for (int j = 0; j < boardSize; ++j) {
            circles[i][j].setRadius(60);
            circles[i][j].setFillColor(Color::Transparent);
            circles[i][j].setPosition(i * boxSize + (boxSize - 2 * circles[i][j].getRadius()) / 2,
                j * boxSize + (boxSize - 2 * circles[i][j].getRadius()) / 2);
        }
    }

    // Set up initial positions - 12 beads each side
    // Red pieces: Top 2 rows (10) + 2 in middle row (left side)
    for (int i = 0; i < 5; i++) {
        circles[i][0].setFillColor(Color::Red); // Top row (5)
        circles[i][1].setFillColor(Color::Red); // Second row (5)
    }
    circles[0][2].setFillColor(Color::Red); // Middle row left (1)
    circles[1][2].setFillColor(Color::Red); // Middle row left (1)

    // Green pieces: Bottom 2 rows (10) + 2 in middle row (right side)  
    for (int i = 0; i < 5; i++) {
        circles[i][3].setFillColor(Color::Green); // Fourth row (5)
        circles[i][4].setFillColor(Color::Green); // Bottom row (5)
    }
    circles[3][2].setFillColor(Color::Green); // Middle row right (1)
    circles[4][2].setFillColor(Color::Green); // Middle row right (1)

    // Center position empty
    circles[2][2].setFillColor(Color::Transparent);
}

int main() {
    initializeConnections();
    srand((unsigned int)time(0));

    const int boardSize = 5;
    const int boxSize = 200;
    const int windowWidth = 2 * boardSize * boxSize;
    const int windowHeight = boardSize * boxSize;

    RenderWindow window(VideoMode(windowWidth, windowHeight), "12 BEAD GAME BY QASIM AND RAYAN");

    GameState currentState = MENU;
    CircleShape circles[boardSize][boardSize];
    bool rightClick = false;
    Vector2i rightClickPosition;
    Color rightClickColor;
    bool playerTurn = true;
    string tossMessage = "";
    string winnerName = "";
    string loadMessage = "";
    bool gameLoaded = false;

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                if (currentState == PLAYING) {
                    savegame(playerTurn, circles);
                }
                window.close();
            }

            if (event.type == Event::KeyPressed) {
                if (currentState == MENU) {
                    if (event.key.code == Keyboard::N) {
                        // New game
                        initializeBoard(circles);
                        playerTurn = toss();
                        tossMessage = playerTurn ? "PLAYER ONE (GREEN) WINS THE TOSS!" : "PLAYER TWO (RED) WINS THE TOSS!";
                        currentState = TOSS_RESULT;
                    }
                    else if (event.key.code == Keyboard::L) {
                        // Load game
                        if (loadgame(playerTurn, circles)) {
                            loadMessage = "GAME LOADED SUCCESSFULLY!";
                            gameLoaded = true;
                        }
                        else {
                            loadMessage = "FAILED TO LOAD GAME! STARTING NEW GAME...";
                            initializeBoard(circles);
                            playerTurn = toss();
                            gameLoaded = false;
                        }
                        currentState = LOAD_RESULT;
                    }
                }
                else if (currentState == TOSS_RESULT || currentState == LOAD_RESULT) {
                    if (event.key.code == Keyboard::Enter) {
                        currentState = PLAYING;
                        rightClick = false;
                    }
                }
                else if (currentState == WIN_SCREEN) {
                    if (event.key.code == Keyboard::Y) {
                        // Play again
                        currentState = MENU;
                        rightClick = false;
                        tossMessage = "";
                        winnerName = "";
                        loadMessage = "";
                    }
                    else if (event.key.code == Keyboard::N) {
                        // Exit
                        window.close();
                    }
                }
                else if (currentState == PLAYING) {
                    // Space bar to deselect
                    if (event.key.code == Keyboard::Space && rightClick) {
                        circles[rightClickPosition.x][rightClickPosition.y].setFillColor(rightClickColor);
                        rightClick = false;
                    }
                    // S key to save game
                    else if (event.key.code == Keyboard::S) {
                        savegame(playerTurn, circles);
                    }
                }
            }

            if (currentState == PLAYING) {
                // Right-click to select a piece
                if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Right) {
                    int column = event.mouseButton.x / boxSize;
                    int row = event.mouseButton.y / boxSize;

                    if (column >= 0 && column < boardSize && row >= 0 && row < boardSize) {
                        Color currentColor = circles[column][row].getFillColor();

                        // Deselect if clicking on already selected piece
                        if (rightClick && column == rightClickPosition.x && row == rightClickPosition.y) {
                            circles[column][row].setFillColor(rightClickColor);
                            rightClick = false;
                        }
                        // Select new piece
                        else if (!rightClick &&
                            ((playerTurn && currentColor == Color::Green) ||
                                (!playerTurn && currentColor == Color::Red))) {
                            rightClickPosition = Vector2i(column, row);
                            rightClickColor = currentColor;
                            circles[column][row].setFillColor(Color::Yellow);
                            rightClick = true;
                        }
                        // Switch selection to different piece of same color
                        else if (rightClick &&
                            ((playerTurn && currentColor == Color::Green) ||
                                (!playerTurn && currentColor == Color::Red))) {
                            // Restore previous selection
                            circles[rightClickPosition.x][rightClickPosition.y].setFillColor(rightClickColor);
                            // Select new piece
                            rightClickPosition = Vector2i(column, row);
                            rightClickColor = currentColor;
                            circles[column][row].setFillColor(Color::Yellow);
                        }
                    }
                }

                // Left-click to move
                if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                    int column = event.mouseButton.x / boxSize;
                    int row = event.mouseButton.y / boxSize;

                    if (column >= 0 && column < boardSize && row >= 0 && row < boardSize &&
                        rightClick && circles[column][row].getFillColor() == Color::Transparent) {

                        bool validMove = false;
                        bool isCapture = false;
                        Vector2i capturedPos;

                        // Check for direct connection (single step move)
                        if (isValidConnection(rightClickPosition.x, rightClickPosition.y, column, row)) {
                            validMove = true;
                        }
                        // Check for jump move (capture)
                        else if (isValidJumpMove(rightClickPosition.x, rightClickPosition.y, column, row, circles, rightClickColor)) {
                            validMove = true;
                            isCapture = true;
                            capturedPos = getJumpedPosition(rightClickPosition.x, rightClickPosition.y, column, row);
                        }

                        if (validMove) {
                            // Move the piece
                            circles[column][row].setFillColor(rightClickColor);
                            circles[rightClickPosition.x][rightClickPosition.y].setFillColor(Color::Transparent);

                            // Handle capture
                            if (isCapture) {
                                circles[capturedPos.x][capturedPos.y].setFillColor(Color::Transparent);
                            }

                            rightClick = false;
                            playerTurn = !playerTurn;

                            // Count remaining pieces and check for win condition
                            int greenCount, redCount;
                            countPieces(circles, greenCount, redCount);

                            if (greenCount == 0) {
                                winnerName = "PLAYER RED WINS!";
                                savegame(playerTurn, circles);
                                currentState = WIN_SCREEN;
                            }
                            else if (redCount == 0) {
                                winnerName = "PLAYER GREEN WINS!";
                                savegame(playerTurn, circles);
                                currentState = WIN_SCREEN;
                            }
                        }
                    }
                }
            }
        }

        // Clear window with black background
        window.clear(Color::Black);

        if (currentState == MENU) {
            // Draw menu - vertically and horizontally centered
            float centerY = windowHeight / 2.0f;

            drawFullyCenteredText(window, "TWELVE BEADS GAME", centerY - 110, 60, Color::Cyan, windowWidth);

            drawFullyCenteredText(window, "Press 'N' for New Game", centerY - 30, 40, Color::Yellow, windowWidth);

            drawFullyCenteredText(window, "Press 'L' to Load Game", centerY + 10, 40, Color::Yellow, windowWidth);

            drawFullyCenteredText(window, "DEVELOPED BY:", centerY + 100, 30, Color::Blue, windowWidth);

            drawFullyCenteredText(window, "QASIM SHAKIL & RAYAN IRFAN", centerY + 130, 30, Color::White, windowWidth);
        }
        else if (currentState == TOSS_RESULT) {
            // Draw toss result - vertically and horizontally centered
            float centerY = windowHeight / 2.0f;

            drawFullyCenteredText(window, "TOSS RESULT", centerY - 80, 60, Color::Cyan, windowWidth);

            drawFullyCenteredText(window, tossMessage, centerY - 10, 40, playerTurn ? Color::Green : Color::Red, windowWidth);

            drawFullyCenteredText(window, "Press ENTER to continue", centerY + 60, 30, Color::Yellow, windowWidth);
        }
        else if (currentState == LOAD_RESULT) {
            // Draw load result - vertically and horizontally centered
            float centerY = windowHeight / 2.0f;

            drawFullyCenteredText(window, "LOAD GAME", centerY - 80, 60, Color::Cyan, windowWidth);

            drawFullyCenteredText(window, loadMessage, centerY - 10, 40, gameLoaded ? Color::Green : Color::Red, windowWidth);

            drawFullyCenteredText(window, "Press ENTER to continue", centerY + 60, 30, Color::Yellow, windowWidth);
        }
        else if (currentState == PLAYING) {
            // Draw the board lines first (behind the pieces)
            drawBoardLines(window, boxSize);

            // Draw the board pieces on top of lines
            for (int i = 0; i < boardSize; ++i) {
                for (int j = 0; j < boardSize; ++j) {
                    window.draw(circles[i][j]);
                }
            }

            // Count and display current pieces
            int greenCount, redCount;
            countPieces(circles, greenCount, redCount);

            // Draw UI text on the right side
            drawRightAlignedText(window, "TWELVE BEADS GAME\n\n", 50, 50, Color::Cyan, windowWidth);
            
            drawRightAlignedText(window, "\n\n\nControls:", 130, 42, Color::Magenta, windowWidth);
            drawRightAlignedText(window, "\n\n\n\n\n\nRight-click: Select piece", 170, 30, Color::Yellow, windowWidth);
            drawRightAlignedText(window, "\n\n\n\n\n\nLeft-click: Move piece", 200, 30, Color::Yellow, windowWidth);
            drawRightAlignedText(window, "\n\n\n\n\n\nSpace: Deselect", 230, 30, Color::Yellow, windowWidth);
            drawRightAlignedText(window, "\n\n\n\n\n\nS: Save game", 260, 30, Color::Yellow, windowWidth);

            drawRightAlignedText(window, "\n\n\n\n\n\n\nPieces Count:", 320, 35, Color::White, windowWidth);
            drawRightAlignedText(window, "\n\n\n\n\n\n\n\nGreen: " + to_string(greenCount), 360, 30, Color::Green, windowWidth);
            drawRightAlignedText(window, "\n\n\n\n\n\n\n\nRed: " + to_string(redCount), 390, 30, Color::Red, windowWidth);

            if (playerTurn) {
                drawRightAlignedText(window, "\n\n\n\n\n\n\n\n\n\n\nPlayer One Turn (GREEN)", 450, 35, Color::Green, windowWidth);
            }
            else {
                drawRightAlignedText(window, "\n\n\n\n\n\n\n\n\n\n\nPlayer Two Turn (RED)", 450, 35, Color::Red, windowWidth);
            }

            drawRightAlignedText(window, "\n\n\n\n\n\n\n\n\n\n\n\nDEVELOPED BY:", 580, 45, Color::Blue, windowWidth);
            drawRightAlignedText(window, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nQASIM SHAKIL & RAYAN IRFAN", 610, 40, Color::White, windowWidth);
        }
        else if (currentState == WIN_SCREEN) {
            // Game Over screen - vertically and horizontally centered
            float centerY = windowHeight / 2.0f;

            drawFullyCenteredText(window, "GAME OVER", centerY - 120, 60, Color::Cyan, windowWidth);

            drawFullyCenteredText(window, winnerName, centerY - 50, 50, Color::Yellow, windowWidth);

            drawFullyCenteredText(window, "All enemy pieces eliminated!", centerY + 10, 35, Color::White, windowWidth);

            drawFullyCenteredText(window, "Press 'Y' to play again", centerY + 80, 35, Color::Green, windowWidth);

            drawFullyCenteredText(window, "Press 'N' to exit", centerY + 120, 35, Color::Red, windowWidth);
        }

        window.display();
    }

    return 0;
}




