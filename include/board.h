#pragma once
#include "pieces.h"

typedef enum{
    ERROR_NO_ERROR,
    ERROR_MOVING_NONE,
    ERROR_ILLEGAL_MOVE,

}ErrorCode;

extern void draw_bitboard(ulong bitboard);
extern void draw_board(void);

extern void reset_board();

extern ErrorCode make_move(int from, int to);

