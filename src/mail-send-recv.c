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
 *		Michael Zucchi <NotZed@ximian.com>
 *
 * Copyright (C) 1999-2008 Novell, Inc. (www.novell.com)
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include <glib/gi18n.h>

#include <e-util/e-util.h>

/* This is our hack, not part of libcamel. */
#include <libemail-engine/camel-null-store.h>

#include <libedataserver/libedataserver.h>
#include <libemail-engine/e-mail-folder-utils.h>
#include <libemail-engine/e-mail-session.h>
#include <libemail-engine/mail-folder-cache.h>
#include <libemail-engine/mail-mt.h>
#include <libemail-engine/mail-ops.h>
#include <libemail-engine/mail-tools.h>

#include "libemail-engine/e-mail-utils.h"
#include "libemail-engine/e-mail-enums.h"
#include "e-mail-data-session.h"

#include "mail-send-recv.h"
#include "utils.h"

/* We would need it sometime. */
extern EMailDataSession *data_session;
extern ESourceRegistry *source_registry;

#define d(x)

#define E_FILTER_SOURCE_INCOMING "incoming" /* performed on incoming email */
#define E_FILTER_SOURCE_OUTGOING  "outgoing"/* performed on outgoing mail */

/* ms between status updates to the gui */
#define STATUS_TIMEOUT (250)

/* pseudo-uri to key the send task on */
#define SEND_URI_KEY "send-task:"

/* Prefix for window size GConf keys */
#define GCONF_KEY_PREFIX "/apps/evolution/mail/send_recv"

/* send/receive email */

/* ********************************************************************** */
/*  This stuff below is independent of the stuff above */

/* this stuff is used to keep track of which folders filters have accessed, and
 * what not. the thaw/refreeze thing doesn't really seem to work though */
struct _folder_info {
	gchar *uri;
	CamelFolder *folder;
	time_t update;

	/* How many times updated, to slow it
	 * down as we go, if we have lots. */
	gint count;
};

struct _send_data {
	GList *infos;

	gint cancelled;

	/* Since we're never asked to update
	 * this one, do it ourselves. */
	CamelFolder *inbox;
	time_t inbox_update;

	GMutex *lock;
	GHashTable *folders;

	GHashTable *active;	/* send_info's by uri */
};

typedef enum {
	SEND_RECEIVE,		/* receiver */
	SEND_SEND,		/* sender */
	SEND_UPDATE,		/* imap-like 'just update folder info' */
	SEND_INVALID
} send_info_t;

typedef enum {
	SEND_ACTIVE,
	SEND_CANCELLED,
	SEND_COMPLETE
} send_state_t;

struct _send_info {
	send_info_t type;		/* 0 = fetch, 1 = send */
	EMailSession *session;
	GCancellable *cancellable;
	CamelService *service;
	gboolean keep_on_server;
	send_state_t state;

	gint again;		/* need to run send again */

	gint timeout_id;
	gchar *what;
	gint pc;

	gchar *send_url;

	/*time_t update;*/
	struct _send_data *data;
};

static CamelFolder *
		receive_get_folder		(CamelFilterDriver *d,
						 const gchar *uri,
						 gpointer data,
						 GError **error);
static void	send_done (gpointer data);

static struct _send_data *send_data = NULL;

static void
free_folder_info (struct _folder_info *info)
{
	/*camel_folder_thaw (info->folder);	*/
	mail_sync_folder (info->folder, FALSE, NULL, NULL);
	g_object_unref (info->folder);
	g_free (info->uri);
	g_free (info);
}

static void
free_send_info (struct _send_info *info)
{
	if (info->session)
		g_object_unref (info->session);
	if (info->cancellable)
		g_object_unref (info->cancellable);
	if (info->service != NULL)
		g_object_unref (info->service);
	if (info->timeout_id != 0)
		g_source_remove (info->timeout_id);
	g_free (info->what);
	g_free (info->send_url);
	g_free (info);
}

static struct _send_data *
setup_send_data (EMailSession *session)
{
	struct _send_data *data;

	if (send_data == NULL) {
		send_data = data = g_malloc0 (sizeof (*data));
		data->lock = g_mutex_new ();
		data->folders = g_hash_table_new_full (
			g_str_hash, g_str_equal,
			(GDestroyNotify) NULL,
			(GDestroyNotify) free_folder_info);
		data->inbox =
			e_mail_session_get_local_folder (
			session, E_MAIL_LOCAL_FOLDER_LOCAL_INBOX);
		g_object_ref (data->inbox);
		data->active = g_hash_table_new_full (
			g_str_hash, g_str_equal,
			(GDestroyNotify) g_free,
			(GDestroyNotify) free_send_info);
	}

	return send_data;
}

