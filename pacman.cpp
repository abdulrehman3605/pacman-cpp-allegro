#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#include <cstdio>
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

using namespace std;
const float FPS = 6.6;
const int SCREEN_W = 460; 
const int SCREEN_H = 550;   

// Struct used to manipulate Blinky's AI information
struct Info{
    double dist;
    int x;
    int y;
    char direction;
};
bool initialSound=true; // boolean used to play the initial sound when the game starts
// Function used for Blinky's AI
double distance(int x1, int y1, int x2, int y2) {
    return abs(sqrt((x2 - x1)*(x2 - x1) +  (y2 - y1)*(y2 - y1)));
}

// Variables for PacMan's movement
// 1 = left, 2 = down, 3 = right, 4 = up.
int intention = 0;	// current movement intention
int previous = 0;	// previous movement intention

enum MYKEYS {
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT
};

// Matrix defining the game map: 2 represents balls, 1 represents walls, and 0 represents corridors
char MAP[24][24] = 
{
    "11111111111111111111111",
    "12222222222122222222221",
    "12111211112121111211121",
    "12111211112121111211121",
    "12222222222222222222221",
    "12111212111111121211121",
    "12222212222122221222221",
    "11111211110101111211111",
    "11111210000000001211111",
    "11111210111011101211111",
    "00000000111011100000000",
    "11111210111011101211111",
    "11111210111111101211111",
    "11111210000000001211111",
    "11111210111111101211111",
    "12222222222122222222221",
    "12111211112121111211121",
    "12221222222022222212221",
    "11121212111111121212111",
    "12222212222122221222221",
    "12111111112121111111121",
    "12222222222222222222221",
    "11111111111111111111111"
};

// Number of balls on the map
int balls = 184;
int k = 0, l = 0; // Auxiliary to control the positioning of the balls

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_BITMAP *map = NULL;
ALLEGRO_BITMAP *map2 = NULL;
ALLEGRO_BITMAP *gameOverScreen = NULL;
ALLEGRO_BITMAP *ballsImage = NULL;
ALLEGRO_BITMAP *pacman = NULL;
ALLEGRO_BITMAP *pac_up = NULL;
ALLEGRO_BITMAP *pac_left = NULL;
ALLEGRO_BITMAP *pac_down = NULL;
ALLEGRO_BITMAP *pac_right = NULL;
ALLEGRO_BITMAP *shutup = NULL;
ALLEGRO_BITMAP *aux = NULL;   // For opening and closing PacMan's mouth
ALLEGRO_BITMAP *blinky = NULL;
ALLEGRO_BITMAP *yellowGhost = NULL;
ALLEGRO_BITMAP *greenGhost = NULL;
ALLEGRO_BITMAP *blueGhost = NULL;
ALLEGRO_SAMPLE *sample = NULL;
ALLEGRO_SAMPLE *win = NULL;
ALLEGRO_SAMPLE *death = NULL;
ALLEGRO_SAMPLE *beginning = NULL;
ALLEGRO_FONT *font = NULL;

int i = 17, j = 11; // PacMan's position
int r = 8, t = 11; // Blue Ghost's position
int aX = 9, aY = 11; // Yellow Ghost's position
int vX = 10, vY = 11; // Green Ghost's position
int g = 11, h = 11; // Blinky's position
int q = 20; // Size of each cell in the map

bool gameOver = false; 
bool playWaka = false;
bool endGame = false;

// Variables for PacMan's position on the screen
int posY = i * q;
int posX = j * q;
// Blinky's position
int bY = g * q;
int bX = h * q;
// Blue Ghost's position
int blueX = t * q;
int blueY = r * q;
// Yellow Ghost's position
int yellowX = aY * q;
int yellowY = aX * q;
// Green Ghost's position
int greenX = vY * q;
int greenY = vX * q;

// Auxiliary variables to remember characters' last movements
int lastRandomPos = -1;
int lastYellowPos = -1;
int lastBluePos = -1;
int lastGreenPos = -1;
int randomIndex = -1;

// Last positions, used for Blinky's movement

// Auxiliary for controlling PacMan's mouth opening and closing
int lastMouth, sim = 0;

bool redraw = true;
bool exitGame = false;

int ulx = g; // Last coordinates of Blinky, used for PacMan's death and for Blinky's movement
int uly = h;

// Last coordinates of 3 ghosts and PacMan (to detect if PacMan dies by crossing paths with a ghost)
int lastYellowX = aX, lastYellowY = aY, lastBlueX = r, lastBlueY = t, lastGreenX = vX, lastGreenY = vY;
int lastPacManX = i, lastPacManY = j, lastRedGhostX = g, lastRedGhostY = h;

