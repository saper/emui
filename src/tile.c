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
#include <ncurses.h>

#include "tile.h"
#include "style.h"
#include "focus.h"
#include "print.h"

static int emui_tile_debug_mode = 0;

// -----------------------------------------------------------------------
void emui_tile_update_geometry(struct emui_tile *t)
{
	struct emui_tile *parent = t->parent;

	if (!parent) {
		// nothing to do for the root tile (screen)
		return;
	}

	// set decoration window geometry
	if (t->properties & P_GEOM_FORCED) {
		if (t->properties & P_HIDDEN) {
			// just leave, don't even try to unhide it
			return;
		} else {
			// nothing to do here
			// dx, dy, dw, dh should be set by the parent already
		}
	} else if (t->properties & P_MAXIMIZED) {
		// tile is maximized - use all available parent's area
		t->dx = parent->x;
		t->dy = parent->y;
		t->dw = parent->w;
		t->dh = parent->h;
	} else {
		t->dx = parent->x + t->rx;
		t->dy = parent->y + t->ry;
		t->dw = t->rw;
		t->dh = t->rh;
	}

	if (t->properties & P_NODECO) {
		t->mt = 0;
		t->mb = 0;
		t->ml = 0;
		t->mr = 0;
	} else {
		t->mt = t->rmt;
		t->mb = t->rmb;
		t->ml = t->rml;
		t->mr = t->rmr;
	}

	// hide tile if outside parent's area
	if ((t->dx >= parent->x + parent->w) || (t->dy >= parent->y + parent->h)) {
		emui_tile_hide(t);
		// give up on hidden tile
		return;
	// unhide and fit otherwise
	} else {
		emui_tile_unhide(t);
	}

	// fit tile width to parent's width
	if (t->dw + t->dx > parent->dw + parent->dx) {
		t->dw = parent->w - (t->dx - parent->x);
	}

	// fit tile to parent's height
	if (t->dh + t->dy > parent->dh + parent->dy) {
		t->dh = parent->h - (t->dy - parent->y);
	}

	// set contents window geometry
	t->x = t->dx + t->ml;
	t->y = t->dy + t->mt;
	t->w = t->dw - t->ml - t->mr;
	t->h = t->dh - t->mt - t->mb;

	// hide if no space for contents
	if ((t->dw <= t->ml+t->mr) || (t->dh <= t->mt+t->mb)) {
		emui_tile_hide(t);
		return;
	} else {
		emui_tile_unhide(t);
	}

	// prepare decoration window
	if (ACTIVE_DECO(t)) {
		if (!t->ncdeco) {
			t->ncdeco = newwin(t->dh, t->dw, t->dy, t->dx);
		} else {
			wresize(t->ncdeco, t->dh, t->dw);
			mvwin(t->ncdeco, t->dy, t->dx);
		}
	}

	// prepare contents widow
	if (!t->ncwin) {
		t->ncwin = newwin(t->h, t->w, t->y, t->x);
	} else {
		wresize(t->ncwin, t->h, t->w);
		mvwin(t->ncwin, t->y, t->x);
	}
}

// -----------------------------------------------------------------------
static void emui_tile_debug(struct emui_tile *t)
{
	int x, y;
	WINDOW *win;
	char buf[256];
	attr_t attr_old;
	short colorpair_old;

	if (t->family == F_WIDGET) {
		if (t->drv->debug) t->drv->debug(t);
		return;
	}

	// format debug string
	buf[255] = '\0';
	snprintf(buf, 256, "d:%i,%i/%ix%i %i,%i/%ix%i%s%s%s%s%s%s%s %s",
		t->dx,
		t->dy,
		t->dw,
		t->dh,
		t->x,
		t->y,
		t->w,
		t->h,
		emui_has_focus(t) ? (emui_is_focused(t) ? "*" : "+") : " ",
		t->properties & P_MAXIMIZED ? "M" : "",
		t->properties & P_HIDDEN ? "H"  : "",
		t->properties & P_GEOM_FORCED ? "F" : "",
		t->properties & P_FOCUS_GROUP ? "G" : "",
		t->properties & P_INTERACTIVE ? "I" : "",
		t->properties & P_DECORATED ? "D" : "",
		t->name
	);

	// find a possibly free space to print
	x = t->w - strlen(buf);
	if (x < 0) x = 0;
	// no decoration space
	if (!t->ncdeco) {
		y = t->h - 1;
		win = t->ncwin;
	// bottom decoration
	} else if (t->mb) {
		y = t->dh - 1;
		win = t->ncdeco;
	// top decoration
	} else if (t->mt) {
		y = 0;
		win = t->ncdeco;
	// only left or right decoration
	} else {
		y = t->h - 1;
		win = t->ncwin;
	}

	wattr_get(win, &attr_old, &colorpair_old, NULL);
	wattrset(win, emui_style_get(S_DEBUG));
	mvwprintw(win, y, x, "%s", buf);
	wattrset(win, colorpair_old | attr_old);

	if (t->drv->debug) t->drv->debug(t);
}

