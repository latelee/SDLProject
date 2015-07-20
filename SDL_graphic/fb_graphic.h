/**
 * @file   fb_graphic.h
 * @author Late Lee <www.latelee.org>
 * @date   2011.06
 * 
 * @brief
 *         本文件包括：初始化图形界面，设置颜色，画像素、直线、方框、圆；
 *                     显示英文、中文字符
 *
 * @log
 *     1. 2011.06
                 主要使用tslib中相关的代码
       2. 2011.10
	             添加画圆的函数				 
 */
 
#ifndef FB_GRAPHIC_H
#define FB_GRAPHIC_H
 
#include <stdio.h>
#include "fb_utils.h"

#define XORMODE	0x80000000

extern int xres, yres;	// define in fb_graphic.c

int  graphic_init (void);
void color_init(int palette[], int len);
void setcolor(unsigned colidx, unsigned value);
void pixel(int x, int y, unsigned colidx);
void line(int x1, int y1, int x2, int y2, unsigned colidx);
void rect (int x1, int y1, int x2, int y2, unsigned colidx);
void fillrect (int x1, int y1, int x2, int y2, unsigned colidx);

void circle(int x, int y, int radius, unsigned colidx);
void fillcircle(int x, int y, int radius, unsigned colidx);
#if 0
void line1(int x1, int y1, int x2, int y2, unsigned colidx);
void circle1(int x, int y, int radius, unsigned colidx);
#endif

/* ascii code */
void put_string_ascii(int x, int y, char *s, unsigned colidx);
void put_string_center_ascii(int x, int y, char *s, unsigned colidx);
/* cineses character */
void put_string_hz(FILE *fp, int x, int y, char *s, unsigned colidx);
void put_string_center_hz(FILE *fp, int x, int y, char *s, unsigned colidx);
/* chineses character & ascii */
void put_font(FILE *fp, int x, int y, char *s, unsigned colidx);
void put_font_center(FILE *fp, int x, int y, char *s, unsigned colidx);

#endif /* FB_GRAPHIC_H */