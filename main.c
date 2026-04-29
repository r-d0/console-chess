#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ncurses.h>

#include "render.h"
#include "pieces.h"

#define MAP_BORDERS 3
#define SQUARE_SPACING 2

#define FILE_A 0x0101010101010101
#define FILE_B 0x0202020202020202
#define FILE_G 0x4040404040404040
#define FILE_H 0x8080808080808080

int white_to_move = 1;

int last_white_square = 0;
int last_black_square = 56;

typedef enum {
    IS_Picking,
    IS_Picked
}InteractiveState;

// LOOKUP TABLEs

ulong king_moves[64];
ulong knight_moves[64];

void fill_king_moves(){
    for (int square = 0; square < 63; square++){
        ulong b = 1ULL << square;
        ulong not_file_a = b & ~FILE_A;
        ulong not_file_h = b & ~FILE_H;

        ulong moves = 0;

        moves |= b << 8;
        moves |= b >> 8;

        moves |= not_file_h << 1;
        moves |= not_file_a >> 1;

        moves |= not_file_h << 9;
        moves |= not_file_h >> 7;
        moves |= not_file_a << 7;
        moves |= not_file_a >> 9;
        king_moves[square] = moves;
    }
}

void fill_knight_moves(){
    for (int square = 0; square < 63; square++){
        ulong b = 1ULL << square;
        ulong moves = 0;
        moves |= (b & ~(FILE_H | FILE_G)) << 10;
        moves |= (b & ~(FILE_A | FILE_B)) << 6;
        moves |= (b & ~(FILE_H | FILE_G)) >> 6;
        moves |= (b & ~(FILE_A | FILE_B)) >> 10;

        moves |= (b & ~FILE_H) << 17;
        moves |= (b & ~FILE_A) << 15;
        moves |= (b & ~FILE_H) >> 15;
        moves |= (b & ~FILE_A) >> 17;
        knight_moves[square] = moves;
    }
}

ulong available_squares = 0ULL;

InteractiveState current_IS = IS_Picking;

int selected_square = 56;
int moving_to_square = -1;

static inline void get_piece_letter(const PieceType piece, char *letter){
    if (piece == PIECE_WP || piece == PIECE_BP)
        *letter = 'P';
    else if (piece == PIECE_WN || piece == PIECE_BN)
        *letter = 'N';
    else if (piece == PIECE_WB || piece == PIECE_BB)
        *letter = 'B';
    else if (piece == PIECE_WR || piece == PIECE_BR)
        *letter = 'R';
    else if (piece == PIECE_WQ || piece == PIECE_BQ)
        *letter = 'Q';
    else if (piece == PIECE_WK || piece == PIECE_BK)
        *letter = 'K';

}

ulong get_available_moves(PieceType piece, int square){
    ulong available_moves = 0ULL;
    switch(piece){
        case PIECE_WP:
            break;
        case PIECE_WB:
            break;
        case PIECE_WN:
            {
                ulong moves = knight_moves[square];
                return moves & ~white_pieces;
            }
            break;
        case PIECE_WR:
            break;
        case PIECE_WQ:
            break;
        case PIECE_WK:
            {
                ulong moves = king_moves[square];
                return moves;// & ~white_pieces;
            }
        case PIECE_BP:
            break;
        case PIECE_BB:
            break;
        case PIECE_BN:
            {
                ulong moves = knight_moves[square];
                return moves & ~black_pieces;
            }
            break;
        case PIECE_BR:
            break;
        case PIECE_BQ:
            break;
        case PIECE_BK:
            {
                ulong moves = king_moves[square];
                return moves & ~black_pieces;
            }
        default:
            break;
    }
    return available_moves;
}

void draw_board_temp(){
    int x,y;
    x=y=MAP_BORDERS;
    for (int rank = 7; rank >= 0; rank--){
        for (int file = 0; file <= 7; file++){
            char piece_letter= '.';
            int square = rank * 8 + file;
            PieceOnSquare p = get_piece_on_square(square);
            PieceType piece = p.piece;
            if (piece != PIECE_NONE){
                get_piece_letter(p.piece, &piece_letter);
                int white_piece = is_white_piece(piece);
                int color = !white_piece + 1;
                if (1ULL << square & available_squares)
                    color= !white_to_move + 3;
                if (square == selected_square)
                    color = 6;
                if (square == moving_to_square)
                    color = 7;
                attron(COLOR_PAIR(color));
                mvprintw(y,x,"%c",piece_letter);
                attroff(COLOR_PAIR(color));
            }else{
                int color = 0;
                if (1ULL << square & available_squares)
                    color = !white_to_move+3;
                if (square == moving_to_square)
                    color = 7;
                attron(COLOR_PAIR(color));
                mvprintw(y,x,"%c",piece_letter);
                attroff(COLOR_PAIR(color));
            }
            x+=SQUARE_SPACING*2;
        }
        x=MAP_BORDERS;
        y+=SQUARE_SPACING;
    }
    attron(COLOR_PAIR(5));
    mvhline(MAP_BORDERS + 7 * SQUARE_SPACING+1, MAP_BORDERS, '_', 15*SQUARE_SPACING);
    mvvline(MAP_BORDERS, MAP_BORDERS + 15 * SQUARE_SPACING, '|', 8*SQUARE_SPACING);
    for (int i = 0; i <= 7; i++){
        mvprintw( MAP_BORDERS + i*SQUARE_SPACING, MAP_BORDERS + 16*SQUARE_SPACING,"%d",8-i);
        mvprintw( MAP_BORDERS + 8 * SQUARE_SPACING+1, MAP_BORDERS + 2*i*SQUARE_SPACING,"%c",'a'+i);
    }
    x=y=MAP_BORDERS;
    attroff(COLOR_PAIR(5));
}

