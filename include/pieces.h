#pragma once

#include <stdint.h>

#define ulong uint64_t

extern ulong white_pawns;
extern ulong white_knights;
extern ulong white_bishops;
extern ulong white_rooks;
extern ulong white_queens;
extern ulong white_king;

extern ulong white_pieces;

extern ulong black_pawns;
extern ulong black_knights;
extern ulong black_bishops;
extern ulong black_rooks;
extern ulong black_queens;
extern ulong black_king;

extern ulong black_pieces;

extern ulong all_pieces;

extern void init_bitboards(void);



typedef enum{
    PIECE_NONE,
    PIECE_WP,
    PIECE_WN,
    PIECE_WB,
    PIECE_WR,
    PIECE_WQ,
    PIECE_WK,
    PIECE_BP,
    PIECE_BN,
    PIECE_BB,
    PIECE_BR,
    PIECE_BQ,
    PIECE_BK,
}PieceType;

typedef struct{
    PieceType piece;
    ulong *bitboard;
}PieceOnSquare;


static inline ulong flip_bitboard(ulong bitboard){
    return __builtin_bswap64(bitboard);
}

static inline PieceOnSquare get_piece_on_square(int square){
    ulong mask = 1ULL << square;
    PieceOnSquare result = {.piece = PIECE_NONE};
    if (!(all_pieces & mask))
        return result;
    if (white_pawns & mask){
        result.piece = PIECE_WP;
        result.bitboard = &white_pawns;
    } else if (white_knights & mask){
        result.piece = PIECE_WN;
        result.bitboard = &white_knights;
    } else if (white_bishops & mask){
        result.piece = PIECE_WB;
        result.bitboard = &white_bishops;
    } else if (white_rooks & mask){
        result.piece = PIECE_WR;
        result.bitboard = &white_rooks;
    } else if (white_queens & mask){
        result.piece = PIECE_WQ;
        result.bitboard = &white_queens;
    } else if (white_king & mask){
        result.piece = PIECE_WK;
        result.bitboard = &white_king;
    } else if (black_pawns & mask){
        result.piece = PIECE_BP;
        result.bitboard = &black_pawns;
    } else if (black_knights & mask){
        result.piece = PIECE_BN;
        result.bitboard = &black_knights;
    } else if (black_bishops & mask){
        result.piece = PIECE_BB;
        result.bitboard = &black_bishops;
    } else if (black_rooks & mask){
        result.piece = PIECE_BR;
        result.bitboard = &black_rooks;
    } else if (black_queens & mask){
        result.piece = PIECE_BQ;
        result.bitboard = &black_queens;
    } else if (black_king & mask){
        result.piece = PIECE_BK;
        result.bitboard = &black_king;
    }

    return result;
}

static inline void combine_bitboards(void){
    white_pieces = white_pawns | white_knights | white_bishops | white_rooks | white_queens | white_king;
    black_pieces = black_pawns | black_knights | black_bishops | black_rooks | black_queens | black_king;
    all_pieces = white_pieces | black_pieces;
}

static inline int is_white_piece(PieceType piece){
    return (piece == PIECE_WP || piece == PIECE_WN || piece == PIECE_WB || piece == PIECE_WR || piece == PIECE_WQ || piece == PIECE_WK);
}

extern ulong get_moves(int square, PieceType piece);
