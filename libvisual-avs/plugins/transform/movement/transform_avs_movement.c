/* Libvisual-AVS - Advanced visual studio for libvisual
 * 
 * Copyright (C) 2005, 2006 Dennis Smit <ds@nerds-incorporated.org>
 *
 * Authors: Dennis Smit <ds@nerds-incorporated.org>
 *
 * $Id: transform_avs_movement.c,v 1.6 2006-09-19 19:05:47 synap Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* FIXME TODO:
 *
 * config UI.
 * fix for other depths than 32bits
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <math.h>

#include <libvisual/libvisual.h>

#include "avs_common.h"
#include "avs.h"
#include "avs_inlines.h"

AvsNumber PI = M_PI;

#define REFFECT_MIN 3
#define REFFECT_MAX 23

#define MAKE_REFFECT(n,x) void _ref##n(double *r, double *d, double max_d, int *xo, int *yo) { x }

void test(double *r);

typedef void t_reffect(double *r, double *d, double max_d, int *xo, int *yo);

MAKE_REFFECT(3, *r+=0.1-0.2*(*d/max_d); *d*=0.96;)
MAKE_REFFECT(4, *d*=0.99*(1.0-sin(*r)/32.0); *r+=0.03*sin(*d/max_d * M_PI * 4);)
MAKE_REFFECT(5, *d*=0.94+(cos(*r*32.0)*0.06);)
MAKE_REFFECT(6, *d*=1.01+(cos(*r*4.0)*0.04); *r+=0.03*sin(*d/max_d * M_PI * 4);)
MAKE_REFFECT(8, *r+=0.1*sin(*d/max_d*M_PI*5);)
MAKE_REFFECT(9, double t; t=sin(*d/max_d*M_PI); *d-=8*t*t*t*t*t; )
MAKE_REFFECT(10,double t; t=sin(*d/max_d*M_PI); *d-=8*t*t*t*t*t; t=cos((*d/max_d)*M_PI/2.0); *r+=0.1*t*t*t; )
MAKE_REFFECT(11, *d*=0.95+(cos(*r*5.0 - M_PI/2.50)*0.03); )
MAKE_REFFECT(12, *r+=0.04; *d*=0.96+cos(*d/max_d*M_PI)*0.05; )
MAKE_REFFECT(13, double t; t=cos(*d/max_d*M_PI); *r+=0.07*t; *d*=0.98+t*0.10; )
MAKE_REFFECT(14, *r+=0.1-0.2*(*d/max_d); *d*=0.96; *xo=8; )
MAKE_REFFECT(15, *d=max_d*0.15;)
MAKE_REFFECT(16, *r=cos(*r*3);)
MAKE_REFFECT(17, *d*=(1-(((*d/max_d)-.35)*.5)); *r+=.1;)

static t_reffect *radial_effects[REFFECT_MAX-REFFECT_MIN+1]=
{
  _ref3, _ref4, _ref5, _ref6, NULL, _ref8, _ref9, _ref10, _ref11, _ref12, _ref13,
  _ref14, _ref15, _ref16, _ref17, NULL/*18*/, NULL/*19*/, NULL/*20*/, NULL/*21*/,
  NULL/*22*/, NULL/*23*/,
};

typedef struct {
    AVSGlobalProxy *proxy;

    uint8_t *swapbuf, *renderbuf;

    uint32_t    *tab;
    uint32_t    width, height;
    uint32_t    subpixel;

    int tab_w, tab_h, tab_subpixel;
    int trans_effect;
    int effect_exp_ch;
    int effect;
    int rectangular;
    int blend;
    int sourcemapped;
    int wrap;

    char *code;

    int lastWidth;
    int lastHeight;
    int lastPitch;
} MovementPrivate;

typedef struct {
    char *list_desc; // The string to show in the listbox.
    char *eval_desc; // The optional string to display in the evaluation editor.
    char uses_eval;   // If this is true, the preset engages the eval library and there is NULL in the radial_effects array for its entry
    char uses_rect;   // This value sets the checkbox for rectangular calculation
} MovementTemplateDescription;

