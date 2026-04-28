#include "board.h"
#include "pieces.h"
#include "stdio.h"

int last_from_index = -1;
int last_to_index = -1;
char color[2];

void draw_bitboard(ulong bitboard){
    // Bitboards go from LSB a1 to MSB h8
    // Every byte is a row, every nth digit in a byte is in the nth column
    // row = rank, column = file.
    // To draw the bitboard, you want to draw starting with the last rank
    // Then draw every square of that rank, then continue
    
    for (int rank = 7; rank >=0; rank--){
        for (int file = 0; file <= 7; file++){
            int square = rank * 8 + file;
            if (bitboard & (1ULL << square)){
                printf("\033[32m" "P" "\033[0m "); 
            }else{
                printf(". ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

void draw_board(){
    for (int rank = 7; rank >= -2; rank--){
        if (rank == -1){
            printf("    ");
            for (int i = 0; i <= 7; i++)
                printf("--");
            printf("\n");
            continue;
        }else if (rank == -2){
            printf("    ");
            for (int file = 0; file <= 7; file++)
                printf("%c ", 'a' + file);
            printf("\n");
            continue;
        }

        for (int file = -2; file <= 7; file++){
            if (file == -2){
                printf("%d ", 1+rank);
                continue;
            } else if (file == -1){
                printf("| ");
                continue;
            }
            int square = rank * 8 + file;
            ulong mask = 1ULL << square;
            const char *white_start = "\033[33m";
            const char *black_start = "\033[31m";
            const char *empty_start = "\033[39;2m";
            const char *end = "\033[0m ";
            if (square == last_from_index || square == last_to_index){
                char start[10];
                sprintf(start, "\033[%s;7m",color);
                white_start = black_start = empty_start = start;
            }

            if (white_pawns & mask){
                printf("%sP%s",white_start,end);
            } else if (white_knights & mask){
                printf("%sN%s",white_start,end);
            } else if (white_bishops & mask){
                printf("%sB%s",white_start,end);
            } else if (white_rooks & mask){
                printf("%sR%s",white_start,end);
            } else if (white_queens & mask){
                printf("%sQ%s",white_start,end);
            } else if (white_king & mask){
                printf("%sK%s",white_start,end);
            } else if (black_pawns & mask){
                printf("%sP%s",black_start,end);
            } else if (black_knights & mask){
                printf("%sN%s",black_start,end);
            } else if (black_bishops & mask){
                printf("%sB%s",black_start,end);
            } else if (black_rooks & mask){
                printf("%sR%s",black_start,end);
            } else if (black_queens & mask){
                printf("%sQ%s",black_start,end);
            } else if (black_king & mask){
                printf("%sK%s",black_start,end);
            } else{
                printf("%s.%s",empty_start,end);
            }
        }
        printf("\n");
    }
    printf("\n");
}


ErrorCode make_move(int from, int to){
    PieceOnSquare from_piece = get_piece_on_square(from);
    PieceOnSquare to_piece = get_piece_on_square(to);

    if (from_piece.piece == PIECE_NONE)
        return ERROR_MOVING_NONE;
    if (!((1ULL << to) & get_moves(from, from_piece.piece))){
        return ERROR_ILLEGAL_MOVE;
    }

    last_from_index = from;
    last_to_index = to;

    sprintf(color, (is_white_piece(from_piece.piece)) ? "33" : "31");


    *from_piece.bitboard &= ~(1ULL << from);
    *from_piece.bitboard |= (1ULL << to);
    if (to_piece.bitboard){
        *to_piece.bitboard &= ~(1ULL << to);
    }
    combine_bitboards();
    return ERROR_NO_ERROR;
}

void reset_board(){
    last_from_index = -1;
    last_to_index = -1;
    init_bitboards();
}
