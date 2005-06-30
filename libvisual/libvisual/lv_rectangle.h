/* Libvisual - The audio visualisation framework.
 * 
 * Copyright (C) 2004, 2005 Dennis Smit <ds@nerds-incorporated.org>
 *
 * Authors: Dennis Smit <ds@nerds-incorporated.org>
 *
 * $Id:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef _LV_RECTANGLE_H
#define _LV_RECTANGLE_H

#include <libvisual/lv_common.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define VISUAL_RECTANGLE(obj)				(VISUAL_CHECK_CAST ((obj, VisRectangle))

typedef struct _VisRectangle VisRectangle;

/**
 * Using the VisRectangle structure you can define rectangular areas in for example
 * VisVideo buffers.
 */
struct _VisRectangle {
	VisObject	object;		/**< The VisObject data. */
	int		x;		/**< X Position of the upper left corner. */
	int		y;		/**< Y Position of the upper left corner. */
	int		width;		/**< The width. */
	int		height;		/**< The height. */
};

/* prototypes */
VisRectangle *visual_rectangle_new (int x, int y, int width, int height);
int visual_rectangle_set (VisRectangle *rect, int x, int y, int width, int height);
int visual_rectangle_position_within (VisRectangle *rect, int x, int y);
int visual_rectangle_within_partially (VisRectangle *dest, VisRectangle *src);
int visual_rectangle_within (VisRectangle *dest, VisRectangle *src);
int visual_rectangle_copy (VisRectangle *dest, VisRectangle *src);
int visual_rectangle_merge (VisRectangle *dest, VisRectangle *within, VisRectangle *src);
int visual_rectangle_normalise (VisRectangle *rect);
int visual_rectangle_normalise_to (VisRectangle *dest, VisRectangle *src);
int visual_rectangle_is_empty (VisRectangle *rect);

int visual_rectangle_denormalise_values (VisRectangle *rect, float fx, float fy, int *x, int *y);
int visual_rectangle_denormalise_many_values (VisRectangle *rect, float *fxlist, float *fylist, int *xlist, int *ylist, int size);
int visual_rectangle_denormalise_values_neg (VisRectangle *rect, float fx, float fy, int *x, int *y);
int visual_rectangle_denormalise_many_values_neg (VisRectangle *rect, float *fxlist, float *fylist, int *xlist, int *ylist, int size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LV_RECTANGLE_H */