typedef enum{
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
}Direction;

char error[256];

int find_new_square(Direction direction){
    switch(direction){
        case DIR_UP:
            {
                int rank = moving_to_square >> 3;
                int file = moving_to_square & 7;
                ulong viable_squares = available_squares | (1ULL << selected_square);
                ulong mask = FILE_A << file;
                ulong moves = viable_squares & mask;
                for (int r = rank + 1; r <= 7; r++){
                    int target = (r << 3) | file;
                    if (moves & (1ULL << target))
                        return target;
                }

                for (int r = rank + 1; r <= 7; r++){
                    int left_file, right_file;
                    left_file = right_file = file;
                    while((left_file > 0) || (right_file < 7)){
                        int r_bits = r << 3;
                        int right_square = r_bits | right_file, left_square = r_bits | left_file;
                        if (right_file < 7){
                            right_file++;
                            if (viable_squares & (1ULL << right_square))
                                return right_square;
                        }
                        if (left_file > 0){
                            left_file--;
                            if (viable_squares & (1ULL << left_square))
                                return left_square;
                        }
                    }
                }
            }
            break;
        case DIR_DOWN:
            {
                int rank = moving_to_square >> 3;
                int file = moving_to_square & 7;
                ulong viable_squares = available_squares | (1ULL << selected_square);
                ulong mask = FILE_A << file;
                ulong moves = viable_squares & mask;
                for (int r = rank - 1; r >= 0; r--){
                    int target = (r << 3) | file;
                    if (moves & (1ULL << target))
                        return target;
                }
                for (int r = rank - 1; r >= 0; r--){
                    int left_file, right_file;
                    left_file = right_file = file;
                    while(left_file > 0 || right_file < 7){
                        int right_square = r << 3 | right_file, left_square = r << 3 | left_file;
                        if (right_file < 7){
                            right_file++;
                            if (viable_squares & (1ULL << right_square))
                                return right_square;
                        }
                        if (left_file > 0){
                            left_file--;
                            if (viable_squares & (1ULL << left_square))
                                return left_square;
                        }
                    }
                }


            }
            break;
        case DIR_LEFT:
            {
                int rank = moving_to_square >> 3;
                int file = moving_to_square & 7;
                ulong viable_squares = available_squares | (1ULL << selected_square);
                ulong mask = 0xffULL << (rank << 3);
                ulong moves = viable_squares & mask;
                moves &= ((1ULL << moving_to_square) - 1);
                if (moves)
                    return 63 - __builtin_clzll(moves);
                for (int f = file - 1; f >= 0; f--){
                    int up_rank, down_rank;
                    up_rank = down_rank = rank;
                    while(up_rank < 7 || down_rank > 0){
                        int down_square = down_rank << 3 | f, up_square = up_rank << 3 | f;
                        if (down_rank > 0){
                            down_rank--;
                            if (viable_squares & (1ULL << down_square))
                                return down_square;
                        }
                        if (up_rank < 7){
                            up_rank++;
                            if (viable_squares & (1ULL << up_square))
                                return up_square;
                        }
                    }
                }

            }
            break;
        case DIR_RIGHT:
            {
                int rank = moving_to_square >> 3;
                int file = moving_to_square & 7;
                ulong viable_squares = available_squares | (1ULL << selected_square);
                ulong mask = 0xffULL << (rank << 3);
                ulong moves = viable_squares & mask;
                moves &= (~0ULL << (moving_to_square + 1));
                if (moves)
                    return __builtin_ctzll(moves);

                for (int f = file + 1; f <= 7; f++){
                    int up_rank, down_rank;
                    up_rank = down_rank = rank;
                    while(down_rank > 0 || up_rank < 7){
                        int down_square = down_rank << 3 | f, up_square = up_rank << 3 | f;
                        if (down_rank > 0){
                            down_rank--;
                            if (viable_squares & (1ULL << down_square))
                                return down_square;
                        }
                        if (up_rank < 7){
                            up_rank++;
                            if (viable_squares & (1ULL << up_square))
                                return up_square;
                        }
                    }
                }


            }
            break;
    }
    return -1;
}


