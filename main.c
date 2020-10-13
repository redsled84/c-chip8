#include <SDL2/SDL.h>

#include "chip8.h"

const int SCREEN_SCALE = 10;
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 320;

int main(int argc, char **argv)
{
	chip8 *c;
	initialize(c);

	//The window we'll be rendering to
	SDL_Window* window = NULL;
	    
	//The surface contained by the window
	SDL_Surface* screenSurface = NULL;

	SDL_Rect chip8_display[W_WIDTH * W_HEIGHT];
	int x = 0, y = 0;
	for (int i = 0; i < W_WIDTH * W_HEIGHT; i++) {
		chip8_display[i].x = x;
		chip8_display[i].y = y;
		chip8_display[i].w = 10;
		chip8_display[i].h = 10;

		if ((10 * i) == 640) {
			y += 10;
		}
		x = (10 * i) % 640;
	}

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) < 0 )
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
	}
	else
	{
		//Create window
		window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( window == NULL )
		{
			printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
		}
		else
		{
			//Get window surface
			screenSurface = SDL_GetWindowSurface( window );

			//Fill the surface white
			SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0x00, 0x00, 0x00 ) );
			
			//Main loop flag
            int quit = 0;

            //Event handler
            SDL_Event e;

            //While application is running
            while( !quit ) {
				//Handle events on queue
	            while( SDL_PollEvent( &e ) != 0 )
	            {
	                //User requests quit
	                if( e.type == SDL_QUIT )
	                {
	                    quit = 1;
	                }
	                //User presses a key
	                else if( e.type == SDL_KEYDOWN )
	                {
	                    //Select surfaces based on key press
	                    switch( e.key.keysym.sym )
	                    {
	                    	case SDLK_UP:
	                        break;

	                        case SDLK_DOWN:
	                        break;

	                        case SDLK_LEFT:
	                        break;

	                        case SDLK_RIGHT:
	                        break;
	                    }
	                }
	            }

	            for (int i = 0; i < W_WIDTH * W_HEIGHT; i++) {
	            	if (c->display[i] == 1) {
						SDL_FillRect(screenSurface, &chip8_display[i], SDL_MapRGB(screenSurface->format, 255, 255, 255));
					} else {
						SDL_FillRect(screenSurface, &chip8_display[i], SDL_MapRGB(screenSurface->format, 0, 0, 0));
					}
	            }
				//Update the surface
				SDL_UpdateWindowSurface( window );
	        }

		}
	}

	//Destroy window
	SDL_DestroyWindow( window );

	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}
