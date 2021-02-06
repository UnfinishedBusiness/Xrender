#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL2_gfxPrimitives.h>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <sys/time.h>
#include <ctime>
#include <algorithm> 
#include <Xrender.h>

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Texture* gTexture = NULL;
SDL_Event e;
Xrender_init_t init;
vector<Xrender_key_event_t> key_events;
vector<Xrender_object_t*> object_stack;
vector<Xrender_timer_t> timers;


bool Xrender_init(Xrender_init_t i)
{
    init = i;
    bool success = true;
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
    else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!\n");
		}
		//Create window
		gWindow = SDL_CreateWindow( init.window_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, init.window_width, init.window_height, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
            SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}
			}
		}
	}
    if (TTF_Init() == -1)
    {
        printf("Could not initialize SDL_TTF!\n");
        success = false;
    }
	return success;
}
bool Xrender_tick()
{
    SDL_SetRenderDrawColor( gRenderer, init.clear_color.r, init.clear_color.g, init.clear_color.b, init.clear_color.a );
	sort(object_stack.begin(), object_stack.end(), [](auto* lhs, auto* rhs) {
        return lhs->zindex < rhs->zindex;
    });
    SDL_Rect dst;
    SDL_RenderClear( gRenderer );
    for (int x = 0; x < object_stack.size(); x++)
    {
        if (object_stack[x]->visable == true) //Texture re-gen
        {
            if (object_stack[x]->texture == NULL) //We need to render the texture!
            {
                if (object_stack[x]->type == "TEXT")
                {
                    //printf("Rendering Text Texture!\n");
                    TTF_Font* f = TTF_OpenFont("./Sans.ttf", object_stack[x]->text.font_size);
                    if (f)
                    {
                        SDL_Color color = {object_stack[x]->text.color.r, object_stack[x]->text.color.g, object_stack[x]->text.color.b};
                        SDL_Surface* surfaceMessage = TTF_RenderText_Solid(f, object_stack[x]->text.textval.c_str(), color);
                        //SDL_DestroyTexture(ObjectStack[x].texture);
                        object_stack[x]->texture = SDL_CreateTextureFromSurface(gRenderer, surfaceMessage);
                        SDL_FreeSurface(surfaceMessage);
                        TTF_CloseFont(f);
                    }
                    else
                    {
                        printf("Could not render text!\n");
                    }
                }
                if (object_stack[x]->type == "IMAGE")
                {
                    SDL_Surface* loadedSurface = IMG_Load( object_stack[x]->image.path.c_str() );
                    if( loadedSurface == NULL )
                    {
                        printf( "Unable to load image %s! SDL_image Error: %s\n", object_stack[x]->image.path.c_str(), IMG_GetError() );
                    }
                    else
                    {
                        object_stack[x]->texture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
                        
                        if( object_stack[x]->texture == NULL )
                        {
                            printf( "Unable to create texture from %s! SDL Error: %s\n", object_stack[x]->image.path.c_str(), SDL_GetError() );
                        }
                        SDL_FreeSurface( loadedSurface );
                    }
                }
            }
        }
        if (object_stack[x]->visable == true)
        {
            if (object_stack[x]->type == "LINE")
            {
                thickLineRGBA(gRenderer, object_stack[x]->line.p1.x, object_stack[x]->line.p1.y, object_stack[x]->line.p2.x, object_stack[x]->line.p2.y, object_stack[x]->line.width, object_stack[x]->line.color.r, object_stack[x]->line.color.g, object_stack[x]->line.color.b, object_stack[x]->opacity);
            }
            else if (object_stack[x]->type == "BOX")
            {
                roundedBoxRGBA(gRenderer, object_stack[x]->box.p1.x, object_stack[x]->box.p1.y, object_stack[x]->box.p2.x, object_stack[x]->box.p2.y, object_stack[x]->box.radius, object_stack[x]->box.color.r, object_stack[x]->box.color.g, object_stack[x]->box.color.b, object_stack[x]->opacity);
            }
            else
            {
                dst.x = object_stack[x]->position.x;
                dst.y = object_stack[x]->position.y;
                if (object_stack[x]->size.w > 0 && object_stack[x]->size.h > 0)
                {
                    dst.w = object_stack[x]->size.w;
                    dst.h = object_stack[x]->size.h;
                }
                else
                {
                    SDL_QueryTexture(object_stack[x]->texture, NULL, NULL, &dst.w, &dst.h);
                }
                SDL_SetTextureBlendMode( object_stack[x]->texture, SDL_BLENDMODE_BLEND );
                SDL_SetTextureAlphaMod( object_stack[x]->texture, object_stack[x]->opacity );
                SDL_RenderCopyEx( gRenderer, object_stack[x]->texture, NULL, &dst, object_stack[x]->angle, NULL, SDL_FLIP_NONE );
            }
        }
    }
    SDL_RenderPresent( gRenderer );
	while ( SDL_PollEvent( &e ) != 0 )
	{
		//User requests quit
		if ( e.type == SDL_QUIT )
		{
			return false;
		}
        /*if (e.type == SDL_KEYUP)
        {
            printf("%s\n", SDL_GetKeyName(e.key.keysym.sym));
        }*/
        for (int x = 0; x < key_events.size(); x++)
        {
            if (key_events.at(x).type == "KEYUP" && e.type == SDL_KEYUP)
            {
                if (key_events.at(x).key == string(SDL_GetKeyName(e.key.keysym.sym)))
                {
                    if (key_events.at(x).callback != NULL)
                    {
                        key_events.at(x).callback();
                    }
                }
            }
            if (key_events.at(x).type == "KEYDOWN" && e.type == SDL_KEYDOWN)
            {
                if (key_events.at(x).key == string(SDL_GetKeyName(e.key.keysym.sym)))
                {
                    if (key_events.at(x).callback != NULL)
                    {
                        key_events.at(x).callback();
                    }
                }
            }
        }
    }
    for (int x = 0; x < timers.size(); x++)
    {
        if ((Xrender_millis() - timers[x].timer) > timers[x].intervol)
        {
            timers[x].timer = Xrender_millis();
            if (timers[x].callback != NULL)
            {
                if (timers[x].callback() == false) //Dont repeat
                {
                    timers.erase(timers.begin() + x);
                }
            }
        }
    }
    return true;
}
void Xrender_close()
{
    for (int x = 0; x < object_stack.size(); x++)
    {
        SDL_DestroyTexture( object_stack[x]->texture );
        object_stack[x]->texture = NULL;
        free(object_stack[x]);
    }
    SDL_DestroyRenderer( gRenderer );
    gRenderer = NULL;
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	IMG_Quit();
	SDL_Quit();
}