void handle_movement(const int key){
    if (current_IS == IS_Picking){
        if (key == 'w' || key == KEY_UP){
            if ((selected_square >> 3) < 7){
                int attempt_square = selected_square + 8;
                PieceOnSquare p = get_piece_on_square(attempt_square);
                if (p.piece != PIECE_NONE){
                    ulong check_bitboard = (white_to_move) ? white_pieces : black_pieces;
                    if ((1ULL << attempt_square) & check_bitboard)
                        selected_square = attempt_square;
                }
            }
        } else if (key == 's' || key == KEY_DOWN){
            if ((selected_square >> 3) > 0){
                int attempt_square = selected_square - 8;
                PieceOnSquare p = get_piece_on_square(attempt_square);
                if (p.piece != PIECE_NONE){
                    ulong check_bitboard = (white_to_move) ? white_pieces : black_pieces;
                    if ((1ULL << attempt_square) & check_bitboard)
                        selected_square = attempt_square;
                }
            }
        } else if (key == 'a' || key == KEY_LEFT){
            if ((selected_square & 7) > 0){
                int attempt_square = selected_square - 1;
                PieceOnSquare p = get_piece_on_square(attempt_square);
                if (p.piece != PIECE_NONE){
                    ulong check_bitboard = (white_to_move) ? white_pieces : black_pieces;
                    if ((1ULL << attempt_square) & check_bitboard)
                        selected_square = attempt_square;
                }
            }
        } else if (key == 'd' || key == KEY_RIGHT){
            if ((selected_square & 7) < 7){
                int attempt_square = selected_square + 1;
                PieceOnSquare p = get_piece_on_square(attempt_square);
                if (p.piece != PIECE_NONE){
                    ulong check_bitboard = (white_to_move) ? white_pieces : black_pieces;
                    if ((1ULL << attempt_square) & check_bitboard)
                        selected_square = attempt_square;
                }
            }
        } else if (key == '\n' || key == ' '){
            PieceOnSquare pos = get_piece_on_square(selected_square);
            PieceType piece = pos.piece;
            if (piece == PIECE_NONE) return;
            if (is_white_piece(piece) == white_to_move){
                current_IS = IS_Picked;
                available_squares = get_available_moves(piece, selected_square);
                moving_to_square = selected_square;
                if (white_to_move){
                    last_white_square = selected_square;
                }else{
                    last_black_square = selected_square;
                }
            }
        }
    } else if (current_IS == IS_Picked){
        if (key == ' ' || key == '\n'){

            if (selected_square != moving_to_square){
                white_to_move = !white_to_move;
                PieceOnSquare p1 = get_piece_on_square(selected_square);
                PieceOnSquare p2 = get_piece_on_square(moving_to_square);
                *p1.bitboard &= ~(1ULL << selected_square);
                *p1.bitboard |= (1ULL << moving_to_square);
                if (p2.bitboard)
                    *p2.bitboard &= ~(1ULL << moving_to_square);
                combine_bitboards();
            }

            if (white_to_move){
                last_black_square = moving_to_square;
                selected_square = (1ULL << last_white_square & white_pieces) ? last_white_square : __builtin_ctzll(white_pieces);
            } else{
                last_white_square = moving_to_square;
                selected_square = (1ULL << last_black_square & black_pieces) ? last_black_square : __builtin_ctzll(black_pieces);
            }
            current_IS = IS_Picking;
            available_squares = 0ULL;
            moving_to_square = -1;
        } else if (key == 'w' || key == KEY_UP){
            int new_square = find_new_square(DIR_UP);
            if (new_square != -1)
                moving_to_square = new_square;
        } else if (key == 's' || key == KEY_DOWN){
            int new_square = find_new_square(DIR_DOWN);
            if (new_square != -1){
                moving_to_square = new_square;
            }
        } else if (key == 'a' || key == KEY_LEFT){
            int new_square = find_new_square(DIR_LEFT);
            if (new_square != -1)
                moving_to_square = new_square;
        } else if (key == 'd' || key == KEY_RIGHT){
            int new_square = find_new_square(DIR_RIGHT);
            if (new_square != -1)
                moving_to_square = new_square;
        }
    }
}


int main(){
    init_bitboards();
    initscr();
    start_color();
    use_default_colors();
    init_pair(1, COLOR_YELLOW, -1); // White pieces
    init_pair(2, COLOR_RED, -1); // Black pieces
    init_pair(3, -1, COLOR_YELLOW); // White moves
    init_pair(4, -1, COLOR_RED); // Black moves
    init_pair(5, COLOR_BLUE, -1); // Board borders
    init_pair(6, -1, COLOR_BLUE); // Picking
    init_pair(7, -1, COLOR_GREEN); // Selected piece
    keypad(stdscr,1);
    noecho();
    curs_set(0);
    selected_square = last_white_square;
    int running = 1;
    int key;


    fill_king_moves();
    fill_knight_moves();

    while(running){
        erase();
        mvprintw(0,0,"%d",moving_to_square);
        mvprintw(1,0,"%d",selected_square);
        attron(COLOR_PAIR(5) | A_UNDERLINE);
        mvprintw(MAP_BORDERS - 2, MAP_BORDERS, "%s to move", (white_to_move) ? "White" : "Black");
        attroff(COLOR_PAIR(5) | A_UNDERLINE);
        mvprintw(30,0,"%s",error);
        draw_board_temp();
        refresh();
        key = getch();
        if (key == 'q')
            running = 0;
        handle_movement(key);
    }

    endwin();

    return 0;
}
