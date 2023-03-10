#include <SFML/Graphics.hpp>
#include <iostream>
#include <windows.h>

const int arrSize = 120;

const int tileSize = 100;

int selected = -1;

const int bP = -1, bR = -2, bN = -3, bB = -4, bQ = -5, bK = -6;
const int wP = 1, wR = 2, wN = 3, wB = 4, wQ = 5, wK = 6;
const int width = 10;

sf::Sprite sprites[12];

struct Game {
    bool whitesTurn = true;
    int whiteCanEnPassant = 0, blackCanEnPassant = 0;
    bool whiteCanCastleLong = true, whiteCanCastleShort = true, blackCanCastleLong = true, blackCanCastleShort = true;
    int pawnPromotingTile = 0;
    int wKPos = 95, bKPos = 25;
    int gameState = 0; // 0 - hra pokraèuje, 1 - mat, 2 - pat
    int board[arrSize] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0,-2,-3,-4,-5,-6,-4,-3,-2, 0,
                           0,-1,-1,-1,-1,-1,-1,-1,-1, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
                           0, 2, 3, 4, 5, 6, 4, 3, 2, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
};

Game mainGame;

void update(Game &game, sf::RenderWindow &window, sf::Event &event);
void render(Game &game, sf::RenderWindow &window);
void move(Game &game, int (&board)[arrSize], int from, int to);
bool canMove(Game &game, int(&board)[arrSize], int from, int to);
int isMateOrStalemate(Game &game, int(&board)[arrSize], int kingPos);
bool tileIsAttacked(Game &game, int (&board)[arrSize], int tile);

int main()
{
    sf::RenderWindow window(sf::VideoMode(tileSize * 11, tileSize * 8), "Chess");

    sf::Texture t;
    t.loadFromFile("pieces.png");

    for (int y = 0; y < 2; y++) {
        for (int x = 0; x < 6; x++) {
            sprites[x + y * 6].setTexture(t);
            sprites[x + y * 6].setTextureRect(sf::IntRect(tileSize * x, tileSize * y, tileSize, tileSize));
        }
    }

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();


        }

        /* UPDATE AND RENDER */

        update(mainGame, window, event);
        render(mainGame, window);
    }

    return 0;
};

/// <summary>
/// Stará se o veškerý user input a pøevádí ho v tahy na šachovnici.
/// </summary>
/// <param name="game"></param>
/// <param name="window"></param>
/// <param name="event"></param>
void update(Game& game, sf::RenderWindow& window, sf::Event& event) {

    sf::Vector2i pos = sf::Mouse::getPosition(window);
                                                                                         // hráè drží figuru a neprobíhá promìna pìšáka
    if (event.type == sf::Event::MouseButtonReleased && event.key.code == sf::Mouse::Left && selected + 1 && !game.pawnPromotingTile) {
        if (pos.x <= 800 && canMove(game, game.board, selected, pos.x / tileSize + 1 + width * (pos.y / tileSize + 2))) {
            move(game, game.board, selected, pos.x / tileSize + 1 + width * (pos.y / tileSize + 2));
            game.whitesTurn = !game.whitesTurn;
            game.gameState = isMateOrStalemate(game, game.board, game.whitesTurn ? game.wKPos : game.bKPos);
        }
        selected = -1;
    }

    if (event.type == sf::Event::MouseButtonPressed && event.key.code == sf::Mouse::Left) {
        if (!game.pawnPromotingTile) {
            selected = pos.x / tileSize + 1 + width * (pos.y / tileSize + 2);
            if (!game.board[selected]) selected = -1;
        }

        // Když probíhá promìna pìšáka
        else if (pos.y >= 310 && pos.y <= 410) {
            if (pos.x >= 170 && pos.x <= 270) {
                game.board[game.pawnPromotingTile] = !game.whitesTurn ? wQ : bQ;
                game.pawnPromotingTile = false;
                game.gameState = isMateOrStalemate(game, game.board, game.whitesTurn ? game.wKPos : game.bKPos);
            }
            else if (pos.x >= 290 && pos.x <= 390) {
                game.board[game.pawnPromotingTile] = !game.whitesTurn ? wN : bN;
                game.pawnPromotingTile = false;
                game.gameState = isMateOrStalemate(game, game.board, game.whitesTurn ? game.wKPos : game.bKPos);
            }
            else if (pos.x >= 410 && pos.x <= 510) {
                game.board[game.pawnPromotingTile] = !game.whitesTurn ? wR : bR;
                game.pawnPromotingTile = false;
                game.gameState = isMateOrStalemate(game, game.board, game.whitesTurn ? game.wKPos : game.bKPos);
            }
            else if (pos.x >= 530 && pos.x <= 630) {
                game.board[game.pawnPromotingTile] = !game.whitesTurn ? wB : bB;
                game.pawnPromotingTile = false;
                game.gameState = isMateOrStalemate(game, game.board, game.whitesTurn ? game.wKPos : game.bKPos);
            }
        }
    }
};