static MovementTemplateDescription __movement_descriptions[] =
{
    {/* 0,*/ "none", "", 0, 0},
    {/* 1,*/ "slight fuzzify", "", 0, 0},
    {/* 2,*/ "shift rotate left", "x=x+1/32; // use wrap for this one", 0, 1},
    {/* 3,*/ "big swirl out", "r = r + (0.1 - (0.2 * d));\r\nd = d * 0.96;", 0, 0},
    {/* 4,*/ "medium swirl", "d = d * (0.99 * (1.0 - sin(r-$PI*0.5) / 32.0));\r\nr = r + (0.03 * sin(d * $PI * 4));", 0, 0},
    {/* 5,*/ "sunburster", "d = d * (0.94 + (cos((r-$PI*0.5) * 32.0) * 0.06));", 0, 0},
    {/* 6,*/ "swirl to center", "d = d * (1.01 + (cos((r-$PI*0.5) * 4) * 0.04));\r\nr = r + (0.03 * sin(d * $PI * 4));", 0, 0},
    {/* 7,*/ "blocky partial out", "", 0, 0},
    {/* 8,*/ "swirling around both ways at once", "r = r + (0.1 * sin(d * $PI * 5));", 0, 0},
    {/* 9,*/ "bubbling outward", "t = sin(d * $PI);\r\nd = d - (8*t*t*t*t*t)/sqrt((sw*sw+sh*sh)/4);", 0, 0},
    {/*10,*/ "bubbling outward with swirl", "t = sin(d * $PI);\r\nd = d - (8*t*t*t*t*t)/sqrt((sw*sw+sh*sh)/4);\r\nt=cos(d*$PI/2.0);\r\nr= r + 0.1*t*t*t;", 0, 0},
    {/*11,*/ "5 pointed distro", "d = d * (0.95 + (cos(((r-$PI*0.5) * 5.0) - ($PI / 2.50)) * 0.03));", 0, 0},
    {/*12,*/ "tunneling", "r = r + 0.04;\r\nd = d * (0.96 + cos(d * $PI) * 0.05);", 0, 0},
    {/*13,*/ "bleedin'", "t = cos(d * $PI);\r\nr = r + (0.07 * t);\r\nd = d * (0.98 + t * 0.10);", 0, 0},
    {/*14,*/ "shifted big swirl out", "// this is a very bad approximation in script. fixme.\r\nd=sqrt(x*x+y*y); r=atan2(y,x);\r\nr=r+0.1-0.2*d; d=d*0.96;\r\nx=cos(r)*d + 8/128; y=sin(r)*d;", 0, 1},
    {/*15,*/ "psychotic beaming outward", "d = 0.15", 0, 0},
    {/*16,*/ "cosine radial 3-way", "r = cos(r * 3)", 0, 0},
    {/*17,*/ "spinny tube", "d = d * (1 - ((d - .35) * .5));\r\nr = r + .1;", 0, 0},
    {/*18,*/ "radial swirlies", "d = d * (1 - (sin((r-$PI*0.5) * 7) * .03));\r\nr = r + (cos(d * 12) * .03);", 1, 0},
    {/*19,*/ "swill", "d = d * (1 - (sin((r - $PI*0.5) * 12) * .05));\r\nr = r + (cos(d * 18) * .05);\r\nd = d * (1-((d - .4) * .03));\r\nr = r + ((d - .4) * .13)", 1, 0},
    {/*20,*/ "gridley", "x = x + (cos(y * 18) * .02);\r\ny = y + (sin(x * 14) * .03);", 1, 1},
    {/*21,*/ "grapevine", "x = x + (cos(abs(y-.5) * 8) * .02);\r\ny = y + (sin(abs(x-.5) * 8) * .05);\r\nx = x * .95;\r\ny = y * .95;", 1, 1},
    {/*22,*/ "quadrant", "y = y * ( 1 + (sin(r + $PI/2) * .3) );\r\nx = x * ( 1 + (cos(r + $PI/2) * .3) );\r\nx = x * .995;\r\ny = y * .995;", 1, 1},
    {/*23,*/ "6-way kaleida (use wrap!)", "y = (r*6)/($PI); x = d;", 1, 1},
};

static inline int effect_uses_eval(int t)
{
  int retval = FALSE;
  if ((t >= REFFECT_MIN) && (t <= REFFECT_MAX))
  {
    if (__movement_descriptions[t].uses_eval)
    {
      retval = TRUE;
    }
  }
  return retval;
}