void blinkyMove(char M[][24], int &x, int &y, int &bX, int &bY) {
    if ((x == 8) && (y == 11) && (j <= y)) { // Prevents Blinky from entering spawn zone
        ulx = x;
        uly = y;
        y--;
        bX = y * q;
        return;
    } else if ((x == 8) && (y == 11) && (j >= y)) {
        ulx = x;
        uly = y;
        y++;
        bX = y * q;
        return;
    }

    Info go[4]; // Calculate distances from the 4 positions around Blinky
    go[0].dist = distance(x + 1, y, i, j); go[0].x = x + 1; go[0].y = y; go[0].direction = 'S';
    go[1].dist = distance(x, y - 1, i, j); go[1].x = x; go[1].y = y - 1; go[1].direction = 'A';
    go[2].dist = distance(x - 1, y, i, j); go[2].x = x - 1; go[2].y = y; go[2].direction = 'W';
    go[3].dist = distance(x, y + 1, i, j); go[3].x = x; go[3].y = y + 1; go[3].direction = 'D';

    double less = 1000;
    int which = 1000;
    for (int c = 0; c < 4; c++) {  // Check which distance is the smallest and accessible
        if ((go[c].dist < less) && (M[go[c].x][go[c].y] != '1')) {
            if ((go[c].x != ulx) || (go[c].y != uly)) {
                less = go[c].dist;
                which = c;
            }
        }
    }
    ulx = x;
    uly = y;

    if (which == 0) {   // Go by the shortest distance
        x++;
        bY = x * q;
    } else if (which == 2) {
        x--;
        bY = x * q;
    } else if (which == 3) {
        y++;
        bX = y * q;
    } else if (which == 1) {
        y--;
        bX = y * q;
    }

    if (x == 10 && y == -1) {
        x = 10;
        y = 23;
        bX = y * q;
        bY = x * q;
    } else if (x == 10 && y == 22) {
        x = 10;
        y = -1;
        bX = y * q;
        bY = x * q;
    }
}

void semiSmart(char M[][24], int &x, int &y, int &gposX, int &gposY, int &lastThisPos) {
    int auxX = x;
    int auxY = y;

    if (i > x && M[auxX + 1][y] != '1') {
        if (lastThisPos == 0) return; // If the last movement was opposite to the current, do nothing
        lastThisPos = 1;
        x++;
        gposY = x * q;
    } else if (i < x && M[auxX - 1][y] != '1') {
        if (lastThisPos == 1) return;
        lastThisPos = 0;
        x--;
        gposY = x * q;
    } else if (j > y && M[x][auxY + 1] != '1') {
        if (lastThisPos == 3) return;
        lastThisPos = 2;
        y++;
        gposX = y * q;
    } else if (j < y && M[x][auxY - 1] != '1') {
        if (lastThisPos == 2) return;
        lastThisPos = 3;
        y--;
        gposX = y * q;
    } else if (x == 10 && y == -1) {
        x = 10;
        y = 23;
        gposX = y * q;
        gposY = x * q;
    } else if (x == 10 && y == 22) {
        x = 10;
        y = -1;
        gposX = y * q;
        gposY = x * q;
    }
}

void randomMove(char M[][24], int &x, int &y, int &gposX, int &gposY, int phantom) {
    int auxX = x;
    int auxY = y;

    srand(time(NULL));
    randomIndex = rand() % 4;

    if (phantom == 1) srand(time(NULL));
    randomIndex = rand() % 4;

    if (phantom == 2) srand(time(NULL));
    randomIndex = rand() % 4;

    if (auxX == 10 && auxY == -1) {
        x = 10;
        y = 22;
        gposX = y * q;
        gposY = x * q;
        return;
    } else if (auxX == 10 && auxY == 23) {
        x = 10;
        y = 0;
        gposX = y * q;
        gposY = x * q;
        return;
    }

    if (randomIndex == 0 && M[auxX - 1][y] != '1') {
        if (lastRandomPos == 1) return;
        lastRandomPos = 0;
        x--;
        gposY = x * q;
    } else if (randomIndex == 1 && M[auxX + 1][y] != '1') {
        if (lastRandomPos == 0) return;
        lastRandomPos = 1;
        x++;
        gposY = x * q;
    } else if (randomIndex == 2 && M[x][auxY + 1] != '1') {
        if (lastRandomPos == 3) return;
        lastRandomPos = 2;
        y++;
        gposX = y * q;
    } else if (randomIndex == 3 && M[x][auxY - 1] != '1') {
        if (lastRandomPos == 2) return;
        lastRandomPos = 3;
        y--;
        gposX = y * q;
    } else {
        semiSmart(M, x, y, gposX, gposY, randomIndex);
    }
}