// -----------------------------------------------------------------------
int emui_tile_draw(struct emui_tile *t)
{
	// tile is hidden, nothing to do
	if (t->properties & P_HIDDEN) {
		return 1;
	}

	// clear ncurses windows
	if (t->ncdeco) {
		werase(t->ncdeco);
	}
	if (t->ncwin) {
		if (t->style) {
			emuifillbg(t, t->style);
		} else {
			// this may be needed in the end
			//werase(t->ncwin);
		}
	}

	// if tile accepts content updates and user specified a handler,
	// then update content before tile is drawn
	if (t->accept_updates && t->user_update_handler) {
		t->user_update_handler(t);
	}

	// draw the tile
	t->drv->draw(t);

	// print debug information
	if (emui_tile_debug_mode) {
		emui_tile_debug(t);
	}

	// update ncurses windows, but don't refresh
	// (we'll do the update in emui_loop())
	if (ACTIVE_DECO(t)) {
		wnoutrefresh(t->ncdeco);
	}
	wnoutrefresh(t->ncwin);

	return 0;
}

// -----------------------------------------------------------------------
static const int emui_tile_compatibile(struct emui_tile *parent, int child_family)
{
	static const int tile_compat[F_NUMFAMILIES][F_NUMFAMILIES] = {
	/*					child:								*/
	/* parent:			F_CONTAINER	F_WINDOW	F_WIDGET	*/
	/* F_CONTAINER */	{1,			1,			1 },
	/* F_WINDOW */		{1,			0,			1 },
	/* F_WIDGET */		{0,			0,			0 },
	};

	return tile_compat[parent->family][child_family];
}

// -----------------------------------------------------------------------
struct emui_tile * emui_tile_create(struct emui_tile *parent, int id, struct emui_tile_drv *drv, int family, int x, int y, int w, int h, int mt, int mb, int ml, int mr, char *name, int properties)
{
	if (!emui_tile_compatibile(parent, family)) {
		return NULL;
	}

	struct emui_tile *t = calloc(1, sizeof(struct emui_tile));
	if (!t) return NULL;

	if (name) {
		t->name = strdup(name);
		if (!t->name) {
			free(t);
			return NULL;
		}
	}

	t->family = family;
	t->properties = properties;
	t->drv = drv;
	t->geometry_changed = 1;
	t->id = id;
	t->accept_updates = 1;

	t->rx = x;
	t->ry = y;
	t->rh = h;
	t->rw = w;

	t->rmt = mt;
	t->rmb = mb;
	t->rml = ml;
	t->rmr = mr;

	if (mt || mb || ml || mr) {
		t->properties |= P_DECORATED;
	}

	emui_tile_child_append(parent, t);
	emui_tile_update_geometry(t);

	return t;
}

// -----------------------------------------------------------------------
int emui_tile_set_focus_key(struct emui_tile *t, int key)
{
	t->key = key;
	return 1;
}

// -----------------------------------------------------------------------
int emui_tile_set_update_handler(struct emui_tile *t, emui_handler_f handler)
{
	t->user_update_handler = handler;
	return 0;
}

// -----------------------------------------------------------------------
int emui_tile_set_change_handler(struct emui_tile *t, emui_handler_f handler)
{
	t->user_change_handler = handler;
	return 0;
}