int lv_movement_init (VisPluginData *plugin);
int lv_movement_cleanup (VisPluginData *plugin);
int lv_movement_events (VisPluginData *plugin, VisEventQueue *events);
int lv_movement_palette (VisPluginData *plugin, VisPalette *pal, VisAudio *audio);
int lv_movement_video (VisPluginData *plugin, VisVideo *video, VisAudio *audio);

static void trans_generate_table(MovementPrivate *priv, char *effect, int rectangular, int wrap, int isBeat, uint32_t *fbin, uint32_t *fbout);
static void trans_generate_blend_table(MovementPrivate *priv);
static void trans_begin(MovementPrivate *priv, int width, int height, char *effect, int isBeat);
static void trans_render(MovementPrivate *priv, uint32_t *fbin, uint32_t *fbout);

VISUAL_PLUGIN_API_VERSION_VALIDATOR

const VisPluginInfo *get_plugin_info (int *count);

const VisPluginInfo *get_plugin_info (int *count)
{
    static const VisTransformPlugin transform[] = {{
        .palette = lv_movement_palette,
        .video = lv_movement_video,
        .vidoptions.depth =
            VISUAL_VIDEO_DEPTH_32BIT,
        .requests_audio = FALSE
    }};

    static const VisPluginInfo info[] = {{
        .type = VISUAL_PLUGIN_TYPE_TRANSFORM,//".[avs]",

        .plugname = "avs_movement",
        .name = "Libvisual AVS Transform: movement element",
        .author = "Dennis Smit <ds@nerds-incorporated.org>",
        .version = "0.1",
        .about = "The Libvisual AVS Transform: movement element",
        .help = "This is the movement element for the libvisual AVS system",

        .init = lv_movement_init,
        .cleanup = lv_movement_cleanup,
        .events = lv_movement_events,

        .plugin = VISUAL_OBJECT (&transform[0])
    }};

    *count = sizeof (info) / sizeof (*info);

    return info;
}

int lv_movement_init (VisPluginData *plugin)
{
    MovementPrivate *priv;
    VisParamContainer *paramcontainer = visual_plugin_get_params (plugin);
    int i;

    static VisParamEntryProxy params[] = {
        VISUAL_PARAM_LIST_ENTRY_INTEGER ("effect", 32767, VISUAL_PARAM_LIMIT_INTEGER(0, 32767), ""),
        VISUAL_PARAM_LIST_ENTRY_INTEGER ("rectangular", 1, VISUAL_PARAM_LIMIT_BOOLEAN, ""),
        VISUAL_PARAM_LIST_ENTRY_INTEGER ("blend", 1, VISUAL_PARAM_LIMIT_BOOLEAN, ""),
        VISUAL_PARAM_LIST_ENTRY_INTEGER ("sourcemapped", 0, VISUAL_PARAM_LIMIT_BOOLEAN, ""),
        VISUAL_PARAM_LIST_ENTRY_INTEGER ("subpixel", 1, VISUAL_PARAM_LIMIT_INTEGER(0, 1000), ""),
        VISUAL_PARAM_LIST_ENTRY_INTEGER ("wrap", 0, VISUAL_PARAM_LIMIT_BOOLEAN, ""),
        VISUAL_PARAM_LIST_ENTRY_STRING ("code",
                "d = d * (1.01 + (cos((r-$PI*0.5) * 4) * 0.04)); r = r + (0.03 * sin(d * $PI * 4));"
                , ""),
        VISUAL_PARAM_LIST_END
    };

    priv = visual_mem_new0 (MovementPrivate, 1);

    priv->proxy = AVS_GLOBAL_PROXY(visual_object_get_private(VISUAL_OBJECT (plugin)));
    visual_object_ref(VISUAL_OBJECT(priv->proxy));

    visual_object_set_private (VISUAL_OBJECT (plugin), priv);

    visual_param_container_add_many_proxy (paramcontainer, params);

    return 0;
}

int lv_movement_cleanup (VisPluginData *plugin)
{
    MovementPrivate *priv = visual_object_get_private (VISUAL_OBJECT (plugin));

    if (priv->swapbuf != NULL)
        visual_mem_free (priv->swapbuf);

    if(priv->code != NULL)
        visual_mem_free (priv->code);

    visual_object_unref(VISUAL_OBJECT(priv->proxy));

    visual_mem_free (priv);

    return 0;
}

