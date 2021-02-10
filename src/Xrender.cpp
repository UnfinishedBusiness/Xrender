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
#include <geometry/geometry.h>
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
		gWindow = SDL_CreateWindow( string((std::string)init["window_title"]).c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (int)init["window_width"], (int)init["window_height"], SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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
void mouse_in(Xrender_object_t* o, nlohmann::json matrix_data, int mouseX, int mouseY) 
{ 
    if (o->data["mouse_over"] == false)
    {
        if (o->mouse_callback != NULL)
        {
            o->mouse_callback(o, {
                {"data", o->data},
                {"matrix_data", matrix_data},
                {"event", "mouse_in"},
                {"mouse_pos", {
                    {"x", mouseX},
                    {"y", mouseY}
                }}
            });
        }
    }
    o->data["mouse_over"] = true;
}
void mouse_out(Xrender_object_t* o, nlohmann::json matrix_data, int mouseX, int mouseY) 
{ 
    if (o->data["mouse_over"] == true)
    {
        if (o->mouse_callback != NULL)
        {
            o->mouse_callback(o, {
                {"data", o->data},
                {"matrix_data", matrix_data},
                {"event", "mouse_out"},
                {"mouse_pos", {
                    {"x", mouseX},
                    {"y", mouseY}
                }}
            });
        }
    }
    o->data["mouse_over"] = false;
}
void render_arc(double cx, double cy, double radius, double start_angle, double end_angle, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    double num_segments = 10;
    double_point_t start;
    double_point_t sweeper;
    double_point_t end;
    double_point_t last_point;
    start.x = cx + (radius * cosf((start_angle) * 3.1415926f / 180.0f));
    start.y = cy + (radius * sinf((start_angle) * 3.1415926f / 180.0f));
    end.x = cx + (radius * cosf((end_angle) * 3.1415926f / 180.0f));
    end.y = cy + (radius * sinf((end_angle) * 3.1415926 / 180.0f));
    double diff = MAX(start_angle, end_angle) - MIN(start_angle, end_angle);
    if (diff > 180) diff = 360 - diff;
    double angle_increment = diff / num_segments;
    double angle_pointer = start_angle + angle_increment;
    last_point = start;
    for (int i = 0; i < num_segments; i++)
    {
        sweeper.x = cx + (radius * cosf((angle_pointer) * 3.1415926f / 180.0f));
        sweeper.y = cy + (radius * sinf((angle_pointer) * 3.1415926f / 180.0f));
        angle_pointer += angle_increment;
        aalineRGBA(gRenderer, last_point.x, (double)init["window_height"] - last_point.y, sweeper.x, (double)init["window_height"] - sweeper.y, r, g, b, a);
        last_point = sweeper;
    }
    aalineRGBA(gRenderer, last_point.x, (double)init["window_height"] - last_point.y, end.x, (double)init["window_height"] - end.y, r, g, b, a);
}
bool Xrender_tick()
{
    tick_performance_timestamp = Xrender_millis();
    Geometry g;
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    mouseY = (int)init["window_height"] - mouseY;
    SDL_SetRenderDrawColor( gRenderer, (uint8_t)init["clear_color"]["r"], (uint8_t)init["clear_color"]["g"], (uint8_t)init["clear_color"]["b"], (uint8_t)init["clear_color"]["a"] );
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
                if (object_stack[x]->data["type"] == "text")
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
                if (object_stack[x]->data["type"] == "image")
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
        if (object_stack[x]->data["visable"] == true)
        {
            //printf("(Rendering %d) %s\n", x, object_stack[x]->data.dump().c_str());
            nlohmann::json data;
            if (object_stack[x]->matrix_data == NULL)
            {
                data = object_stack[x]->data;
            }
            else
            {
                data = object_stack[x]->matrix_data(object_stack[x]->data);
            }
            
            if (object_stack[x]->data["type"] == "line")
            {
                if (g.line_intersects_with_arc({{(double)data["start"]["x"], (double)data["start"]["y"]}, {(double)data["end"]["x"], (double)data["end"]["y"]}}, {(double)mouseX, (double)mouseY}, 10))
                {
                    mouse_in(object_stack[x], data, mouseX, mouseY);
                }
                else
                {
                    mouse_out(object_stack[x], data, mouseX, mouseY);
                }
                if (object_stack[x]->data["width"] == 1)
                {
                    aalineRGBA(gRenderer, (double)data["start"]["x"], (double)init["window_height"] - (double)data["start"]["y"], (double)data["end"]["x"], (double)init["window_height"] - (double)data["end"]["y"], (double)data["color"]["r"], data["color"]["g"], data["color"]["b"], data["color"]["a"]);
                }
                else
                {
                    thickLineRGBA(gRenderer, (double)data["start"]["x"], (double)init["window_height"] - (double)data["start"]["y"], (double)data["end"]["x"], (double)init["window_height"] -  (double)data["end"]["y"], data["width"], data["color"]["r"], data["color"]["g"], data["color"]["b"], data["color"]["a"]);
                }
            }
            else if (object_stack[x]->data["type"] == "arc")
            {
                if (g.distance({(double)data["center"]["x"], (double)data["center"]["y"]}, {(double)mouseX, (double)mouseY}) > ((double)data["radius"] - 5) &&
                    g.distance({(double)data["center"]["x"], (double)data["center"]["y"]}, {(double)mouseX, (double)mouseY}) < ((double)data["radius"] + 5) &&
                    g.measure_polar_angle({(double)data["center"]["x"], (double)data["center"]["y"]}, {(double)mouseX, (double)mouseY}) > (double)data["start_angle"] &&
                    g.measure_polar_angle({(double)data["center"]["x"], (double)data["center"]["y"]}, {(double)mouseX, (double)mouseY}) < (double)data["end_angle"])
                {
                    mouse_in(object_stack[x], data, mouseX, mouseY);
                }
                else
                {
                    mouse_out(object_stack[x], data, mouseX, mouseY);
                }
                //arcRGBA(gRenderer, (double)data["center"]["x"], (double)data["center"]["y"], (double)data["radius"], (double)data["start_angle"], (double)data["end_angle"], data["color"]["r"], data["color"]["g"], data["color"]["b"], data["color"]["a"]);
                render_arc((double)data["center"]["x"], (double)data["center"]["y"], (double)data["radius"], (double)data["start_angle"], (double)data["end_angle"], data["color"]["r"], data["color"]["g"], data["color"]["b"], data["color"]["a"]);
            }
            else if (object_stack[x]->data["type"] == "circle")
            {
                if (g.distance({(double)data["center"]["x"], (double)data["center"]["y"]}, {(double)mouseX, (double)mouseY}) > ((double)data["radius"] - 5) &&
                    g.distance({(double)data["center"]["x"], (double)data["center"]["y"]}, {(double)mouseX, (double)mouseY}) < ((double)data["radius"] + 5))
                {
                    mouse_in(object_stack[x], data, mouseX, mouseY);
                }
                else
                {
                    mouse_out(object_stack[x], data, mouseX, mouseY);
                }
                aacircleRGBA(gRenderer, (double)data["center"]["x"], (double)init["window_height"] - (double)data["center"]["y"], (double)data["radius"], data["color"]["r"], data["color"]["g"], data["color"]["b"], data["color"]["a"]);
            }
            else if (object_stack[x]->data["type"] == "box")
            {
                if (mouseX > (int)data["tl"]["x"] && mouseX < (int)data["br"]["x"] && mouseY > (int)data["tl"]["y"] && mouseY < (int)data["br"]["y"])
                {
                    mouse_in(object_stack[x], data, mouseX, mouseY);
                }
                else
                {
                    mouse_out(object_stack[x], data, mouseX, mouseY);
                }
                roundedBoxRGBA(gRenderer, (double)data["tl"]["x"], (double)init["window_height"] - (double)data["tl"]["y"], (double)data["br"]["x"], (double)init["window_height"] - (double)data["br"]["y"], (double)data["corner_radius"], data["color"]["r"], data["color"]["g"], data["color"]["b"], data["color"]["a"]);
            }
            else
            {
                dst.x = object_stack[x]->data["position"]["x"];
                dst.y = (int)init["window_height"] - (int)object_stack[x]->data["position"]["y"];
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
                dst.y -= (int)object_stack[x]->data["size"]["height"];
                SDL_SetTextureBlendMode( object_stack[x]->texture, SDL_BLENDMODE_BLEND );
                SDL_SetTextureAlphaMod( object_stack[x]->texture, object_stack[x]->data["color"]["a"] );
                SDL_RenderCopyEx( gRenderer, object_stack[x]->texture, NULL, &dst, object_stack[x]->data["angle"], NULL, SDL_FLIP_NONE);
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
        if ( e.type == SDL_WINDOWEVENT )
		{
			if (e.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                init["window_width"] =  (int)e.window.data1;
                init["window_height"] =  (int)e.window.data2;
            }
		}
        /*if (e.type == SDL_KEYUP)
        {
            printf("%s\n", SDL_GetKeyName(e.key.keysym.sym));
        }*/
        if(e.type == SDL_MOUSEWHEEL)
        {
            if(e.wheel.y > 0) // scroll up
            {
                // Put code for handling "scroll up" here!
            }
            else if(e.wheel.y < 0) // scroll down
            {
                // Put code for handling "scroll down" here!
            }

            if(e.wheel.x > 0) // scroll right
            {
                // ...
            }
            else if(e.wheel.x < 0) // scroll left
            {
                // ...
            }
        }
        if (e.type == SDL_MOUSEBUTTONDOWN)
        {
            if (e.button.button == SDL_BUTTON_LEFT)
            {
                for (int x = 0; x < object_stack.size(); x++)
                {
                    if (object_stack[x]->data["visable"] == true && object_stack[x]->data["mouse_over"] == true)
                    {
                        if (object_stack[x]->mouse_callback != NULL)
                        {
                            nlohmann::json matrix_data = object_stack[x]->data;
                            if (object_stack[x]->matrix_data != NULL)
                            {
                                matrix_data = object_stack[x]->matrix_data(object_stack[x]->data);
                            }
                            object_stack[x]->mouse_callback(object_stack[x], {
                                {"data", object_stack[x]->data},
                                {"matrix_data", matrix_data},
                                {"event", "left_click_down"},
                                {"mouse_pos", {
                                    {"x", mouseX},
                                    {"y", mouseY}
                                }}
                            });
                        }
                    }
                }
            }
            if (e.button.button == SDL_BUTTON_RIGHT)
            {
                for (int x = 0; x < object_stack.size(); x++)
                {
                    if (object_stack[x]->data["visable"] == true && object_stack[x]->data["mouse_over"] == true)
                    {
                        if (object_stack[x]->mouse_callback != NULL)
                        {
                            nlohmann::json matrix_data = object_stack[x]->data;
                            if (object_stack[x]->matrix_data != NULL)
                            {
                                matrix_data = object_stack[x]->matrix_data(object_stack[x]->data);
                            }
                            object_stack[x]->mouse_callback(object_stack[x], {
                                {"data", object_stack[x]->data},
                                {"matrix_data", matrix_data},
                                {"event", "right_click_down"},
                                {"mouse_pos", {
                                    {"x", mouseX},
                                    {"y", mouseY}
                                }}
                            });
                        }
                    }
                }
            }
            if (e.button.button == SDL_BUTTON_MIDDLE)
            {
                for (int x = 0; x < object_stack.size(); x++)
                {
                    if (object_stack[x]->data["visable"] == true && object_stack[x]->data["mouse_over"] == true)
                    {
                        if (object_stack[x]->mouse_callback != NULL)
                        {
                            nlohmann::json matrix_data = object_stack[x]->data;
                            if (object_stack[x]->matrix_data != NULL)
                            {
                                matrix_data = object_stack[x]->matrix_data(object_stack[x]->data);
                            }
                            object_stack[x]->mouse_callback(object_stack[x], {
                                {"data", object_stack[x]->data},
                                {"matrix_data", matrix_data},
                                {"event", "middle_click_down"},
                                {"mouse_pos", {
                                    {"x", mouseX},
                                    {"y", mouseY}
                                }}
                            });
                        }
                    }
                }
            }
        }
        if (e.type == SDL_MOUSEBUTTONUP)
        {
            if (e.button.button == SDL_BUTTON_LEFT)
            {
                for (int x = 0; x < object_stack.size(); x++)
                {
                    if (object_stack[x]->data["visable"] == true && object_stack[x]->data["mouse_over"] == true)
                    {
                        if (object_stack[x]->mouse_callback != NULL)
                        {
                            nlohmann::json matrix_data = object_stack[x]->data;
                            if (object_stack[x]->matrix_data != NULL)
                            {
                                matrix_data = object_stack[x]->matrix_data(object_stack[x]->data);
                            }
                            object_stack[x]->mouse_callback(object_stack[x], {
                                {"data", object_stack[x]->data},
                                {"matrix_data", matrix_data},
                                {"event", "left_click_up"},
                                {"mouse_pos", {
                                    {"x", mouseX},
                                    {"y", mouseY}
                                }}
                            });
                        }
                    }
                }
            }
            if (e.button.button == SDL_BUTTON_RIGHT)
            {
                for (int x = 0; x < object_stack.size(); x++)
                {
                    if (object_stack[x]->data["visable"] == true && object_stack[x]->data["mouse_over"] == true)
                    {
                        if (object_stack[x]->mouse_callback != NULL)
                        {
                            nlohmann::json matrix_data = object_stack[x]->data;
                            if (object_stack[x]->matrix_data != NULL)
                            {
                                matrix_data = object_stack[x]->matrix_data(object_stack[x]->data);
                            }
                            object_stack[x]->mouse_callback(object_stack[x], {
                                {"data", object_stack[x]->data},
                                {"matrix_data", matrix_data},
                                {"event", "right_click_up"},
                                {"mouse_pos", {
                                    {"x", mouseX},
                                    {"y", mouseY}
                                }}
                            });
                        }
                    }
                }
            }
            if (e.button.button == SDL_BUTTON_MIDDLE)
            {
                for (int x = 0; x < object_stack.size(); x++)
                {
                    if (object_stack[x]->data["visable"] == true && object_stack[x]->data["mouse_over"] == true)
                    {
                        if (object_stack[x]->mouse_callback != NULL)
                        {
                            nlohmann::json matrix_data = object_stack[x]->data;
                            if (object_stack[x]->matrix_data != NULL)
                            {
                                matrix_data = object_stack[x]->matrix_data(object_stack[x]->data);
                            }
                            object_stack[x]->mouse_callback(object_stack[x], {
                                {"data", object_stack[x]->data},
                                {"matrix_data", matrix_data},
                                {"event", "middle_click_up"},
                                {"mouse_pos", {
                                    {"x", mouseX},
                                    {"y", mouseY}
                                }}
                            });
                        }
                    }
                }
            }
        }
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