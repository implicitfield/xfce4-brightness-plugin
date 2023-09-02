/*
 * * Copyright (C) 2009-2011 Ali <aliov@xfce.org>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#include <libxfce4util/libxfce4util.h>

#include "helpers.h"
#include "xfpm-brightness.h"

static void xfpm_brightness_finalize   (GObject *object);

#define XFPM_BRIGHTNESS_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), XFPM_TYPE_BRIGHTNESS, XfpmBrightnessPrivate))

struct XfpmBrightnessPrivate
{
    XRRScreenResources *resource;
    Atom		backlight;
    gint 		output;
    gboolean		helper_has_hw;
    gchar               *filename;
    
    gint32		max_level;
    gint32		current_level;
    gint32		min_level;
    gint32		step;
};

G_DEFINE_TYPE_WITH_PRIVATE (XfpmBrightness, xfpm_brightness, G_TYPE_OBJECT)

static gboolean
xfpm_brightness_setup_helper (XfpmBrightness *brightness)
{
    brightness->priv->filename = gcm_backlight_helper_get_best_backlight();

    if ( brightness->priv->filename == NULL ) {
	brightness->priv->helper_has_hw = FALSE;
    } else {
	brightness->priv->helper_has_hw = TRUE;
	brightness->priv->min_level = 0;
	get_max_brightness(&brightness->priv->max_level, brightness->priv->filename);
	brightness->priv->step = brightness->priv->max_level <= 20 ? 1 : brightness->priv->max_level / 10;
    }

    return brightness->priv->helper_has_hw;
}

static gboolean
xfpm_brightness_helper_get_level (XfpmBrightness *brg, gint32 *level)
{
    gint32 ret;

    if ( ! brg->priv->helper_has_hw )
	return FALSE;

    get_brightness(&ret, brg->priv->filename);

    g_debug ("xfpm_brightness_helper_get_level: get-brightness returned %i", ret);

    if ( ret >= 0 )
    {
	*level = ret;
	return TRUE;
    }

    return FALSE;
}

static gboolean
xfpm_brightness_helper_up (XfpmBrightness *brightness, gint32 *new_level)
{
    gint32 hw_level;
    gboolean ret = FALSE;
    gint32 set_level;
    
    ret = xfpm_brightness_helper_get_level (brightness, &hw_level);
    
    if ( !ret )
	return FALSE;

    if ( hw_level >= brightness->priv->max_level )
    {
	*new_level = brightness->priv->max_level;
	return TRUE;
    }

    set_level = MIN (hw_level + brightness->priv->step, brightness->priv->max_level );

    g_warn_if_fail (set_brightness (set_level, brightness->priv->filename));

    ret = xfpm_brightness_helper_get_level (brightness, new_level);
    
    if ( !ret )
    {
	g_warning ("xfpm_brightness_helper_up failed for %d", set_level);
	return FALSE;
    }
	
    /* Nothing changed in the hardware*/
    if ( *new_level == hw_level )
    {
	g_warning ("xfpm_brightness_helper_up did not change the hw level to %d", set_level);
	return FALSE;
    }
    
    return TRUE;
}

static gboolean
xfpm_brightness_helper_down (XfpmBrightness *brightness, gint32 *new_level)
{
    gint32 hw_level;
    gboolean ret;
    gint32 set_level;
    
    ret = xfpm_brightness_helper_get_level (brightness, &hw_level);
    
    if ( !ret )
	return FALSE;
    
    if ( hw_level <= brightness->priv->min_level )
    {
	*new_level = brightness->priv->min_level;
	return TRUE;
    }

    set_level = MAX (hw_level - brightness->priv->step, brightness->priv->min_level);

    g_warn_if_fail (set_brightness (set_level, brightness->priv->filename));

    ret = xfpm_brightness_helper_get_level (brightness, new_level);
    
    if ( !ret )
    {
	g_warning ("xfpm_brightness_helper_down failed for %d", set_level);
	return FALSE;
    }
    
    /* Nothing changed in the hardware*/
    if ( *new_level == hw_level )
    {
	g_warning ("xfpm_brightness_helper_down did not change the hw level to %d", set_level);
	return FALSE;
    }
    
    return TRUE;
}

static void
xfpm_brightness_class_init (XfpmBrightnessClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = xfpm_brightness_finalize;
}

static void
xfpm_brightness_init (XfpmBrightness *brightness)
{
    brightness->priv = xfpm_brightness_get_instance_private (brightness);
    
    brightness->priv->resource = NULL;
    brightness->priv->helper_has_hw = FALSE;
    brightness->priv->max_level = 0;
    brightness->priv->min_level = 0;
    brightness->priv->current_level = 0;
    brightness->priv->output = 0;
    brightness->priv->filename = NULL;
    brightness->priv->step = 0;
}

static void
xfpm_brightness_free_data (XfpmBrightness *brightness)
{
    if ( brightness->priv->resource )
	XRRFreeScreenResources (brightness->priv->resource);
}

static void
xfpm_brightness_finalize (GObject *object)
{
    XfpmBrightness *brightness;

    brightness = XFPM_BRIGHTNESS (object);

    xfpm_brightness_free_data (brightness);

    G_OBJECT_CLASS (xfpm_brightness_parent_class)->finalize (object);
}

XfpmBrightness *
xfpm_brightness_new (void)
{
    XfpmBrightness *brightness = NULL;
    brightness = g_object_new (XFPM_TYPE_BRIGHTNESS, NULL);
    return brightness;
}

gboolean
xfpm_brightness_setup (XfpmBrightness *brightness)
{
    xfpm_brightness_free_data (brightness);

    if ( xfpm_brightness_setup_helper (brightness) )
        return TRUE;

    g_debug ("no brightness controls available");
    return FALSE;
}

gboolean xfpm_brightness_up (XfpmBrightness *brightness, gint32 *new_level)
{
    gboolean ret = FALSE;

    if ( brightness->priv->helper_has_hw )
	ret = xfpm_brightness_helper_up (brightness, new_level);

    return ret;
}

gboolean xfpm_brightness_down (XfpmBrightness *brightness, gint32 *new_level)
{
    gboolean ret = FALSE;

    if ( brightness->priv->helper_has_hw )
	ret = xfpm_brightness_helper_down (brightness, new_level);

    return ret;
}

gboolean xfpm_brightness_has_hw (XfpmBrightness *brightness)
{
    return brightness->priv->helper_has_hw;
}

gint32 xfpm_brightness_get_max_level (XfpmBrightness *brightness)
{
    return brightness->priv->max_level;
}

gboolean xfpm_brightness_get_level	(XfpmBrightness *brightness, gint32 *level)
{
    gboolean ret = FALSE;

    if ( brightness->priv->helper_has_hw )
        ret = xfpm_brightness_helper_get_level (brightness, level);

    return ret;
}

gboolean xfpm_brightness_set_level (XfpmBrightness *brightness, gint32 level)
{
    gboolean ret = FALSE;

    if ( brightness->priv->helper_has_hw )
       ret = set_brightness (level, brightness->priv->filename);

    return ret;
}

gboolean xfpm_brightness_dim_down (XfpmBrightness *brightness)
{
    gboolean ret = FALSE;

    if ( brightness->priv->helper_has_hw )
       ret = set_brightness (brightness->priv->min_level, brightness->priv->filename);

    return ret;
}
