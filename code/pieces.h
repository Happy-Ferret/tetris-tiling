#ifndef __TETRIS_PIECES_H__
#define __TETRIS_PIECES_H__

/* Converts the information in a piece_data structure into a binary piece */
piece_t piece_data_get_piece(const struct piece_data *piece);

void print_piece(FILE *file, piece_t piece);
void print_board(FILE *file, board_t board);

/* Adds a piece to the board at the specified position and returns the new
 * board */
static inline board_t
add_piece_to_board(piece_t piece, board_t board, unsigned int pos)
{
    if (pos < PIECE_VERTICAL_OFFSET)
        return board | piece >> (PIECE_VERTICAL_OFFSET - pos) * PIECE_WIDTH;
    else
        return board | piece << (pos - PIECE_VERTICAL_OFFSET) * PIECE_WIDTH;
}

/* Returns non-zero when placing the piece on the board at the given position
 * results in a conflict. */
static inline board_t
check_piece_conflict(piece_t piece, board_t board, unsigned int pos)
{
    board_t shifted_board;
    if (pos < PIECE_VERTICAL_OFFSET)
        shifted_board = board << (PIECE_VERTICAL_OFFSET - pos) * PIECE_WIDTH;
    else
        shifted_board = board >> (pos - PIECE_VERTICAL_OFFSET) * PIECE_WIDTH;

    if (pos < PIECE_VERTICAL_OFFSET)
        shifted_board |= (board_t)0x3f
                << (PIECE_VERTICAL_OFFSET - pos - 1) * PIECE_WIDTH;

    if (pos > PIECE_VERTICAL_OFFSET
            && pos < PIECE_VERTICAL_OFFSET + BOARD_HEIGHT + 1)
        shifted_board |= (board_t)0x3f
                << (BOARD_HEIGHT + PIECE_VERTICAL_OFFSET - pos) * PIECE_WIDTH;
    
    return shifted_board & piece;
}

#endif /* ! defined __TETRIS_PIECES_H__ */

