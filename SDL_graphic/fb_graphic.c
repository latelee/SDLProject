/**
 * @file   fb_graphic.c
 * @author Late Lee <www.latelee.org>
 * @date   2011.06
 * 
 * @brief
 *         本文件包括：初始化图形界面，设置颜色，画像素、直线、方框、圆；
 *                     显示英文、中文字符
 *
 * @note   1、本想将显示中英文函数放到另一文件，但略显麻烦，故与图形相关的均放在一起。
           2、使用汉字库显示，支持16点阵和24点阵，在代码或Makefile中须定义HZK16或HZK24
		   3、本文件与fb_utils.c一起使用，初始化步骤如下：
		   	fb_init();	// 初始化frambuffer(即LCD)
			graphic_init();	// 初始化图形界面
			color_init(palette, NR_COLORS);	// 初始化颜色值(与图形相关函数最后一个参数
			                                   即为palette数组中的颜色值索引，须定义palette)
 * @log
 *     1. 2011.06
                 主要使用tslib中相关的代码
       2. 2011.10
	             添加画圆的函数				 
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#include "fb_graphic.h"

//#define HZK24
#ifdef HZK24 /* 24 */
#include "ascii24.h"
#define ASCII_CODE ascii24
#define FONT_SIZE  24			/* size: 24 */
#endif

#ifdef HZK16	/* 16 */

#include "ascii16.h"
#define ASCII_CODE ascii16
#define FONT_SIZE  16			/* size: 16 */
#endif

// todo here

#if !defined (HZK24) && !defined (HZK)
#error "you must define HZK16 or HZK24!"
#endif

#define BYTES (FONT_SIZE/8)			/* for HZ: 3 bytes  2 bytes*/
#define BUF_SIZE (BYTES * FONT_SIZE)		/* HZ buff 3*24 = 72 bytes 2*16 = 32 bytes */

#define ASCII_BYTES (BYTES-1)		/* 2 1*/
#define ASCII_SIZE (FONT_SIZE * ASCII_BYTES)	/* ASCII buffer: 24*2 = 48 bytes 16 * 1 = 16 bytes */
#define ASCII_WIDTH (FONT_SIZE/2)		/* ASCII: 16*8 24*12 */

//#define DEBUG_MSG
#if defined DEBUG_MSG
#define debug(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define debug(fmt, ...)
#endif /* debug msg */

static unsigned char *fbuffer;
static unsigned char **line_addr;
static int bytes_per_pixel;
static unsigned colormap [256];
int xres, yres;
int fd;
int r_len, r_offset;
int g_len, g_offset;
int b_len, b_offset;

extern FRAMEBUFFER *fb;	// define in fb_utils.c

union multiptr {
	unsigned char *p8;
	unsigned short *p16;
	unsigned long *p32;
};

int graphic_init (void)
{
	if (!fb) {
		fb_init();
	}
	
	if (fb) {
		xres = fb->width;
		yres = fb->height;
		bytes_per_pixel = fb->bytes_per_pixel;
		fbuffer = fb->fbmem;
		line_addr = fb->line_addr;
		fd = fb->fd;
		r_len = fb->fbinfo.red.length;
		r_offset = fb->fbinfo.red.offset;

		g_len = fb->fbinfo.green.length;
		g_offset = fb->fbinfo.green.offset;

		b_len = fb->fbinfo.blue.length;
		b_offset = fb->fbinfo.blue.offset;

		debug("xres: %d yres: %d bytes_per_pixel: %d\n", xres, yres, bytes_per_pixel);
		debug("(unsigned): %d\n", sizeof(unsigned));
		return 0;
	} else {
		debug("init fb first!\n");
		return -1;
	}
}
void color_init(int palette[], int len)
{
	int i = 0;
	for (i = 0; i < len; i++)
		setcolor (i, palette [i]);
}
void setcolor(unsigned colidx, unsigned value)
{
	unsigned res;
	unsigned short red, green, blue;
	struct fb_cmap cmap;

#ifdef DEBUG
	if (colidx > 255) {
		fprintf (stderr, "WARNING: color index = %u, must be <256\n",
			 colidx);
		return;
	}
#endif

	switch (bytes_per_pixel) {
	default:
	case 1:
		res = colidx;
		red = (value >> 8) & 0xff00;
		green = value & 0xff00;
		blue = (value << 8) & 0xff00;
		cmap.start = colidx;
		cmap.len = 1;
		cmap.red = &red;
		cmap.green = &green;
		cmap.blue = &blue;
		cmap.transp = NULL;

        	if (ioctl (fd, FBIOPUTCMAP, &cmap) < 0)
        	        perror("ioctl FBIOPUTCMAP");
		break;
	case 2:
	case 4:
		red = (value >> 16) & 0xff;
		green = (value >> 8) & 0xff;
		blue = value & 0xff;
		res = ((red >> (8 - r_len)) << r_offset) |
                      ((green >> (8 - g_len)) << g_offset) |
                      ((blue >> (8 - b_len)) << b_offset);
	}
        colormap [colidx] = res;
}