int lv_movement_events (VisPluginData *plugin, VisEventQueue *events)
{
    MovementPrivate *priv = visual_object_get_private (VISUAL_OBJECT (plugin));
    VisParamEntry *param;
    VisEvent ev;

    while (visual_event_queue_poll (events, &ev)) {
        switch (ev.type) {
            case VISUAL_EVENT_PARAM:
                param = ev.event.param.param;

                if (visual_param_entry_is (param, "effect")) {
                    priv->effect = visual_param_entry_get_integer (param);

                    if (priv->effect != 32767) {
                        if (priv->effect >= 0 && priv->effect < 23) {
                            visual_param_entry_set_string (
                                    visual_param_container_get (param->parent, "code"),
                                    __movement_descriptions[priv->effect].eval_desc);
                        }
                    }

                } else if (visual_param_entry_is (param, "rectangular"))
                    priv->rectangular = visual_param_entry_get_integer (param);
                else if (visual_param_entry_is (param, "blend"))
                    priv->blend = visual_param_entry_get_integer (param);
                else if (visual_param_entry_is (param, "sourcemapped"))
                    priv->sourcemapped = visual_param_entry_get_integer (param);
                else if (visual_param_entry_is (param, "subpixel"))
                    priv->subpixel = visual_param_entry_get_integer (param);
                else if (visual_param_entry_is (param, "wrap"))
                    priv->wrap = visual_param_entry_get_integer (param);
                else if (visual_param_entry_is (param, "code")) 
                    priv->code = visual_param_entry_get_string (param);
                break;

            default:
                break;
        }
    }

    return 0;
}

int lv_movement_palette (VisPluginData *plugin, VisPalette *pal, VisAudio *audio)
{
    return 0;
}

int lv_movement_video (VisPluginData *plugin, VisVideo *video, VisAudio *audio)
{
    MovementPrivate *priv = visual_object_get_private (VISUAL_OBJECT (plugin));
    uint8_t *pixels = visual_video_get_pixels (video);
    uint8_t *vidbuf, *vidoutbuf;
    int isBeat = visual_audio_is_beat(audio);
    int i;

    printf("lv_movement_video %p\n", video);

    if (priv->lastWidth != video->width || priv->lastHeight != video->height || priv->lastPitch != video->pitch) {


        trans_begin(priv, video->width, video->height, priv->code, isBeat);

        if (priv->swapbuf != NULL)
            visual_mem_free (priv->swapbuf);

        /* FIXME: would allocate way too much on subregion buffers, think about this. */
        priv->swapbuf = visual_mem_malloc0 (visual_video_get_size (video));
        priv->renderbuf = visual_mem_malloc0(video->width * video->height * video->bpp);
    }

    vidbuf = priv->swapbuf;
    for (i = 0; i < video->height; i++) {
        visual_mem_copy (vidbuf, pixels + (video->pitch * i), video->width * video->bpp);
        vidbuf += video->width * video->bpp;
    }


    trans_generate_table(priv, priv->code, 0, 0, isBeat, (uint32_t *) priv->swapbuf, (uint32_t *) priv->renderbuf);

    visual_mem_set(priv->renderbuf, 0, video->width * video->height * video->bpp);
    trans_render(priv, (uint32_t *) priv->swapbuf,  (uint32_t *) priv->renderbuf);

    visual_mem_copy(pixels, priv->renderbuf, video->width * video->height * video->bpp);

    priv->lastWidth = video->width;
    priv->lastHeight = video->height;
    priv->lastPitch = video->pitch;

    return VISUAL_OK;
}

