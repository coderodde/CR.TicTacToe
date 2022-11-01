#ifdef _WIN32
// Otherwise, Visual Studio (2022) complains about scanf.
#define _CRT_SECURE_NO_WARNINGS 
#endif // _WIN32 

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define HEIGHT 3 
#define WIDTH 3

#define BOARD_SPRITE_HEIGHT 13
#define BOARD_SPRITE_WIDTH 25

#define BOARD_CELL_SPRITE_HEIGHT 3
#define BOARD_CELL_SPRITE_WIDTH 7

#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define MAX(x,y) (((x) > (y)) ? (x) : (y))

static const int POSITIVE_INFINITY = +1000 * 1000 * 1000;
static const int NEGATIVE_INFINITY = -1000 * 1000 * 1000;

static char BOARD_SPRITE[BOARD_SPRITE_HEIGHT]
                        [BOARD_SPRITE_WIDTH];

static char BOARD_X_SPRITE[BOARD_CELL_SPRITE_HEIGHT]
                          [BOARD_CELL_SPRITE_WIDTH];

static char BOARD_O_SPRITE[BOARD_CELL_SPRITE_HEIGHT]
                          [BOARD_CELL_SPRITE_WIDTH];

static const char* BOARD_SPRITE_SOURCE =
"+-------+-------+-------+"
"|       |       |       |"
"|   1   |   2   |   3   |"
"|       |       |       |"
"+-------+-------+-------+"
"|       |       |       |"
"|   4   |   5   |   6   |"
"|       |       |       |"
"+-------+-------+-------+"
"|       |       |       |"
"|   7   |   8   |   9   |"
"|       |       |       |"
"+-------+-------+-------+";

static const char* BOARD_X_SPRITE_SOURCE =
" \\\\ // "
"  |||  "
" // \\\\ ";

static const char* BOARD_O_SPRITE_SOURCE =
"  ooo  "
"  o o  "
"  ooo  ";

static void load_board_sprite()
{
    for (size_t y = 0; y < BOARD_SPRITE_HEIGHT; ++y) {
        for (size_t x = 0; x < BOARD_SPRITE_WIDTH; ++x) {
            BOARD_SPRITE[y][x] = 
                BOARD_SPRITE_SOURCE[BOARD_SPRITE_WIDTH * y + x];
        }
    }
}

static void  load_board_x_cell_sprite()
{
    for (size_t y = 0; y < BOARD_CELL_SPRITE_HEIGHT; ++y) {
        for (size_t x = 0; x < BOARD_CELL_SPRITE_WIDTH; ++x) {
            BOARD_X_SPRITE[y][x] =
                BOARD_X_SPRITE_SOURCE[BOARD_CELL_SPRITE_WIDTH * y + x];
        }
    }
}

static void  load_board_o_cell_sprite()
{
    for (size_t y = 0; y < BOARD_CELL_SPRITE_HEIGHT; ++y) {
        for (size_t x = 0; x < BOARD_CELL_SPRITE_WIDTH; ++x) {
            BOARD_O_SPRITE[y][x] =
                BOARD_O_SPRITE_SOURCE[BOARD_CELL_SPRITE_WIDTH * y + x];
        }
    }
}

static void load_all_sprites()
{
    load_board_sprite();
    load_board_x_cell_sprite();
    load_board_o_cell_sprite();
}

static const int PREFERENCE_FILTER[HEIGHT][WIDTH] = {
    { 5, 0 , 5 },
    { 0, 20, 0 },
    { 5, 0 , 5 },
};

typedef struct movement_t
{
    size_t x;
    size_t y;
} movement_t;

typedef enum PlayerColor 
{
    PLAYER_X, // Minimizing player; human.
    PLAYER_O, // Maximizing player; AI.
} PlayerColor;

PlayerColor Invert_Player_Color(PlayerColor player_color) {
    return player_color == PLAYER_X ? PLAYER_O : PLAYER_X;
}

typedef enum BoardCellColor
{
    CELL_COLOR_EMPTY_1 = '1',
    CELL_COLOR_EMPTY_2 = '2',
    CELL_COLOR_EMPTY_3 = '3',
    CELL_COLOR_EMPTY_4 = '4',
    CELL_COLOR_EMPTY_5 = '5',
    CELL_COLOR_EMPTY_6 = '6',
    CELL_COLOR_EMPTY_7 = '7',
    CELL_COLOR_EMPTY_8 = '8',
    CELL_COLOR_EMPTY_9 = '9',
    CELL_COLOR_X       = 'X',
    CELL_COLOR_O       = 'O',
} BoardCellColor;

