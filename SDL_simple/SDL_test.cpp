
/*
VS 2003使用SDL注意事项：
1、包含SDL.lib、SDLmain.lib
2、工程属性选"Use MFC in a Shared DLL"
3、main函数形式int main(int argc, char* argv[])
4、其它
*/
#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")

#include <stdio.h>
#include "include/SDL/SDL.h"

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const char* WINDOW_TITLE = "SDL Test";

int main(int argc, char* argv[])
{
    SDL_Surface* screen;
    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 0,  SDL_HWSURFACE | SDL_DOUBLEBUF);
    SDL_WM_SetCaption(WINDOW_TITLE, 0);

    while (1)
    {
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                goto done;
            }
        } 
    }

done:
    SDL_Quit();

    return 0;
}