bool started;

int initialize() {
    if (!al_init()) {
        cout << "Failed to load Allegro" << endl;
        return 0;
    }
    if (!al_install_keyboard()) {
        cout << "Failed to initialize the keyboard" << endl;
        return 0;
    }
    timer = al_create_timer(0.8 / FPS);
    if (!timer) {
        cout << "Failed to initialize the timer" << endl;
        return 0;
    }

    if (!al_install_audio()) {
        fprintf(stderr, "failed to initialize audio!\n");
        return -1;
    }

    if (!al_init_acodec_addon()) {
        fprintf(stderr, "failed to initialize audio codecs!\n");
        return -1;
    }

    if (!al_reserve_samples(2)) {
        fprintf(stderr, "failed to reserve samples!\n");
        return -1;
    }
    win = al_load_sample("assets/sounds/you win.wav");
    death = al_load_sample("assets/sounds/wasted.wav");

    beginning = al_load_sample("assets/sounds/beggining.wav");
    if (!win) {
        printf("Winning audio clip sample not loaded! \n");
        return -1;
    }
    if (!death) {
        printf("Death clip sample not loaded! \n");
        return -1;
    }
    if (!beginning) {
        printf("Beginning clip sample not loaded! \n");
        return -1;
    }

    sample = al_load_sample("assets/sounds/waka.wav"); // Music to be loaded

    if (!sample) {
        printf("Audio clip sample not loaded!\n");
        return -1;
    }

    if (!al_init_image_addon()) {
        cout << "Failed to initialize al_init_image_addon!" << endl;
        return 0;
    }

    display = al_create_display(SCREEN_W, SCREEN_H);
    if (!display) {
        cout << "Failed to initialize the display" << endl;
        al_destroy_timer(timer);
        return 0;
    }

    map = al_load_bitmap("assets/maps/map.bmp");
    if (!map) {
        cout << "Failed to load the map!" << endl;
        al_destroy_display(display);
        return 0;
    }
    al_draw_bitmap(map, 0, 0, 0);

    map2 = al_load_bitmap("assets/maps/map2.bmp");
    if (!map2) {
        cout << "Failed to load map 2!" << endl;
        al_destroy_display(display);
        return 0;
    }

    gameOverScreen = al_load_bitmap("assets/maps/gameover.bmp");
    if (!gameOverScreen) {
        cout << "Failed to load gameover.bmp!" << endl;
        al_destroy_display(display);
        return 0;
    }

    pacman = al_load_bitmap("assets/characters/pacman/pacman.png");
    pac_up = al_load_bitmap("assets/characters/pacman/pac_up.png");
    pac_down = al_load_bitmap("assets/characters/pacman/pac_down.png");
    pac_left = al_load_bitmap("assets/characters/pacman/pac_left.png");
    pac_right = al_load_bitmap("assets/characters/pacman/pac_right.png");
    shutup = al_load_bitmap("assets/characters/pacman/shutup.png");
    blinky = al_load_bitmap("assets/characters/ghosts/blinky.png");
    blueGhost = al_load_bitmap("assets/characters/ghosts/blue.png");
    yellowGhost = al_load_bitmap("assets/characters/ghosts/yellow.png");
    greenGhost = al_load_bitmap("assets/characters/ghosts/green.png");

    if (!pacman) {
        cout << "Failed to load PacMan!" << endl;
        al_destroy_display(display);
        return 0;
    }
    al_draw_bitmap(pacman, posX, posY, 0);

    if (!blinky) {
        cout << "Failed to load Blinky!" << endl;
        al_destroy_display(display);
        return 0;
    }
    al_draw_bitmap(blinky, bX, bY, 0);

    if (!blueGhost) {
        cout << "Failed to load Blue Ghost!" << endl;
        al_destroy_display(display);
        return 0;
    }
    al_draw_bitmap(blueGhost, blueX, blueY, 0);

    if (!yellowGhost) {
        cout << "Failed to load Yellow Ghost!" << endl;
        al_destroy_display(display);
        return 0;
    }
    al_draw_bitmap(yellowGhost, yellowX, yellowY, 0);

    if (!greenGhost) {
        cout << "Failed to load Green Ghost!" << endl;
        al_destroy_display(display);
        return 0;
    }
    al_draw_bitmap(greenGhost, greenX, greenY, 0);

    ballsImage = al_load_bitmap("assets/maps/balls.png");
    if (!ballsImage) {
        cout << "Failed to load balls!" << endl;
        al_destroy_display(display);
        return 0;
    }
    al_draw_bitmap(ballsImage, k * 20, l * 20, 0);

    al_init_font_addon();
    al_init_ttf_addon();

    // Font initialization
    if (!al_init_ttf_addon()) {
        cout << "Failed to initialize Allegro TTF add-on." << endl;
        return -1;
    }

    font = al_load_font("./assets/fonts/LiberationMono-Bold.ttf", 28, 0);
    if (!font) {
        cout << "Failed to load font." << endl;
        al_destroy_display(display);
        return -1;
    }

    event_queue = al_create_event_queue();
    if (!event_queue) {
        cout << "Failed to create event queue" << endl;
        al_destroy_display(display);
        al_destroy_timer(timer);
        return 0;
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_flip_display();
    al_start_timer(timer);

    return 1;
}

int main(int argc, char* argv[]) {
    int counter = 0;
    int score = 0;
    if (!initialize()) return -1;

    while (!exitGame) {
        cout << counter << endl;
        counter++;

        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            if (intention != 1 && j >= 22) {
                i = 10;
                j = 0;
                posX = j * q;
                posY = i * q;
            } else if (intention != 3 && j <= -1) {
                i = 10;
                j = 22;
                posX = j * q;
                posY = i * q;
            }

            if (intention == 4) {
                if (MAP[i - 1][j] != '1') {
                    pacman = pac_up;
                    lastMouth = 2;
                    i--;
                    posY = i * q;
                    previous = 4;
                } else {
                    if (previous == 1 && MAP[i][j - 1] != '1') {
                        pacman = pac_left;
                        j--;
                        posX = j * q;
                    }
                    if (previous == 3 && MAP[i][j + 1] != '1') {
                        lastMouth = 3;
                        pacman = pac_right;
                        j++;
                        posX = j * q;
                    }
                }
                if (MAP[i][j] == '2') {
                    MAP[i][j] = '0';
                    balls--;
                    score++;
                }
            }

            if (intention == 2) {
                if (MAP[i + 1][j] != '1') {
                    pacman = pac_down;
                    lastMouth = 0;
                    i++;
                    posY = i * q;
                    previous = 2;
                } else {
                    if (previous == 1 && MAP[i][j - 1] != '1') {
                        pacman = pac_left;
                        j--;
                        posX = j * q;
                    }
                    if (previous == 3 && MAP[i][j + 1] != '1') {
                        lastMouth = 3;
                        pacman = pac_right;
                        j++;
                        posX = j * q;
                    }
                }
                if (MAP[i][j] == '2') {
                    MAP[i][j] = '0';
                    balls--;
                    score++;
                }
            }

            if (intention == 1) {
                if (MAP[i][j - 1] != '1') {
                    pacman = pac_left;
                    lastMouth = 1;
                    j--;
                    posX = j * q;
                    previous = 1;
                } else {
                    if (previous == 2 && MAP[i + 1][j] != '1') {
                        lastMouth = 0;
                        pacman = pac_down;
                        i++;
                        posY = i * q;
                    }
                    if (previous == 4 && MAP[i - 1][j] != '1') {
                        lastMouth = 2;
                        pacman = pac_up;
                        i--;
                        posY = i * q;
                    }
                }
                if (MAP[i][j] == '2') {
                    MAP[i][j] = '0';
                    balls--;
                    score++;
                }
            }

            if (intention == 3) {
                if (MAP[i][j + 1] != '1') {
                    pacman = pac_right;
                    lastMouth = 3;
                    j++;
                    posX = j * q;
                    previous = 3;
                } else {
                    if (previous == 2 && MAP[i + 1][j] != '1') {
                        lastMouth = 0;
                        pacman = pac_down;
                        i++;
                        posY = i * q;
                    }
                    if (previous == 4 && MAP[i - 1][j] != '1') {
                        lastMouth = 2;
                        pacman = pac_up;
                        i--;
                        posY = i * q;
                    }
                }
                if (MAP[i][j] == '2') {
                    MAP[i][j] = '0';
                    balls--;
                    score++;
                }
            }

            if (sim % 2 == 0) {
                aux = pacman;
                pacman = shutup; // If the variable sim is even, redraw pacman with mouth closed
                redraw = true;
            } else {
                pacman = aux;
                redraw = true; // Otherwise, redraw pacman normally

                switch (lastMouth) {
                    case 0:
                        pacman = pac_down; break;
                    case 1:
                        pacman = pac_left; break;
                    case 2:
                        pacman = pac_up; break;
                    case 3:
                        pacman = pac_right; break;
                }
            }

            sim++;
        }

        else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            break;

        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (ev.keyboard.keycode) {
                case ALLEGRO_KEY_UP:
                    previous = intention;
                    intention = 4;
                    break;
                case ALLEGRO_KEY_DOWN:
                    previous = intention;
                    intention = 2;
                    break;
                case ALLEGRO_KEY_LEFT:
                    previous = intention;
                    intention = 1;
                    break;
                case ALLEGRO_KEY_RIGHT:
                    previous = intention;
                    intention = 3;
                    break;
            }
        }

        else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
            switch (ev.keyboard.keycode) {	
                case ALLEGRO_KEY_ESCAPE:
                    exitGame = true;
                    break;
            } 
        }

        al_draw_textf(font, al_map_rgb(200, 200, 200), 0, 505, 0, "Score: %d", score);

        if (redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;
            al_clear_to_color(al_map_rgb(0, 0, 0));

            if (!gameOver)
                al_draw_bitmap(map, 0, 0, 0);
            else {
                al_draw_bitmap(map2, 0, 0, 0);
                al_destroy_sample(sample);
                al_play_sample(death, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                al_rest(0.8);
                endGame = true;
                al_draw_bitmap(gameOverScreen, 0, 0, 0);
                al_rest(1.3);
            }

            al_draw_textf(font, al_map_rgb(200, 200, 200), 0, 505, 0, "Score: %d", score);

            for (k = 0; k < 24; k++) {
                for (l = 0; l < 24; l++) {
                    if (balls == 0) continue;
                    else if (MAP[k][l] == '2')
                        al_draw_bitmap(ballsImage, l * 20, k * 20, 0);
                }
            }

            al_draw_bitmap(pacman, posX, posY, 0);
            al_draw_bitmap(yellowGhost, yellowX, yellowY, 0);
            al_draw_bitmap(blinky, bX, bY, 0);
            al_draw_bitmap(greenGhost, greenX, greenY, 0);
            al_draw_bitmap(blueGhost, blueX, blueY, 0);

            randomMove(MAP, r, t, blueX, blueY, 0);

            if (counter >= 70) randomMove(MAP, aX, aY, yellowX, yellowY, 1);
            if (counter >= 140) randomMove(MAP, vX, vY, greenX, greenY, 2);
            if (counter >= 210) blinkyMove(MAP, g, h, bX, bY);

            al_flip_display();
        }

        if (playWaka) {
            al_play_sample(sample, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
            playWaka = false;
        }

        if (initialSound) {
            al_play_sample(beginning, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            initialSound = false;
            al_rest(3.1);
            playWaka = true;
        }

        if ((g == i && h == j) || (i == r && j == t) || (i == aX && j == aY) || (i == vX && j == vY)) {
            gameOver = true;
        } else if ((lastPacManX == g && lastPacManY == h && i == ulx && j == uly) || 
                   (lastPacManX == r && lastPacManY == t && i == lastBlueX && j == lastBlueY) ||
                   (lastPacManX == aX && lastPacManY == aY && i == lastYellowX && j == lastYellowY) ||
                   (lastPacManX == vX && lastPacManY == vY && i == lastGreenX && j == lastGreenY)) {
            gameOver = true;
        }

        if (endGame) {
            al_destroy_bitmap(pacman);
            al_destroy_bitmap(ballsImage);
            al_destroy_bitmap(blinky);
            al_destroy_bitmap(blueGhost);
            al_destroy_bitmap(yellowGhost);
            al_destroy_bitmap(greenGhost);
            al_rest(3.92);
            return 0;
        }

        if (balls == 0) {
            al_destroy_sample(sample);
            al_play_sample(win, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            al_rest(4.8);
            return 0;
        }

        lastPacManX = i;
        lastPacManY = j;
        lastRedGhostX = g;
        lastRedGhostY = h;
        lastYellowX = aX;
        lastYellowY = aY;
        lastGreenX = vX;
        lastGreenY = vY;
        lastBlueX = r;
        lastBlueY = t;
    }

    al_destroy_bitmap(map);
    al_destroy_font(font);
    al_destroy_bitmap(pacman);
    al_destroy_bitmap(ballsImage);
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    al_destroy_sample(sample);
    al_destroy_bitmap(blinky);
    al_destroy_bitmap(blueGhost);
    al_destroy_bitmap(yellowGhost);
    al_destroy_bitmap(greenGhost);
    al_destroy_bitmap(gameOverScreen);

    return 0;
}
