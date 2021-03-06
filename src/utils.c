#include <glib/gi18n.h>
#include <string.h>
#include <glib.h>
#include "utils.h"
#include "libemail-engine/mail-mt.h"
#include "libemail-engine/e-mail-session.h"
#include <libedataserver/libedataserver.h>

/* 
 * EDS_MAIL_DEBUG should be a CSV
 * export EDS_MAIL_DEBUG=folder,store,session,micro,ipc
 * */

static int mail_debug_flag = 0;
extern EMailSession *session;  
extern ESourceRegistry *source_registry;

void
mail_debug_init ()
{
	const char *log = g_getenv ("EDS_MAIL_DEBUG");
	char **tokens;

	if (log && *log) {
		int i=0;
		tokens = g_strsplit (log, ",", 0);
		
		while (tokens[i]) {
			if (strcmp (tokens[i], "folder") == 0)
				mail_debug_flag |= EMAIL_DEBUG_FOLDER;
			else if (strcmp (tokens[i], "store") == 0)
				mail_debug_flag |= EMAIL_DEBUG_STORE;
			else if (strcmp (tokens[i], "session") == 0)
				mail_debug_flag |= EMAIL_DEBUG_SESSION;
			else if (strcmp (tokens[i], "operation") == 0)
				mail_debug_flag |= EMAIL_DEBUG_OPERATION;			
			else if (strcmp (tokens[i], "micro") == 0)
				mail_debug_flag |= EMAIL_DEBUG_MICRO;		
			else if (strcmp (tokens[i], "ipc") == 0)
				mail_debug_flag |= EMAIL_DEBUG_IPC;						
			else if (strcmp(tokens[i], "all") == 0)
				mail_debug_flag |= EMAIL_DEBUG_SESSION|EMAIL_DEBUG_STORE|EMAIL_DEBUG_STORE|EMAIL_DEBUG_MICRO|EMAIL_DEBUG_IPC;
			i++;
		}

		g_strfreev (tokens);
	}
}

gboolean
mail_debug_log (EMailDebugFlag flag)
{
	return (mail_debug_flag & flag) != 0;
}

char *
mail_get_service_url (CamelService *service)
{
	char *url;	
	CamelURL *curl;

	curl = camel_service_new_camel_url (service);
	url = camel_url_to_string (curl, CAMEL_URL_HIDE_ALL);
	camel_url_free (curl);

	return url;
}

/* Multi Operation holder */


struct _multi_op_object_msg {
	MailMsg base;

	GObject *object;
	gboolean ret;
	gboolean (*do_op) (GObject *object, gpointer data, GError **error);	
	void (*done) (gboolean success, gpointer data, GError *error);
	gpointer data;
};

static gchar *
multi_op_object_desc (struct _multi_op_object_msg *m)
{
	gchar *res;

	if (CAMEL_IS_SERVICE(m->object))
		res = g_strdup_printf(_("Operating on store '%p'"), camel_service_get_name ((CamelService *)m->object, TRUE));
	else if (CAMEL_IS_FOLDER (m->object))
		res = g_strdup_printf(_("Operating on folder '%p'"), camel_folder_get_full_name((CamelFolder *)m->object));
	else
		res = g_strdup_printf(_("Operating on object '%p'"), m->object);

	return res;
}

static void
multi_op_object_exec (struct _multi_op_object_msg *m)
{
	m->ret = m->do_op (m->object, m->data, &m->base.error);
}

static void
multi_op_object_done (struct _multi_op_object_msg *m)
{
	if (m->done)
		m->done(m->ret, m->data, m->base.error);
}

static void
multi_op_object_free (struct _multi_op_object_msg *m)
{
	g_object_unref (m->object);
}

static MailMsgInfo multi_op_object = {
	sizeof (struct _multi_op_object_msg),
	(MailMsgDescFunc) multi_op_object_desc,
	(MailMsgExecFunc) multi_op_object_exec,
	(MailMsgDoneFunc) multi_op_object_done,
	(MailMsgFreeFunc) multi_op_object_free
};

void
mail_operate_on_object (GObject *object, 
		        GCancellable *cancellable,
		        gboolean (*do_op) (GObject *object, gpointer data, GError **error),
		        void (*done) (gboolean ret, gpointer data, GError *error), 
		        gpointer data)
{
	struct _multi_op_object_msg *m;

	m = mail_msg_new(&multi_op_object);
	m->object= object;
	g_object_ref (object);
	m->data = data;
	m->do_op = do_op;
	m->done = done;
	if (G_IS_CANCELLABLE (cancellable))
		m->base.cancellable = cancellable;
	mail_msg_unordered_push (m);
}

/* POP/SMS Provider Utility functions */

/* non storage store has to be sequential between same accounts. We'll use this to lock it*/
static GStaticMutex provider_hash_lock = G_STATIC_MUTEX_INIT;
static GHashTable *provider_hash = NULL;

static void
provider_mutex_free (GMutex *lock)
{
	g_mutex_free (lock);
}

static GHashTable *
mail_get_provider_hash ()
{
	g_static_mutex_lock (&provider_hash_lock);
	if (!provider_hash)
		provider_hash = g_hash_table_new_full ( g_str_hash, g_str_equal, (GDestroyNotify)g_free, (GDestroyNotify)provider_mutex_free);

	g_static_mutex_unlock (&provider_hash_lock);	

	return provider_hash;
}

