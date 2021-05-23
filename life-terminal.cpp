#include <ncurses.h>
#include <getopt.h>
#include <string>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <filesystem>

#ifndef CTRL //Key+control function. c = key.
#define CTRL(c) ((c) & 037)
#endif

#define MAX_PATH_LENGTH 256 //Define max path length. 256 should be fine.
#define BRIGHT_WHITE 15 //Define the bright white color used in the inverted palette.
#define INVERTED 1 //Define inverted color pair index

bool is_number(char string[], char length = -1) { //If length is not specified, this function needs a null-terminated string.
    if (length == -1) {length = 100;}
    char letter = '0';
    char valid_numbers[12] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '-'};
    for (char ltrindex = 0; ltrindex < length; ltrindex ++) {
        char letter = string[ltrindex];
        bool letter_valid = false;
        if (letter == '\0') {
            break;
        }
        for (char nrindex = 0; nrindex < 12; nrindex ++) {
            char valid_number = valid_numbers[nrindex];
            if (valid_number == letter) {
                letter_valid = true;
            }
        }
        if (!letter_valid) {
            return false;
        }
    }
    return true;

}

class FileWindow {
    public:
        //char path[MAX_PATH_LENGTH];
        std::string path;
        int *win_width;
        int *win_height;
        FileWindow(int *_win_width, int *_win_height, std::string _message) {
            win_width = _win_width;
            win_height = _win_height;
            filewin = newwin(3, (*win_width)-1, (*win_height)-3, 0);
            wbkgdset(filewin, 'P'); //Fixing the weird characters I got in my terminal emulator.
            wbkgd(filewin, COLOR_PAIR(INVERTED));
            //char c_str_path[MAX_PATH_LENGTH];
            //getcwd(c_str_path, MAX_PATH_LENGTH); //Get current working directory
            //path = c_str_path;
            path_length = path.length();
            path_index = path_length;
            message = _message;
            draw();
        }
        std::string get_input() {
            struct timespec wait_time, wait_time2;
            wait_time.tv_nsec = 2000000L;
            while (true) {
                int key = getch();
                if (key == KEY_ENTER || key == int('\n')) {
                    break;
                } else if (key == CTRL('c') || key == 27) {
                    destroy();
                    return "";
                } else if (key == KEY_LEFT) {
                    path_index --;
                } else if (key == KEY_RIGHT) {
                    path_index ++;
                } else if (key == KEY_BACKSPACE || key == 127) {
                    if (path_index > 0) {
                        path.erase(path.begin()+path_index-1);
                        path_index --;
                        path_length --;
                    }
                } else if (key == KEY_DC) {
                    if (path_index < path_length) {
                        path.erase(path.begin()+path_index);
                        path_length --;
                    }
                } else if (key == KEY_STAB || key == '\t') {
                    // No autocompletion (yet). (Maybe coming soon..?)
                } else if (isascii(key)) {
                    path.insert(path_index, 1, (char)key);
                    path_length ++;
                    path_index ++;
                }
                if (path_index < 0) {
                        path_index = 0;
                }
                if (path_index > path_length) {
                        path_index = path_length;
                }
                draw();
                nanosleep(&wait_time, &wait_time2);
            }
            destroy();
            return path; //Return text
        }
    private:
        int path_index;
        int path_length;
        std::string message;
        WINDOW *filewin;
        void draw() {
            werase(filewin);
            wmove(filewin, 0, 0);
            wprintw(filewin, message.c_str());
            wmove(filewin, 1, 0);
            wprintw(filewin, path.c_str());
            wmove(filewin, 1, path_index);
            wrefresh(filewin);
        }
        void destroy() {
            //Remove window
            wbkgd(filewin, COLOR_PAIR(0));
            wclear(filewin);
            wrefresh(filewin);
            delwin(filewin);
        }
};



class Tile {
    public:
        int x;
        int y;
        int *offset_x;
        int *offset_y;
        int *board_width;
        int *board_height;
        int *win_width;
        int *win_height;
        bool alive = false;
        bool next_alive = false;
        bool in_updatelist = false;
        int neighbour_amount = 0;
        WINDOW *scr;
        std::vector<Tile*> *updatelist;
        std::vector<std::vector<Tile>> *other_tiles;
        Tile(WINDOW *_scr, std::vector<Tile*> *_updatelist, std::vector<std::vector<Tile>> *_other_tiles, int x_c, int y_c, int *ox, int *oy, int *bw, int *bh, int *ww, int *wh) {
            x = x_c;
            y = y_c;
            offset_x = ox;
            offset_y = oy;
            scr = _scr;
            updatelist = _updatelist;
            other_tiles = _other_tiles;
            board_width = bw;
            board_height = bh;
            win_width = ww;
            win_height = wh;
        }

