#include <SDL2/SDL.h>

#include "chip8.h"

const int SCREEN_SCALE = 10;
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 320;

void game_loop(SDL_Window *, SDL_Surface *, SDL_Rect *, chip8 *);

int main(int argc, char **argv)
{
    chip8 *c = malloc(sizeof(chip8));

    initialize(c);
    load_file(c, "demo.ch8");

    for (int i = 0x200; i < 0x200 + 100; i++) {
        // printf("%x\n", c->memory[i]);
    }


    SDL_Window* window = NULL;
    SDL_Surface* screen_surface = NULL;

    // simulated chip-8 display
    SDL_Rect display_rects[W_WIDTH * W_HEIGHT];
    int x = 0, y = 0;
    for (int i = 0; i < W_WIDTH * W_HEIGHT; i++) {
        display_rects[i].x = x;
        display_rects[i].y = y;
        display_rects[i].w = 10;
        display_rects[i].h = 10;

        if ((10 * (i + 1)) % 640 == 0) {
            y += 10;
        }
        x = (10 * (i + 1)) % 640;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    } else {
        window = SDL_CreateWindow("Chip-8 Display", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == NULL) {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        } else {
            game_loop(window, screen_surface, display_rects, c);
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    free(c);

    return 0;
}

void game_loop(SDL_Window *window, SDL_Surface *screen_surface, SDL_Rect *display_rects, chip8 *c)
{
    // screen surface from initialized window structure
    screen_surface = SDL_GetWindowSurface(window);

    // black background color
    SDL_FillRect(screen_surface, NULL, SDL_MapRGB(screen_surface->format, 0x00, 0x00, 0x00));
    
    int quit = 0;

    SDL_Event e;

    // begin game loop
    while (!quit) {
        // sdl event queue
        while (SDL_PollEvent( &e ) != 0) {
            // quit
            if(e.type == SDL_QUIT) {
                quit = 1;
            // keyboard I/O
            } else if (e.type == SDL_KEYDOWN) {
                for (int i = 0; i < 16; i++) {
                    c->keys[i] = 0;
                }

                switch (e.key.keysym.sym) {
                    // hexadecimal key mapping (key -> index/value)
                    /* "7" => 1, "8" => 2, "9" => 3, "u" => 4, "i" => 5, "o" => 6, "j" => 7,
                     * "k" => 8, "l" => 9, "," => 0, "m" => A, "." => B, "0" => C, "p" => D,
                     * ";" => E, "/" => F
                     */

                    case SDLK_7:
                    {
                        c->keys[0x1] = 1;
                    break;
                    }
                    case SDLK_8:
                    {
                        c->keys[0x2] = 1;
                    break;
                    }
                    case SDLK_9:
                    {
                        c->keys[0x3] = 1;
                    break;
                    }
                    case SDLK_u:
                    {
                        c->keys[0x4] = 1;
                    break;
                    }
                    case SDLK_i:
                    {
                        c->keys[0x5] = 1;
                    break;
                    }
                    case SDLK_o:
                    {
                        c->keys[0x6] = 1;
                    break;
                    }
                    case SDLK_j:
                    {
                        c->keys[0x7] = 1;
                    break;
                    }
                    case SDLK_k:
                    {
                        c->keys[0x8] = 1;
                    break;
                    }
                    case SDLK_l:
                    {
                        c->keys[0x9] = 1;
                    break;
                    }
                    case SDLK_COMMA:
                    {
                        c->keys[0x0] = 1;
                    break;
                    }
                    case SDLK_m:
                    {
                        c->keys[0xA] = 1;
                    break;
                    }
                    case SDLK_PERIOD:
                    {
                        c->keys[0xB] = 1;
                    break;
                    }
                    case SDLK_0:
                    {
                        c->keys[0xC] = 1;
                    break;
                    }
                    case SDLK_p:
                    {
                        c->keys[0xD] = 1;
                    break;
                    }
                    case SDLK_SEMICOLON:
                    {
                        c->keys[0xE] = 1;
                    break;
                    }
                    case SDLK_SLASH:
                    {
                        c->keys[0xF] = 1;
                    break;
                    }
                }
            }
        }

        // execute the chip-8 interpreter
        if (c->pause == 0) {
            execute(c);
        }

        // draw monochrome chip-8 display
        for (int i = 0; i < W_WIDTH * W_HEIGHT; i++) {
            if (c->display[i] == 1) {
                SDL_FillRect(screen_surface, &display_rects[i], SDL_MapRGB(screen_surface->format, 255, 255, 255));
            } else {
                SDL_FillRect(screen_surface, &display_rects[i], SDL_MapRGB(screen_surface->format, 0, 0, 0));
            }
        }

        // draw rects to the surface
        SDL_UpdateWindowSurface( window );
    }
}
