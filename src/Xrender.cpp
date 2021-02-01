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

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

auto program_start_time = chrono::steady_clock::now();

unsigned long Xrender_millis()
{
  auto end = chrono::steady_clock::now();
  unsigned long m = (unsigned long)chrono::duration_cast<chrono::milliseconds>(end - program_start_time).count();
  return m;
}

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Texture* gTexture = NULL;
SDL_Event e;
Xrender_init_t init;
vector<Xrender_key_event_t> key_events;
vector<Xrender_object_t*> object_stack;

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
            else //We are a texture
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
                    key_events.at(x).callback();
                }
            }
            if (key_events.at(x).type == "KEYDOWN" && e.type == SDL_KEYDOWN)
            {
                if (key_events.at(x).key == string(SDL_GetKeyName(e.key.keysym.sym)))
                {
                    key_events.at(x).callback();
                }
            }
        }
    }
    return true;
}
void Xrender_push_key_event(Xrender_key_event_t k)
{
    key_events.push_back(k);
}
Xrender_object_t *Xrender_push_text(string id_name, string textval, int font_size, Xrender_color_t color, SDL_Rect position)
{
    Xrender_object_t *o = new Xrender_object_t;
    o->id_name = id_name;
    o->type = "TEXT";
    o->zindex = 0;
    o->visable = true;
    o->opacity = 255;
    o->position = position;
    o->size.w = 0;
    o->size.h = 0;
    o->angle = 0;
    o->text.textval = textval;
    o->text.font_size = font_size;
    o->text.color = color;
    o->texture = NULL;
    object_stack.push_back(o);
    return o;
}
Xrender_object_t *Xrender_push_image(string id_name, string path, SDL_Rect position, int width, int height)
{
    Xrender_object_t *o = new Xrender_object_t;
    o->id_name = id_name;
    o->type = "IMAGE";
    o->zindex = 0;
    o->visable = true;
    o->opacity = 255;
    o->position = position;
    o->size.w = width;
    o->size.h = height;
    o->image.path = path;
    o->angle = 0;
    o->texture = NULL;
    object_stack.push_back(o);
    return o;
}
Xrender_object_t *Xrender_push_line(string id_name, SDL_Rect p1, SDL_Rect p2, int width)
{
    Xrender_object_t *o = new Xrender_object_t;
    o->id_name = id_name; 
    o->type = "LINE";
    o->zindex = 0;
    o->visable = true;
    o->opacity = 255;
    o->position.x = 0;
    o->position.y = 0;
    o->size.w = 0;
    o->size.h = 0;
    o->line.p1.x = p1.x;
    o->line.p1.y = p1.y;
    o->line.p2.x = p2.x;
    o->line.p2.y = p2.y;
    o->line.width = width;
    o->line.color.r = 0;
    o->line.color.g = 0;
    o->line.color.b = 0;
    o->angle = 0;
    o->texture = NULL;
    object_stack.push_back(o);
    return o;
}
Xrender_object_t *Xrender_push_box(string id_name, SDL_Rect p1, SDL_Rect p2, int radius)
{
    Xrender_object_t *o = new Xrender_object_t;
    o->id_name = id_name; 
    o->type = "BOX";
    o->zindex = 0;
    o->visable = true;
    o->opacity = 255;
    o->position.x = 0;
    o->position.y = 0;
    o->size.w = 0;
    o->size.h = 0;
    o->box.p1.x = p1.x;
    o->box.p1.y = p1.y;
    o->box.p2.x = p2.x;
    o->box.p2.y = p2.y;
    o->box.radius = radius;
    o->angle = 0;
    o->texture = NULL;
    object_stack.push_back(o);
    return o;
}
void Xrender_rebuilt_object(Xrender_object_t *o)
{
    SDL_DestroyTexture( o->texture );
    o->texture = NULL;
}
void Xrender_dump_object_stack()
{
    printf("Beginning stack dump:\n");
    for (int x = 0; x < object_stack.size(); x++)
    {
        if (object_stack[x]->type == "TEXT")
        {
            printf("[Object %d](TEXT)\n", x);
            printf("\tid_name=%s\n", object_stack[x]->id_name.c_str());
            printf("\tgroup_name=%s\n", object_stack[x]->group_name.c_str());
            printf("\tzindex=%d\n", object_stack[x]->zindex);
            printf("\tvisable=%s\n", object_stack[x]->visable ? "true" : "false");
            printf("\topacity=%d\n", object_stack[x]->opacity);
            printf("\tangle=%.4f\n", object_stack[x]->angle);
            printf("\ttextval=%s\n", object_stack[x]->text.textval.c_str());
        }
        if (object_stack[x]->type == "IMAGE")
        {
            printf("[Object %d](IMAGE)\n", x);
            printf("\tid_name=%s\n", object_stack[x]->id_name.c_str());
            printf("\tgroup_name=%s\n", object_stack[x]->group_name.c_str());
            printf("\tzindex=%d\n", object_stack[x]->zindex);
            printf("\tvisable=%s\n", object_stack[x]->visable ? "true" : "false");
            printf("\topacity=%d\n", object_stack[x]->opacity);
            printf("\tangle=%.4f\n", object_stack[x]->angle);
            printf("\tpath=%s\n", object_stack[x]->image.path.c_str());
        }
        if (object_stack[x]->type == "LINE")
        {
            printf("[Object %d](LINE)\n", x);
            printf("\tid_name=%s\n", object_stack[x]->id_name.c_str());
            printf("\tgroup_name=%s\n", object_stack[x]->group_name.c_str());
            printf("\tzindex=%d\n", object_stack[x]->zindex);
            printf("\tvisable=%s\n", object_stack[x]->visable ? "true" : "false");
            printf("\topacity=%d\n", object_stack[x]->opacity);
            printf("\tangle=%.4f\n", object_stack[x]->angle);
            printf("\tp1=(%d, %d)\n", object_stack[x]->line.p1.x, object_stack[x]->line.p1.y);
            printf("\tp2=(%d, %d)\n", object_stack[x]->line.p2.x, object_stack[x]->line.p2.y);
            printf("\twidth=%d\n", object_stack[x]->line.width);
        }
    }
    printf("End stack dump:\n");
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