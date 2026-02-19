#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "fatpup/engine.h"
#include "fatpup/position.h"
#include "fatpup/square.h"

namespace
{
constexpr const char* kAnsiYellow = "\033[33m";
constexpr const char* kAnsiReset = "\033[0m";

const char* PieceToSymbol(const fatpup::Square& square)
{
    switch (square.pieceWithColor())
    {
    case fatpup::Pawn | fatpup::Black: return u8"♙";
    case fatpup::Knight | fatpup::Black: return u8"♘";
    case fatpup::Bishop | fatpup::Black: return u8"♗";
    case fatpup::Rook | fatpup::Black: return u8"♖";
    case fatpup::Queen | fatpup::Black: return u8"♕";
    case fatpup::King | fatpup::Black: return u8"♔";
    case fatpup::Pawn | fatpup::White: return u8"♟";
    case fatpup::Knight | fatpup::White: return u8"♞";
    case fatpup::Bishop | fatpup::White: return u8"♝";
    case fatpup::Rook | fatpup::White: return u8"♜";
    case fatpup::Queen | fatpup::White: return u8"♛";
    case fatpup::King | fatpup::White: return u8"♚";
    default: return nullptr;
    }
}

char PieceToFenChar(const fatpup::Square& square)
{
    const unsigned char piece = square.piece();
    char symbol = 0;
    switch (piece)
    {
    case fatpup::Pawn: symbol = 'P'; break;
    case fatpup::Knight: symbol = 'N'; break;
    case fatpup::Bishop: symbol = 'B'; break;
    case fatpup::Rook: symbol = 'R'; break;
    case fatpup::Queen: symbol = 'Q'; break;
    case fatpup::King: symbol = 'K'; break;
    default: return 0;
    }

    if (!square.isWhite())
    {
        symbol = static_cast<char>(std::tolower(static_cast<unsigned char>(symbol)));
    }

    return symbol;
}

void PrintBoard(const fatpup::Position& position)
{
    std::cout << "\n";
    for (int row = fatpup::BOARD_SIZE - 1; row >= 0; --row)
    {
        std::cout << kAnsiYellow << (row + 1) << kAnsiReset << " ";
        for (int col = 0; col < fatpup::BOARD_SIZE; ++col)
        {
            const fatpup::Square square = position.square(row, col);
            const char* pieceSymbol = PieceToSymbol(square);
            const char* emptySymbol = (((row + col) & 1) ? u8"■" : " ");
            std::cout << (pieceSymbol ? pieceSymbol : emptySymbol) << " ";
        }
        std::cout << "\n";
    }
    std::cout << kAnsiYellow << "  a b c d e f g h" << kAnsiReset << "\n\n";
}

bool IsSquareSymbol(char file, char rank)
{
    const char fileLower = static_cast<char>(std::tolower(static_cast<unsigned char>(file)));
    return (fileLower >= 'a' && fileLower <= 'h') && (rank >= '1' && rank <= '8');
}

std::string NormalizeMoveInput(const std::string& rawInput)
{
    std::string normalized;
    normalized.reserve(rawInput.size());
    for (char c : rawInput)
    {
        if (std::isspace(static_cast<unsigned char>(c)) || c == '-' || c == 'x')
        {
            continue;
        }
        normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return normalized;
}

int PromotionPieceFromSymbol(char symbol)
{
    switch (symbol)
    {
    case 'q': return fatpup::Queen;
    case 'r': return fatpup::Rook;
    case 'b': return fatpup::Bishop;
    case 'n': return fatpup::Knight;
    default: return 0;
    }
}

bool ParseAndResolveMove(const fatpup::Position& position, const std::string& rawInput, fatpup::Move* resultMove, std::string* error)
{
    const std::string input = NormalizeMoveInput(rawInput);
    if (input.length() != 4 && input.length() != 5)
    {
        *error = "Move must look like e2e4 or e7e8q.";
        return false;
    }

    if (!IsSquareSymbol(input[0], input[1]) || !IsSquareSymbol(input[2], input[3]))
    {
        *error = "Square names must be in range a1..h8.";
        return false;
    }

    const int srcRow = fatpup::symbolToRowIdx(input[1]);
    const int srcCol = fatpup::symbolToColumnIdx(input[0]);
    const int dstRow = fatpup::symbolToRowIdx(input[3]);
    const int dstCol = fatpup::symbolToColumnIdx(input[2]);
    const std::vector<fatpup::Move> candidates = position.possibleMoves(srcRow, srcCol, dstRow, dstCol);
    if (candidates.empty())
    {
        *error = "Illegal move in current position.";
        return false;
    }

    if (candidates.size() == 1)
    {
        if (input.length() == 5)
        {
            const int requestedPromotion = PromotionPieceFromSymbol(input[4]);
            if (requestedPromotion == 0)
            {
                *error = "Promotion piece must be one of q, r, b, n.";
                return false;
            }

            if (candidates[0].fields.promoted_to != 0 && candidates[0].fields.promoted_to != requestedPromotion)
            {
                *error = "Promotion piece does not match legal move.";
                return false;
            }
        }

        *resultMove = candidates[0];
        return true;
    }

    if (input.length() != 5)
    {
        *error = "Promotion requires a 5th symbol (q, r, b, or n), for example a7a8q.";
        return false;
    }

    const int requestedPromotion = PromotionPieceFromSymbol(input[4]);
    if (requestedPromotion == 0)
    {
        *error = "Promotion piece must be one of q, r, b, n.";
        return false;
    }

    for (const fatpup::Move& move : candidates)
    {
        if (move.fields.promoted_to == requestedPromotion)
        {
            *resultMove = move;
            return true;
        }
    }

    *error = "Requested promotion piece is not legal for this move.";
    return false;
}

void PrintStatus(const fatpup::Position& position)
{
    const fatpup::Position::State state = position.getState();
    if (state == fatpup::Position::State::Check)
    {
        std::cout << "Check.\n";
    }
    else if (state == fatpup::Position::State::Checkmate)
    {
        std::cout << "Checkmate. " << (position.isWhiteTurn() ? "Black" : "White") << " wins.\n";
    }
    else if (state == fatpup::Position::State::Stalemate)
    {
        std::cout << "Stalemate.\n";
    }
}

void PrintHelp()
{
    std::cout
        << "Commands:\n"
        << "  e2e4 / e7e8q  make a move (use q/r/b/n for promotion)\n"
        << "  board          print board\n"
        << "  moves          list legal moves for side to move\n"
        << "  back           take 1 move back (2 plies)\n"
        << "  getfen         print current position as FEN\n"
        << "  game [white|black]  start a new game and choose your side\n"
        << "  fen <string>   set position from FEN\n"
        << "  help           show commands\n"
        << "  quit           exit\n\n";
}

void PrintLegalMoves(const fatpup::Position& position)
{
    const std::vector<fatpup::Move> moves = position.possibleMoves();
    if (moves.empty())
    {
        std::cout << "No legal moves.\n";
        return;
    }

    std::cout << "Legal moves (" << moves.size() << "): ";
    for (std::size_t i = 0; i < moves.size(); ++i)
    {
        if (i)
        {
            std::cout << ", ";
        }
        std::cout << position.moveToString(moves[i]);
    }
    std::cout << "\n";
}

void StartGame(fatpup::Position* position, fatpup::Engine* engine, bool userPlaysWhite)
{
    position->setInitial();
    engine->SetPosition(*position);
    std::cout << "New game started. You play " << (userPlaysWhite ? "White" : "Black") << ".\n";
    PrintBoard(*position);
}

std::string Trim(const std::string& text)
{
    std::size_t start = 0;
    while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start])))
    {
        ++start;
    }

    std::size_t end = text.size();
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1])))
    {
        --end;
    }

    return text.substr(start, end - start);
}

