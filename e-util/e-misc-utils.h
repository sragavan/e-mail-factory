/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the program; if not, see <http://www.gnu.org/licenses/>
 *
 *
 * Authors:
 *		Chris Lahey <clahey@ximian.com>
 *
 * Copyright (C) 1999-2008 Novell, Inc. (www.novell.com)
 *
 */

#if !defined (__E_UTIL_H_INSIDE__) && !defined (LIBEUTIL_COMPILATION)
#error "Only <e-util/e-util.h> should be included directly."
#endif

#ifndef E_MISC_UTILS_H
#define E_MISC_UTILS_H

#include <sys/types.h>
#include <gtk/gtk.h>
#include <limits.h>

#include <libedataserver/libedataserver.h>

G_BEGIN_DECLS

#define E_ASCII_DTOSTR_BUF_SIZE (DBL_DIG + 12 + 10)

GtkWidget *	e_builder_get_widget		(GtkBuilder *builder,
						 const gchar *widget_name);
void		e_load_ui_builder_definition	(GtkBuilder *builder,
						 const gchar *basename);

G_END_DECLS

#endif /* E_MISC_UTILS_H */