typedef enum WinningStatus {
    WIN_X   = 'X', // X won.
    WIN_O   = 'O', // O won.
    WIN_TIE = 'T', // Tie.
    WIN_NA  = 'N', // Status not available.
} WinningStatus;

typedef struct board_t
{
    char* board_data;
    char string_representation[BOARD_SPRITE_HEIGHT][BOARD_SPRITE_WIDTH];
} board_t;

static void board_t_init(board_t* board)
{
    board->board_data = malloc(sizeof(BoardCellColor) * HEIGHT * WIDTH);
}

static BoardCellColor board_t_get_cell_color(board_t* board,
                                             size_t x,
                                             size_t y)
{
    return board->board_data[y * WIDTH + x];
}

static BoardCellColor
board_t_get_cell_color_via_movement(board_t* board, movement_t movement)
{
    size_t x = movement.x;
    size_t y = movement.y;
    return board_t_get_cell_color(board, x, y);
}

static bool board_t_can_make_movement(board_t* board, movement_t movement)
{
    if (movement.x >= WIDTH || movement.y >= HEIGHT) {
        return false;
    }

    BoardCellColor color =
        board_t_get_cell_color(board,
                               movement.x,
                               movement.y);

    return color != CELL_COLOR_X && color != CELL_COLOR_O;
}

void board_t_set_cell_color(board_t* board,
                            size_t x,
                            size_t y,
                            BoardCellColor color)
{
    board->board_data[HEIGHT * y + x] = color;
}

static void board_t_set_cell_color_via_movement(board_t* board,
                                                movement_t movement,
                                                BoardCellColor color)
{
    board->board_data[movement.y * WIDTH + movement.x] = color;
}

static void board_t_set_initial_cell_values(board_t* board)
{
    board->board_data = malloc(sizeof(BoardCellColor) * HEIGHT * WIDTH);

    board_t_set_cell_color(board, 0, 0, CELL_COLOR_EMPTY_1);
    board_t_set_cell_color(board, 1, 0, CELL_COLOR_EMPTY_2);
    board_t_set_cell_color(board, 2, 0, CELL_COLOR_EMPTY_3);
    board_t_set_cell_color(board, 0, 1, CELL_COLOR_EMPTY_4);
    board_t_set_cell_color(board, 1, 1, CELL_COLOR_EMPTY_5);
    board_t_set_cell_color(board, 2, 1, CELL_COLOR_EMPTY_6);
    board_t_set_cell_color(board, 0, 2, CELL_COLOR_EMPTY_7);
    board_t_set_cell_color(board, 1, 2, CELL_COLOR_EMPTY_8);
    board_t_set_cell_color(board, 2, 2, CELL_COLOR_EMPTY_9);
}

static void board_t_free(board_t* board)
{
    free(board->board_data);
}

static bool board_t_cell_is_empty(board_t* board, size_t x, size_t y) 
{
    BoardCellColor color = board_t_get_cell_color(board, x, y);

    switch (color) {
    case CELL_COLOR_X:
    case CELL_COLOR_O:
        return false;

    default:
        return true;
    }
}

static movement_t convert_board_selector_to_move(BoardCellColor color) 
{
    movement_t movement;

    switch (color) {

    case CELL_COLOR_EMPTY_1:
        movement.x = 0;
        movement.y = 0;
        return movement;

    case CELL_COLOR_EMPTY_2:
        movement.x = 1;
        movement.y = 0;
        return movement;

    case CELL_COLOR_EMPTY_3:
        movement.x = 2;
        movement.y = 0;
        return movement;

    case CELL_COLOR_EMPTY_4:
        movement.x = 0;
        movement.y = 1;
        return movement;

    case CELL_COLOR_EMPTY_5:
        movement.x = 1;
        movement.y = 1;
        return movement;

    case CELL_COLOR_EMPTY_6:
        movement.x = 2;
        movement.y = 1;
        return movement;

    case CELL_COLOR_EMPTY_7:
        movement.x = 0;
        movement.y = 2;
        return movement;

    case CELL_COLOR_EMPTY_8:
        movement.x = 1;
        movement.y = 2;
        return movement;

    case CELL_COLOR_EMPTY_9:
        movement.x = 2;
        movement.y = 2;
        return movement;

    default:
        abort();
        return movement; // Keep compiler happy.
    }
}