static void trans_generate_table(MovementPrivate *priv, char *effect, int rectangular, int wrap, int isBeat, uint32_t *fbin, uint32_t *fbout)
{
    uint32_t *transp = priv->tab;

    int x, p;
    //double max_d = sqrt((double)(priv->width*priv->width + priv->height*priv->height)) / 2.0;
    //double divmax_d = 1.0/max_d;

    if (priv->tab_w != priv->width || priv->tab_h != priv->height || priv->trans_effect != priv->effect || priv->effect_exp_ch)
    {

        priv->tab_w = priv->width;
        priv->tab_h = priv->height;
        priv->trans_effect=priv->effect;
        priv->tab_subpixel=(priv->subpixel && priv->tab_w*priv->tab_h < (1<<22) &&
                  ((priv->trans_effect >= REFFECT_MIN && priv->trans_effect <= REFFECT_MAX
                  && priv->trans_effect != 1 && priv->trans_effect != 2 && priv->trans_effect != 7
                  )||priv->trans_effect ==32767));


        printf("generate_table trans_effect %d\n", priv->trans_effect);

        x = priv->width * priv->height;
        p = 0;

        if(priv->trans_effect == 1)
        {
            while(x--)
            {
                int r = (p++)+(rand()%3)-1 + ((rand()%3)-1)*priv->width;
                *transp++ = min(priv->width*priv->height-1, max(r, 0));
            }
        }
        else if (priv->trans_effect == 2)
        {
            int y = priv->height;
            while(y--)
            {
                int x = priv->width;
                int lp = priv->width/64;
                while(x--)
                {
                    *transp++ = p+lp++;
                    if(lp >= priv->width) lp-=priv->width;
                }
                p+=priv->width;
            }
        }
        else if (priv->trans_effect == 7)
        {
            int y;
            for(y = 0; y < priv->height; y++)
            {
                for(x = 0; x < priv->width; x++)
                {
                    if(x&2 || y&2)
                    {
                        *transp++ = x+y*priv->width;
                    }
                    else
                    {
                        int xp = priv->width/2+(((x&~1)-priv->width/2)*7)/8;
                        int yp = priv->height/2+(((y&~1)-priv->width/2)*7)/8;
                        *transp++=xp*yp*priv->width;
                    }
                }
            }
        }
        else if(priv->trans_effect >= REFFECT_MIN && priv->trans_effect <= REFFECT_MAX && !effect_uses_eval(priv->trans_effect))
        {
            printf("trans_effect generate_table level 1 %d\n", priv->trans_effect-REFFECT_MIN);
            double max_d = sqrt((priv->width*priv->width+priv->height*priv->height)/4.0);
            int y;
            t_reffect *ref = radial_effects[priv->trans_effect-REFFECT_MIN];
            if(ref) for (y = 0; y < priv->height; y++)
            {
                for(x = 0; x < priv->width; x++)
                {
                    double r, d;
                    double xd, yd;
                    int ow, oh, xo=0, yo=0;
                    xd = x-(priv->width/2);
                    yd = y-(priv->height/2);
                    d = sqrt(xd*xd + yd*yd);
                    r = atan2(yd, xd);
    
                    ref(&r, &d, max_d, &xo, &yo);
    
                    double tmp1, tmp2;
                    tmp1 = ((priv->height/2) + sin(r)*d + 0.5) + (yo*priv->height)*(1.0/256.0);
                    tmp2 = ((priv->width/2) + cos(r)*d + 0.5) + (xo*priv->width)*(1.0/256.0);
                    oh = (int)tmp1;
                    ow = (int)tmp2;
                    if(priv->subpixel)
                    {
                        int xpartial=(int)(32.0*(tmp2-ow));
                        int ypartial=(int)(32.0*(tmp1-oh));
                        if(priv->wrap)
                        {
                            ow%=(priv->width - 1);
                            oh%=(priv->height - 1);
                            if(ow<0)ow+=priv->width - 1;
                            if(oh<0)oh+=priv->height - 1;
                        }
                        else
                        {
                            if(ow < 0) { xpartial=0; ow=0; }
                            if(ow > priv->width-1) { xpartial=31; ow=priv->width-2; }
                            if(oh < 0) { xpartial=0; oh=0; }
                            if(oh > priv->height - 1) { xpartial=31; oh=priv->height-2; }
                        }
                        *transp++ = (ow+oh*priv->width) | (ypartial<<22) | (xpartial<<27);
                    }
                    else
                    {
                        if(priv->wrap)
                        {
                            ow%=(priv->width);
                            oh%=(priv->height);
                            if(ow < 0)ow+=priv->width;
                            if(oh < 0)oh+=priv->height;
                        }
                        else
                        {
                            if(ow < 0) ow = 0;
                            if(ow >= priv->width) ow=priv->width-1;
                            if(oh < 0) oh = 0;
                            if(oh >= priv->height) oh=priv->height-1;
                        }
                        *transp++ = ow+oh*priv->width;
                    }
                }
            }
        }
        else if(priv->trans_effect == 32767 || effect_uses_eval(priv->trans_effect))
        {
            printf("trans effect level 2\n");
            double max_d = sqrt((double)(priv->width*priv->width+priv->height*priv->height))/2.0;
            double divmax_d=1.0/max_d;
            int y;
            int offs;
            int is_rect = priv->trans_effect == 32767 ? priv->rectangular : __movement_descriptions[priv->trans_effect].uses_rect;
    
            {
                AvsNumber d, r, px, py, pw, ph;
        
                pw = priv->width;
                ph = priv->height;
        
                AvsRunnableContext *ctx = avs_runnable_context_new();
                AvsRunnableVariableManager *vm = avs_runnable_variable_manager_new();
                AvsRunnable *obj = avs_runnable_new(ctx);
                avs_runnable_set_variable_manager(obj, vm);
            
                avs_runnable_variable_bind(vm, "d", &d);
                avs_runnable_variable_bind(vm, "r", &r);
                avs_runnable_variable_bind(vm, "x", &px);
            
                avs_runnable_variable_bind(vm, "y", &py);
                avs_runnable_variable_bind(vm, "sw", &pw);
                avs_runnable_variable_bind(vm, "sh", &ph);
                avs_runnable_variable_bind(vm, "$PI", &PI);
            
                avs_runnable_compile(obj, (unsigned char *)effect, strlen(effect));
    
                {
                    double w2=priv->width/2;
                    double h2=priv->height/2;
                    double xsc = 1.0/w2,ysc=1.0/h2;
    
                    for(y = 0; y < priv->height; y++)
                    {
                        for(x = 0; x < priv->width; x++)
                        {
                            double xd, yd;
                            int ow, oh;
                            xd = x-w2;
                            yd = y-h2;
                            px=xd*xsc;
                            py=yd*ysc;
                            d=sqrt(xd*xd+yd*yd)*divmax_d;
                            r=atan2(yd, xd) + PI*0.5;
    
                            avs_runnable_execute(obj);
    
                            double tmp1, tmp2;
                            if(!is_rect)
                            {
                                d *= max_d;
                                r -= PI/2.0;
                                tmp1=((priv->height) + sin(r) * d);
                                tmp2=((priv->height) + cos(r) * d);
                            }
                            else
                            {
                                tmp1=((py+1.0)*h2);
                                tmp2=((px+1.0)*w2);
                            }
                            if(priv->subpixel)
                            {
                                oh=(int) tmp1;
                                ow=(int) tmp2;
                                int xpartial=(int)(32.0*(tmp2-ow));
                                int ypartial=(int)(32.0*(tmp1-oh));
                                if(priv->wrap)
                                {
                                    ow%=(priv->width-1);
                                    oh%=(priv->height-1);
                                    if(ow < 0) ow+=priv->width - 1;
                                    if(oh < 0) oh+=priv->height - 1;
                                }
                                else
                                {
                                    if(ow < 0) { xpartial=0; ow=0; }
                                    if(ow >= priv->width-1) { xpartial=31; ow=priv->width-2; }
                                    if(oh < 0) { ypartial=0; oh=0; }
                                    if(oh >= priv->height-1) { ypartial=31; oh=priv->height-2; }
                                }
                                *transp++ = (ow+oh*priv->width) | (ypartial<<22) | (xpartial<<27);
                            }
                            else
                            {
                                tmp1+=0.5;
                                tmp2+=0.5;
                                oh=(int) tmp1;
                                ow=(int) tmp2;
                                if(priv->wrap)
                                {
                                    ow%=(priv->width);
                                    oh%=(priv->height);
                                    if( ow < 0)ow+=priv->width;;
                                    if( oh < 0)oh+=priv->height;
                                }
                                else
                                {
                                    if(ow < 0) ow=0;
                                    if(ow >= priv->width) ow=priv->width-1;
                                    if(oh < 0) oh=0;
                                    if(oh >= priv->height) oh=priv->height-1;
                                }
                                *transp++ = ow+oh*priv->width;
                            }
                        }
                    }
                }
                priv->effect_exp_ch=0;
            }
            if(!(isBeat & 0x80000000))
            {
                printf("copying fbin->fbout\n");
                if((priv->sourcemapped&2)&&isBeat) priv->sourcemapped^=1;
                if(priv->sourcemapped&1)
                {
                    if(!priv->blend) memset(fbout, 0, priv->width*priv->height*sizeof(int));
                    else visual_mem_copy(fbout, fbin, priv->width*priv->height*sizeof(int));
                }
            }
        }
    }
}