/*
static void
receive_cancel (struct _send_info *info)
{
	if (info->state == SEND_ACTIVE) {
		g_cancellable_cancel (info->cancellable);
		info->state = SEND_CANCELLED;
	}
}
*/

static void
free_send_data (void)
{
	struct _send_data *data = send_data;

	g_return_if_fail (g_hash_table_size (data->active) == 0);

	if (data->inbox) {
		mail_sync_folder (data->inbox, FALSE, NULL, NULL);
		/*camel_folder_thaw (data->inbox);		*/
		g_object_unref (data->inbox);
	}

	g_list_free (data->infos);
	g_hash_table_destroy (data->active);
	g_hash_table_destroy (data->folders);
	g_mutex_free (data->lock);
	g_free (data);
	send_data = NULL;
}

#if 0
static void
cancel_send_info (gpointer key,
                  struct _send_info *info,
                  gpointer data)
{
	receive_cancel (info);
}

static void
hide_send_info (gpointer key,
                struct _send_info *info,
                gpointer data)
{
	if (info->timeout_id != 0) {
		g_source_remove (info->timeout_id);
		info->timeout_id = 0;
	}
}
#endif

static GStaticMutex status_lock = G_STATIC_MUTEX_INIT;


static void
set_send_status (struct _send_info *info,
                 const gchar *desc,
                 gint pc)
{
	g_static_mutex_lock (&status_lock);

	g_free (info->what);
	info->what = g_strdup (desc);
	info->pc = pc;

	g_static_mutex_unlock (&status_lock);
}

static void
set_transport_service (struct _send_info *info,
                       const gchar *transport_uid)
{
	CamelService *service;

	g_static_mutex_lock (&status_lock);

	service = camel_session_ref_service ((CamelSession *)info->session, transport_uid);

	if (CAMEL_IS_TRANSPORT (service)) {
		if (info->service != NULL)
			g_object_unref (info->service);
		info->service = g_object_ref (service);
	}

	if (service != NULL)
		g_object_unref (service);

	g_static_mutex_unlock (&status_lock);
}

/* for camel operation status */
static void
operation_status (CamelOperation *op,
                  const gchar *what,
                  gint pc,
                  struct _send_info *info)
{
	set_send_status (info, what, pc);
}

/*
static gchar *
format_url (CamelService *service)
{
	CamelProvider *provider;
	CamelSettings *settings;
	const gchar *display_name;
	const gchar *host = NULL;
	const gchar *path = NULL;
 	gchar *pretty_url = NULL;

	provider = camel_service_get_provider (service);
	settings = camel_service_ref_settings (service);
	display_name = camel_service_get_display_name (service);

	if (CAMEL_IS_NETWORK_SETTINGS (settings))
		host = camel_network_settings_get_host (
			CAMEL_NETWORK_SETTINGS (settings));

	if (CAMEL_IS_LOCAL_SETTINGS (settings))
		path = camel_local_settings_get_path (
			CAMEL_LOCAL_SETTINGS (settings));

	g_return_val_if_fail (provider != NULL, NULL);

	if (display_name != NULL && *display_name != '\0') {
		if (host != NULL && *host != '\0')
			pretty_url = g_markup_printf_escaped (	
				"<b>%s (%s)</b>: %s",
				display_name, provider->protocol, host);
		else if (path != NULL)
			pretty_url = g_markup_printf_escaped (
 				"<b>%s (%s)</b>: %s",
				display_name, provider->protocol, path);
		else
			pretty_url = g_markup_printf_escaped (
				"<b>%s (%s)</b>",
				display_name, provider->protocol);

	} else {
		if (host != NULL && *host != '\0')
			pretty_url = g_markup_printf_escaped (
				"<b>%s</b>: %s",
				provider->protocol, host);
		else if (path != NULL)
			pretty_url = g_markup_printf_escaped (				
				"<b>%s</b>: %s",
				provider->protocol, path);				
		else
			pretty_url = g_markup_printf_escaped (
				"<b>%s</b>", provider->protocol);
	}

	g_object_unref (settings);
	return pretty_url;
}
*/

