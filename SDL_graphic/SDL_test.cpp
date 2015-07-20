#include "include/SDL//SDL.h"

#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")

#if 0
#include <stdio.h>

#define WIDTH 640
#define HEIGHT 480
#define BPP 4
#define DEPTH 32

void setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
    Uint32 *pixmem32;
    Uint32 colour;  
 
    colour = SDL_MapRGB( screen->format, r, g, b );
  
    pixmem32 = (Uint32*) screen->pixels  + y + x;
    *pixmem32 = colour;
}


void DrawScreen(SDL_Surface* screen, int h)
{ 
    int x, y, ytimesw;
  
    if(SDL_MUSTLOCK(screen)) 
    {
        if(SDL_LockSurface(screen) < 0) return;
    }

    for(y = 0; y < screen->h; y++ ) 
    {
        ytimesw = y*screen->pitch/BPP;
        for( x = 0; x < screen->w; x++ ) 
        {
            setpixel(screen, x, ytimesw, (x*x)/256+3*y+h, (y*y)/256+x+h, h);
        }
    }

    if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
  
    SDL_Flip(screen); 
}


int main(int argc, char* argv[])
{
    SDL_Surface *screen;
    SDL_Event event;
  
    int keypress = 0;
    int h=0; 
  
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ) return 1;
   
    if (!(screen = SDL_SetVideoMode(WIDTH, HEIGHT, DEPTH, SDL_HWSURFACE)))
    {
        SDL_Quit();
        return 1;
    }
  
    while(!keypress) 
    {
         DrawScreen(screen,h++);
         while(SDL_PollEvent(&event)) 
         {      
              switch (event.type) 
              {
                  case SDL_QUIT:
	              keypress = 1;
	              break;
                  case SDL_KEYDOWN:
                       keypress = 1;
                       break;
              }
         }
    }

    SDL_Quit();
  
    return 0;
}
#endif
#if 01
const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const char* WINDOW_TITLE = "SDL Start";

void SDL_Pixel(SDL_Surface* surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    Uint8* p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp)
    {
    case 1:
        *p = pixel;
        break;
    case 2:
        *(Uint16 *)p = pixel;
        break;
    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        }
        else
        {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;
    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

// PC上一般为32位深，可写得简单些
#if 0
void SDL_PixelRGB(SDL_Surface* surface, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
    int bpp = surface->format->BytesPerPixel;
    Uint8* p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    Uint32 pixel = SDL_MapRGB(surface->format, r, g, b);

    switch (bpp)
    {

    case 1:
        *p = pixel;
        break;
    case 2:
        *(Uint16 *)p = pixel;
        break;
    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        }
        else
        {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;
    // 32 bpp
    case 4:
        //Uint32* bufp = (Uint32 *)surface->pixels + y * surface->pitch + x * bpp;
        //*bufp = pixel;
        *(Uint32 *)p = pixel;
        break;
    }
}
#endif

// 32位深，4bpp
void SDL_PixelRGB(SDL_Surface* surface, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
    Uint8* p = (Uint8 *)surface->pixels + y * surface->pitch + (x << 2);
    Uint32 pixel = SDL_MapRGB(surface->format, r, g, b);
    *(Uint32 *)p = pixel;
}

void line(SDL_Surface* surface, int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b)
{
	int tmp;
	int dx = x2 - x1;
	int dy = y2 - y1;

	if (abs(dx) < abs(dy)) {
		if (y1 > y2) {
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		x1 <<= 16;
		/* dy is apriori >0 */
		dx = (dx << 16) / dy;
		while (y1 <= y2) {
			SDL_PixelRGB(surface, x1 >> 16, y1, r, g, b);
			x1 += dx;
			y1++;
		}
	} else {
		if (x1 > x2) {
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		y1 <<= 16;
		dy = dx ? (dy << 16) / dx : 0;
		while (x1 <= x2) {
			SDL_PixelRGB(surface, x1, y1 >> 16, r, g, b);
			y1 += dy;
			x1++;
		}
	}
    SDL_Flip(surface);
}

static void _circle_8(SDL_Surface* surface, int x, int y, int xc, int yc, Uint8 r, Uint8 g, Uint8 b)
{
	SDL_PixelRGB(surface, x + xc, y + yc, r, g, b);
	SDL_PixelRGB(surface, x - xc, y + yc, r, g, b);
	SDL_PixelRGB(surface, x + xc, y - yc, r, g, b);
	SDL_PixelRGB(surface, x - xc, y - yc, r, g, b);
	SDL_PixelRGB(surface, x + yc, y + xc, r, g, b);
	SDL_PixelRGB(surface, x - yc, y + xc, r, g, b);
	SDL_PixelRGB(surface, x + yc, y - xc, r, g, b);
	SDL_PixelRGB(surface, x - yc, y - xc, r, g, b);
}

// Bresenham
void circle(SDL_Surface* surface, int x, int y, int radius, Uint8 r, Uint8 g, Uint8 b)
{
	int xc = 0;
	int yc = radius;
	int d = 3 - (radius << 1);	// 3 - radius*2
	
	while (xc <= yc) {
		_circle_8(surface, x, y, xc, yc, r, g, b);
		if (d < 0) {
			d += (xc << 2)+ 6;	// d = d+xc*4+6
		} else {
			d += ( (xc - yc) << 2) + 10;	// d=d+(xc-yc)*4+10
			yc--;
		}
		xc++;
		//sleep(1);
	}

    SDL_Flip(surface);
}

int main(int argc, char *argv[])
{
   SDL_Init( SDL_INIT_VIDEO );

   SDL_Surface* screen = SDL_SetVideoMode( WINDOW_WIDTH, WINDOW_HEIGHT, 0, 
      SDL_HWSURFACE | SDL_DOUBLEBUF );
   SDL_WM_SetCaption( WINDOW_TITLE, 0 );

   SDL_Event event;
   printf("hello world.");

   SDL_LockSurface(screen);
   //SDL_PixelRGB(screen, 1, 1, 255, 0, 0);
   line(screen, 1, 1, 100, 100, 255, 0, 0);
   circle(screen, 320, 240, 50, 255, 0, 0);
   SDL_UnlockSurface(screen);

   bool gameRunning = true;

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
#endif