static void trans_generate_blend_table(MovementPrivate *priv)
{
    int i,j;

    for (j=0; j < 256; j++)
        for (i=0; i < 256; i++)
            priv->proxy->blendtable[i][j] = (unsigned char)((i / 255.0) * (float)j);

}

static void trans_begin(MovementPrivate *priv, int width, int height, char *effect, int isBeat)
{

    if (priv->tab != NULL)
        visual_mem_free(priv->tab);
    
    priv->width = width;
    priv->height = height;
    priv->tab = malloc(width * height * sizeof(int));

    priv->subpixel = (priv->subpixel && width*height < (1<<22) &&
        ((priv->effect >= REFFECT_MIN && priv->effect <= REFFECT_MAX
        && priv->effect != 1 && priv->effect != 2 && priv->effect != 7
        )||priv->trans_effect == 32767));
    
    trans_generate_blend_table(priv);

    if (1 /* !isBeat & 0x80000000 */) {
        /* ... */
    }
}

#define OFFSET_MASK ((1<<22)-1)

static void trans_render(MovementPrivate *priv, uint32_t *fbin, uint32_t *fbout)
{
    uint32_t *inp = fbin;
    uint32_t *outp = fbout;
    unsigned int *transp = priv->tab, x;

    if(!priv->effect) return;

    int skip_pix = 0;

    x = (priv->width * priv->height ) / 4;
    if (priv->sourcemapped&1) {
        printf("trans render level 1\n");
        inp += skip_pix;
        transp += skip_pix;
        if (priv->subpixel) {
            printf("trans render level 1.1\n");
            int i;
            while (x--) {
                fbout[transp[0]&OFFSET_MASK] = BLEND_MAX(inp[0],fbout[transp[0]&OFFSET_MASK]);
                fbout[transp[1]&OFFSET_MASK] = BLEND_MAX(inp[1],fbout[transp[1]&OFFSET_MASK]);
                fbout[transp[2]&OFFSET_MASK] = BLEND_MAX(inp[2],fbout[transp[2]&OFFSET_MASK]);
                fbout[transp[3]&OFFSET_MASK] = BLEND_MAX(inp[3],fbout[transp[3]&OFFSET_MASK]);
                inp+=4;
                transp+=4;
            }

            x = (priv->width * priv->height) & 3;
            if (x>0) while (x--) {
                fbout[transp[0]&OFFSET_MASK] = BLEND_MAX(inp++[0], fbout[transp[0]&OFFSET_MASK]);
                transp++;
            }
        } else { 
            printf("trans render level 1.2\n");
            while(x--)
            {
                fbout[transp[0]]=BLEND_MAX(inp[0], fbout[transp[0]]);
                fbout[transp[1]]=BLEND_MAX(inp[1], fbout[transp[1]]);
                fbout[transp[2]]=BLEND_MAX(inp[2], fbout[transp[2]]);
                fbout[transp[3]]=BLEND_MAX(inp[3], fbout[transp[3]]);
                inp+=4;
                transp+=4;
            }
            x = (priv->width * priv->height)&3;
            if (x > 0) while (x--)
            {
                fbout[transp[0]]=BLEND_MAX(inp++[0], fbout[transp[0]]);
                transp++;
            }
        }
        if (priv->blend) {
            printf("trans render level 1.3\n");
            fbin += skip_pix;
            fbout += skip_pix;
            x=(priv->width * priv->height)/4;
            while (x--) {
                fbout[0] = BLEND_AVG(fbout[0], fbin[0]);
                fbout[1] = BLEND_AVG(fbout[1], fbin[1]);
                fbout[2] = BLEND_AVG(fbout[2], fbin[2]);
                fbout[3] = BLEND_AVG(fbout[3], fbin[3]);
                fbout+=4;
                fbin+=4;
            }
            x = (priv->width * priv->height)&3;
            while(x--)
            {
                fbout[0]=BLEND_AVG(fbout[0], fbin[0]);
                fbout++;
                fbin++;
            }
        }
    } else {
        printf("trans render level 1.4\n");
        inp += skip_pix;
        outp += skip_pix;
        transp += skip_pix;
        if(priv->subpixel && priv->blend)
        {
            while(x--)
            {
                int offs=transp[0]&OFFSET_MASK;
                outp[0]=BLEND_AVG(inp[0],BLEND4(priv->proxy, (unsigned int *)fbin+offs, priv->width, ((transp[0]>>24)&(31<<3)),((transp[0]>>19)&(31<<3))));
                outp[1]=BLEND_AVG(inp[1],BLEND4(priv->proxy, (unsigned int *)fbin+offs, priv->width, ((transp[1]>>24)&(31<<3)),((transp[0]>>19)&(31<<3))));
                outp[2]=BLEND_AVG(inp[2],BLEND4(priv->proxy, (unsigned int *)fbin+offs, priv->width, ((transp[2]>>24)&(31<<3)),((transp[0]>>19)&(31<<3))));
                outp[3]=BLEND_AVG(inp[3],BLEND4(priv->proxy, (unsigned int *)fbin+offs, priv->width, ((transp[3]>>24)&(31<<3)),((transp[0]>>19)&(31<<3))));
                transp+=4;
                outp+=4;
                inp+=4;
            }
            x = (priv->width * priv->height)&3;
            while (x--)
            {
                int offs=transp[0]&OFFSET_MASK;
                outp++[0]=BLEND_AVG(inp[0], BLEND4(priv->proxy, (unsigned int *)fbin+offs,priv->width, ((transp[0]>>24)&(31<<3)), ((transp[0]>>19)&(31<<3))));
                transp++;
                inp++;
            }
        } else if (priv->subpixel) {
            printf("trans render level 1.5\n");
            while(x--)
            {
                int offs=transp[0]&OFFSET_MASK;
                outp[0]=BLEND4(priv->proxy, (unsigned int *)fbin+offs, priv->width, ((transp[0]>>24)&(31<<3)),((transp[0]>>19)&(31<<3)));
                outp[1]=BLEND4(priv->proxy, (unsigned int *)fbin+offs, priv->width, ((transp[0]>>24)&(31<<3)),((transp[0]>>19)&(31<<3)));
                outp[2]=BLEND4(priv->proxy, (unsigned int *)fbin+offs, priv->width, ((transp[0]>>24)&(31<<3)),((transp[0]>>19)&(31<<3)));
                outp[3]=BLEND4(priv->proxy, (unsigned int *)fbin+offs, priv->width, ((transp[0]>>24)&(31<<3)),((transp[0]>>19)&(31<<3)));
                transp+=4;
                outp+=4;
            }
            x = (priv->width * priv->height) & 3;
            while(x--)
            {
                int offs=transp[0]&OFFSET_MASK;
                outp++[0]=BLEND4(priv->proxy, (unsigned int *)fbin+offs, priv->width, ((transp[0]>>24)&(31<<3)),((transp[0]>>19)&(31<<3)));
                transp++;
            }

        } else if (priv->blend) {
        printf("trans render level 1.6\n");
            while(x--)
            {
                outp[0]=BLEND_AVG(inp[0],fbin[transp[0]]);
                outp[1]=BLEND_AVG(inp[1],fbin[transp[1]]);
                outp[2]=BLEND_AVG(inp[2],fbin[transp[2]]);
                outp[3]=BLEND_AVG(inp[3],fbin[transp[3]]);
                outp+=4;
                inp+=4;
                transp+=4;
            }
            x = (priv->width * priv->height)&3;
            if(x > 0) while(x--)
            {
                outp++[0]=BLEND_AVG(inp++[0], fbin[transp++[0]]);
            }
        } else {
        printf("trans render level 1.7\n");
            while(x--)
            {
                outp[0]=fbin[transp[0]];
                outp[1]=fbin[transp[1]];
                outp[2]=fbin[transp[2]];
                outp[3]=fbin[transp[3]];
                outp+=4;
                transp+=4;
            }
            x = (priv->width * priv->height)&3;
            if(x > 0) while(x--)
            {
                outp++[0]=fbin[transp++[0]];
            }
        }
    }
}




