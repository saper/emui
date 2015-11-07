//  Copyright (c) 2015 Jakub Filipowicz <jakubf@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "emui.h"
#include "tile.h"
#include "event.h"
#include "style.h"
#include "print.h"

struct fpscounter {
	struct timeval t;
	double fps;
	int modulo;
};

// -----------------------------------------------------------------------
void emui_fpscounter_draw(struct emui_tile *t)
{
	struct fpscounter *d = t->priv_data;

	if (emui_get_frame() % d->modulo == 0) {
		time_t o_sec = d->t.tv_sec;
		suseconds_t o_usec = d->t.tv_usec;

		gettimeofday(&(d->t), NULL);
		double frame_time = (d->t.tv_sec - o_sec) * 1000000.0 + (d->t.tv_usec - o_usec);
		d->fps = d->modulo * 1000000.0 / frame_time;
	}

	emuixyprt(t, 0, 0, t->style, "%2.3f", d->fps);
}

// -----------------------------------------------------------------------
void emui_fpscounter_destroy_priv_data(struct emui_tile *t)
{
	free(t->priv_data);
}

// -----------------------------------------------------------------------
struct emui_tile_drv emui_fpscounter_drv = {
	.draw = emui_fpscounter_draw,
	.update_children_geometry = NULL,
	.event_handler = NULL,
	.destroy_priv_data = emui_fpscounter_destroy_priv_data,
};

// -----------------------------------------------------------------------
struct emui_tile * emui_fpscounter(struct emui_tile *parent, int x, int y, int style)
{
	struct emui_tile *t;

	t = emui_tile_create(parent, -1, &emui_fpscounter_drv, F_WIDGET, x, y, 6, 1, 0, 0, 0, 0, NULL, P_NONE);

	t->style = style;
	t->priv_data = calloc(1, sizeof(struct fpscounter));

	struct fpscounter *d = t->priv_data;

	unsigned fps = emui_get_fps();
	d->modulo = fps / 4;
	if (d->modulo <= 0) {
		d->modulo = 1;
	}

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
