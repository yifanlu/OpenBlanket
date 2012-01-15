//
//  screensaver.c
//  Open source re-implementation of the screensaver module
//  A reference for custom modules
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

#include <screensaver.h>

int init(void *config, struct Module *module, struct Context **ctxp)
{	
	struct lipc_event se_elm[5];
	struct Context *ctx;
	int errno;
	Display *screendisplay;
	Visual *screenvisual;
	int screenwidth;
	int screenheight;
	
	ctx = calloc(sizeof(struct Context), 1);
	if(ctx == 0)
	{
		if(IS_LOGGING(ERROR_LOG))
		{
			syslog(LOG_ERR, "E %s:OUT_OF_MEMORY::failed to allocate %s", "init", "ctx");
		}
		return 12;
	}
	
	*ctxp = ctx;
	ctx->mapped = 0;
	ctx->type = 0;
	
	se_elm[0].lipc_source = "com.lab126.powerd";
	se_elm[0].lipc_eventName = "goingToScreenSaver";
	se_elm[0].lipc_callback = &module_screensaver_map_screensaver;
	se_elm[1].lipc_source = "com.lab126.powerd";
	se_elm[1].lipc_eventName = "outOfScreenSaver";
	se_elm[1].lipc_callback = &module_screensaver_unmap;
	se_elm[2].lipc_source = "com.lab126.powerd";
	se_elm[2].lipc_eventName = "userShutdown";
	se_elm[2].lipc_callback = &module_screensaver_map_blank;
	se_elm[3].lipc_source = "com.lab126.powerd";
	se_elm[3].lipc_eventName = "outOfShutdown";
	se_elm[3].lipc_callback = &module_screensaver_unmap;
	se_elm[4].lipc_source = "com.lab126.hal.screensaver";
	se_elm[4].lipc_eventName = "goingToScreenSaver";
	se_elm[4].lipc_callback = &module_screensaver_map_screensaver;
	
	if((errno = blanket_loader_add_callbacks(config, module, 5, se_elm, (void*)&module_screensaver_repaint, 0x8000)) != 0)
	{
		syslog(LOG_ERR, "E screensaver:ADD_MODULE_CALLBACKS_FAILED:err=%d:%s", errno, strerror(errno));
		free(ctx);
		*ctxp = NULL;
		return errno;
	}
	
	blanket_image_get_window(module, &ctx->window, "blanket_screensaver", 0, 0xFFFFFFFF);
	screendisplay = blanket_image_screendisplay();
	screenvisual = blanket_image_screenvisual();
	screenwidth = blanket_image_screenwidth();
	screenheight = blanket_image_screenheight();
	ctx->surface = cairo_xlib_surface_create(screendisplay, ctx->window, screenvisual, screenwidth, screenheight);
	ctx->ss_cr = cairo_create(ctx->surface);
	
	return 0;
}

int deinit(void *config, struct Module *module, struct Context *ctx)
{
	int errno;
	if((errno = module_screensaver_unmap(module, 0, ctx)) != 0)
	{
		if(IS_LOGGING(ERROR_LOG))
		{
			syslog(LOG_ERR, "E screensaver:UNMAP_FAILED:err=%d:%s", errno, strerror(errno));
		}
		return errno;
	}
	
	blanket_image_window_destroy(module, ctx->window);
	cairo_surface_destroy(ctx->surface);
	cairo_destroy(ctx->ss_cr);
	ctx->surface = NULL;
	ctx->ss_cr = NULL;
	free(ctx);
	
	return 0;
}

int module_screensaver_unmap(struct Module *module, int unused, struct Context *ctx)
{
	if(IS_LOGGING(DEBUG_LOG))
	{
		syslog(LOG_DEBUG, "D def:enter:%s:%d", "module_screensaver_unmap", __LINE__);
	}
	
	if(ctx != NULL)
	{
		blanket_image_window_teardown(module, ctx->window);
	}
	
	ctx->mapped = 0;
	ctx->type = 0;
	
	if(IS_LOGGING(DEBUG_LOG))
	{
		syslog(LOG_DEBUG, "D def:leave:%s:%d", "module_screensaver_unmap", __LINE__);
	}
	
	return 0;
}

