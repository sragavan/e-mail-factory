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
 *   Peter Williams <peterw@ximian.com>
 *   Michael Zucchi <notzed@ximian.com>
 *   Jonathon Jongsma <jonathon.jongsma@collabora.co.uk>
 *
 * Copyright (C) 1999-2008 Novell, Inc. (www.novell.com)
 * Copyright (C) 2009 Intel Corporation
 *
 */

#ifndef MAIL_FOLDER_CACHE_H
#define MAIL_FOLDER_CACHE_H

#include <camel/camel.h>

/* Standard GObject macros */
#define MAIL_TYPE_FOLDER_CACHE \
	(mail_folder_cache_get_type ())
#define MAIL_FOLDER_CACHE(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), MAIL_TYPE_FOLDER_CACHE, MailFolderCache))
#define MAIL_FOLDER_CACHE_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), MAIL_TYPE_FOLDER_CACHE, MailFolderCacheClass))
#define MAIL_IS_FOLDER_CACHE(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), MAIL_TYPE_FOLDER_CACHE))
#define MAIL_IS_FOLDER_CACHE_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), MAIL_TYPE_FOLDER_CACHE))
#define MAIL_FOLDER_CACHE_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), MAIL_TYPE_FOLDER_CACHE, MailFolderCacheClass))

G_BEGIN_DECLS

/* Avoid a circular dependency. */
struct _EMailSession;

typedef struct _MailFolderCache MailFolderCache;
typedef struct _MailFolderCacheClass MailFolderCacheClass;
typedef struct _MailFolderCachePrivate MailFolderCachePrivate;

/**
 * NoteDoneFunc:
 *
 * The signature of a function to be registered as a callback for
 * mail_folder_cache_note_store()
 */
typedef gboolean	(*NoteDoneFunc)		(MailFolderCache *cache,
						 CamelStore *store,
						 CamelFolderInfo *info,
						 gpointer data);

/**
 * MailFolderCache:
 *
 * Contains only private data that should be read and manipulated using the
 * functions below.
 */
struct _MailFolderCache {
	GObject parent;
	MailFolderCachePrivate *priv;
};

struct _MailFolderCacheClass {
	GObjectClass parent_class;

	/* Signals */
	void		(*folder_available)	(MailFolderCache *cache,
						 CamelStore *store,
						 const gchar *folder_name);
	void		(*folder_unavailable)	(MailFolderCache *cache,
						 CamelStore *store,
						 const gchar *folder_name);
	void		(*folder_deleted)	(MailFolderCache *cache,
						 CamelStore *store,
						 const gchar *folder_name);
	void		(*folder_renamed)	(MailFolderCache *cache,
						 CamelStore *store,
						 const gchar *old_folder_name,
						 const gchar *new_folder_name);
	void		(*folder_unread_updated)
						(MailFolderCache *cache,
						 CamelStore *store,
						 const gchar *folder_name,
						 gint unread);
	void		(*folder_changed)	(MailFolderCache *cache,
						 CamelStore *store,
						 gint new_messages,
						 const gchar *msg_uid,
						 const gchar *msg_sender,
						 const gchar *msg_subject);
};

GType		mail_folder_cache_get_type	(void) G_GNUC_CONST;
MailFolderCache *
		mail_folder_cache_new		(struct _EMailSession *session);
struct _EMailSession *
		mail_folder_cache_get_session	(MailFolderCache *cache);
void		mail_folder_cache_note_store	(MailFolderCache *cache,
						 CamelStore *store,
						 GCancellable *cancellable,
						 NoteDoneFunc done,
						 gpointer data);
void		mail_folder_cache_note_folder	(MailFolderCache *cache,
						 CamelFolder *folder);
gboolean	mail_folder_cache_get_folder_from_uri
						(MailFolderCache *cache,
						 const gchar *uri,
						 CamelFolder **folderp);
gboolean	mail_folder_cache_get_folder_info_flags
						(MailFolderCache *cache,
						 CamelFolder *folder,
						 CamelFolderInfoFlags *flags);
gboolean	mail_folder_cache_get_folder_has_children
						(MailFolderCache *cache,
						 CamelFolder *folder,
						 gboolean *found);
void		mail_folder_cache_get_local_folder_uris
						(MailFolderCache *cache,
						 GQueue *out_queue);
void		mail_folder_cache_get_remote_folder_uris
						(MailFolderCache *cache,
						 GQueue *out_queue);
void		mail_folder_cache_service_removed
						(MailFolderCache *cache,
						 CamelService *service);
void		mail_folder_cache_service_enabled
						(MailFolderCache *cache,
						 CamelService *service);
void		mail_folder_cache_service_disabled
						(MailFolderCache *cache,
						 CamelService *service);

G_END_DECLS

#endif /* MAIL_FOLDER_CACHE_H */
