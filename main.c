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


ulong available_squares = 0ULL;


int selected_square = 56;
int moving_to_square = -1;


char error[256];

typedef enum {
    IS_Picking,
    IS_Picked
}InteractiveState;

InteractiveState current_IS = IS_Picking;

// LOOKUP TABLES

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

static inline int center_dist(int sq) {
    int rank = sq >> 3;
    int file = sq & 7;
    int dr = 2 * rank - 7;
    int df = 2 * file - 7;
    return dr * dr + df * df;
}

int find_square(Direction direction) {
    ulong check_bitboard;
    int check_square;
    if (moving_to_square == -1) {
        check_bitboard = (white_to_move) ? white_pieces : black_pieces;
        check_square = selected_square;
    } else {
        check_bitboard = available_squares;
        check_square = moving_to_square;
    }

    ulong viable_squares = (check_bitboard | (1ULL << selected_square)) & ~(1ULL << check_square);

    int src_rank = check_square >> 3;
    int src_file = check_square & 7;

    int best = -1;
    int best_primary = 0;
    int best_secondary = 0;

    while (viable_squares) {
        int sq = __builtin_ctzll(viable_squares);
        viable_squares &= viable_squares - 1; // pop lowest set bit

        int dy = (sq >> 3) - src_rank;
        int dx = (sq & 7)  - src_file;
        int abs_dx = dx < 0 ? -dx : dx;
        int abs_dy = dy < 0 ? -dy : dy;

        int primary, secondary;
        switch (direction) {
            case DIR_UP:    if (dy <= 0) continue; primary = dy;     secondary = abs_dx; break;
            case DIR_DOWN:  if (dy >= 0) continue; primary = abs_dy; secondary = abs_dx; break;
            case DIR_RIGHT: if (dx <= 0) continue; primary = dx;     secondary = abs_dy; break;
            case DIR_LEFT:  if (dx >= 0) continue; primary = abs_dx; secondary = abs_dy; break;
            default: continue;
        }

        if (best == -1 ||
                primary * best_secondary > best_primary * secondary || 
                (primary * best_secondary == best_primary * secondary &&
                 (primary < best_primary ||
                  (primary == best_primary && center_dist(sq) < center_dist(best)))))
        {
            best = sq;
            best_primary = primary;
            best_secondary = secondary;
        }
    }

    return best;
}




void handle_movement(const int key){
    if (current_IS == IS_Picking){
        if (key == 'w' || key == KEY_UP){
            int new_square = find_square(DIR_UP);
            if (new_square != -1)
                selected_square = new_square;
        } else if (key == 's' || key == KEY_DOWN){
            int new_square = find_square(DIR_DOWN);
            if (new_square != -1)
                selected_square = new_square;
        } else if (key == 'a' || key == KEY_LEFT){
            int new_square = find_square(DIR_LEFT);
            if (new_square != -1)
                selected_square = new_square;
        } else if (key == 'd' || key == KEY_RIGHT){
            int new_square = find_square(DIR_RIGHT);
            if (new_square != -1)
                selected_square = new_square;
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
    }else if (current_IS == IS_Picked){
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
                if (white_to_move){
                    last_black_square = moving_to_square;
                    selected_square = (1ULL << last_white_square & white_pieces) ? last_white_square : __builtin_ctzll(white_pieces);
                } else{
                    last_white_square = moving_to_square;
                    selected_square = (1ULL << last_black_square & black_pieces) ? last_black_square : __builtin_ctzll(black_pieces);
                }
            }

            current_IS = IS_Picking;
            available_squares = 0ULL;
            moving_to_square = -1;
        }else if (key == 27){
            current_IS = IS_Picking;
            available_squares = 0ULL;
            moving_to_square = -1;
        } else if (key == 'w' || key == KEY_UP){
            int new_square = find_square(DIR_UP);
            if (new_square != -1)
                moving_to_square = new_square;
        } else if (key == 's' || key == KEY_DOWN){
            int new_square = find_square(DIR_DOWN);
            if (new_square != -1){
                moving_to_square = new_square;
            }
        } else if (key == 'a' || key == KEY_LEFT){
            int new_square = find_square(DIR_LEFT);
            if (new_square != -1)
                moving_to_square = new_square;
        } else if (key == 'd' || key == KEY_RIGHT){
            int new_square = find_square(DIR_RIGHT);
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
    set_escdelay(0);

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