static inline void __setpixel (union multiptr loc, unsigned xormode, unsigned color)
{
	switch(bytes_per_pixel) {
	case 1:
	default:
		if (xormode)
			*loc.p8 ^= color;
		else
			*loc.p8 = color;
		break;
	case 2:
		if (xormode)
			*loc.p16 ^= color;
		else
			*loc.p16 = color;
		break;
	case 4:
		if (xormode)
			*loc.p32 ^= color;
		else
			*loc.p32 = color;
		break;
	}
}

void pixel(int x, int y, unsigned colidx)
{
	unsigned xormode;
	union multiptr loc;

	if ((x < 0) || ((__u32)x >= xres) ||
	    (y < 0) || ((__u32)y >= yres))
		return;

	xormode = colidx & XORMODE;
	colidx &= ~XORMODE;

#ifdef DEBUG
	if (colidx > 255) {
		fprintf (stderr, "WARNING: color value = %u, must be <256\n",
			 colidx);
		return;
	}
#endif

	loc.p8 = line_addr [y] + x * bytes_per_pixel;
	__setpixel(loc, xormode, colormap [colidx]);
}

void line(int x1, int y1, int x2, int y2, unsigned colidx)
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
			pixel(x1 >> 16, y1, colidx);
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
			pixel (x1, y1 >> 16, colidx);
			y1 += dy;
			x1++;
		}
	}
}

void rect (int x1, int y1, int x2, int y2, unsigned colidx)
{
	line (x1, y1, x2, y1, colidx);
	line (x2, y1, x2, y2, colidx);
	line (x2, y2, x1, y2, colidx);
	line (x1, y2, x1, y1, colidx);
}

void fillrect (int x1, int y1, int x2, int y2, unsigned colidx)
{
	int tmp;
	unsigned xormode;
	union multiptr loc;

	/* Clipping and sanity checking */
	if (x1 > x2) { tmp = x1; x1 = x2; x2 = tmp; }
	if (y1 > y2) { tmp = y1; y1 = y2; y2 = tmp; }
	if (x1 < 0) x1 = 0; if ((__u32)x1 >= xres) x1 = xres - 1;
	if (x2 < 0) x2 = 0; if ((__u32)x2 >= xres) x2 = xres - 1;
	if (y1 < 0) y1 = 0; if ((__u32)y1 >= yres) y1 = yres - 1;
	if (y2 < 0) y2 = 0; if ((__u32)y2 >= yres) y2 = yres - 1;

	if ((x1 > x2) || (y1 > y2))
		return;

	xormode = colidx & XORMODE;
	colidx &= ~XORMODE;

#ifdef DEBUG
	if (colidx > 255) {
		fprintf (stderr, "WARNING: color value = %u, must be <256\n",
			 colidx);
		return;
	}
#endif

	colidx = colormap [colidx];

	for (; y1 <= y2; y1++) {
		loc.p8 = line_addr [y1] + x1 * bytes_per_pixel;
		for (tmp = x1; tmp <= x2; tmp++) {
			__setpixel (loc, xormode, colidx);
			loc.p8 += bytes_per_pixel;
		}
	}
}

void put_cross(int x, int y, unsigned colidx)
{
	line (x - 10, y, x - 2, y, colidx);
	line (x + 2, y, x + 10, y, colidx);
	line (x, y - 10, x, y - 2, colidx);
	line (x, y + 2, x, y + 10, colidx);

#if 1
	line (x - 6, y - 9, x - 9, y - 9, colidx + 1);
	line (x - 9, y - 8, x - 9, y - 6, colidx + 1);
	line (x - 9, y + 6, x - 9, y + 9, colidx + 1);
	line (x - 8, y + 9, x - 6, y + 9, colidx + 1);
	line (x + 6, y + 9, x + 9, y + 9, colidx + 1);
	line (x + 9, y + 8, x + 9, y + 6, colidx + 1);
	line (x + 9, y - 6, x + 9, y - 9, colidx + 1);
	line (x + 8, y - 9, x + 6, y - 9, colidx + 1);
#else
	line (x - 7, y - 7, x - 4, y - 4, colidx + 1);
	line (x - 7, y + 7, x - 4, y + 4, colidx + 1);
	line (x + 4, y - 4, x + 7, y - 7, colidx + 1);
	line (x + 4, y + 4, x + 7, y + 7, colidx + 1);
#endif
}