static send_info_t
get_receive_type (CamelService *service)
{
	CamelProvider *provider;
	CamelURL *url;
	const gchar *uid;
	gboolean is_local_delivery;

	/* Disregard CamelNullStores. */
	if (CAMEL_IS_NULL_STORE (service))
		return SEND_INVALID;

	url = camel_service_new_camel_url (service);
	is_local_delivery = em_utils_is_local_delivery_mbox_file (url);
	camel_url_free (url);

	/* mbox pointing to a file is a 'Local delivery'
	 * source which requires special processing. */
	if (is_local_delivery)
		return SEND_RECEIVE;

	provider = camel_service_get_provider (service);

	if (!provider)
		return SEND_INVALID;

	/* skip some well-known services */
	uid = camel_service_get_uid (service);
	if (g_strcmp0 (uid, E_MAIL_SESSION_LOCAL_UID) == 0)
		return SEND_INVALID;
	if (g_strcmp0 (uid, E_MAIL_SESSION_VFOLDER_UID) == 0)
 		return SEND_INVALID;

	if (provider->object_types[CAMEL_PROVIDER_STORE]) {
		if (provider->flags & CAMEL_PROVIDER_IS_STORAGE)
			return SEND_UPDATE;
		else
			return SEND_RECEIVE;
	}

	if (provider->object_types[CAMEL_PROVIDER_TRANSPORT])
		return SEND_SEND;

	return SEND_INVALID;
}

static gint
operation_status_timeout (gpointer data)
{
	//struct _send_info *info = data;

	return FALSE;
}

static CamelService *
ref_default_transport (EMailSession *session)
{
	ESource *source;
	CamelService *service;
	const gchar *extension_name;
	const gchar *uid;

	source = e_source_registry_ref_default_mail_identity (source_registry);

	if (source == NULL)
		return NULL;

	extension_name = E_SOURCE_EXTENSION_MAIL_SUBMISSION;
	if (e_source_has_extension (source, extension_name)) {
		ESourceMailSubmission *extension;
		gchar *uid;

		extension = e_source_get_extension (source, extension_name);
		uid = e_source_mail_submission_dup_transport_uid (extension);

		g_object_unref (source);
		source = e_source_registry_ref_source (source_registry, uid);

		g_free (uid);
	} else {
		g_object_unref (source);
		source = NULL;
	}

	if (source == NULL)
		return NULL;

	uid = e_source_get_uid (source);
	service = camel_session_ref_service (CAMEL_SESSION (session), uid);

	g_object_unref (source);

	return service;
}

static struct _send_data *
build_infra (EMailSession *session, gboolean allow_send)
{
	struct _send_data *data;
	struct _send_info *info;
	CamelService *transport = NULL;
	GList *link;
	GList *list = NULL;
	GList *accounts=NULL;

	transport = ref_default_transport (session);

	accounts = mail_get_all_accounts();

	data = setup_send_data (session);

	link = accounts;
	while (link) {
		ESource *source;
		CamelService *service;
		const gchar *uid;

		
		source = (ESource *)link->data;
		if (!e_source_get_enabled(source)) {
			link = link->next;
			continue;
		}

		uid = e_source_get_uid (source);
		service = camel_session_ref_service (
			CAMEL_SESSION (session), uid);

		/* see if we have an outstanding download active */
		info = g_hash_table_lookup (data->active, uid);
		if (info == NULL) {
			send_info_t type = SEND_INVALID;

			type = get_receive_type (service);

			if (type == SEND_INVALID || type == SEND_SEND) {
				link = link->next;
				continue;
			}

			info = g_malloc0 (sizeof (*info));
			info->type = type;
			info->session = g_object_ref (session);

			d(printf("adding source %s\n", source->url));
			info->service = g_object_ref (service);
			info->keep_on_server = mail_get_keep_on_server (service);
			info->cancellable = camel_operation_new ();
			info->state = allow_send ? SEND_ACTIVE : SEND_COMPLETE;
			info->timeout_id = g_timeout_add (
				STATUS_TIMEOUT, operation_status_timeout, info);

			g_signal_connect (
				info->cancellable, "status",
				G_CALLBACK (operation_status), info);

			g_hash_table_insert (
				data->active, g_strdup(uid), info);
			list = g_list_prepend (list, info);

		} else if (info->timeout_id == 0)
			info->timeout_id = g_timeout_add (
				STATUS_TIMEOUT, operation_status_timeout, info);

		info->data = data;

		link = link->next;
	}

	g_list_free_full (accounts, (GDestroyNotify) g_object_unref);


	/* Skip displaying the SMTP row if we've got no outbox,
	 * outgoing account or unsent mails. */
	CamelFolder *local_outbox;
	local_outbox =
		e_mail_session_get_local_folder (
		session, E_MAIL_LOCAL_FOLDER_OUTBOX);
	
	if (allow_send && local_outbox && CAMEL_IS_TRANSPORT (transport)
	 && (camel_folder_get_message_count (local_outbox) -
		camel_folder_get_deleted_message_count (local_outbox)) != 0) {
		info = g_hash_table_lookup (data->active, SEND_URI_KEY);
		if (info == NULL) {
			info = g_malloc0 (sizeof (*info));
			info->type = SEND_SEND;

			info->service = g_object_ref (transport);
			info->keep_on_server = FALSE;
			info->cancellable = camel_operation_new ();
			info->state = SEND_ACTIVE;
			info->timeout_id = g_timeout_add (
				STATUS_TIMEOUT, operation_status_timeout, info);

			g_signal_connect (
				info->cancellable, "status",
				G_CALLBACK (operation_status), info);

			g_hash_table_insert (data->active, g_strdup(SEND_URI_KEY), info);
			list = g_list_prepend (list, info);
		} else if (info->timeout_id == 0)
			info->timeout_id = g_timeout_add (
				STATUS_TIMEOUT, operation_status_timeout, info);

		info->data = data;

	}

	data->infos = list;

	return data;
}