void ParseFenCounters(const std::string& fen, int* halfMoveClock, int* fullMoveNumber)
{
    *halfMoveClock = 0;
    *fullMoveNumber = 1;

    std::istringstream input(fen);
    std::vector<std::string> fields;
    std::string field;
    while (input >> field)
    {
        fields.push_back(field);
    }

    if (fields.size() >= 6)
    {
        try
        {
            const int parsedHalfMove = std::stoi(fields[4]);
            if (parsedHalfMove >= 0)
            {
                *halfMoveClock = parsedHalfMove;
            }
        }
        catch (...)
        {
        }

        try
        {
            const int parsedFullMove = std::stoi(fields[5]);
            if (parsedFullMove >= 1)
            {
                *fullMoveNumber = parsedFullMove;
            }
        }
        catch (...)
        {
        }
    }
}

void UpdateFenCountersForMove(const fatpup::Position& positionBeforeMove, fatpup::Move move, int* halfMoveClock, int* fullMoveNumber)
{
    const fatpup::Square srcSquare = positionBeforeMove.square(move.fields.src_row, move.fields.src_col);
    const bool isPawnMove = (srcSquare.piece() == fatpup::Pawn);
    const bool isCapture = positionBeforeMove.isMoveCapture(move);

    if (isPawnMove || isCapture)
    {
        *halfMoveClock = 0;
    }
    else
    {
        ++(*halfMoveClock);
    }

    if (!positionBeforeMove.isWhiteTurn())
    {
        ++(*fullMoveNumber);
    }
}