int module_screensaver_map_screensaver(struct Module *module, int unused, struct Context *ctx)
{
	int errno;
	
	if(IS_LOGGING(DEBUG_LOG))
	{
		syslog(LOG_DEBUG, "D def:enter:%s:%d", "module_screensaver_map_screensaver", __LINE__);
	}
	
	if((errno = module_screensaver_map_generic(module, ctx, 1)) != 0)
	{
		if(IS_LOGGING(DEBUG_LOG))
		{
			syslog(LOG_DEBUG, "D def:leave:%s:%d", "module_screensaver_map_screensaver", __LINE__);
		}
	}
	
	return errno;
}

int module_screensaver_map_blank(struct Module *module, int unused, struct Context *ctx)
{
	int ret;
	
	if(IS_LOGGING(DEBUG_LOG))
	{
		syslog(LOG_DEBUG, "D def:enter:%s:%d", "module_screensaver_map_blank", __LINE__);
	}
	
	ret = module_screensaver_map_generic(module, ctx, 2);
	
	if(IS_LOGGING(DEBUG_LOG))
	{
		syslog(LOG_DEBUG, "D def:leave:%s:%d", "module_screensaver_map_blank", __LINE__);
	}
	
	return ret;
}

int module_screensaver_map_generic(struct Module *module, struct Context *ctx, int type)
{
	int ret;
	
	if(IS_LOGGING(DEBUG_LOG))
	{
		syslog(LOG_DEBUG, "D def:enter:%s:%d", "module_screensaver_map_generic", __LINE__);
	}
	
	if(module == NULL)
	{
		if(IS_LOGGING(ERROR_LOG))
		{
			syslog(LOG_ERR, "E %s:BAD_ARGS::argument is not valid, require %s", "module_screensaver_map_generic", "(module != NULL)");
		}
		
		ret = 22;
	}
	else if(ctx == 0)
	{
		if(IS_LOGGING(ERROR_LOG))
		{
			syslog(LOG_ERR, "E %s:BAD_ARGS::argument is not valid, require %s", "module_screensaver_map_generic", "(ctx != NULL)");
		}
		ret = 22;
	}
	else if(ctx->mapped == 0)
	{
		if((ret = module_screensaver_prerender(module, ctx, type)) != 0)
		{
			if(IS_LOGGING(ERROR_LOG))
			{
				syslog(LOG_ERR, "E screensaver:PRERENDER_FAILED:err=%d,type=%d:%s", ret, type, strerror(ret));
			}
		}
		else
		{
			blanket_image_window_bringup(module, ctx->window);
			ctx->mapped = 1;
		}
	}
	else if(ctx->mapped == 1)
	{
		if(ctx->type == type)
		{
			cairo_paint(ctx->ss_cr);
			ret = 0;
		}
		else
		{
			if((ret = module_screensaver_prerender(module, ctx, type)) != 0)
			{
				if(IS_LOGGING(ERROR_LOG))
				{
					syslog(LOG_ERR, "E screensaver:PRERENDER_FAILED:err=%d,type=%d:%s", ret, type, strerror(ret));
				}
			}
			else
			{
				cairo_paint(ctx->ss_cr);
			}
		}
	}
	
	if(IS_LOGGING(DEBUG_LOG))
	{
		syslog(LOG_DEBUG, "D def:leave:%s:%d", "module_screensaver_map_generic", __LINE__);
	}
	
	return ret;
}