/// <summary>
/// Vykresluje vše v oknì hry.
/// </summary>
/// <param name="game"></param>
/// <param name="window"></param>
void render(Game& game, sf::RenderWindow& window) {
    sf::Vector2i pos = sf::Mouse::getPosition(window);

    window.clear();

    // Vykresluje šachovnici
    for (int i = 0; i < 64; i++) {
        sf::RectangleShape square(sf::Vector2f(tileSize, tileSize));
        if (((i + i / 8) % 2)) {
            square.setPosition(sf::Vector2f((i % 8) * tileSize, (i / 8) * tileSize));
            square.setFillColor(sf::Color(80, 40, 27));
        }
        else {
            square.setPosition(sf::Vector2f((i % 8) * tileSize, (i / 8) * tileSize));
            square.setFillColor(sf::Color::White);
        }
        window.draw(square);
    }

    // Vykresluje pole, kam se mùže figura pohnout
    if (selected + 1) {
        for (int i = 0; i < arrSize; i++) {
            if (canMove(game, game.board, selected, i)) {
                sf::RectangleShape square(sf::Vector2f(tileSize, tileSize));
                square.setPosition(sf::Vector2f((i % width - 1) * tileSize, (i / width - 2) * tileSize));
                square.setFillColor(sf::Color::Red);
                window.draw(square);
            }
        }

    }

    // Vykresluje figury
    for (int i = 21; i < arrSize; i++) {
        if (game.board[i] != 0) {
            //kromì zrovna držené
            if (i == selected) continue;

            int index = game.board[i] > 0 ? (game.board[i] - 1) : (-game.board[i] + 5);
            sprites[index].setPosition((i % width - 1) * tileSize, (i / width - 2) * tileSize);
            window.draw(sprites[index]);
        }
    }

    // Vykresluje drženou figuru
    if (selected + 1) {
        int index = game.board[selected] > 0 ? (game.board[selected] - 1) : (abs(game.board[selected]) + 5);
        sprites[index].setPosition(pos.x - tileSize / 2, pos.y - tileSize / 2);
        window.draw(sprites[index]);
    }

    // Renderuje výbìr pøi promoci pìšáka
    if (game.pawnPromotingTile) {
        sf::RectangleShape rect(sf::Vector2f(tileSize * 8, tileSize * 8));
        rect.setFillColor(sf::Color(0, 0, 0, 100));
        window.draw(rect);

        rect.setSize(sf::Vector2f(tileSize * 8, tileSize * 2));
        rect.setPosition(sf::Vector2f(0, 260));
        rect.setFillColor(sf::Color(255, 255, 255, 200));
        window.draw(rect);

        //4,2,1,3
        int black = !game.whitesTurn ? 0 : 6;
        sprites[4 + black].setPosition(170, 310);
        window.draw(sprites[4 + black]);
        sprites[2 + black].setPosition(290, 310);
        window.draw(sprites[2 + black]);
        sprites[1 + black].setPosition(410, 310);
        window.draw(sprites[1 + black]);
        sprites[3 + black].setPosition(530, 310);
        window.draw(sprites[3 + black]);
    }

    // renderuje mat nebo pat
    sf::Texture texture;
    texture.loadFromFile("checkmate_stalemate.png");
    if (game.gameState) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setPosition(sf::Vector2f(230, 280));

        if (game.gameState == 1) {
            if (game.whitesTurn) {
                sprite.setTextureRect(sf::IntRect(0, 0, 340, 240));
            }
            else {
                sprite.setTextureRect(sf::IntRect(341, 0, 340, 240));
            }
        }
        else if (game.gameState == 2) {
            sprite.setTextureRect(sf::IntRect(0, 241, 340, 240));
        }
        window.draw(sprite);
    }

    // renderuje, èí je tah
    if (game.whitesTurn) {
        sf::RectangleShape rect(sf::Vector2f(300, 800));
        rect.setPosition(800, 0);
        rect.setFillColor(sf::Color::White);
        window.draw(rect);

        sf::Sprite sprite(texture, sf::IntRect(360, 360, 300, 120));
        sprite.setPosition(sf::Vector2f(800, 340));
        window.draw(sprite);
    }
    else {
        sf::Sprite sprite(texture, sf::IntRect(360, 240, 300, 120));
        sprite.setPosition(sf::Vector2f(800, 340));
        window.draw(sprite);
    }

    window.display();

};