void ApplyMoveAndTrackCounters(fatpup::Position* position, fatpup::Move move, int* halfMoveClock, int* fullMoveNumber)
{
    UpdateFenCountersForMove(*position, move, halfMoveClock, fullMoveNumber);
    *position += move;
}

void RebuildFromHistory(
    const fatpup::Position& startPosition,
    const std::vector<fatpup::Move>& moveHistory,
    fatpup::Position* position,
    fatpup::Engine* engine,
    int startHalfMoveClock,
    int startFullMoveNumber,
    int* halfMoveClock,
    int* fullMoveNumber)
{
    *position = startPosition;
    *halfMoveClock = startHalfMoveClock;
    *fullMoveNumber = startFullMoveNumber;

    for (const fatpup::Move move : moveHistory)
    {
        ApplyMoveAndTrackCounters(position, move, halfMoveClock, fullMoveNumber);
    }

    engine->SetPosition(*position);
}

std::string PositionToFen(const fatpup::Position& position, int halfMoveClock, int fullMoveNumber)
{
    std::string fen;

    for (int row = fatpup::BOARD_SIZE - 1; row >= 0; --row)
    {
        int emptySquares = 0;
        for (int col = 0; col < fatpup::BOARD_SIZE; ++col)
        {
            const fatpup::Square square = position.square(row, col);
            const char pieceChar = PieceToFenChar(square);
            if (pieceChar == 0)
            {
                ++emptySquares;
                continue;
            }

            if (emptySquares > 0)
            {
                fen += static_cast<char>('0' + emptySquares);
                emptySquares = 0;
            }

            fen += pieceChar;
        }

        if (emptySquares > 0)
        {
            fen += static_cast<char>('0' + emptySquares);
        }

        if (row > 0)
        {
            fen += '/';
        }
    }

    fen += position.isWhiteTurn() ? " w " : " b ";

    std::string castlingAvailability;
    const fatpup::Square e1 = position.square("e1");
    const fatpup::Square e8 = position.square("e8");

    if (e1.pieceWithColor() == (fatpup::King | fatpup::White))
    {
        const fatpup::Square h1 = position.square("h1");
        const fatpup::Square a1 = position.square("a1");
        if (h1.pieceWithColor() == (fatpup::Rook | fatpup::White) && h1.isFlagSet(fatpup::CanCastle))
        {
            castlingAvailability += 'K';
        }
        if (a1.pieceWithColor() == (fatpup::Rook | fatpup::White) && a1.isFlagSet(fatpup::CanCastle))
        {
            castlingAvailability += 'Q';
        }
    }

    if (e8.pieceWithColor() == (fatpup::King | fatpup::Black))
    {
        const fatpup::Square h8 = position.square("h8");
        const fatpup::Square a8 = position.square("a8");
        if (h8.pieceWithColor() == (fatpup::Rook | fatpup::Black) && h8.isFlagSet(fatpup::CanCastle))
        {
            castlingAvailability += 'k';
        }
        if (a8.pieceWithColor() == (fatpup::Rook | fatpup::Black) && a8.isFlagSet(fatpup::CanCastle))
        {
            castlingAvailability += 'q';
        }
    }

    fen += castlingAvailability.empty() ? "-" : castlingAvailability;
    fen += " ";

    std::string enPassant = "-";
    for (int row = 0; row < fatpup::BOARD_SIZE; ++row)
    {
        for (int col = 0; col < fatpup::BOARD_SIZE; ++col)
        {
            if (position.square(row, col).isFlagSet(fatpup::EnPassant))
            {
                enPassant = std::string{
                    static_cast<char>('a' + col),
                    static_cast<char>('1' + row)
                };
                break;
            }
        }
        if (enPassant != "-")
        {
            break;
        }
    }
    fen += enPassant;
    fen += " ";
    fen += std::to_string(halfMoveClock);
    fen += " ";
    fen += std::to_string(std::max(1, fullMoveNumber));

    return fen;
}
} // namespace

