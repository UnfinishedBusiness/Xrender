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
#include <json/json.h>

unsigned long tick_performance; //Measurement of how long the tick function is taking in ms
unsigned long tick_performance_timestamp; //Millis timestamp of when last tick function began

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Texture* gTexture = NULL;
SDL_Event e;
nlohmann::json init;
vector<Xrender_key_event_t> key_events;
vector<Xrender_object_t*> object_stack;
vector<Xrender_timer_t> timers;


bool Xrender_init(nlohmann::json i)
{
    init = i;
    /*
        Make sure init paramaters are set and if not, then default! 
    */
    if (!init.contains("window_title"))
    {
        init["window_title"] = "Xrender";
    }
    if (!init.contains("window_width"))
    {
        init["window_width"] = 900;
    }
    if (!init.contains("window_height"))
    {
        init["window_height"] = 700;
    }
    if (!init.contains("show_cursor"))
    {
        init["show_cursor"] = true;
    }
    if (!init.contains("clear_color"))
    {
        init["clear_color"] = {
            {"r", 200},
            {"g", 200},
            {"b", 200},
            {"a", 255}
        };
    }

    bool success = true;
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
    else
	{
		//Set texture filtering to linear
		if(!SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ))
		{
			printf( "Warning: Linear texture filtering not enabled!\n");
		}
		//Create window
		gWindow = SDL_CreateWindow( string((std::string)init["window_title"]).c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (int)init["window_width"], (int)init["window_height"], SDL_WINDOW_SHOWN );
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
    SDL_ShowCursor((bool)init["show_cursor"]);
	return success;
}
bool Xrender_tick()
{
    tick_performance_timestamp = Xrender_millis();
    SDL_SetRenderDrawColor( gRenderer, (uint8_t)init["clear_color"]["r"], (uint8_t)init["clear_color"]["g"], (uint8_t)init["clear_color"]["g"], (uint8_t)init["clear_color"]["a"] );
	sort(object_stack.begin(), object_stack.end(), [](auto* lhs, auto* rhs) {
        return lhs->data["zindex"] < rhs->data["zindex"];
    });
    SDL_Rect dst;
    SDL_RenderClear( gRenderer );
    for (int x = 0; x < object_stack.size(); x++)
    {
        if (object_stack[x]->data["visable"] == true) //Texture re-gen
        {
            if (object_stack[x]->texture == NULL) //We need to render the texture!
            {
                if (object_stack[x]->type == "text")
                {
                    //printf("Rendering Text Texture!\n");
                    TTF_Font* f = TTF_OpenFont(string(object_stack[x]->data["font"]).c_str(), object_stack[x]->data["font_size"]);
                    if (f)
                    {
                        SDL_Color color = {object_stack[x]->data["color"]["r"], object_stack[x]->data["color"]["g"], object_stack[x]->data["color"]["b"]};
                        SDL_Surface* surfaceMessage = TTF_RenderText_Solid(f, string(object_stack[x]->data["textval"]).c_str(), color);
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
                if (object_stack[x]->type == "image")
                {
                    SDL_Surface* loadedSurface = IMG_Load(string(object_stack[x]->data["path"]).c_str());
                    if( loadedSurface == NULL )
                    {
                        printf( "Unable to load image %s! SDL_image Error: %s\n", string(object_stack[x]->data["path"]).c_str(), IMG_GetError() );
                    }
                    else
                    {
                        object_stack[x]->texture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
                        
                        if( object_stack[x]->texture == NULL )
                        {
                            printf( "Unable to create texture from %s! SDL Error: %s\n", string(object_stack[x]->data["path"]).c_str(), SDL_GetError() );
                        }
                        SDL_FreeSurface( loadedSurface );
                    }
                }
            }
        }
        if (object_stack[x]->data["visable"]== true)
        {
            nlohmann::json data;
            if (object_stack[x]->matrix_data == NULL)
            {
                data = object_stack[x]->data;
            }
            else
            {
                data = object_stack[x]->matrix_data(object_stack[x]->data);
            }
            if (object_stack[x]->type == "line")
            {
                if (object_stack[x]->data["width"] == 1)
                {
                    aalineRGBA(gRenderer, data["start"]["x"], data["start"]["y"], data["end"]["x"], data["end"]["y"], data["color"]["r"], data["color"]["g"], data["color"]["b"], data["color"]["a"]);
                }
                else
                {
                    thickLineRGBA(gRenderer, data["start"]["x"], data["start"]["y"], data["end"]["x"], data["end"]["y"], data["width"], data["color"]["r"], data["color"]["g"], data["color"]["b"], data["color"]["a"]);
                }
            }
            else if (object_stack[x]->type == "arc")
            {
                arcRGBA(gRenderer, data["center"]["x"], data["center"]["y"], data["radius"], data["start_angle"], data["end_angle"], data["color"]["r"], data["color"]["g"], data["color"]["b"], data["color"]["a"]);
            }
            else if (object_stack[x]->type == "circle")
            {
                aacircleRGBA(gRenderer, data["center"]["x"], data["center"]["y"], data["radius"], data["color"]["r"], data["color"]["g"], data["color"]["b"], data["color"]["a"]);
            }
            else if (object_stack[x]->type == "box")
            {
                roundedBoxRGBA(gRenderer, data["tl"]["x"], data["tl"]["y"], data["br"]["x"], data["br"]["y"], data["corner_radius"], data["color"]["r"], data["color"]["g"], data["color"]["b"], data["color"]["a"]);
            }
            else
            {
                dst.x = object_stack[x]->data["position"]["x"];
                dst.y = object_stack[x]->data["position"]["y"];
                if (object_stack[x]->data["size"]["width"] > 0 && object_stack[x]->data["size"]["height"] > 0)
                {
                    dst.w = object_stack[x]->data["size"]["width"];
                    dst.h = object_stack[x]->data["size"]["height"];
                }
                else
                {
                    SDL_QueryTexture(object_stack[x]->texture, NULL, NULL, &dst.w, &dst.h);
                    object_stack[x]->data["size"]["width"] = dst.w;
                    object_stack[x]->data["size"]["height"] = dst.h;
                }
                SDL_SetTextureBlendMode( object_stack[x]->texture, SDL_BLENDMODE_BLEND );
                SDL_SetTextureAlphaMod( object_stack[x]->texture, object_stack[x]->data["color"]["a"] );
                SDL_RenderCopyEx( gRenderer, object_stack[x]->texture, NULL, &dst, object_stack[x]->data["angle"], NULL, SDL_FLIP_NONE );
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
            if (key_events.at(x).type == "keyup" && e.type == SDL_KEYUP)
            {
                if (key_events.at(x).key == string(SDL_GetKeyName(e.key.keysym.sym)))
                {
                    if (key_events.at(x).callback != NULL)
                    {
                        key_events.at(x).callback();
                    }
                }
            }
            if (key_events.at(x).type == "keydown" && e.type == SDL_KEYDOWN)
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
    tick_performance = Xrender_millis() - tick_performance_timestamp;
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