int module_screensaver_prerender(struct Module *module, struct Context *ctx, int type)
{
	int ret;
	char current_name[5];
	char *image_path;
	cairo_surface_t *surface;
	cairo_status_t c_err;
	int width;
	int height;
	cairo_t *context;
	int s_width;
	int s_height;
	
	if(IS_LOGGING(DEBUG_LOG))
	{
		syslog(LOG_DEBUG, "D def:enter:%s:%d", "module_screensaver_prerender", __LINE__);
	}
	
	if(module == NULL)
	{
		if(IS_LOGGING(ERROR_LOG))
		{
			syslog(LOG_ERR, "E %s:BAD_ARGS::argument is not valid, require %s", "module_screensaver_prerender", "(module != NULL)");
		}
		ret = 22;
	}
	else if(ctx == NULL)
	{
		if(IS_LOGGING(ERROR_LOG))
		{
			syslog(LOG_ERR, "E %s:BAD_ARGS::argument is not valid, require %s", "module_screensaver_prerender", "(ctx != NULL)");
		}
		ret = 22;
	}
	else if(type != 1)
	{
		ctx->type = type;
		if(type < 1)
		{
			if(IS_LOGGING(ERROR_LOG))
			{
				syslog(LOG_ERR, "E screensaver:INVALID_SCREEN_SELECTED:type=%d:screen type is not valid", type);
			}
			ret = 22;
		}
		else
		{
			if(type == 2)
			{
				cairo_set_source_rgb(ctx->ss_cr, 0.0, 0.0, 1.875);
				cairo_rectangle(ctx->ss_cr, 0.0, 0.0, (double)blanket_image_screenwidth(), (double)blanket_image_screenheight());
				cairo_fill(ctx->ss_cr);
				cairo_paint(ctx->ss_cr);
			}
			ret = 0;
		}
	}
	else
	{
		module_screensaver_nextScreenSaverName(current_name);
 		image_path = blanket_image_get_asset_name("screensaver", current_name, "bg", "png");
		surface = cairo_image_surface_create_from_png(image_path);
		width = 0;
		height = 0;
		context = NULL;
		if((c_err = cairo_surface_status(surface)) != 0)
		{
			if(IS_LOGGING(ERROR_LOG))
			{
				syslog(LOG_ERR, "E %s:CAIRO_FAILED:sts=%d:%s", "module_screensaver_prerender", c_err, cairo_status_to_string(c_err));
			}
			if(IS_LOGGING(ERROR_LOG))
			{
				syslog(LOG_ERR, "E %s:CAIRO_SURFACE_CREATE_FAILED:png=%s:", "module_screensaver_prerender", image_path); 
			}
		}
		else
		{
			width = cairo_image_surface_get_width(surface);
			height = cairo_image_surface_get_height(surface);
			context = cairo_create(surface);
			if(IS_LOGGING(DEBUG_LOG))
			{
				syslog(LOG_DEBUG, "D def:Image %s is %dx%d pixels (%s:%s:%d)", image_path, width, height, "module_screensaver_prerender", "/home/build/src/yoshi/juno/OFFICIAL/platform/bin/blanket/loaders", __LINE__);
			}
		}
		free(image_path);
		s_width = blanket_image_screenwidth();
		s_height = blanket_image_screenheight();
		cairo_set_source_surface(ctx->ss_cr, surface, s_width - width, s_height - height);
		cairo_paint(ctx->ss_cr);
		cairo_surface_destroy(surface);
		cairo_destroy(context);
		ret = 0;
	}
	
	if(IS_LOGGING(DEBUG_LOG))
	{
		syslog(LOG_DEBUG, "D def:leave:%s:%d", "module_screensaver_prerender", __LINE__);
	}
	return ret;
}