static void
update_folders (gchar *uri,
                struct _folder_info *info,
                gpointer data)
{
	time_t now = *((time_t *) data);

	d(printf("checking update for folder: %s\n", info->uri));

	/* let it flow through to the folders every 10 seconds */
	/* we back off slowly as we progress */
	if (now > info->update + 10 + info->count *5) {
		d(printf("upating a folder: %s\n", info->uri));
		/*camel_folder_thaw(info->folder);
		  camel_folder_freeze (info->folder);*/
		info->update = now;
		info->count++;
	}
}

static void
receive_status (CamelFilterDriver *driver,
                enum camel_filter_status_t status,
                gint pc,
                const gchar *desc,
                gpointer data)
{
	struct _send_info *info = data;
	time_t now = time (NULL);

	/* let it flow through to the folder, every now and then too? */
	g_hash_table_foreach (info->data->folders, (GHFunc) update_folders, &now);

	if (info->data->inbox && now > info->data->inbox_update + 20) {
		d(printf("updating inbox too\n"));
		/* this doesn't seem to work right :( */
		/*camel_folder_thaw(info->data->inbox);
		  camel_folder_freeze (info->data->inbox);*/
		info->data->inbox_update = now;
	}

	/* we just pile them onto the port, assuming it can handle it.
	 * We could also have a receiver port and see if they've been processed
	 * yet, so if this is necessary its not too hard to add */
	/* the mail_gui_port receiver will free everything for us */
	switch (status) {
	case CAMEL_FILTER_STATUS_START:
	case CAMEL_FILTER_STATUS_END:
		set_send_status (info, desc, pc);
		break;
	case CAMEL_FILTER_STATUS_ACTION:
		set_transport_service (info, desc);
		break;
	default:
		break;
	}
}

/* when receive/send is complete */
static void
receive_done (gint still_more, gpointer data)
{
	struct _send_info *info = data;
	const gchar *uid;

	uid = camel_service_get_uid (info->service);
	g_return_if_fail (uid != NULL);

	/* if we've been called to run again - run again */
	if (info->type == SEND_SEND && info->state == SEND_ACTIVE && info->again) {
		EMailSession *session;
		CamelFolder *local_outbox;

		session = info->session;
		local_outbox =
			e_mail_session_get_local_folder (
			session, E_MAIL_LOCAL_FOLDER_OUTBOX);

		g_return_if_fail (CAMEL_IS_TRANSPORT (info->service));

		info->again = 0;
		mail_send_queue (
			info->session,
			local_outbox,
			CAMEL_TRANSPORT (info->service),
			E_FILTER_SOURCE_OUTGOING,
			info->cancellable,
			receive_get_folder, info,
			receive_status, info,
			send_done, info);
		return;
	}

	//FIXME Set SEND completed here
	/*	if (info->state == SEND_CANCELLED)
			text = _("Canceled.");
		else {
			text = _("Complete.");
			info->state = SEND_COMPLETE;
		}
	*/

	/* remove/free this active download */
	d(printf("%s: freeing info %p\n", G_STRFUNC, info));
	if (info->type == SEND_SEND)
		g_hash_table_steal (info->data->active, SEND_URI_KEY);
	else
		g_hash_table_steal (info->data->active, uid);
	info->data->infos = g_list_remove (info->data->infos, info);

	if (g_hash_table_size (info->data->active) == 0) {
		//FIXME: THIS MEANS SEND RECEIVE IS COMPLETED
		free_send_data ();
	}

	free_send_info (info);
}

static void
send_done (gpointer data)
{
	receive_done (-1, data);
}
/* although we dont do anythign smart here yet, there is no need for this interface to
 * be available to anyone else.
 * This can also be used to hook into which folders are being updated, and occasionally
 * let them refresh */
