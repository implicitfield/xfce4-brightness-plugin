/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2010 Richard Hughes <richard@hughsie.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef HELPERS_H
#define HELPERS_H

#include <glib-object.h>

#define GCM_BACKLIGHT_HELPER_EXIT_CODE_SUCCESS			0
#define GCM_BACKLIGHT_HELPER_EXIT_CODE_FAILED			1
#define GCM_BACKLIGHT_HELPER_EXIT_CODE_ARGUMENTS_INVALID	3
#define GCM_BACKLIGHT_HELPER_EXIT_CODE_INVALID_USER		4

#define GCM_BACKLIGHT_HELPER_SYSFS_LOCATION			"/sys/class/backlight"

gchar* gcm_backlight_helper_get_best_backlight (void);
int set_brightness(unsigned in, gchar* filename);
int get_brightness(unsigned* out, gchar* filename);
int get_max_brightness(unsigned* out, gchar* filename);

#endif