/// <summary>
/// Vrací true, pokud protihráè ovládá dané pole. V opaèném pøípadì vrací false.
/// </summary>
/// <param name="game"></param>
/// <param name="board"></param>
/// <param name="tile"></param>
/// <returns></returns>
bool tileIsAttacked(Game& game, int(&board)[arrSize], int tile) {
    if (game.whitesTurn) {
        // PÌŠÁCI
        if (board[tile - width - 1] == bP || board[tile - width + 1] == bP) {
            return true;
        }

        // VÌŽE + DÁMA
        // øádek
        for (int i = tile + 1; i % width <= 8; i++) {
            if (board[i]) {
                if (board[i] == bR || board[i] == bQ) return true;
                break;
            }
        }
        for (int i = tile - 1; i % width >= 1; i--) {
            if (board[i]) {
                if (board[i] == bR || board[i] == bQ) return true;
                break;
            }
        }
        // sloupec
        for (int i = tile + width; i / width <= 9; i += width) {
            if (board[i]) {
                if (board[i] == bR || board[i] == bQ) return true;
                break;
            }
        }
        for (int i = tile - width; i / width >= 2; i -= width) {
            if (board[i]) {
                if (board[i] == bR || board[i] == bQ) return true;
                break;
            }
        }

        // KONÌ
        if (board[tile - 21] == bN || board[tile + 21] == bN || board[tile - 19] == bN || board[tile + 19] == bN ||
            board[tile - 8] == bN || board[tile + 8] == bN || board[tile - 12] == bN || board[tile + 12] == bN) return true;

        // STØELCI + DÁMA
        // nahoøe vlevo až dole vpravo
        for (int i = tile - width - 1; i % width >= 1 && i / width >= 2; i -= width + 1) {
            if (board[i]) {
                if (board[i] == bB || board[i] == bQ) return true;
                break;
            }
        }
        for (int i = tile + width + 1; i % width <= 8 && i / width <= 9; i += width + 1) {
            if (board[i]) {
                if (board[i] == bB || board[i] == bQ) return true;
                break;
            }
        }
        // nahoøe vpravo až dole vlevo
        for (int i = tile - width + 1; i % width <= 8 && i / width >= 2; i -= width - 1) {
            if (board[i]) {
                if (board[i] == bB || board[i] == bQ) return true;
                break;
            }
        }
        for (int i = tile + width - 1; i % width >= 1 && i / width <= 9; i += width - 1) {
            if (board[i]) {
                if (board[i] == bB || board[i] == bQ) return true;
                break;
            }
        }

        // KRÁL
        if (board[tile - 11] == bK || board[tile + 11] == bK || board[tile - 10] == bK || board[tile + 10] == bK ||
            board[tile - 9] == bK || board[tile + 9] == bK || board[tile - 1] == bK || board[tile + 1] == bK) return true;
    }
    else {
        // PÌŠÁCI
        if (board[tile + width - 1] == wP || board[tile + width + 1] == wP) {
            return true;
        }

        // VÌŽE + DÁMA
        // øádek
        for (int i = tile + 1; i % width <= 8; i++) {
            if (board[i]) {
                if (board[i] == wR || board[i] == wQ) return true;
                break;
            }
        }
        for (int i = tile - 1; i % width >= 1; i--) {
            if (board[i]) {
                if (board[i] == wR || board[i] == wQ) return true;
                break;
            }
        }
        // sloupec
        for (int i = tile + width; i / width <= 9; i += width) {
            if (board[i]) {
                if (board[i] == wR || board[i] == wQ) return true;
                break;
            }
        }
        for (int i = tile - width; i / width >= 2; i -= width) {
            if (board[i]) {
                if (board[i] == wR || board[i] == wQ) return true;
                break;
            }
        }

        // KONÌ
        if (board[tile - 21] == wN || board[tile + 21] == wN || board[tile - 19] == wN || board[tile + 19] == wN ||
            board[tile - 8] == wN || board[tile + 8] == wN || board[tile - 12] == wN || board[tile + 12] == wN) return true;

        // STØELCI + DÁMA
        // nahoøe vlevo až dole vpravo
        for (int i = tile - width - 1; i % width >= 1 && i / width >= 2; i -= width + 1) {
            if (board[i]) {
                if (board[i] == wB || board[i] == wQ) return true;
                break;
            }
        }
        for (int i = tile + width + 1; i % width <= 8 && i / width <= 9; i += width + 1) {
            if (board[i]) {
                if (board[i] == wB || board[i] == wQ) return true;
                break;
            }
        }
        // nahoøe vpravo až dole vlevo
        for (int i = tile - width + 1; i % width <= 8 && i / width >= 2; i -= width - 1) {
            if (board[i]) {
                if (board[i] == wB || board[i] == wQ) return true;
                break;
            }
        }
        for (int i = tile + width - 1; i % width >= 1 && i / width <= 9; i += width - 1) {
            if (board[i]) {
                if (board[i] == wB || board[i] == wQ) return true;
                break;
            }
        }

        // KRÁL
        if (board[tile - 11] == wK || board[tile + 11] == wK || board[tile - 10] == wK || board[tile + 10] == wK ||
            board[tile - 9] == wK || board[tile + 9] == wK || board[tile - 1] == wK || board[tile + 1] == wK) return 9;
    }
    return false;
}

