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

#include "emui.h"

struct emui_tile *layout;
struct emui_tile *win0, *win1, *win2, *win3, *win4;
struct emui_tile *wid1, *wid2, *wid3, *wid4, *wid5;

// -----------------------------------------------------------------------
struct emui_tile * create_win1(struct emui_tile *parent)
{
	struct emui_tile *win, *wid1, *wid2, *wid3;

	win = emui_window_new(parent, 0, 1, 30, 20, "Counters", P_NONE);
	emui_tile_set_focus_key(win, KEY_F(1));

	wid1 = emui_lineedit_new(win, 0, 3, 20, 20, TT_TEXT, P_NONE);
	emui_lineedit_set_text(wid1, "Sooyoung");

	wid2 = emui_label_new(win, 0, 0, 10, AL_LEFT, S_TEXT, "Frame:");
	wid2 = emui_framecounter_new(win, 17, 0, S_TEXT_BOLD);
	wid3 = emui_label_new(win, 0, 1, 10, AL_CENTER, S_DEBUG, "FPS:");
	wid3 = emui_fpscounter_new(win, 17, 1, S_TEXT_BOLD);

	return win;
}

// -----------------------------------------------------------------------
struct emui_tile * create_win2(struct emui_tile *parent)
{
	struct emui_tile *win, *wid1, *wid2, *wid3, *wid4, *cont1, *cont2;

	win = emui_window_new(parent, 30, 1, 30, 20, "Edits", P_NONE);
	emui_tile_set_focus_key(win, KEY_F(2));

	cont1 = emui_dummy_cont_new(win, 0, 0, win->w, 1);
	cont2 = emui_dummy_cont_new(win, 0, 1, win->w, 1);

	wid1 = emui_lineedit_new(cont1, 0, 0, 10, 10, TT_TEXT, P_NONE);
	emui_lineedit_set_text(wid1, "EMUI");

	wid2 = emui_lineedit_new(cont1, 15, 0, 10, 10, TT_TEXT, P_NONE);
	emui_lineedit_set_text(wid2, "is");

	wid3 = emui_lineedit_new(cont2, 0, 0, 10, 10, TT_TEXT, P_NONE);
	emui_lineedit_set_text(wid3, "a lot");

	wid4 = emui_lineedit_new(cont2, 15, 0, 10, 10, TT_TEXT, P_NONE);
	emui_lineedit_set_text(wid4, "of fun");

	return win;
}

// -----------------------------------------------------------------------
struct emui_tile * create_win3(struct emui_tile *parent)
{
	struct emui_tile *win, *wid1;

	win = emui_window_new(parent, 60, 1, 30, 20, "Other", P_NONE);
	emui_tile_set_focus_key(win, KEY_F(3));
	wid1 = emui_lineedit_new(win, 0, 0, 20, 20, TT_TEXT, P_NONE);
	emui_lineedit_set_text(wid1, "Sample txt");

	return win;
}
// -----------------------------------------------------------------------
int main(int argc, char **argv)
{
	struct emui_tile *tabs;
	struct emui_tile *win1, *win2, *win3;

	emui_tile_debug_set(1);

	layout = emui_init(30);
	//tabs = emui_tabs_new(layout);
	tabs = emui_splitter_new(layout, AL_TOP, 20, 30, 20);

	win1 = create_win1(tabs);
	win2 = create_win2(tabs);
	//win3 = create_win3(tabs);

	emui_loop();
	emui_destroy();

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