        int count_neighbours() {
            int neighbours = 0;
            /*
            VVV
            VOV
            VXV
            */
            int min_x = x-1;
            int min_y = y-1;
            int max_x = x+1;
            int max_y = y+1;
            if (min_x < 0) {
                min_x = 0;
            }
            if (min_y < 0) {
                min_y = 0;
            }
            if (max_x >= *board_width) {
                max_x = x;
            }
            if (max_y >= *board_height) {
                max_y = y;
            }
            for (int x_iter = min_x; x_iter <= max_x; x_iter ++) {
                for (int y_iter = min_y; y_iter <= max_y; y_iter ++) {
                    if (x_iter == x && y_iter == y) {
                        continue;
                    }
                    if (((*other_tiles)[x_iter][y_iter]).alive) {
                        neighbours ++;
                    }
                }
            }
            return neighbours;
            
        }

        void update_calculate() {
            in_updatelist = false;
            neighbour_amount = count_neighbours();
            if (neighbour_amount >= 4) {
                next_alive = false;
            } else if (neighbour_amount == 3) {
                next_alive = true;
            } else if (neighbour_amount < 2) {
                next_alive = false;
            } if (neighbour_amount == 2) {
                next_alive = alive;
            }
        }

        void update_do() {
            set_alive(next_alive, true);
        };
        
        void draw() {
            int coord_x = (*offset_x)+x, coord_y = (*offset_y)+y;

            if (coord_x < 0 || coord_y < 0 || coord_x >= *win_width || coord_y >= *win_height) {
                return;
            }

            move(coord_y, coord_x);
            if (alive) {
                waddch(scr, 'O');
            } else {
                waddch(scr, ' ');
            }
        }

        void set_alive(bool a, bool update = false) {
            if (a != alive && update) {
                int min_x = x-1;
                int min_y = y-1;
                int max_x = x+1;
                int max_y = y+1;
                if (min_x < 0) {
                    min_x = 0;
                }
                if (min_y < 0) {
                    min_y = 0;
                }
                if (max_x >= *board_width) {
                    max_x = x;
                }
                if (max_y >= *board_height) {
                    max_y = y;
                }
                for (int x_iter = min_x; x_iter <= max_x; x_iter ++) {
                    for (int y_iter = min_y; y_iter <= max_y; y_iter ++) {
                        if (!((*other_tiles)[x_iter][y_iter].in_updatelist)) {
                            (*updatelist).push_back(&((*other_tiles)[x_iter][y_iter]));
                            (*other_tiles)[x_iter][y_iter].in_updatelist = true;
                        }
                    }
                }
            }
            alive = a;
            draw();
        }
        
};

class Board {
    public:
        int width;
        int height;
        bool playing = false;
        bool running = true;
        int offset_x = 0;
        int offset_y = 0;
        int win_width = 20;
        int win_height = 20;
        std::vector<Tile*> updatelist;
        std::vector<std::vector<Tile>> tiles;
        Board(WINDOW *_scr, int width_inp, int height_inp) {
            width = width_inp;
            height = height_inp;
            scr = _scr;
            updatelist.reserve(width*height);
            tiles.reserve(width);
            getmaxyx(scr, win_height, win_width);
            for (int tile_x = 0; tile_x < width; tile_x ++) {
                std::vector<Tile> current_column_vector;
                current_column_vector.reserve(height);
                for (int tile_y = 0; tile_y < height; tile_y ++) {
                    Tile tile(scr, &updatelist, &tiles, tile_x, tile_y, &offset_x, &offset_y, &width, &height, &win_width, &win_height);
                    current_column_vector.push_back(tile);
                }
                current_column_vector.shrink_to_fit();
                tiles.push_back(current_column_vector);
            }
            tiles.shrink_to_fit();
            
        }
        int TakeControl() {
            nodelay(scr, true);
            keypad(scr, TRUE);
            noecho();
            clear();
            loop();
            nodelay(scr, false);
            return 1;
        }
        void reset_cursor() {
            set_cursor(cursor_x, cursor_y);
        }
    private:
        int cursor_x = 0;
        int cursor_y = 0;
        struct UpdateSpeed {
            int tile_update_counter = 0;
            int tile_update_waittick = 250;
            long *tick_sleeptime;
            long std_sleeptime = 1000000L;
        };
        UpdateSpeed update_speed;
        WINDOW *scr;
        void draw() {
            wrefresh(scr);
        }