/// <summary>
/// Vrací true, pokud je daný tah legální. V opaèném pøípadì vrací false.
/// </summary>
/// <param name="game"></param>
/// <param name="board"></param>
/// <param name="from"></param>
/// <param name="to"></param>
/// <returns></returns>
bool canMove(struct Game& game, int(&board)[arrSize], int from, int to) {
    if (to / width < 2 || to / width > 9 || to % width == 0 || to % width == 9) return false;
    //if (to == 110 || to == 1) return false; // absolutnì nerozumím, proè to bez tohole øádku nefunguje
    Game gameCopy;
    memcpy(&gameCopy, &game, sizeof(game));
    move(gameCopy, gameCopy.board, from, to);
    int kingPos = gameCopy.whitesTurn ? gameCopy.wKPos : gameCopy.bKPos;
    if (tileIsAttacked(gameCopy, gameCopy.board, kingPos)) return false;

    // ROŠÁDA
    if (board[from] == wK && !tileIsAttacked(game, board, 95)) {
        if ((to == 91 || to == 93) && game.whiteCanCastleLong && board[91] == wR && board[92] == 0 && !tileIsAttacked(game, board, 92) &&
            board[93] == 0 && !tileIsAttacked(game, board, 93) && board[94] == 0 && !tileIsAttacked(game, board, 94)) {
            return true;
        }
        if ((to == 97 || to == 98) && game.whiteCanCastleShort && board[96] == 0 && !tileIsAttacked(game, board, 96) &&
            board[97] == 0 && !tileIsAttacked(game, board, 97) && board[98] == wR) {
            return true;
        }
    }
    else if (board[from] == bK && !tileIsAttacked(game, board, 25)) {
        if ((to == 21 || to == 23) && game.blackCanCastleLong && board[21] == bR && board[22] == 0 && !tileIsAttacked(game, board, 22) &&
            board[23] == 0 && !tileIsAttacked(game, board, 23) && board[24] == 0 && !tileIsAttacked(game, board, 24)) {
            return true;
        }
        if ((to == 27 || to == 28) && game.blackCanCastleShort && board[26] == 0 && !tileIsAttacked(game, board, 26) && 
            board[27] == 0 && !tileIsAttacked(game, board, 27) && board[28] == bR) {
            return true;
        }
    }

    // OBECNÁ PRAVIDLA
    if (from == to ||
        (board[from] > 0 && board[to] > 0) ||
        (board[from] < 0 && board[to] < 0) ||
        (game.whitesTurn && board[from] < 0) ||
        (!game.whitesTurn && board[from] > 0))
        return false;

    // PÌŠÁCI
    if (board[from] == wP) {
        // o 1 dopøedu
        if (from - to == width && board[to] == 0) {
            return true;
        }
        // o 2 dopøedu, pokud je v základní pozici
        else if (from / width == 8 && to / width == 6 && from % width == to % width && board[to] == 0 && board[to + width] == 0) {
            return true;
        }
        // sebrání protihráèovy figury
        else if (board[to] < 0 && (from - to == width + 1 || from - to == width - 1) && abs(from % width - to % width) == 1) {
            return true;
        }
        // braní mimochodem
        else if (game.whiteCanEnPassant && board[to] == 0 && to - game.whiteCanEnPassant == -width && abs(from - game.whiteCanEnPassant) == 1) {
            return true;
        }

        return false;
    }

    if (board[from] == bP) {
        // o 1 dopøedu
        if (from - to == -width && board[to] == 0) {
            return true;
        }
        // o 2 dopøedu, pokud je v základní pozici
        else if (from / width == 3 && to / width == 5 && from % width == to % width && board[to] == 0 && board[to - width] == 0) {
            return true;
        }
        // sebrání protihráèovy figury
        else if (board[to] > 0 && (from - to == -width - 1 || from - to == -width + 1) && abs(from % width - to % width) == 1) {
            return true;
        }
        // braní mimochodem
        else if (game.blackCanEnPassant && board[to] == 0 && to - game.blackCanEnPassant == width && abs(from - game.blackCanEnPassant) == 1) {
            return true;
        }

        return false;
    }

    // OSTATNÍ FIGURY
    switch (abs(board[from])) {

        // VÌŽ
    case 2:
        // v øádku
        if (from / width == to / width) {
            if (from > to) {
                for (int i = from - 1; i > to; i--) {
                    if (board[i]) return false;
                }
                return true;
            }
            for (int i = from + 1; i < to; i++) if (board[i]) return false;
            return true;
        }
        // ve sloupci
        else if (from % width == to % width) {
            if (from > to) {
                for (int i = from - width; i > to; i -= width) if (board[i]) return false;
                return true;
            }
            for (int i = from + width; i < to; i += width) if (board[i]) return false;
            return true;
        }

        return false;

        // KÙÒ
    case 3:
        if (abs(from - to) == 21 || abs(from - to) == 19 || abs(from - to) == 12 || abs(from - to) == 8) {
            return true;
        }

        return false;

        // STØELEC
    case 4:
        // šikmo
        // z levého horního do pravého spodního
        if (from % width - from / width == to % width - to / width) {
            if (from > to) {
                for (int i = from - width - 1; i > to; i -= width + 1) if (board[i]) return false;
                return true;
            }
            for (int i = from + width + 1; i < to; i += width + 1) if (board[i]) return false;
            return true;
        }
        // z pravého horního do levého spodního
        else if (from % width + from / width == to % width + to / width) {
            if (from > to) {
                for (int i = from - width + 1; i > to; i -= width - 1) if (board[i]) return false;
                return true;
            }
            for (int i = from + width - 1; i < to; i += width - 1) if (board[i]) return false;
            return true;
        }

        return false;

        // DÁMA
    case 5:
        // v øádku
        if (from / width == to / width) {
            if (from > to) {
                for (int i = from - 1; i > to; i--) {
                    if (board[i]) return false;
                }
                return true;
            }
            for (int i = from + 1; i < to; i++) if (board[i]) return false;
            return true;
        }
        // ve sloupci
        else if (from % width == to % width) {
            if (from > to) {
                for (int i = from - width; i > to; i -= width) if (board[i]) return false;
                return true;
            }
            for (int i = from + width; i < to; i += width) if (board[i]) return false;
            return true;
        }

        // šikmo
        // z levého horního do pravého spodního
        if (from % width - from / width == to % width - to / width) {
            if (from > to) {
                for (int i = from - width - 1; i > to; i -= width + 1) if (board[i]) return false;
                return true;
            }
            for (int i = from + width + 1; i < to; i += width + 1) if (board[i]) return false;
            return true;
        }
        // z pravého horního do levého spodního
        else if (from % width + from / width == to % width + to / width) {
            if (from > to) {
                for (int i = from - width + 1; i > to; i -= width - 1) if (board[i]) return false;
                return true;
            }
            for (int i = from + width - 1; i < to; i += width - 1) if (board[i]) return false;
            return true;
        }

        return false;

        // KRÁL
    case 6:
        if (abs(from - to) == 11 || abs(from - to) == 10 || abs(from - to) == 9 || abs(from - to) == 1) {
            return true;
        }

        return false;
    }

    std::cout << "How did we get here?\n";
    return false;
};

