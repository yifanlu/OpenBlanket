//
//  blanket.h
//  libBlanket header
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

#ifndef BLANKET
#define BLANKET

#include <cairo/cairo.h>
#include <pango/pango.h>
#include <X11/Xlib.h>

extern int g_blanket_llog_mask;

#define DEBUG_LOG 0x8000
#define INFO_LOG 0x800000
#define WARNING_LOG 0x1000000
#define ERROR_LOG 0x2000000
#define IS_LOGGING(log) ((g_blanket_llog_mask & log) == log)

struct lipc_event {
	const char *lipc_source;
	const char *lipc_eventName;
	void *lipc_callback;
	struct {
		void *s;
		void *unk;
	} param;
};

struct Module {
	char *name;
	void *handle;
	void *init_sym;
	void *deinit_sym;
	Window window;
	struct Context *ctx;
	struct lipc_event *callback_list;
	struct Module *self;
	Mask event_mask;
	int unk1;
	int unk2;
};

char *blanket_image_get_asset_name(const char *stem, char *name, const char *type, const char *ext);
void blanket_image_get_window(struct Module *module, Window *windowp, const char *name, int, int);
int blanket_image_gettext_draw_defaults(cairo_t *cr, const char *prefix, PangoAlignment alignment, int, void *);
int blanket_image_gettext_pango_rect(const char *mid, PangoRectangle *rect);
void blanket_image_progressbar_destroy(void *handle);
cairo_surface_t *blanket_image_progressbar_get_surface(void *handle);
int blanket_image_progressbar_init(void **handlep, int width, int height, int border); // TODO: Find struct for handle's type
void blanket_image_progressbar_update(void *handle, int width);
Display *blanket_image_screendisplay();
int blanket_image_screenheight();
Visual *blanket_image_screenvisual();
int blanket_image_screenwidth();
int blanket_image_set_fbdev_mode(int mode);
int blanket_image_window_bringup(struct Module *module, Window win);
void blanket_image_window_destroy(struct Module *module, Window subwin);
void blanket_image_window_teardown(struct Module *module, Window win);
int blanket_loader_add_callbacks(void *config, struct Module *module, int num_lipcCallbacks, struct lipc_event *lipcCallbackList, void *x11Callback, int);
int blanket_util_gettext_list(const char *mid, char **values, int max_idx, void *convert_func); // Unsure of this one
int blanket_util_gettext_value(const char *mid, char **values, void *convert_func); // Also unsure of this one

#endif