GList *
mail_get_all_accounts ()
{
	GList *list;
	GList *accounts=NULL;
	const gchar *extension_name;

	extension_name = E_SOURCE_EXTENSION_MAIL_ACCOUNT;
	list = e_source_registry_list_sources (source_registry, extension_name);
	accounts = g_list_concat (accounts, list);

	extension_name = E_SOURCE_EXTENSION_MAIL_TRANSPORT;
	list = e_source_registry_list_sources (source_registry, extension_name);
	accounts = g_list_concat (accounts, list);

	return accounts;
}

GList *
mail_get_transport_accounts ()
{
	GList *accounts=NULL;
	const gchar *extension_name;

	extension_name = E_SOURCE_EXTENSION_MAIL_TRANSPORT;
	accounts = e_source_registry_list_sources (source_registry, extension_name);

	return accounts;
}

GList *
mail_get_store_accounts ()
{
	GList *accounts=NULL;
	const gchar *extension_name;

	extension_name = E_SOURCE_EXTENSION_MAIL_ACCOUNT;
	accounts = e_source_registry_list_sources (source_registry, extension_name);

	return accounts;
}

void
mail_provider_fetch_lock (const char *source)
{
	GMutex *lock;
	GHashTable *hash = mail_get_provider_hash();

	g_static_mutex_lock (&provider_hash_lock);
	lock = g_hash_table_lookup (hash, source);
	if (!lock) {
		lock = g_mutex_new();
		g_hash_table_insert (hash, g_strdup(source), lock);
	}
	g_static_mutex_unlock (&provider_hash_lock);

	g_mutex_lock (lock);
}

void
mail_provider_fetch_unlock (const char *source)
{
	GMutex *lock;
	GHashTable *hash = mail_get_provider_hash();

	g_static_mutex_lock (&provider_hash_lock);
	/* lock can't disappear from hash */
	lock = g_hash_table_lookup (hash, source);
	g_static_mutex_unlock (&provider_hash_lock);

	g_mutex_unlock (lock);
}

/* This simulates and creates INBOX per non-store specific folder. */
CamelFolder *
mail_provider_fetch_inbox_folder (const char *source, GCancellable *cancellable, GError **error)
{
	CamelStore *local = e_mail_session_get_local_store (session);
	CamelFolder *destination=NULL;
	GList *accounts, *list;
	ESource *account = NULL;
	const char *email;
	char *folder_name;
	ESourceMailIdentity *extension;
	const gchar *extension_name;

	accounts = mail_get_store_accounts();
	list=accounts;
	while (list) {
		const char *uid;
		account = (ESource *) list->data;

		uid = e_source_get_uid (account);
		if (uid && strcmp (uid, source) == 0) {
			break;
		}

		list = list->next;
		account = NULL;
	}
	
	if (!account) {
		printf("Fetch Inbox Folder: Account not found: %s\n", source);
		return NULL;
	}

	extension_name = E_SOURCE_EXTENSION_MAIL_IDENTITY;
	extension = e_source_get_extension (account, extension_name);
	email = e_source_mail_identity_get_address (extension);

	g_list_free_full (accounts, (GDestroyNotify) g_object_unref);

	folder_name = g_strdup_printf ("%s/Inbox", email);

	destination = camel_store_get_folder_sync (local, folder_name, 0, cancellable, error);
	if (!destination) {
		/* If its first time, create that folder & Draft/Sent. */
		CamelFolderInfo *info;
		char *path_name;
		ESourceExtension *extension;

		g_clear_error(error);

		info = camel_store_create_folder_sync (local, NULL, folder_name, cancellable, error);
		if (!info) 
			g_warning ("Unable to create POP3 folder: %s\n", folder_name);
		else {
			extension_name = E_SOURCE_EXTENSION_MAIL_COMPOSITION;
			extension = e_source_get_extension (account, extension_name);

			destination = camel_store_get_folder_sync (local, folder_name, 0, cancellable, error);
			g_free (folder_name);
			folder_name = g_strdup_printf ("%s/Drafts", email);

			info = camel_store_create_folder_sync (local, NULL, folder_name, cancellable, error);
			path_name = g_strdup_printf ("folder://local/%s", folder_name);
			e_source_mail_composition_set_drafts_folder (
				E_SOURCE_MAIL_COMPOSITION (extension),
				path_name);
			g_free(path_name);
			
			g_free (folder_name);
			folder_name = g_strdup_printf ("%s/Sent", email);
	
			info = camel_store_create_folder_sync (local, NULL, folder_name, cancellable, error);
			path_name = g_strdup_printf ("folder://local/%s", folder_name);

			extension_name = E_SOURCE_EXTENSION_MAIL_SUBMISSION;
			extension = e_source_get_extension (account, extension_name);
			e_source_mail_submission_set_sent_folder (E_SOURCE_MAIL_SUBMISSION(extension), path_name);
			g_free(path_name);

			/* FIXME This is a blocking D-Bus method call. */
			if (!e_source_write_sync (account, cancellable, error)) {
				g_warning ("%s", (*error)->message);
			}


		}
	}
	g_free (folder_name);

	return destination;
}

gboolean
mail_get_keep_on_server (CamelService *service)
{
	GObjectClass *class;
	CamelSettings *settings;
	gboolean keep_on_server = FALSE;

	settings = camel_service_ref_settings (service);
	class = G_OBJECT_GET_CLASS (settings);

	/* XXX This is a POP3-specific setting. */
	if (g_object_class_find_property (class, "keep-on-server") != NULL)
		g_object_get (
			settings, "keep-on-server",
			&keep_on_server, NULL);

	g_object_unref (settings);
	return keep_on_server;
}

/* Error Quark */
GQuark
mail_daemon_error_quark (void)
{
	static GQuark q = 0;
	if (q == 0)
		q = g_quark_from_static_string ("mail-daemon-error-quark");

	return q;
}