/// <summary>
/// Provádí tah a to i v pøípadì, že není legální.
/// </summary>
/// <param name="game"></param>
/// <param name="board"></param>
/// <param name="from"></param>
/// <param name="to"></param>
void move(Game &game, int (&board)[arrSize], int from, int to) {
    if (game.whitesTurn) game.blackCanEnPassant = 0;
    else game.whiteCanEnPassant = 0;
    
    // ROŠÁDA
    if (board[from] == wK && from == 95) {
        if (to == 91 || to == 93) {
            board[91] = 0;
            board[93] = wK;
            board[94] = wR;
            board[95] = 0;
            game.wKPos = 93;
            return;
        }
        if (to == 97 || to == 98) {
            board[98] = 0;
            board[97] = wK;
            board[96] = wR;
            board[95] = 0; 
            game.wKPos = 97;
            return;
        }
    }
    else if (board[from] == bK && from == 25) {
        if (to == 21 || to == 23) {
            board[21] = 0;
            board[23] = bK;
            board[24] = bR;
            board[25] = 0;
            game.bKPos = 23;
            return;
        }
        if (game.blackCanCastleShort && board[26] == 0 && !tileIsAttacked(game, board, 26) && board[27] == 0 && !tileIsAttacked(game, board, 27)
            && board[28] == bR && (to == 27 || to == 29)) {
            board[28] = 0;
            board[27] = bK;
            board[26] = bR;
            board[25] = 0;
            game.bKPos = 27;
            return; 
        }
    }
    
    // PÌŠÁCI
    if (board[from] == wP) {
        // o 1 dopøedu
        if (from - to == width) {
            if (to / width == 2) {
                game.pawnPromotingTile = to;
            }
            board[to] = board[from];
            board[from] = 0;
            return;
        }
        // o 2 dopøedu, pokud je v základní pozici
        else if (from - to == width * 2) {
            game.blackCanEnPassant = to;
            board[to] = board[from];
            board[from] = 0;
            return;
        }
        // sebrání protihráèovy figury
        else if ((from - to == width + 1 || from - to == width - 1) && game.board[to] < 0) {
            if (to / width == 2) {
                game.pawnPromotingTile = to;
            }
            board[to] = board[from];
            board[from] = 0;
            return;
        }
        // en passant
        else {
            board[to] = board[from];
            board[from] = 0;
            board[to + width] = 0;
            return;
        }

        return;
    }

    if (board[from] == bP) {
        // o 1 dopøedu
        if (from - to == -width) {
            if (to / width == 9) {
                game.pawnPromotingTile = to;
            }
            board[to] = board[from];
            board[from] = 0;
            return;
        }
        // o 2 dopøedu, pokud je v základní pozici
        else if (from - to == -width * 2) {
            game.whiteCanEnPassant = to;
            board[to] = board[from];
            board[from] = 0;
            return;
        }
        // sebrání protihráèovy figury
        else if ((from - to == -width - 1 || from - to == -width + 1) && game.board[to] > 0) {
            if (to / width == 9) {
                game.pawnPromotingTile = to;
            }
            board[to] = board[from];
            board[from] = 0;
            return;
        }
        // en passant
        else {
            board[to] = board[from];
            board[from] = 0;
            board[to - width] = 0;
            return;
        }

        return;
    }

    // OSTATNÍ FIGURY
    switch (abs(board[from])) {

        // VÌŽ
    case 2:
        if (game.whitesTurn) {
            if (game.whiteCanCastleLong && from == 91) game.whiteCanCastleLong = false;
            else if (game.whiteCanCastleShort && from == 98) game.whiteCanCastleShort = false;
        }
        else {
            if (game.blackCanCastleLong && from == 21) game.blackCanCastleLong = false;
            else if (game.blackCanCastleShort && from == 28) game.blackCanCastleShort = false;
        }
        board[to] = board[from];
        board[from] = 0;

        return;

        // KÙÒ
    case 3:
        board[to] = board[from];
        board[from] = 0;

        return;

        // STØELEC
    case 4:
        board[to] = board[from];
        board[from] = 0;

        return;

        // DÁMA
    case 5:
        board[to] = board[from];
        board[from] = 0;

        return;

        // KRÁL
    case 6:
        if (game.whitesTurn) {
            game.wKPos = to;
            game.whiteCanCastleLong = false;
            game.whiteCanCastleShort = false;
        }
        else {
            game.bKPos = to;
            game.blackCanCastleLong = false;
            game.blackCanCastleShort = false;
        }
        board[to] = board[from];
        board[from] = 0;

        return;
    }
    
    std::cout << "How did we get here?\n";
    return;
}

/// <summary>
/// Vrátí 0 pokud to není ani mat ani pat
/// Vrátí 1 pro mat a 2 pro pat
/// </summary>
/// <param name="game"></param>
/// <param name="board"></param>
/// <param name="kingPos"></param>
/// <returns></returns>
int isMateOrStalemate(Game &game, int(&board)[arrSize], int kingPos) {
    if (game.whitesTurn) {
        for (int i = 0; i < arrSize; i++) {
            for (int j = 0; j < arrSize; j++) {
                if (board[i] > 0 && canMove(game, board, i, j)) {
                    return false;
                }
            }
        }
    }
    else {
        for (int i = 0; i < arrSize; i++) {
            for (int j = 0; j < arrSize; j++) {
                if (board[i] < 0 && canMove(game, board, i, j)) {
                    return false;
                }
            }
        }
    }

    if (tileIsAttacked(game, board, kingPos)) {
        std::cout << "CHECKMATE! " << (game.whitesTurn ? "Black" : "White") << " wins!";
        return 1;
    }
    std::cout << "Draw by STALEMATE!";
    return 2;
}
