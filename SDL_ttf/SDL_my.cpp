
/*
VS 2003使用SDL注意事项：
1、使用#pragma comment包含SDL.lib、SDLmain.lib
2、工程属性选"Use MFC in a Shared DLL"
3、main函数形式int main(int argc, char* argv[])
4、其它
*/

#include <stdio.h>
#include "SDL_test.h"
#include "include/SDL/SDL.h"
//#include "include/SDL/SDL_ttf.h"
#include "SDL_ttf.h"

#pragma comment(lib, "lib/SDL.lib")
#pragma comment(lib, "lib/SDLmain.lib")
//#pragma comment(lib, "lib/SDL_ttf.lib")
#pragma comment(lib, "lib/freetype2410MT.lib")

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const char* WINDOW_TITLE = "SDL Test";

SDL_Surface* g_screen = NULL;
SDL_Surface* g_message = NULL;
TTF_Font* g_font = NULL;

int InitGraphic(SDL_Surface** screen)
{
    const SDL_VideoInfo *info;
    Uint8  video_bpp;
    Uint32 videoflags;

    // Initialize SDL
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
        return 1;
    }
    atexit(SDL_Quit);

    // Alpha blending doesn't work well at 8-bit color
    info = SDL_GetVideoInfo();
    if ( info->vfmt->BitsPerPixel > 8 )
    {
        video_bpp = info->vfmt->BitsPerPixel;
    }
    else
    {
        video_bpp = 16;
    }
    videoflags = SDL_SWSURFACE | SDL_DOUBLEBUF;

    // Set 640x480 video mode
    if ( (*screen=SDL_SetVideoMode(WINDOW_WIDTH,WINDOW_HEIGHT,video_bpp,videoflags)) == NULL ) {
        fprintf(stderr, "Couldn't set %ix%i video mode: %s\n",WINDOW_WIDTH,WINDOW_HEIGHT,SDL_GetError());
        return 2;
    }

    SDL_WM_SetCaption(WINDOW_TITLE, 0);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int sdl_init(void)
{
    int ret = 0;
    ret = InitGraphic(&g_screen);
    if (ret != 0)
        return ret;
    if (TTF_Init() == -1)
    {
        return 3;
    }

    return 0;
}

void sdl_doit()
{
    SDL_Event event;
    int done = 0;

    while (!done)
    {
        /* Slow down polling */
        SDL_Delay(500);

        /* Check for events */
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                //case SDL_KEYDOWN:
                /* Any keypress quits the app... */
            case SDL_QUIT:
                done = 1;
                break;
            default:
                break;
            }
        }
    }
}

void sdl_exit()
{
    SDL_FreeSurface(g_screen);
    SDL_FreeSurface(g_message);
    TTF_CloseFont(g_font);
    TTF_Quit();
    SDL_Quit();
}

//---------------------------

void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL )
{
    //Holds offsets
    SDL_Rect offset;

    //Get offsets
    offset.x = x;
    offset.y = y;

    //Blit
    SDL_BlitSurface( source, clip, destination, &offset );
}

void show_font()
{
    SDL_Color color = {255, 255, 255, 0};
    //g_font = TTF_OpenFont("sfd/FreeSerif.ttf", 32);

    g_font = TTF_OpenFont("fz_songti.ttf", 32);
    if (g_font == NULL)
    {
        printf("Open font failed!\n");
        return;
    }

    printf("%d %d %d %d", TTF_FontHeight(g_font), TTF_FontAscent(g_font), TTF_FontDescent(g_font), TTF_FontLineSkip(g_font));
    g_message = TTF_RenderText_Solid(g_font, "Sphinx of black quartz, judge my vow!", color);
    if (g_message == NULL)
    {
        printf("Render Text failed!\n");
        return;
    }

    apply_surface(0, 0, g_message, g_screen);

    SDL_Flip(g_screen); // refresh

}
int my_test(int argc, char* argv[])
{
    sdl_init();

    show_font();

    sdl_doit();
    sdl_exit();

    return 0;
}
