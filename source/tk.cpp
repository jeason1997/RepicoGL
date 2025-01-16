#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/internal/zgl.h>
#include <GL/glx.h>
#include <MemfbDefs.h>
#include "picotk.h"
#include "screen_config.h"
#include "debugger.h"

void *FrameBuffer = (void *) NULL;

struct TinyFBDevGLXContext {
    GLContext *gl_context; /* base class */
    void *screen;
    ZBuffer *zb; // 绘制用buffer
    int line_length;
};

static FBDevGLXContext *ctx;

/**
 * 初始化
 */
static void initialize_fbdev() {

    // 按照屏幕大小创建FrameBuffer空间
    FrameBuffer = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * SCREEN_BITS_PER_PIXEL / 8);

    if (FrameBuffer == NULL) {
        fprintf(stderr, "error: unable create framebuffer: %s\n",
                strerror(errno));
        exit(1);
    }
    
    fmt->println("info: succ create framebuffer");
}


static void shutdown_fbdev(void) {
    // 释放FrameBuffer
    free(FrameBuffer);
}

void errorcatcher();        /* routine to handle errors */

void tkSwapBuffers(void) {
    fbdev_glXSwapBuffers(ctx);
}

int ui_loop(int argc, char **argv, const char *name) {

    int done;
    int i;
    int k;

    // TODO: 增加键盘输入支持

    // 初始化glx上下文
    ctx = fbdev_glXCreateContext();
    
    if (ctx == NULL) {
        return -1;
    }

    initialize_fbdev();
    
    fbdev_glXMakeCurrent(ctx);
    
    fmt->printf("debug:tk.cpp: succ create cxt at %x, gl_context at %x\n", ctx, ctx->gl_context);
    
    fmt->printf("debug:tk.cpp: succ create zbuf{%d,%d,%d,%d} at %x\n", ctx->gl_context->zb->xsize, ctx->gl_context->zb->ysize, ctx->gl_context->zb->linesize, ctx->gl_context->zb->mode, ctx->gl_context->zb);

    picotk_init();
    
    fmt->printf("debug:tk.cpp: toggle reshaping (%d,%d)\n", SCREEN_WIDTH, SCREEN_HEIGHT);
    
    reshape(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    fmt->printf("debug:tk.cpp: reshaped (%d,%d), start render loop\n", SCREEN_WIDTH, SCREEN_HEIGHT);

    done = 0;

    while (!done) {

        // updateKBD(&kbuf, &modifiers, &pscancode);
        /* printf("kbuf == %x\n", kbuf);*/
        /*switch (kbuf) {
            case _KEY_UP:
                k = KEY_UP;
                break;
            case _KEY_DOWN:
                k = KEY_DOWN;
                break;
            case _KEY_LEFT:
                k = KEY_LEFT;
                break;
            case _KEY_RIGHT:
                k = KEY_RIGHT;
                break;
            case _KEY_ESCAPE:
                done = 1;
                break;
            default:

                break;
        }
        key(k, 0);*/
        idle();
    }
    // terminKBD();
    shutdown_fbdev();
    return 0;
}


/*
 * Here on an unrecoverable error.
 *
 */
void errorcatcher() {
}

/**
 * 获得帧缓存
 * @return
 */
void *getFrameBuffer() {
    return FrameBuffer;
}

size_t sizeOfFrameBuffer() {
    return (size_t) SCREEN_HEIGHT * SCREEN_LINE_BYTES_LENGTH;
}