static void board_t_make_movement(board_t* board,
                                  movement_t movement,
                                  BoardCellColor board_cell_color)
{
    if (board_t_can_make_movement(board, movement)) {
        board_t_set_cell_color(board,
                               movement.x,
            movement.y,
            board_cell_color);
    }
}

static board_t* board_t_copy(board_t* board)
{
    board_t* copy = malloc(sizeof(*copy));
    board_t_init(copy);

    for (size_t y = 0; y < HEIGHT; ++y) {
        for (size_t x = 0; x < WIDTH; ++x) {
            board_t_set_cell_color(copy, 
                                   x, 
                                   y, 
                                   board_t_get_cell_color(board, x, y));
        }
    }

    return copy;
}

static void apply_board_sprite(board_t* board)
{
    for (size_t y = 0; y < BOARD_SPRITE_HEIGHT; ++y) {
        for (size_t x = 0; x < BOARD_SPRITE_WIDTH; ++x) {
            board->string_representation[y][x] = BOARD_SPRITE[y][x];
        }
    }
}

static void apply_board_cell_sprite(board_t* board,
                                    BoardCellColor color,
                                    size_t cell_x,   
                                    size_t cell_y) {
    if (color == CELL_COLOR_X) {
        for (size_t char_cell_y = 0;
            char_cell_y < BOARD_CELL_SPRITE_HEIGHT;
            char_cell_y++) {

            for (size_t char_cell_x = 0;
                char_cell_x < BOARD_CELL_SPRITE_WIDTH; 
                char_cell_x++) {

                size_t global_char_x = 
                    (BOARD_CELL_SPRITE_WIDTH + 1) * cell_x + char_cell_x + 1;

                size_t global_char_y = 
                    (BOARD_CELL_SPRITE_HEIGHT + 1) * cell_y + char_cell_y + 1;

                board->string_representation[global_char_y][global_char_x] = 
                              BOARD_X_SPRITE[char_cell_y][char_cell_x];
            }
        }
    } else if (color == CELL_COLOR_O) {
        for (size_t char_cell_y = 0;
            char_cell_y < BOARD_CELL_SPRITE_HEIGHT;
            char_cell_y++) {

            for (size_t char_cell_x = 0;
                char_cell_x < BOARD_CELL_SPRITE_WIDTH;
                char_cell_x++) {

                size_t global_char_x =
                    (BOARD_CELL_SPRITE_WIDTH + 1) * cell_x + char_cell_x + 1;

                size_t global_char_y =
                    (BOARD_CELL_SPRITE_HEIGHT + 1) * cell_y + char_cell_y + 1;

                board->string_representation[global_char_y][global_char_x] =
                    BOARD_O_SPRITE[char_cell_y][char_cell_x];
            }
        }
    }
}

static void do_print(board_t* board)
{
    for (size_t y = 0; y < BOARD_SPRITE_HEIGHT; ++y) {
        for (size_t x = 0; x < BOARD_SPRITE_WIDTH; ++x) {
            printf("%c", board->string_representation[y][x]);
        }

        puts("");
    }
}

static void board_t_print(board_t* board)
{
    apply_board_sprite(board);

    for (size_t y = 0; y < HEIGHT; ++y) {
        for (size_t x = 0; x < WIDTH; ++x) {
            apply_board_cell_sprite(board,
                                    board_t_get_cell_color(board, x, y),
                                    x,
                                    y);
        }
    }

    do_print(board);
}

static bool board_t_has_available_spots(board_t* board)
{
    for (size_t y = 0; y < HEIGHT; ++y) {
        for (size_t x = 0; x < WIDTH; ++x) {
            BoardCellColor color = board_t_get_cell_color(board, x, y);

            if (color != CELL_COLOR_X && color != CELL_COLOR_O) {
                return true;
            }
        }
    }

    return false;
}

static WinningStatus
cell_color_to_winning_status(BoardCellColor board_cell_color)
{
    switch (board_cell_color) {
    case CELL_COLOR_X:
        return WIN_X;

    case CELL_COLOR_O:
        return WIN_O;
    }

    abort();
    return 0;
}