static CamelFolder *
receive_get_folder (CamelFilterDriver *d,
                    const gchar *uri,
                    gpointer data,
                    GError **error)
{
	struct _send_info *info = data;
	CamelFolder *folder;
	EMailSession *session;
	struct _folder_info *oldinfo;
	gpointer oldkey, oldinfoptr;

	g_mutex_lock (info->data->lock);
	oldinfo = g_hash_table_lookup (info->data->folders, uri);
	g_mutex_unlock (info->data->lock);

	if (oldinfo) {
		g_object_ref (oldinfo->folder);
		return oldinfo->folder;
	}

	session = info->session;

	/* FIXME Not passing a GCancellable here. */
	folder = e_mail_session_uri_to_folder_sync (
		session, uri, 0, NULL, error);
	if (!folder)
		return NULL;

	/* we recheck that the folder hasn't snuck in while we were loading it... */
	/* and we assume the newer one is the same, but unref the old one anyway */
	g_mutex_lock (info->data->lock);

	if (g_hash_table_lookup_extended (
			info->data->folders, uri, &oldkey, &oldinfoptr)) {
		oldinfo = (struct _folder_info *) oldinfoptr;
		g_object_unref (oldinfo->folder);
		oldinfo->folder = folder;
	} else {
		oldinfo = g_malloc0 (sizeof (*oldinfo));
		oldinfo->folder = folder;
		oldinfo->uri = g_strdup (uri);
		g_hash_table_insert (info->data->folders, oldinfo->uri, oldinfo);
	}

	g_object_ref (folder);

	g_mutex_unlock (info->data->lock);

	return folder;
}

/* ********************************************************************** */

static void
get_folders (CamelStore *store,
             GPtrArray *folders,
             CamelFolderInfo *info)
{
	while (info) {
		if (camel_store_can_refresh_folder (store, info, NULL)) {
			if ((info->flags & CAMEL_FOLDER_NOSELECT) == 0) {
				gchar *folder_uri;

				folder_uri = e_mail_folder_uri_build (
					store, info->full_name);
				g_ptr_array_add (folders, folder_uri);
			}
		}

		get_folders (store, folders, info->child);
		info = info->next;
	}
}

static void
main_op_cancelled_cb (GCancellable *main_op,
                      GCancellable *refresh_op)
{
	g_cancellable_cancel (refresh_op);
}

struct _refresh_folders_msg {
	MailMsg base;

	struct _send_info *info;
	GPtrArray *folders;
	CamelStore *store;
	CamelFolderInfo *finfo;
};

static gchar *
refresh_folders_desc (struct _refresh_folders_msg *m)
{
	return g_strdup_printf(_("Checking for new mail"));
}

static void
refresh_folders_exec (struct _refresh_folders_msg *m,
                      GCancellable *cancellable,
                      GError **error)
{
	CamelFolder *folder;
	EMailSession *session;
	gint i;
	GError *local_error = NULL;
	gulong handler_id = 0;

	if (cancellable)
		handler_id = g_signal_connect (
			m->info->cancellable, "cancelled",
			G_CALLBACK (main_op_cancelled_cb), cancellable);

	get_folders (m->store, m->folders, m->finfo);

	camel_operation_push_message (m->info->cancellable, _("Updating..."));

	session = m->info->session;

	for (i = 0; i < m->folders->len; i++) {
		folder = e_mail_session_uri_to_folder_sync (
			session,
			m->folders->pdata[i], 0,
			cancellable, &local_error);
		if (folder) {
			/* FIXME Not passing a GError here. */
			camel_folder_synchronize_sync (
				folder, FALSE, cancellable, NULL);
			camel_folder_refresh_info_sync (folder, cancellable, NULL);
			g_object_unref (folder);
		} else if (local_error != NULL) {
			g_warning ("Failed to refresh folders: %s", local_error->message);
			g_clear_error (&local_error);
		}

		if (g_cancellable_is_cancelled (m->info->cancellable))
			break;

		if (m->info->state != SEND_CANCELLED)
			camel_operation_progress (
				m->info->cancellable, 100 * i / m->folders->len);
	}

	camel_operation_pop_message (m->info->cancellable);

	if (cancellable)
		g_signal_handler_disconnect (m->info->cancellable, handler_id);
}

static void
refresh_folders_done (struct _refresh_folders_msg *m)
{
	receive_done (-1, m->info);
}

static void
refresh_folders_free (struct _refresh_folders_msg *m)
{
	gint i;

	for (i = 0; i < m->folders->len; i++)
		g_free (m->folders->pdata[i]);
	g_ptr_array_free (m->folders, TRUE);

	camel_store_free_folder_info (m->store, m->finfo);
	g_object_unref (m->store);
}