int main()
{
    fatpup::Engine* engine = fatpup::Engine::Create("minimax");
    if (!engine)
    {
        std::cerr << "Failed to create minimax engine.\n";
        return 1;
    }

    fatpup::Position position;
    bool userPlaysWhite = true;
    std::vector<fatpup::Move> moveHistory;
    fatpup::Position startPosition;
    int startHalfMoveClock = 0;
    int startFullMoveNumber = 1;
    int halfMoveClock = 0;
    int fullMoveNumber = 1;

    StartGame(&position, engine, userPlaysWhite);
    startPosition = position;
    moveHistory.clear();
    halfMoveClock = startHalfMoveClock;
    fullMoveNumber = startFullMoveNumber;

    PrintHelp();
    PrintStatus(position);

    while (true)
    {
        const fatpup::Position::State state = position.getState();
        const bool gameOver = (state == fatpup::Position::State::Checkmate || state == fatpup::Position::State::Stalemate);
        const bool userToMove = (position.isWhiteTurn() == userPlaysWhite);

        if (!gameOver && !userToMove)
        {
            std::cout << (position.isWhiteTurn() ? "White" : "Black") << " (engine) is thinking...\n";
            fatpup::Move engineMove = engine->GetBestMove();
            if (engineMove.isEmpty())
            {
                std::cout << "Engine has no legal move.\n";
            }
            else
            {
                const bool engineMoveByWhite = position.isWhiteTurn();
                const std::string moveText = position.moveToString(engineMove);
                ApplyMoveAndTrackCounters(&position, engineMove, &halfMoveClock, &fullMoveNumber);
                moveHistory.push_back(engineMove);
                std::cout << (engineMoveByWhite ? "White" : "Black") << " played: " << moveText << "\n";
            }

            PrintBoard(position);
            PrintStatus(position);
            continue;
        }

        std::cout << (position.isWhiteTurn() ? "White" : "Black") << "> ";
        std::string line;
        if (!std::getline(std::cin, line))
        {
            break;
        }

        std::string command = line;
        std::transform(command.begin(), command.end(), command.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        if (command == "quit" || command == "exit")
        {
            break;
        }
        if (command == "help")
        {
            PrintHelp();
            continue;
        }
        if (command == "board")
        {
            PrintBoard(position);
            continue;
        }
        if (command == "moves")
        {
            PrintLegalMoves(position);
            continue;
        }
        if (command == "back")
        {
            if (moveHistory.empty())
            {
                std::cout << "No moves to take back.\n";
                continue;
            }

            const std::size_t pliesToRevert = std::min<std::size_t>(2, moveHistory.size());
            moveHistory.resize(moveHistory.size() - pliesToRevert);
            RebuildFromHistory(
                startPosition,
                moveHistory,
                &position,
                engine,
                startHalfMoveClock,
                startFullMoveNumber,
                &halfMoveClock,
                &fullMoveNumber);

            std::cout << "Reverted " << pliesToRevert << " ply.\n";
            PrintBoard(position);
            PrintStatus(position);
            continue;
        }
        if (command == "getfen")
        {
            std::cout << PositionToFen(position, halfMoveClock, fullMoveNumber) << "\n";
            continue;
        }
        if (command == "game" || (command.size() > 5 && command.substr(0, 5) == "game "))
        {
            const std::string side = Trim(command.substr(4));
            if (side.empty() || side == "white")
            {
                userPlaysWhite = true;
            }
            else if (side == "black")
            {
                userPlaysWhite = false;
            }
            else
            {
                std::cout << "Usage: game [white|black]\n";
                continue;
            }

            StartGame(&position, engine, userPlaysWhite);
            startPosition = position;
            moveHistory.clear();
            startHalfMoveClock = 0;
            startFullMoveNumber = 1;
            halfMoveClock = startHalfMoveClock;
            fullMoveNumber = startFullMoveNumber;
            PrintStatus(position);
            continue;
        }
        if (command.size() > 4 && command.substr(0, 4) == "fen ")
        {
            const std::string fen = line.substr(4);
            fatpup::Position newPosition;
            if (!newPosition.setFEN(fen))
            {
                std::cout << "Invalid FEN.\n";
                continue;
            }

            position = newPosition;
            engine->SetPosition(position);
            startPosition = position;
            moveHistory.clear();
            ParseFenCounters(fen, &startHalfMoveClock, &startFullMoveNumber);
            halfMoveClock = startHalfMoveClock;
            fullMoveNumber = startFullMoveNumber;
            PrintBoard(position);
            PrintStatus(position);
            continue;
        }

        if (gameOver)
        {
            std::cout << "Game is over. Use 'game [white|black]' or 'quit'.\n";
            continue;
        }

        if (!userToMove)
        {
            std::cout << "It's engine's turn.\n";
            continue;
        }

        fatpup::Move userMove;
        std::string parseError;
        if (!ParseAndResolveMove(position, line, &userMove, &parseError))
        {
            std::cout << parseError << "\n";
            continue;
        }

        const std::string moveText = position.moveToString(userMove);
        const bool userMoveByWhite = position.isWhiteTurn();
        ApplyMoveAndTrackCounters(&position, userMove, &halfMoveClock, &fullMoveNumber);
        engine->MoveDone(userMove);
        moveHistory.push_back(userMove);

        std::cout << (userMoveByWhite ? "White" : "Black") << " played: " << moveText << "\n";
        PrintBoard(position);
        PrintStatus(position);
    }

    delete engine;
    return 0;
}
