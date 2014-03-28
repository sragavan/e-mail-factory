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
 *		Dan Winship <danw@ximian.com>
 *		Peter Williams <peterw@ximian.com>
 *		Jeffrey Stedfast <fejj@ximian.com>
 *
 * Copyright (C) 1999-2008 Novell, Inc. (www.novell.com)
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#include <glib/gstdio.h>

#include <glib/gi18n.h>

#include "e-mail-session.h"
#include "mail-folder-cache.h"
#include "mail-tools.h"

/* **************************************** */

#ifndef G_OS_WIN32

static gchar *
mail_tool_get_local_movemail_path (CamelStore *store,
                                   GError **error)
{
	const gchar *uid;
	guchar *safe_uid, *c;
	const gchar *data_dir;
	gchar *path, *full;
	struct stat st;

	uid = camel_service_get_uid (CAMEL_SERVICE (store));
	safe_uid = (guchar *) g_strdup ((const gchar *) uid);
	for (c = safe_uid; *c; c++)
		if (strchr ("/:;=|%&#!*^()\\, ", *c) || !isprint ((gint) *c))
			*c = '_';

	data_dir = mail_session_get_data_dir ();
	path = g_build_filename (data_dir, "spool", NULL);

	if (g_stat (path, &st) == -1 && g_mkdir_with_parents (path, 0700) == -1) {
		g_set_error (
			error, G_FILE_ERROR,
			g_file_error_from_errno (errno),
			_("Could not create spool directory '%s': %s"),
			path, g_strerror (errno));
		g_free (path);
		return NULL;
	}

	full = g_strdup_printf ("%s/movemail.%s", path, safe_uid);
	g_free (path);
	g_free (safe_uid);

	return full;
}

#endif

gchar *
mail_tool_do_movemail (CamelStore *store,
                       GError **error)
{
#ifndef G_OS_WIN32
	CamelService *service;
	CamelProvider *provider;
	CamelSettings *settings;
	gchar *src_path;
	gchar *dest_path;
	struct stat sb;
	gboolean success;

	g_return_val_if_fail (CAMEL_IS_STORE (store), NULL);

	service = CAMEL_SERVICE (store);
	provider = camel_service_get_provider (service);

	g_return_val_if_fail (provider != NULL, NULL);

	if (g_strcmp0 (provider->protocol, "mbox") != 0) {
		/* This is really only an internal error anyway */
		g_set_error (
			error, CAMEL_SERVICE_ERROR,
			CAMEL_SERVICE_ERROR_URL_INVALID,
			_("Trying to movemail a non-mbox source '%s'"),
			camel_service_get_uid (CAMEL_SERVICE (store)));
		return NULL;
	}

	settings = camel_service_ref_settings (service);

	src_path = camel_local_settings_dup_path (
		CAMEL_LOCAL_SETTINGS (settings));

	g_object_unref (settings);

	/* Set up our destination. */
	dest_path = mail_tool_get_local_movemail_path (store, error);
	if (dest_path == NULL)
		return NULL;

	/* Movemail from source to dest_path */
	success = camel_movemail (src_path, dest_path, error) != -1;

	g_free (src_path);

	if (g_stat (dest_path, &sb) < 0 || sb.st_size == 0) {
		g_unlink (dest_path); /* Clean up the movemail.foo file. */
		g_free (dest_path);
		return NULL;
	}

	if (!success) {
		g_free (dest_path);
		return NULL;
	}

	return dest_path;
#else
	/* Unclear yet whether camel-movemail etc makes any sense on
	 * Win32, at least it is not ported yet.
	 */
	g_warning ("%s: Not implemented", __FUNCTION__);
	return NULL;
#endif
}

gchar *
mail_tool_generate_forward_subject (CamelMimeMessage *msg)
{
	const gchar *subject;
	gchar *fwd_subj;
	const gint max_subject_length = 1024;

	subject = camel_mime_message_get_subject (msg);

	if (subject && *subject) {
		/* Truncate insanely long subjects */
		if (strlen (subject) < max_subject_length) {
			fwd_subj = g_strdup_printf ("[Fwd: %s]", subject);
		} else {
			/* We can't use %.*s because it depends on the
			 * locale being C/POSIX or UTF-8 to work correctly
			 * in glibc. */
			fwd_subj = g_malloc (max_subject_length + 11);
			memcpy (fwd_subj, "[Fwd: ", 6);
			memcpy (fwd_subj + 6, subject, max_subject_length);
			memcpy (fwd_subj + 6 + max_subject_length, "...]", 5);
		}
	} else {
		const CamelInternetAddress *from;
		gchar *fromstr;

		from = camel_mime_message_get_from (msg);
		if (from) {
			fromstr = camel_address_format (CAMEL_ADDRESS (from));
			fwd_subj = g_strdup_printf ("[Fwd: %s]", fromstr);
			g_free (fromstr);
		} else
			fwd_subj = g_strdup ("[Fwd: No Subject]");
	}

	return fwd_subj;
}

struct _camel_header_raw *
mail_tool_remove_xevolution_headers (CamelMimeMessage *message)
{
	struct _camel_header_raw *scan, *list = NULL;

	for (scan = ((CamelMimePart *) message)->headers; scan; scan = scan->next)
		if (!strncmp (scan->name, "X-Evolution", 11))
			camel_header_raw_append (&list, scan->name, scan->value, scan->offset);

	for (scan = list; scan; scan = scan->next)
		camel_medium_remove_header ((CamelMedium *) message, scan->name);

	return list;
}

void
mail_tool_restore_xevolution_headers (CamelMimeMessage *message,
                                      struct _camel_header_raw *xev)
{
	CamelMedium *medium;

	medium = CAMEL_MEDIUM (message);

	for (; xev; xev = xev->next)
		camel_medium_add_header (medium, xev->name, xev->value);
}

CamelMimePart *
mail_tool_make_message_attachment (CamelMimeMessage *message)
{
	CamelMimePart *part;
	const gchar *subject;
	struct _camel_header_raw *xev;
	gchar *desc;

	subject = camel_mime_message_get_subject (message);
	if (subject)
		desc = g_strdup_printf (_("Forwarded message - %s"), subject);
	else
		desc = g_strdup (_("Forwarded message"));

	/* rip off the X-Evolution headers */
	xev = mail_tool_remove_xevolution_headers (message);
	camel_header_raw_clear (&xev);

	/* remove Bcc headers */
	camel_medium_remove_header (CAMEL_MEDIUM (message), "Bcc");

	part = camel_mime_part_new ();
	camel_mime_part_set_disposition (part, "inline");
	camel_mime_part_set_description (part, desc);
	camel_medium_set_content (
		CAMEL_MEDIUM (part), CAMEL_DATA_WRAPPER (message));
	camel_mime_part_set_content_type (part, "message/rfc822");
	g_free (desc);

	return part;
}