static MailMsgInfo refresh_folders_info = {
	sizeof (struct _refresh_folders_msg),
	(MailMsgDescFunc) refresh_folders_desc,
	(MailMsgExecFunc) refresh_folders_exec,
	(MailMsgDoneFunc) refresh_folders_done,
	(MailMsgFreeFunc) refresh_folders_free
};

static gboolean
receive_update_got_folderinfo (MailFolderCache *folder_cache,
                               CamelStore *store,
                               CamelFolderInfo *info,
                               gpointer data)
{
	if (info) {
		GPtrArray *folders = g_ptr_array_new ();
		struct _refresh_folders_msg *m;
		struct _send_info *sinfo = data;

		m = mail_msg_new (&refresh_folders_info);
		m->store = store;
		g_object_ref (store);
		m->folders = folders;
		m->info = sinfo;
		m->finfo = info;

		mail_msg_unordered_push (m);

		/* do not free folder info, we will free it later */
		return FALSE;
	} else {
		receive_done (-1, data);
	}

	return TRUE;
}

static void
receive_update_got_store (CamelStore *store,
                          struct _send_info *info)
{
	EMailSession *session;
	MailFolderCache *folder_cache;

	session = info->session;
	folder_cache = e_mail_session_get_folder_cache (session);

	if (store) {
		mail_folder_cache_note_store (
			folder_cache,
			store, info->cancellable,
			receive_update_got_folderinfo, info);
	} else {
		receive_done (-1, info);
	}
}

static void
send_receive (EMailSession *session,
              gboolean allow_send)
{
	CamelFolder *local_outbox;
	struct _send_data *data;
	GList *scan;
	
	if (send_data) /* Send Receive is already in progress */
		return;

	if (!camel_session_get_online (CAMEL_SESSION (session)))
		return;
	local_outbox = e_mail_session_get_local_folder (
			session, E_MAIL_LOCAL_FOLDER_OUTBOX);

	data = build_infra (session, allow_send);

	for (scan = data->infos; scan != NULL; scan = scan->next) {
		struct _send_info *info = scan->data;

		if (!CAMEL_IS_SERVICE (info->service))
			continue;

		switch (info->type) {
		case SEND_RECEIVE:
			mail_fetch_mail (
				CAMEL_STORE (info->service),
				CAMEL_FETCH_OLD_MESSAGES, -1,
				E_FILTER_SOURCE_INCOMING,
				mail_provider_fetch_lock, mail_provider_fetch_unlock, mail_provider_fetch_inbox_folder,
				info->cancellable,
				receive_get_folder, info,
				receive_status, info,
				receive_done, info);
			break;
		case SEND_SEND:
			/* todo, store the folder in info? */
			mail_send_queue (
				session, local_outbox,
				CAMEL_TRANSPORT (info->service),
				E_FILTER_SOURCE_OUTGOING,
				info->cancellable,
				receive_get_folder, info,
				receive_status, info,
				send_done, info);
			break;
		case SEND_UPDATE:
			receive_update_got_store (
				CAMEL_STORE (info->service), info);
			break;
		default:
			break;
		}
	}

	return ;
}

void
mail_send_receive (EMailSession *session)
{
	return send_receive (session, TRUE);
}

void
mail_receive (EMailSession *session)
{
	return send_receive (session, FALSE);
}

struct _auto_data {
	ESource *account;
	EMailSession *session;
	gint period;		/* in seconds */
	gint timeout_id;
};

static GHashTable *auto_active;

static gboolean
auto_timeout (gpointer data)
{
	EMailSession *session;
	struct _auto_data *info = data;
	CamelService *service;
	const char *uid;

	session = info->session;
	uid = e_source_get_uid (info->account);
	
	service = camel_session_ref_service (
		CAMEL_SESSION (session), uid);
	printf("Timeout for %s\n", uid);
	g_return_val_if_fail (CAMEL_IS_SERVICE (service), TRUE);


	if (camel_session_get_online (CAMEL_SESSION (session)))
		mail_receive_service (service);

	return TRUE;
}

static gboolean
auto_timeout_once (gpointer data)
{
	auto_timeout (data);

	return FALSE;
}

static void
auto_account_removed (ESourceRegistry *registry,
                      ESource *source,
                      gpointer dummy)
{
	struct _auto_data *info = g_object_get_data((GObject *)source, "mail-autoreceive");

	g_return_if_fail (info != NULL);

	if (info->timeout_id) {
		g_source_remove (info->timeout_id);
		info->timeout_id = 0;
	}
	if (data_session)
		e_mail_session_emit_account_removed (data_session, e_source_get_uid(source));		
}