#if 0
// good enough?
void line1(int x1, int y1, int x2, int y2, unsigned colidx)
{
	int i;
	float x_len, y_len;
	float abs_xlen, abs_ylen;
	float tmp = 0.0;
	
	x_len = (float)x2 - x1;
	y_len = (float)y2 - y1;
	
	abs_xlen = abs(x_len);
	abs_ylen = abs(y_len);
	
	if (abs_xlen > abs_ylen) {
		for (i = 0; i < abs_xlen; i++)
		{
			tmp = (float)y_len / abs_xlen * i;
			if (x_len < 0)
				pixel(x1 - i, y1 + tmp, colidx);
			else
				pixel(x1 + i, y1 + tmp, colidx);
		}
	} else {
		for (i = 0; i < abs_ylen; i++)
		{
			tmp = (float)x_len / abs_ylen * i;
			if (y_len < 0)
				pixel(x1 + tmp, y1 - i, colidx);
			else
				pixel(x1 + tmp, y1 + i, colidx);
		}
	}
}
#endif

static void _circle_8(int x, int y, int xc, int yc, unsigned colidx)
{
	pixel(x + xc, y + yc, colidx);
	pixel(x - xc, y + yc, colidx);
	pixel(x + xc, y - yc, colidx);
	pixel(x - xc, y - yc, colidx);
	pixel(x + yc, y + xc, colidx);
	pixel(x - yc, y + xc, colidx);
	pixel(x + yc, y - xc, colidx);
	pixel(x - yc, y - xc, colidx);
}

// Bresenham
void circle(int x, int y, int radius, unsigned colidx)
{
	int xc = 0;
	int yc = radius;
	int d = 3 - (radius << 1);	// 3 - radius*2
	
	while (xc <= yc) {
		_circle_8(x, y, xc, yc, colidx);
		if (d < 0) {
			d += (xc << 2)+ 6;	// d = d+xc*4+6
		} else {
			d += ( (xc - yc) << 2) + 10;	// d=d+(xc-yc)*4+10
			yc--;
		}
		xc++;
		//sleep(1);
	}
}

void fillcircle(int x, int y, int radius, unsigned colidx)
{
	int i  = 0;
	int xc = 0;
	int yc = radius;
	int d  = 3 - (radius << 1);	// 3 - radius*2
	
	while (xc <= yc) {
		for (i = xc; i <= yc; i++)
			_circle_8(x, y, xc, i, colidx);	// fill it
		if (d < 0) {
			d += (xc << 2)+ 6;	// d = d+xc*4+6
		} else {
			d += ( (xc - yc) << 2) + 10;	// d=d+(xc-yc)*4+10
			yc--;
		}
		xc++;
	}
}
#if 0
// Bresenham ??
void circle1(int x, int y, int radius, unsigned colidx)
{
	int xc = 0;
	int yc = radius;
	int d;
	//d = 3 - (radius << 1);	// 3 - radius*2
	d = 1 - radius;
	
	while (xc <= yc) {
		_circle_8(x, y, xc, yc, colidx);
		if (d < 0) {
			d += (xc << 1)+ 3;	// d = d+xc*2+3
		} else {
			d += ( (xc - yc) << 1) + 5;	// d=d+(xc-yc)*2+5
			yc--;
		}
		xc++;
		//sleep(1);
	}
}
#endif
/**
 * __display_ascii - Display an ASCII code on touch screen
 * @x: Column
 * @y: Row
 * @ascii: Which ASCII code to display
 * @colidx: Color index(?)
 * This routine display an ASCII code that stored in an array(eg, ASCII_CODE).
 * 16x8 ASCII code takes 1 byte, 24*12 ASCII code takes 2 bytes, so we need
 * -ASCII_BYTES-.
 */
static void __display_ascii(int x, int y, char *ascii, unsigned colidx)
{
	int i, j, k;
	unsigned char *p_ascii;
	int offset;

	offset = (*ascii - 0x20 ) * ASCII_SIZE; /* find the code in the array */
	p_ascii = ASCII_CODE + offset;

	for(i=0;i<FONT_SIZE;i++)
		for(j=0;j<ASCII_BYTES;j++)
			for(k=0;k<8;k++) {
				if( p_ascii[i*ASCII_BYTES+j] & (0x80>>k) )
				//if(*( p_ascii + i*ASCII_BYTES+j) & (0x80>>k))
					pixel(x + j*8 + k, y + i, colidx);
				//else
				//	pixel (x + j*8 + k, y + i, XORMODE);
				}
}

/**
 * put_string_ascii - Display an ASCII string on touch screen
 * @x: Column
 * @y: Row
 * @s: Which string to display
 * @colidx: Color index
 */
void put_string_ascii(int x, int y, char *s, unsigned colidx)
{
	while (*s != 0) {
		__display_ascii(x, y, s, colidx);
		x += ASCII_WIDTH;
		s++;
	}
}