static WinningStatus board_t_get_winner_status(board_t* board)
{
    for (size_t i = 0; i < 3; i++) {
        // Check horizontal winning condition
        BoardCellColor cell1 = board_t_get_cell_color(board, 0, i);
        BoardCellColor cell2 = board_t_get_cell_color(board, 1, i);
        BoardCellColor cell3 = board_t_get_cell_color(board, 2, i);

        if (cell1 == cell2 
            && cell2 == cell3 
            && (cell1 == CELL_COLOR_X || cell1 == CELL_COLOR_O)) {
            return cell_color_to_winning_status(cell1);
        }

        // Check vertical winning condition
        cell1 = board_t_get_cell_color(board, i, 0);
        cell2 = board_t_get_cell_color(board, i, 1);
        cell3 = board_t_get_cell_color(board, i, 2);

        if (cell1 == cell2 
            && cell2 == cell3
            && (cell1 == CELL_COLOR_X || cell1 == CELL_COLOR_O)) {
            return cell_color_to_winning_status(cell1);
        }
    }

    // Check for diagonal winning condition
    BoardCellColor cell1 = board_t_get_cell_color(board, 0, 0);
    BoardCellColor cell2 = board_t_get_cell_color(board, 1, 1);
    BoardCellColor cell3 = board_t_get_cell_color(board, 2, 2);

    if (cell1 == cell2 
        && cell2 == cell3 
        && (cell1 == CELL_COLOR_X || cell1 == CELL_COLOR_O)) {
        return cell_color_to_winning_status(cell1);
    }

    cell1 = board_t_get_cell_color(board, 2, 0);
    cell2 = board_t_get_cell_color(board, 1, 1);
    cell3 = board_t_get_cell_color(board, 0, 2);

    if (cell1 == cell2
        && cell2 == cell3
        && (cell1 == CELL_COLOR_X || cell1 == CELL_COLOR_O)) {
        return cell_color_to_winning_status(cell1);
    }

    // There's no winner so we check if there's any empty space left yet.
    if (!board_t_has_available_spots(board)) {
        return WIN_TIE;
    }

    return WIN_NA;
}

static movement_t compute_next_ai_movement(board_t* board)
{
    int best_score = -10000;
    movement_t best_movement;

    for (size_t y = 0; y < HEIGHT; ++y) {
        for (size_t x = 0; x < WIDTH; ++x) {
            movement_t attempted_movement = { y, x };

            if (board_t_can_make_movement(board, attempted_movement)) {
                board_t* copy = board_t_copy(board);

                board_t_set_cell_color_via_movement(copy,
                                                    attempted_movement, 
                                                    CELL_COLOR_O);

                int tentative_score = alpha_beta_pruning(copy, 
                                                         0, 
                                                         NEGATIVE_INFINITY, 
                                                         POSITIVE_INFINITY, 
                                                         PLAYER_X);
                board_t_free(copy);

                if (best_score < tentative_score) {
                    best_score = tentative_score;
                    best_movement = attempted_movement;
                }
            }
        }
    }

    return best_movement;
}

static BoardCellColor 
player_color_to_board_cell_color(PlayerColor player_color)
{
    switch (player_color) {
    case PLAYER_X:
        return CELL_COLOR_X;
    case PLAYER_O:
        return CELL_COLOR_O;
    }

    abort();
    return 0;
}

static bool is_valid_position_character(char ch) {
    return '1' <= ch && ch <= '9';
}

static size_t millis() {
#ifdef _WIN32
    return (size_t)GetTickCount();
#else   
    struct timeval tv;
    gettimeofday(&v);
    return (size_t)(tv.sec * 1000 + tv.usec / 1000);
#endif
}