static void
auto_account_finalized (struct _auto_data *info)
{
	if (info->session != NULL)
		g_object_unref (info->session);
	if (info->timeout_id)
		g_source_remove (info->timeout_id);
	g_free (info);
}

static void
auto_account_commit (struct _auto_data *info)
{
	gint period, check;
	const gchar *extension_name;
	ESourceExtension *extension;

	extension_name = E_SOURCE_EXTENSION_REFRESH;
	extension = e_source_get_extension (info->account, extension_name);

	check = e_source_get_enabled (info->account) 
		&& e_source_refresh_get_enabled ((ESourceRefresh *)extension);

	period = e_source_refresh_get_interval_minutes ((ESourceRefresh *)extension) * 60;
	period = MAX (60, period);

	if (info->timeout_id
	    && (!check
		|| period != info->period)) {
		g_source_remove (info->timeout_id);
		info->timeout_id = 0;
	}
	info->period = period;
	if (check && info->timeout_id == 0)
		info->timeout_id = g_timeout_add_seconds (info->period, auto_timeout, info);
}

static void
auto_account_added (ESourceRegistry *registry,
                    ESource *source,
                    EMailSession *session)
{
	struct _auto_data *info;

	if (!e_source_get_enabled(source))
		return;

	info = g_malloc0 (sizeof (*info));
	info->account = source;
	info->session = g_object_ref (session);
	g_object_set_data_full (
		G_OBJECT (source), "mail-autoreceive", info,
		(GDestroyNotify) auto_account_finalized);
	auto_account_commit (info);
	if (data_session)
		e_mail_session_emit_account_added (data_session, e_source_get_uid(source));	
}

static void
auto_account_changed (ESourceRegistry *registry,
		      ESource *source,
                      gpointer dummy)
{
	struct _auto_data *info = g_object_get_data((GObject *)source, "mail-autoreceive");

	g_return_if_fail (info != NULL);

	auto_account_commit (info);
	if (data_session)
		e_mail_session_emit_account_changed (data_session, e_source_get_uid (source));	
}

static void
auto_online (EMailSession *session)
{
	struct _auto_data *info;
	gboolean can_update_all;
	GList *accounts = NULL;
	GList *link;

	accounts = mail_get_store_accounts();

	link = accounts;
	while (link) {
		ESource *source = (ESource *)link->data;
		if (!e_source_get_enabled(source))
			continue;

		info = g_object_get_data (
			G_OBJECT (source), "mail-autoreceive");
		if (info && (info->timeout_id || can_update_all))
			g_idle_add (auto_timeout_once, info);

		link = link->next;
	}

	g_list_free_full (accounts, (GDestroyNotify) g_object_unref);
}

/* call to setup initial, and after changes are made to the config */
/* FIXME: Need a cleanup funciton for when object is deactivated */
void
mail_autoreceive_init (EMailSession *session)
{
	GList *accounts = NULL;
	GList *link;

	if (auto_active)
		return;

	accounts = mail_get_store_accounts();

	auto_active = g_hash_table_new (g_str_hash, g_str_equal);

	g_signal_connect (
		source_registry, "source-added",
		G_CALLBACK (auto_account_added), session);
	g_signal_connect (
		source_registry, "source-enabled",
		G_CALLBACK (auto_account_added), session);
	g_signal_connect (
		source_registry, "source-removed",
		G_CALLBACK (auto_account_removed), NULL);
	g_signal_connect (
		source_registry, "source-disabled",
		G_CALLBACK (auto_account_removed), NULL);

	g_signal_connect (
		source_registry, "source-changed",
		G_CALLBACK (auto_account_changed), NULL);

	link = accounts;
	while (link) {
		auto_account_added (
			source_registry, (ESource *) link->data,
			session);
		link = link->next;
	}

	if (1) {
		auto_online (session);

		/* also flush outbox on start */
		mail_send (session);
	}

	/* Accounts shouldn't free, that is what runs the account. */
	g_list_free (accounts);
	
	/* FIXME: Check for online status and sync after online */
}

/* We setup the download info's in a hashtable, if we later
 * need to build the gui, we insert them in to add them. */