/* not test */
void put_string_center_ascii(int x, int y, char *s, unsigned colidx)
{
	size_t sl = strlen (s);
        put_string_ascii (x - (sl / 2) * ASCII_WIDTH,
                    y - FONT_SIZE / 2, s, colidx);
}

/**
 * __display_font_16 - Display a 16x16 (chinese) character on touch screen
 * @fp: File pointer points to HZK(ie, HZK16)
 * @x: Column
 * @y: Row
 * @font: Which (chinese) character to display
 * @colidx: Color index
 * This routine ONLY display 16*16 character.
 * Every character takes two bytes, we show the first 8 bits, then the second 8 bits,
 * then the whole world will be shown before us.
 */
static void __display_font_16 (FILE *fp, int x, int y, char *font, unsigned colidx)
{
	int i, j, k;
	unsigned char mat[BUF_SIZE]={0};
	int qh,wh;
	unsigned long offset;
	qh = *font   - 0xa0;
	wh = *(font+1) - 0xa0;
	offset = ( 94*(qh-1) + (wh-1) ) * BUF_SIZE; /* offset of the character in HZK */

	/* read it */
	fseek(fp,offset,SEEK_SET);
	fread(mat,BUF_SIZE,1,fp);

	/* show it */
	for(i=0;i<FONT_SIZE;i++)
		for(j=0;j<BYTES;j++)
			for(k=0;k<8;k++) {
				if(mat [i*BYTES+j] & (0x80>>k))
					pixel (x + j*8 + k, y + i, colidx);
				//else
				//	pixel (x + j*8 + k, y + i, XORMODE);
				}
}

/**
 * __display_font_24 - Display a 24x24 (chinese) character on touch screen
 * @fp: File pointer points to HZK(ie, HZK24)
 * @x: Column
 * @y: Row
 * @font: Which (chinese) character to display
 * @colidx: Color index
 */
static void __display_font_24(FILE *fp, int x, int y, char *font, unsigned colidx)
{
	unsigned int i, j;
	unsigned char mat[FONT_SIZE][BYTES]={{0}};
	int qh,wh;
	unsigned long offset;
	qh = *font   - 0xaf;
	wh = *(font+1) - 0xa0;
	offset = ( 94*(qh-1) + (wh-1) ) * BUF_SIZE;

	fseek(fp,offset,SEEK_SET);
	fread(mat,BUF_SIZE,1,fp);

	for(i=0;i<FONT_SIZE;i++)
		for(j=0;j<FONT_SIZE;j++) {
			if( mat[j][i>>3] & (0x80>>(i&7)) )
			// if ( mat[j][i/8] & (0x80>>i%8) ) /* org */
				pixel(x + j, y + i, colidx);
			//else
			//	pixel (x + j, y + i, XORMODE);
			}
}

/**
 * put_string_hz - Display a (chinese) character string on touch screen
 * @fp: File pointer points to HZK(ie, HZK24 or HZK16)
 * @x: Column
 * @y: Row
 * @s: Which string to display
 * @colidx: Color index
 */
void put_string_hz(FILE *fp, int x, int y, char *s, unsigned colidx)
{
	while (*s != 0) {
		#ifdef HZK24
		__display_font_24(fp, x, y, s, colidx); /* for HZK24 */
		#else
		__display_font_16(fp, x, y, s, colidx);
		#endif
		x += FONT_SIZE;
		s += 2;	/* 2 bytes */
	}
}

/* not test */
void put_string_center_hz (FILE *fp, int x, int y, char *s, unsigned colidx)
{
	size_t sl = strlen ((char *)s);
        put_string_hz (fp, x - (sl/2) * FONT_SIZE, y - FONT_SIZE/2, s, colidx);
}

/**
 * put_font - Display an ASCII or/and (chinese) character string on touch screen
 * @fp: File pointer points to HZK(ie, HZK24 or HZK16)
 * @x: Column
 * @y: Row
 * @s: Which string to display
 * @colidx: Color index
 */
void put_font(FILE *fp, int x, int y, char *s, unsigned colidx)
{
	while (*s != 0) {
		if ( (*s>0xa0) && (*(s+1)>0xa0) ) {
			#ifdef HZK24
			__display_font_24(fp, x, y, s, colidx); 	/* for HZK24 */
			#else
			__display_font_16(fp, x, y, s, colidx);	/* for HZK16 */
			#endif
			x += FONT_SIZE;
			s += 2;	/* 2 bytes */
		} else {
			__display_ascii (x, y, (char *)s, colidx);
			x += ASCII_WIDTH;
			s++;	/* 1 byte */
		}
	}
}
/* not test */
void put_font_center(FILE *fp, int x, int y, char *s, unsigned colidx)
{
	size_t sl = strlen ((char *)s);
        put_font (fp, x - (sl/2) * FONT_SIZE, y - FONT_SIZE/2, s, colidx);
}