void module_screensaver_nextScreenSaverName(char current_namep[5])
{	
	int errno;
	int fd;
	char current_name[5];
	char next_name[5];
	struct stat sb;
	char *image_path;
	
	if(IS_LOGGING(DEBUG_LOG))
	{
		syslog(LOG_DEBUG, "D def:enter:%s:%d", "module_screensaver_nextScreenSaverName", __LINE__);
	}
	
	if((errno = g_mkdir_with_parents("/var/local/blanket/screensaver", S_IRWXU)) < 0)
	{
		if(IS_LOGGING(ERROR_LOG))
		{
			syslog(LOG_ERR, "E screensaver:MKDIR_WITH_PARENTS_FAILED:err=%d,path=%s,mode=%d:%s", errno, "/var/local/blanket/screensaver", S_IRWXU, strerror(errno));
		}
	}
	if((fd = open("/var/local/blanket/screensaver/last_ss", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) < 0)
	{
		if(IS_LOGGING(ERROR_LOG))
		{
			syslog(LOG_ERR, "E %s:OPEN_FILE_FAILED:err=%d,path=%s,mode=%d:%s", "module_screensaver_nextScreenSaverName", fd, "/var/local/blanket/screensaver/last_ss", O_CREAT | O_RDWR, strerror(fd));
		}
	}
	else
	{
		ssize_t bytesread = read(fd, current_name, 4);
		close(fd);
		if(!(bytesread >= 4 && current_name[0] == 's' && current_name[1] == 's'))
		{
			strcpy(current_name, "ss00");
		}
		current_name[4] = 0;
		image_path = blanket_image_get_asset_name("screensaver", current_name, "bg", "png");
		if(image_path == NULL)
		{
			if(IS_LOGGING(ERROR_LOG))
			{
				syslog(LOG_ERR, "E %s:DOES_NOT_EXIST:object=%s:", "module_screensaver_nextScreenSaverName", "bg_path");
			}
			strcpy(current_name, "ss00");
		}
		else
		{
			if((errno = stat(image_path, &sb)) < 0)
			{
				if(IS_LOGGING(ERROR_LOG))
				{
					syslog(LOG_ERR, "E screensaver:STAT_FAILED:err=%d,path=%s:%s", errno, image_path, strerror(errno));
				}
				strcpy(current_name, "ss00");
			}
			else
			{
				if(!S_ISREG(sb.st_mode))
				{
					if(IS_LOGGING(ERROR_LOG))
					{
						syslog(LOG_ERR, "E screensaver:FILE_IS_NOT_REGULAR:path=%s,mode=%d:", image_path, S_ISREG(sb.st_mode));
					}
					strcpy(current_name, "ss00");
				}
			}
			free(image_path);
			current_name[4] = 0;
			strncpy(next_name, current_name, 5);
			next_name[4] = 0;
			if(next_name[3] == '9')
			{
				next_name[3] = '0';
				if(next_name[2] == '9')
				{
					next_name[2] = '0';
				}
				else
				{
					next_name[2]++;
				}
			}
			else
			{
				next_name[3]++;
			}
			if((fd = open("/var/local/blanket/screensaver/last_ss", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) < 0)
			{
				if(IS_LOGGING(ERROR_LOG))
				{
						syslog(LOG_ERR, "E %s:OPEN_FILE_FAILED:err=%d,path=%s,mode=%d:%s", "module_screensaver_nextScreenSaverName", fd, "/var/local/blanket/screensaver/last_ss", O_CREAT | O_RDWR, strerror(fd));
				}
			}
			else
			{
				write(fd, next_name, 4);
				close(fd);
			}
		}
	}
	
	strcpy(current_namep, current_name);
		
	if(IS_LOGGING(DEBUG_LOG))
	{
		syslog(LOG_DEBUG, "D def:leave:%s:%d", "module_screensaver_nextScreenSaverName", __LINE__);
	}
}

int module_screensaver_repaint(struct Module *module, XEvent *xev, struct Context *ctx)
{
	int ret;
	
	if(IS_LOGGING(DEBUG_LOG))
	{
		syslog(LOG_DEBUG, "D def:enter:%s:%d", "module_screensaver_repaint", __LINE__);
	}
	
	if(ctx->mapped == 0)
	{
		if(IS_LOGGING(DEBUG_LOG))
		{
			syslog(LOG_DEBUG, "D screensaver:XEVENT_WHEN_UNMAPPED:type=%d:exiting repaint", xev->type);
		}
		ret = 0;
	}
	else if(xev->type == Expose)
	{
		if(((char*)xev)[0x24] > 0) // TODO: Find out what this element is
		{
		}
		else if(ctx->ss_cr == NULL)
		{
			if(IS_LOGGING(ERROR_LOG))
			{
				syslog(LOG_ERR, "E %s:DOES_NOT_EXIST:object=%s:", "module_screensaver_repaint", "ctx->ss_cr");
			}
		}
		else
		{
			cairo_paint(ctx->ss_cr);
		}	
		ret = 0;
	}
	else
	{
		if(IS_LOGGING(WARNING_LOG))
		{
			syslog(4, "W screensaver:UNHANDLED_X11_EVENT:type=%d:", xev->type);
		}
		ret = 38;
	}
	
	if(IS_LOGGING(DEBUG_LOG))
	{
		syslog(LOG_DEBUG, "D def:leave:%s:%d", "module_screensaver_repaint", __LINE__);
	}
	
	return ret;
}