GCancellable *
mail_receive_service (CamelService *service)
{
	struct _send_info *info;
	struct _send_data *data;
	CamelFolder *local_outbox;
	const gchar *uid;
	EMailSession *session;

	send_info_t type = SEND_INVALID;

	g_return_val_if_fail (CAMEL_IS_SERVICE (service), NULL);

	session = (EMailSession *)camel_service_get_session (service);
	uid = camel_service_get_uid (service);

	data = setup_send_data (session);
	info = g_hash_table_lookup (data->active, uid);

	if (info != NULL)
		return info->cancellable;

	type = get_receive_type (service);

	if (type == SEND_INVALID || type == SEND_SEND)
		return NULL;

	info = g_malloc0 (sizeof (*info));
	info->type = type;
	info->session = g_object_ref (session);
	info->service = g_object_ref (service);
	info->keep_on_server = mail_get_keep_on_server (service);	
	info->cancellable = camel_operation_new ();
	info->data = data;
	info->state = SEND_ACTIVE;
	info->timeout_id = 0;

	g_signal_connect (
		info->cancellable, "status",
		G_CALLBACK (operation_status), info);

	d(printf("Adding new info %p\n", info));

	g_hash_table_insert (data->active, g_strdup(uid), info);

	switch (info->type) {
	case SEND_RECEIVE:
		mail_fetch_mail (
			CAMEL_STORE (service),
			CAMEL_FETCH_OLD_MESSAGES, -1,
			E_FILTER_SOURCE_INCOMING,
			mail_provider_fetch_lock, mail_provider_fetch_unlock, mail_provider_fetch_inbox_folder,
			info->cancellable,
			receive_get_folder, info,
			receive_status, info,
			receive_done, info);
		break;
	case SEND_SEND:
		/* todo, store the folder in info? */
		local_outbox =
			e_mail_session_get_local_folder (
			session, E_MAIL_LOCAL_FOLDER_OUTBOX);
		mail_send_queue (
			info->session,
			local_outbox,
			CAMEL_TRANSPORT (service),
			E_FILTER_SOURCE_OUTGOING,
			info->cancellable,
			receive_get_folder, info,
			receive_status, info,
			send_done, info);
		break;
	case SEND_UPDATE:
		receive_update_got_store (CAMEL_STORE (service), info);
		break;
	default:
		g_return_val_if_reached (NULL);
	}

	return info->cancellable;
}

GCancellable *
mail_receive_account (EMailSession *session,
                      ESource *account)
{
	CamelService *service;

	service = camel_session_ref_service (
		CAMEL_SESSION (session), e_source_get_uid(account));

	return mail_receive_service (service);
}

GCancellable *
mail_send (EMailSession *session)
{
	CamelFolder *local_outbox;
	CamelService *service;
	struct _send_info *info;
	struct _send_data *data;
	send_info_t type = SEND_INVALID;
	const gchar *transport_uid;
	ESource *account;
	const gchar *extension_name;

	account = e_source_registry_ref_default_mail_identity (source_registry);

	if (account == NULL)
		return NULL;

	extension_name = E_SOURCE_EXTENSION_MAIL_SUBMISSION;
	if (e_source_has_extension (account, extension_name)) {
		ESourceMailSubmission *extension;
		gchar *uid;

		extension = e_source_get_extension (account, extension_name);
		uid = e_source_mail_submission_dup_transport_uid (extension);

		g_object_unref (account);
		account = e_source_registry_ref_source (source_registry, uid);

		g_free (uid);
	} else {
		g_object_unref (account);
		account = NULL;
	}

	if (account == NULL)
		return NULL;

	transport_uid = e_source_get_uid (account);

	data = setup_send_data (session);
	info = g_hash_table_lookup (data->active, SEND_URI_KEY);
	if (info != NULL) {
		info->again++;
		d(printf("send of %s still in progress\n", transport_uid));
		return info->cancellable;
	}

	service = camel_session_ref_service (
		CAMEL_SESSION (session), transport_uid);
	if (!CAMEL_IS_TRANSPORT (service)) {
 		return NULL;
	}

	d(printf("starting non-interactive send of '%s'\n", account->transport->url));
	type = get_receive_type (service);

	if (type == SEND_INVALID) {
		return NULL;
	}

	info = g_malloc0 (sizeof (*info));
	info->type = SEND_SEND;
	info->session = g_object_ref (session);
	info->service = g_object_ref (service);
	info->keep_on_server = FALSE;
	info->cancellable = camel_operation_new();
	info->data = data;
	info->state = SEND_ACTIVE;
	info->timeout_id = 0;

	d(printf("Adding new info %p\n", info));

	g_hash_table_insert (data->active, g_strdup(SEND_URI_KEY), info);

	/* todo, store the folder in info? */
	local_outbox =
		e_mail_session_get_local_folder (
		session, E_MAIL_LOCAL_FOLDER_OUTBOX);

	mail_send_queue (
		session, local_outbox,
		CAMEL_TRANSPORT (service),
		E_FILTER_SOURCE_OUTGOING,
		info->cancellable,
		receive_get_folder, info,
		receive_status, info,
		send_done, info);

	return info->cancellable;
}
