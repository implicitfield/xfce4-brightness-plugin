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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#include "helpers.h"

gchar *
gcm_backlight_helper_get_best_backlight (void)
{
	gchar *filename;
	guint i;
	gboolean ret;
	GDir *dir = NULL;
	GError *error = NULL;
	const gchar *first_device;

	/* available kernel interfaces in priority order */
	static const gchar *backlight_interfaces[] = {
		"nv_backlight",
		"asus_laptop",
		"toshiba",
		"eeepc",
		"thinkpad_screen",
		"intel_backlight",
		"acpi_video1",
		"mbp_backlight",
		"acpi_video0",
		"fujitsu-laptop",
		"sony",
		"samsung",
		NULL,
	};

	/* search each one */
	for (i=0; backlight_interfaces[i] != NULL; i++) {
		filename = g_build_filename (GCM_BACKLIGHT_HELPER_SYSFS_LOCATION,
					     backlight_interfaces[i], NULL);
		ret = g_file_test (filename, G_FILE_TEST_EXISTS);
		if (ret)
			goto out;
		g_free (filename);
	}

	/* nothing found in the ordered list */
	filename = NULL;

	/* find any random ones */
	dir = g_dir_open (GCM_BACKLIGHT_HELPER_SYSFS_LOCATION, 0, &error);
	if (dir == NULL) {
		g_warning ("failed to find any devices: %s", error->message);
		g_error_free (error);
		goto out;
	}

	/* get first device if any */
	first_device = g_dir_read_name (dir);
	if (first_device != NULL) {
		filename = g_build_filename (GCM_BACKLIGHT_HELPER_SYSFS_LOCATION,
					     first_device, NULL);
	}
out:
	if (dir != NULL)
		g_dir_close (dir);
	return filename;
}

static gboolean
gcm_backlight_helper_write (const gchar *filename, gint value, GError **error)
{
	gchar *text = NULL;
	gint retval;
	gint length;
	gint fd = -1;
	gboolean ret = TRUE;

	fd = open (filename, O_WRONLY);
	if (fd < 0) {
		ret = FALSE;
		g_set_error (error, 1, 0, "failed to open filename: %s", filename);
		goto out;
	}

	/* convert to text */
	text = g_strdup_printf ("%i", value);
	length = strlen (text);

	/* write to device file */
	retval = write (fd, text, length);
	if (retval != length) {
		ret = FALSE;
		g_set_error (error, 1, 0, "writing '%s' to %s failed", text, filename);
		goto out;
	}
out:
	if (fd >= 0)
		close (fd);
	g_free (text);
	return ret;
}

int set_brightness(unsigned in, gchar* filename) {
	gchar *filename_file = g_build_filename (filename, "brightness", NULL);
	GError *error = NULL;
	gboolean ret = gcm_backlight_helper_write (filename_file, in, &error);
	if (!ret) {
		g_error_free (error);
		return GCM_BACKLIGHT_HELPER_EXIT_CODE_ARGUMENTS_INVALID;
	}

	return GCM_BACKLIGHT_HELPER_EXIT_CODE_SUCCESS;
}

int get_brightness(unsigned* out, gchar* filename) {
	gchar *contents = NULL;
	GError *error = NULL;
	if (filename == NULL) {
		return GCM_BACKLIGHT_HELPER_EXIT_CODE_INVALID_USER;
	}

	gchar *filename_file = g_build_filename (filename, "brightness", NULL);
	gboolean ret = g_file_get_contents (filename_file, &contents, NULL, &error);
	if (!ret) {
		g_error_free (error);
		return GCM_BACKLIGHT_HELPER_EXIT_CODE_ARGUMENTS_INVALID;
	}

	*out = atoi(contents);
	return GCM_BACKLIGHT_HELPER_EXIT_CODE_SUCCESS;
}

int get_max_brightness(unsigned* out, gchar* filename) {
	gchar *contents = NULL;
	gchar *filename_file = NULL;
	GError *error = NULL;
	if (filename == NULL) {
		return GCM_BACKLIGHT_HELPER_EXIT_CODE_INVALID_USER;
	}

	filename_file = g_build_filename (filename, "max_brightness", NULL);
	gboolean ret = g_file_get_contents (filename_file, &contents, NULL, &error);
	if (!ret) {
		g_print ("Could not get the maximum value of the backlight: %s\n", error->message);
		g_error_free (error);
		out = NULL;
		return GCM_BACKLIGHT_HELPER_EXIT_CODE_ARGUMENTS_INVALID;
	}

        *out = atoi(contents);
	return GCM_BACKLIGHT_HELPER_EXIT_CODE_SUCCESS;
}
