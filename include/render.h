#pragma once

extern char error_message[128];

typedef enum {ACTION_QUIT, ACTION_MOVE, ACTION_NONE} Action;

typedef struct{
    Action type;
    int from_square;
    int to_square;
} PlayerAction;
