/*******************************************************************
VS 2003使用SDL注意事项：
1、包含SDL.lib、SDLmain.lib
2、工程属性“Use of MFC”选"Use MFC in a Shared DLL"
3、main函数形式int main(int argc, char* argv[])
4、其它(SDL.dll要放到合适的目录)
*******************************************************************/

/*******************************************************************************************
                          基于SDL的简易YUV播放器
一、SDLTest1
0、使用方法：YUVPlayer.exe [YUV视频文件] [宽] [高]
1、播放YUV文件，显示图像窗口固定为640x480
2、针对不同格式，需要修改图像宽、高，QCIF、CIF格式（QCIF：176x144 CIF：352x288）
3、由于进行了缩放，图像不一定会填充满显示的窗口。

二、SDLTest2
　针对每一帧为一个YUV文件的情况。需要知道有多少个文件，需要知道文件名称。

BUG：延时靠经验，不好
*******************************************************************************************/

#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const char* WINDOW_TITLE = "SDL YUV Player";

int imgWidth  = 0; //图像宽;
int imgHeight = 0; //图像高;

SDL_Surface* screen;
SDL_Overlay* overlay;
SDL_Event event;
SDL_Rect rect;

typedef struct yuv_pic_tag
{
    char* pv_data[4];
    int   v_linesize[4]; //number of bytes per line
} yuv_pic;


int SDLInit()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 0,  SDL_HWSURFACE | SDL_DOUBLEBUF);  // 显示界面大小
    SDL_WM_SetCaption(WINDOW_TITLE, 0);
    // 创建用于显示图像的overlay
    overlay = SDL_CreateYUVOverlay(imgWidth, imgHeight, SDL_YV12_OVERLAY, screen);    // 图像大小
    if (overlay == NULL)
    {
        printf("Create YUV Overlay failed.\n");
        return -1;
    }

    return 0;
}

// YUV为一个文件
int SDLTest1(int argc, char* argv[])
{
    FILE* fp;
    int frameSize = 0; 
    int picSize = 0;
    int ret = 0;
    int i = 0;
    char* framePtr = NULL;
    yuv_pic yuvPicture;
    
    if (argc != 4)
    {
        printf("usage: %s [yuv file] [width]  [height].\n", argv[0]);
        return -1;
    }

    fp = fopen(argv[1], "rb");
    if (fp == NULL)
    {
        printf("open file error.\n");
        return -1;
    }

    imgWidth  = atoi(argv[2]);
    imgHeight = atoi(argv[3]);
    frameSize = imgWidth * imgHeight;
    picSize   = frameSize * 3 / 2;

    SDLInit();

    framePtr = (char *) malloc(sizeof(char) * picSize);
    if (framePtr == NULL)
    {
        printf("malloc failed.\n");
        return -1;
    }
    memset(framePtr, '\0', picSize);

    SDL_LockYUVOverlay(overlay);

    yuvPicture.pv_data[0] = framePtr;
    yuvPicture.pv_data[1] = framePtr + frameSize;
    yuvPicture.pv_data[2] = framePtr + frameSize + frameSize/4;
    yuvPicture.v_linesize[0] = imgWidth;
    yuvPicture.v_linesize[1] = imgWidth / 2;
    yuvPicture.v_linesize[2] = imgWidth / 2;

    overlay->pixels[0] = (unsigned char *)yuvPicture.pv_data[0];
    overlay->pixels[2] = (unsigned char *)yuvPicture.pv_data[1];
    overlay->pixels[1] = (unsigned char *)yuvPicture.pv_data[2];

    overlay->pitches[0] = yuvPicture.v_linesize[0];
    overlay->pitches[2] = yuvPicture.v_linesize[1];
    overlay->pitches[1] = yuvPicture.v_linesize[2];

    SDL_UnlockYUVOverlay(overlay);
    
    rect.x = 0;
    rect.y = 0;
    rect.w = WINDOW_WIDTH;
    rect.h = WINDOW_HEIGHT;

    while ((ret = (int)fread(framePtr, 1, picSize, fp)) > 0)
    {
        i++;
        fseek(fp, picSize * i, SEEK_SET);

        SDL_LockYUVOverlay(overlay);

        SDL_DisplayYUVOverlay(overlay, &rect);  // 显示

        SDL_UnlockYUVOverlay(overlay);
        SDL_PollEvent(&event);
        switch (event.type)
        {
        case SDL_QUIT:
            goto done;
            break;
        default:
            break;
        }
        SDL_Delay(40);
        printf("displaying frame %d size: %d\n", i, ret);
    }

done:
    printf("done.\n");
    fclose(fp);
    free(framePtr);
    SDL_Quit();

    return 0;
}

// 每一帧为一个YUV文件。
int SDLTest2(int argc, char* argv[])
{
    FILE* fp;
    int frameSize = 0; 
    int picSize = 0;
    int ret = 0;
    int i = 1;
    char filename[32] = {0};
    char* framePtr = NULL;
    yuv_pic yuvPicture;


    imgWidth  = 352;
    imgHeight = 240;

    frameSize = imgWidth * imgHeight;
    picSize   = frameSize * 3 / 2;

    framePtr = (char *) malloc(sizeof(char) * picSize);
    if (framePtr == NULL)
    {
        printf("malloc failed.\n");
        return -1;
    }
    memset(framePtr, '\0', picSize);

    SDLInit();

    SDL_LockYUVOverlay(overlay);

    yuvPicture.pv_data[0] = framePtr;
    yuvPicture.pv_data[1] = framePtr + frameSize;
    yuvPicture.pv_data[2] = framePtr + frameSize + frameSize/4;
    yuvPicture.v_linesize[0] = imgWidth;
    yuvPicture.v_linesize[1] = imgWidth / 2;
    yuvPicture.v_linesize[2] = imgWidth / 2;

    overlay->pixels[0] = (unsigned char *)yuvPicture.pv_data[0];
    overlay->pixels[2] = (unsigned char *)yuvPicture.pv_data[1];
    overlay->pixels[1] = (unsigned char *)yuvPicture.pv_data[2];

    overlay->pitches[0] = yuvPicture.v_linesize[0];
    overlay->pitches[2] = yuvPicture.v_linesize[1];
    overlay->pitches[1] = yuvPicture.v_linesize[2];

    SDL_UnlockYUVOverlay(overlay);
    
    rect.x = 0;
    rect.y = 0;
    rect.w = WINDOW_WIDTH;
    rect.h = WINDOW_HEIGHT;

    int fileNum = 112;
    while (i < fileNum + 1)
    {
        sprintf(filename, "tennis/tt%03d.yuv", i);
        fp = fopen(filename, "rb");
        if (fp == NULL)
        {
            printf("open file error.\n");
            return -1;
        }
        ret = (int)fread(framePtr, 1, picSize, fp);

        SDL_LockYUVOverlay(overlay);

        SDL_DisplayYUVOverlay(overlay, &rect);  // 显示

        SDL_UnlockYUVOverlay(overlay);
        SDL_PollEvent(&event);
        switch (event.type)
        {
        case SDL_QUIT:
            goto done;
            break;
        default:
            break;
        }
        SDL_Delay(40);
        fclose(fp);
        i++;
        printf("displaying file %s size: %d\n", filename, ret);
    }

done:
    printf("done.\n");
    free(framePtr);
    SDL_Quit();

    return 0;
}

int main(int argc, char* argv[])
{
    SDLTest1(argc, argv);
    //SDLTest2(argc, argv);
    return 0;
}