int alpha_beta_pruning(board_t* board,
                       int depth,
                       int alpha,
                       int beta,
                       PlayerColor player_color)
{
    WinningStatus winning_status = board_t_get_winner_status(board);

    if (winning_status == WIN_X) {
        return -100 + depth;
    } else if (winning_status == WIN_O) {
        return 100 - depth;
    } else if (winning_status == WIN_TIE) {
        return 0;
    }

    if (player_color == PLAYER_O) {
        int value = NEGATIVE_INFINITY;
        int best_score = -1000;
        movement_t best_movement;

        for (size_t y = 0; y < HEIGHT; ++y) {
            for (size_t x = 0; x < WIDTH; ++x) {
                movement_t attempted_movement;
                attempted_movement.y = y;
                attempted_movement.x = x;

                if (board_t_can_make_movement(board, attempted_movement)) {
                    BoardCellColor saved_cell_color =
                        board_t_get_cell_color_via_movement(
                            board,
                            attempted_movement);

                    board_t_set_cell_color_via_movement(
                        board, 
                        attempted_movement, 
                        CELL_COLOR_O);

                    int tentative_score = alpha_beta_pruning(board,
                                                             depth + 1,
                                                             alpha, 
                                                             beta,
                                                             PLAYER_X);
                    board_t_set_cell_color_via_movement(
                        board, 
                        attempted_movement, 
                        saved_cell_color);
                    
                    if (best_score < tentative_score) {
                        best_score = tentative_score;
                        best_movement = attempted_movement;
                    }

                    value = MAX(value, tentative_score);

                    if (value >= beta) {
                        break;
                    }

                    alpha = MAX(alpha, value);
                }
            }
        }

        return value + PREFERENCE_FILTER[best_movement.y]
                                        [best_movement.x];

    } else { // Simulating human player:
        int value = POSITIVE_INFINITY;
        int best_score = 1000;
        movement_t best_movement;
        
        for (size_t y = 0; y < HEIGHT; ++y) {
            for (size_t x = 0; x < WIDTH; ++x) {
                movement_t attempted_movement;
                attempted_movement.y = y;
                attempted_movement.x = x;

                if (board_t_can_make_movement(board, attempted_movement)) {
                    BoardCellColor saved_cell_color =
                        board_t_get_cell_color_via_movement(
                            board, 
                            attempted_movement);

                    board_t_set_cell_color_via_movement(
                        board,
                        attempted_movement, 
                        CELL_COLOR_X);

                    int tentative_score = alpha_beta_pruning(board,
                                                             depth + 1,
                                                             alpha, 
                                                             beta,  
                                                             PLAYER_O);
                    board_t_set_cell_color_via_movement(
                        board,
                        attempted_movement,
                        saved_cell_color);

                    if (best_score > tentative_score) {
                        best_score = tentative_score;
                        best_movement = attempted_movement;
                    }

                    value = MIN(value, tentative_score);

                    if (value <= alpha) {
                        break;
                    }

                    beta = MIN(beta, value);
                }
            }
        }

        return value - PREFERENCE_FILTER[best_movement.y]
                                        [best_movement.x];
    }
}

static PlayerColor generate_random_player_color()
{
    return rand() % 2 == 0 ? PLAYER_X : PLAYER_O;
}

void bot_mode()
{
    board_t board;
    board_t_init(&board);
    board_t_set_initial_cell_values(&board);

    bool gameInProgress = true;
    PlayerColor player_color = generate_random_player_color();

    puts("Your mark is X, AI is O.");
    board_t_print(&board);

    while (true) {
        if (player_color == PLAYER_X) {
            puts(">>> It's your turn.");
        } else {
            puts(">>> AI's turn.");
        }

        if (player_color == PLAYER_X) {
            movement_t desired_movement;
            desired_movement.x = WIDTH;
            desired_movement.y = HEIGHT;

            do
            {
                printf("Please enter your desired move: ");

                fflush(stdin);

                char position_choice;

                scanf("%c", &position_choice);

                if (!is_valid_position_character(position_choice)) {
                    puts("");
                    continue;
                }

                desired_movement = 
                    convert_board_selector_to_move(position_choice);

            } while (!board_t_can_make_movement(&board, desired_movement));

            board_t_set_cell_color_via_movement(
                &board, 
                desired_movement,
                player_color_to_board_cell_color(player_color));

        } else {
            // This belongs to the AI.
            size_t duration = millis();
            movement_t best_movement = compute_next_ai_movement(&board);
            duration = millis() - duration;

            printf("AI duration: %zu milliseconds.\n", duration);

            board_t_set_cell_color_via_movement(
                &board,
                best_movement,
                player_color_to_board_cell_color(player_color));
        }

        board_t_print(&board);

        WinningStatus winning_status = board_t_get_winner_status(&board);

        if (winning_status == WIN_X) {
            puts("You won!");
            gameInProgress = false;
        } else if (winning_status == WIN_O) {
            puts("AI won.");
            gameInProgress = false;
        } else if (winning_status == WIN_TIE) {
            puts("It's a tie.");
            gameInProgress = false;
        }

        if (gameInProgress == false) {
            puts("");
            return;
        }

        // Invert the player
        player_color = Invert_Player_Color(player_color);
    }
}

int main(void)
{
    // v
    // v
    // v For random choice whether X or O makes the first move.
    srand(time(NULL)); 
    load_all_sprites();
    bot_mode();
    return 0;
}