        void redraw() {
            wclear(scr);
            for (int x = 0; x < width; x++) {
                for (int y = 0; y < height; y++) {
                    Tile *tile = &tiles[x][y];
                    (*tile).draw();
                }
            }
            reset_cursor();
            draw();
        }

        void set_view(int x, int y) {
            offset_x = x;
            offset_y = y;
            redraw();
        }

        void move_view(int x, int y) {
            set_view(offset_x+x, offset_y+y);
        }

        void set_cursor(int x, int y) {
            int newposx = x+offset_x;
            int newposy = y+offset_y;

            if (newposx < 0) {
                move_view(-newposx, 0);
                return;
            } else if (newposx >= win_width) {
                move_view(newposx-win_width-1, 0);
                return;
            } else if (newposy < 0) {
                move_view(0, -newposy);
                return;
            } else if (newposy >= win_height) {
                move_view(0, newposy-win_height-1);
                return;
            }

            if (x < 0 || y < 0 || x >= width || y >= height) {
                return;
            }

            cursor_x = x;
            cursor_y = y;
            wmove(scr, newposy, newposx);
        }
        void move_cursor(int x, int y) {
            set_cursor(cursor_x+x, cursor_y+y);
        }

        void handle_keypress() {
            int key = getch();
            if (key == int('q') || key == int('Q')) {
                running = false;
            } else if (key == KEY_DOWN) {
                move_cursor(0, 1);
            } else if (key == KEY_UP){
                move_cursor(0, -1);
            } else if (key == KEY_RIGHT) {
                move_cursor(1, 0);
            } else if (key == KEY_LEFT){
                move_cursor(-1, 0);
            } else if (key == int('p') || key == int('P')){
                playing = !playing;
            } else if (key == KEY_ENTER || key == int('\n') || key == int('x') || key == int('X')) {
                Tile *tile = &tiles[cursor_x][cursor_y];
                (*tile).set_alive(!(*tile).alive, true);
                reset_cursor();
            } else if (key == int('t') || key == int('T')) { //Do one Tick
                update_tiles();
            } else if (key == int('=') || key == int('+')) {
                update_speed.tile_update_waittick /= 2;
                if (update_speed.tile_update_waittick == 0) {
                    update_speed.tile_update_waittick = 1;
                    (*update_speed.tick_sleeptime) /= 2;
                }
            } else if (key == int('-') || key == int('_')) {
                if (*update_speed.tick_sleeptime < update_speed.std_sleeptime) {
                    *update_speed.tick_sleeptime *= 2;
                } else {
                    update_speed.tile_update_waittick *= 2;
                }
            } else if (key == CTRL('c') || key == CTRL('C')) {
                running = false;
            } else if (key == CTRL('o') || key == CTRL('O')) {
                FileWindow filewin(&win_width, &win_height, "What is the name of the file you want to open?");
                filewin.get_input();
            }
        }

        void update_tiles() {
            std::vector<Tile*> temporary_update_list = updatelist;
            updatelist.clear();
            
            for (Tile *tile : temporary_update_list) {
                tile->update_calculate();
            }
            for (Tile *tile : temporary_update_list) {
                tile->update_do();
            }
            reset_cursor();
        }

        int loop() {
            struct timespec wait_time, wait_time2;
            wait_time.tv_sec = 0;
            wait_time.tv_nsec = 1000000L;
            update_speed.tick_sleeptime = &wait_time.tv_nsec;
            getmaxyx(scr, win_height, win_width);
            while (running) {
                int old_winheight = win_height;
                int old_winwidth = win_width;
                getmaxyx(scr, win_height, win_width);
                if (win_height != old_winheight || win_width != old_winwidth) {
                    redraw();
                }
                handle_keypress();
                if (playing && (update_speed.tile_update_counter % update_speed.tile_update_waittick == 0)) {
                    update_tiles();
                    update_speed.tile_update_counter = 1;
                }
                draw();
                update_speed.tile_update_counter ++;
                nanosleep(&wait_time, &wait_time2);
            }
            return 1;
        }
};

