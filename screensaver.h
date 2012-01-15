//
//  screensaver.h
//  screensaver module header
//
//  By Yifan Lu
//  
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef SCREENSAVER_MODULE
#define SCREENSAVER_MODULE

#include <blanket.h>
#include <cairo/cairo-xlib.h>
#include <fcntl.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>
#include <unistd.h>

struct Context {
	int mapped;
	int type;
	cairo_t *ss_cr;
	cairo_surface_t *surface;
	Window window;
};

int init(void *config, struct Module *module, struct Context **ctxp);
int deinit(void *config, struct Module *module, struct Context *ctx);
int module_screensaver_unmap(struct Module *module, int unused, struct Context *ctx);
int module_screensaver_map_screensaver(struct Module *module, int unused, struct Context *ctx);
int module_screensaver_map_blank(struct Module *module, int unused, struct Context *ctx);
int module_screensaver_map_generic(struct Module *module, struct Context *ctx, int type);
int module_screensaver_prerender(struct Module *module, struct Context *ctx, int type);
void module_screensaver_nextScreenSaverName(char current_namep[5]);
int module_screensaver_repaint(struct Module *module, XEvent *xev, struct Context *ctx);

#endif
