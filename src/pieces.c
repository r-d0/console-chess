#include "pieces.h"



ulong white_pawns = 0ULL;
ulong white_knights = 0ULL;
ulong white_bishops = 0ULL;
ulong white_rooks = 0ULL;
ulong white_queens = 0ULL;
ulong white_king = 0ULL;

ulong white_pieces = 0ULL;

ulong black_pawns = 0ULL;
ulong black_knights = 0ULL;
ulong black_bishops = 0ULL;
ulong black_rooks = 0ULL;
ulong black_queens = 0ULL;
ulong black_king = 0ULL;

ulong black_pieces = 0ULL;

ulong all_pieces = 0ULL;


void init_bitboards(){
    white_pawns = 0xFF00ULL;
    white_knights = 0x42ULL;
    white_bishops = 0x24ULL;
    white_rooks = 0x81ULL;
    white_queens = 0x8ULL;
    white_king = 0x10ULL;


    black_pawns = flip_bitboard(white_pawns);
    black_knights = flip_bitboard(white_knights);
    black_bishops = flip_bitboard(white_bishops);
    black_rooks = flip_bitboard(white_rooks);
    black_queens = flip_bitboard(white_queens);
    black_king = flip_bitboard(white_king);

    combine_bitboards();
}

ulong get_king_moves(int square){
    int deltas[8][2] = {{0,1}, {0, -1}, {1,0}, {-1,0}, {1,1}, {-1, -1}, {1, -1}, {-1, 1}};
    int rank = square / 8;
    int file = square % 8;
    ulong mask = 0ULL;
    for (int i = 0; i < 8; i++){
        int new_file = file + deltas[i][0];
        int new_rank = rank + deltas[i][1];
        if (new_file < 0 || new_file > 7 || new_rank < 0 || new_rank > 7) continue;
        mask |= (1ULL << (new_rank * 8 + new_file));
    }

    return mask;
}

ulong get_knight_moves(int square){
    int deltas[8][2] = {{1,2}, {1, -2}, {2,1}, {-2,1}, {-1,-2}, {-2, -1}, {2, -1}, {-1, 2}};
    int rank = square / 8;
    int file = square % 8;
    ulong mask = 0ULL;
    for (int i = 0; i < 8; i++){
        int new_file = file + deltas[i][0];
        int new_rank = rank + deltas[i][1];
        if (new_file < 0 || new_file > 7 || new_rank < 0 || new_rank > 7) continue;
        mask |= (1ULL << (new_rank * 8 + new_file));
    }

    return mask;
}

ulong get_moves(int square, PieceType piece){
    switch (piece){
        case PIECE_WK: return get_king_moves(square) & ~white_pieces;
        case PIECE_BK: return get_king_moves(square) & ~black_pieces;
        case PIECE_WN: return get_knight_moves(square) & ~white_pieces;
        case PIECE_BN: return get_knight_moves(square) & ~black_pieces;
        default: return 0ULL;
    }
}