int main(int argc, char *argv[]) {
    // Get parameters
    const struct option long_options[] =
    {
        {"version", no_argument, NULL, 'v'},
        {"about", no_argument, NULL, 'a'},
        {"help", no_argument, NULL, 'h'},
        {"board-dimensions", required_argument, NULL, 'd'},
        {0,0,0,0},
    };

    //Standard values that can be changed using flags
    int playingfield_width = 800;
    int playingfield_height = 400;
    char pattern = 0; //Empty

    // loop over all of the options
    int ch;
    while ((ch = getopt_long(argc, argv, ":vahd:", long_options, NULL)) != -1) {
        // check to see if a single character or long option came through
        switch (ch) {
            case 'v':
                printf("Version 1.0.0\n");
                return 0;
                break;
            case 'a':
                printf("This is a Conway's game of life program for in the commandline. It is written in C++ and ncurses is used to display the board. This project is open source and licensed GPL3. Github page: https://github.com/Thijmer/life-terminal\n");
                return 0;
                break;
            case 'h':
                printf("Conway's game of life is a game invented by the mathematician John Conway.\n The rules are simple. The screen is a grid, and each square (in this case letter) represents a cell. A cell can be dead or alive. An alive cell can only stay alive if there are two or three alive cells around it. If there are too few neighbours, the cell dies because of underpopulation. If there are too many neighbours, it dies because of overpopulation. A dead cell becomes alife (=born) if it has three neighbours.\nIn this implementation, alive cells are represented as 'O's and dead cells are just empty space.\n");
                printf("\n\e[1mControls\e[0m\n");
                printf("You can move your cursor by pressing the arrow keys.\n");
                printf("You can make a dead cell alive or a living cell dead by pressing 'X' or enter on your keyboard.\n");
                printf("You can move one generation ahead by pressing 'T' on your keyboard.\n");
                printf("You can make the game progress automatically by pressing 'P' on your keyboard.\n");
                printf("You can slow down or speed up this automatical progress mode by pressing + or - on your keyboard. (You don't have to use shift to use '+'. '=' works too.)\n");
                return 0;
                break;
            case 'd': {
                char width_string[6];
                char height_string[6];
                char *local_optarg = optarg;
                char width_string_index = 0;
                char height_string_index = 0;
                char *letterptr;
                char letter;
                char state = 0;
                bool valid = true;
                for (letterptr=optarg; letter=*letterptr; letterptr++) {
                    if (letter == 'x' || letter == '*') {
                        state = 1;
                        continue;
                    }
                    if (state) {
                        height_string[height_string_index] = letter;
                        height_string_index ++;
                    } else {
                        width_string[width_string_index] = letter;
                        width_string_index ++;
                    }
                    if (width_string_index > 5 | height_string_index > 5) {
                        valid = false;
                    } if (!is_number(width_string, width_string_index) || !is_number(height_string, height_string_index)) {
                        valid = false;
                    }
                    if (!valid) {
                        fprintf(stderr, "Could not parse the dimension string. Please make sure you follow the format given in the readme file.\n");
                        return 128;
                    }
                }
                width_string[width_string_index] = '\0';
                height_string[height_string_index] = '\0';
                playingfield_width = atoi(width_string);
                playingfield_height = atoi(height_string);
                if (playingfield_height < 1 || playingfield_width < 1) {
                    fprintf(stderr, "The board has to be at least 1x1.\n");
                }
                break;
            } case ':':
                fprintf(stderr, "Option '%c' expects a value.\n", optopt);
                return 128;
                break;
            case '?':
                fprintf(stderr, "This program does not take an option '%c'.\n", optopt);
                return 128;
                break;

        }
    }


	initscr(); // Start curses mode
    raw();
    start_color(); //Color stuff
    use_default_colors();
    if (COLORS >= 16) {
        init_color(BRIGHT_WHITE, 1000,1000,1000);
        init_pair(INVERTED, COLOR_BLACK, BRIGHT_WHITE);
    } else {
        init_pair(INVERTED, COLOR_BLACK, COLOR_WHITE);
    }
    Board board(stdscr, playingfield_width, playingfield_height); // Make the board.
    board.TakeControl(); // Let the board take control of the screen.
	endwin(); // End curses mode


	return 0;
}