// -----------------------------------------------------------------------
int emui_tile_set_key_handler(struct emui_tile *t, emui_key_handler_f handler)
{
	t->user_key_handler = handler;
	return 0;
}

// -----------------------------------------------------------------------
int emui_tile_set_properties(struct emui_tile *t, unsigned properties)
{
	if (properties & ~P_USER_SETTABLE) {
		return -1;
	}

	t->properties |= properties;

	return 0;
}

// -----------------------------------------------------------------------
int emui_tile_set_name(struct emui_tile *t, char *name)
{
	free(t->name);
	t->name = strdup(name);

	return 0;
}

// -----------------------------------------------------------------------
int emui_tile_set_style(struct emui_tile *t, int style)
{
	t->style = style;
	return 0;
}

// -----------------------------------------------------------------------
void emui_tile_child_append(struct emui_tile *parent, struct emui_tile *t)
{
	// lint into children list
	t->parent = parent;
	t->prev = parent->ch_last;
	if (parent->ch_last) {
		parent->ch_last->next = t;
	} else {
		parent->ch_first = t;
	}
	parent->ch_last = t;

	// link into focus group list
	emui_focus_group_add(parent, t);
}

// -----------------------------------------------------------------------
void emui_tile_child_unlink(struct emui_tile *t)
{
	if (!t->parent) return;

	if (t->prev) {
		t->prev->next = t->next;
	}
	if (t->next) {
		t->next->prev = t->prev;
	}
	if (t == t->parent->ch_first) {
		t->parent->ch_first = t->next;
	}
	if (t == t->parent->ch_last) {
		t->parent->ch_last = t->prev;
	}
}

// -----------------------------------------------------------------------
static void emui_tile_free(struct emui_tile *t)
{
	t->drv->destroy_priv_data(t);
	free(t->name);

	// free ncurses windows
	delwin(t->ncwin);
	if (t->ncdeco) {
		delwin(t->ncdeco);
	}

	free(t);
}

// -----------------------------------------------------------------------
static void emui_tile_destroy_children(struct emui_tile *t)
{
	struct emui_tile *ch = t->ch_first;
	while (ch) {
		struct emui_tile *next_ch = ch->next;
		emui_tile_destroy_children(ch);
		emui_tile_free(ch);
		ch = next_ch;
	}
}

// -----------------------------------------------------------------------
void emui_tile_destroy(struct emui_tile *t)
{
	if (!t) return;

	// remove the tile from parent's child list
	emui_tile_child_unlink(t);

	// remove from focus group
	emui_focus_group_unlink(t);

	// destroy all child tiles
	emui_tile_destroy_children(t);

	// remove the tile itself
	emui_tile_free(t);
}

// -----------------------------------------------------------------------
void emui_tile_hide(struct emui_tile *t)
{
	if (t->properties & P_HIDDEN) return;

	t->properties |= P_HIDDEN;
	t = t->ch_first;
	while (t) {
		emui_tile_hide(t);
		t = t->next;
	}
}

// -----------------------------------------------------------------------
void emui_tile_unhide(struct emui_tile *t)
{
	if (!(t->properties & P_HIDDEN)) return;

	t->properties &= ~P_HIDDEN;
	t = t->ch_first;
	while (t) {
		emui_tile_unhide(t);
		t = t->next;
	}
}

// -----------------------------------------------------------------------
void emui_tile_debug_set(int i)
{
	emui_tile_debug_mode = i;
}

// -----------------------------------------------------------------------
void emui_tile_set_id(struct emui_tile *t, int id)
{
	t->id = id;
}

// -----------------------------------------------------------------------
int emui_tile_get_id(struct emui_tile *t)
{
	return t->id;
}

// -----------------------------------------------------------------------
int emui_tile_changed(struct emui_tile *t)
{
	if (t->user_change_handler && t->user_change_handler(t)) {
		t->content_invalid = 1;
	} else {
		t->content_invalid = 0;
	}

	return t->content_invalid;
}

// vim: tabstop=4 shiftwidth=4 autoindent
