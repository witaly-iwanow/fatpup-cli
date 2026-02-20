Sample console UI for the fatpup chess library. It is a user-vs-engine game based on a very basic minimax engine.

## Get the code

    git clone https://github.com/witaly-iwanow/fatpup-cli.git

As opposed to older SDL and SFML integrations, this repo uses CMake FetchContent, instead of git submodules (not a fan, sorry).

## Build and run
```
cmake -S . -B build
cmake --build build -j
./build/fatpup-cli
```
```
New game started. You play White.

8 ♖ ♘ ♗ ♕ ♔ ♗ ♘ ♖
7 ♙ ♙ ♙ ♙ ♙ ♙ ♙ ♙
6 □ ▧ □ ▧ □ ▧ □ ▧
5 ▧ □ ▧ □ ▧ □ ▧ □
4 □ ▧ □ ▧ □ ▧ □ ▧
3 ▧ □ ▧ □ ▧ □ ▧ □
2 ♟ ♟ ♟ ♟ ♟ ♟ ♟ ♟
1 ♜ ♞ ♝ ♛ ♚ ♝ ♞ ♜
  a b c d e f g h

Commands:
  e2e4 / e7e8q  make a move (use q/r/b/n for promotion)
  board          print board
  moves          list legal moves for side to move
  score          print current game score in PGN format
  back           take 1 move back (2 plies)
  getfen         print current position as FEN
  game [white|black]  start a new game and choose your side
  fen <string>   set position from FEN
  help           show commands
  quit           exit

White> e2e4
White played: e2-e4

8 ♖ ♘ ♗ ♕ ♔ ♗ ♘ ♖
7 ♙ ♙ ♙ ♙ ♙ ♙ ♙ ♙
6 □ ▧ □ ▧ □ ▧ □ ▧
5 ▧ □ ▧ □ ▧ □ ▧ □
4 □ ▧ □ ▧ ♟ ▧ □ ▧
3 ▧ □ ▧ □ ▧ □ ▧ □
2 ♟ ♟ ♟ ♟ □ ♟ ♟ ♟
1 ♜ ♞ ♝ ♛ ♚ ♝ ♞ ♜
  a b c d e f g h

Black (engine) is thinking...
Black played: e7-e5

8 ♖ ♘ ♗ ♕ ♔ ♗ ♘ ♖
7 ♙ ♙ ♙ ♙ ▧ ♙ ♙ ♙
6 □ ▧ □ ▧ □ ▧ □ ▧
5 ▧ □ ▧ □ ♙ □ ▧ □
4 □ ▧ □ ▧ ♟ ▧ □ ▧
3 ▧ □ ▧ □ ▧ □ ▧ □
2 ♟ ♟ ♟ ♟ □ ♟ ♟ ♟
1 ♜ ♞ ♝ ♛ ♚ ♝ ♞ ♜
  a b c d e f g h

White>
```
