/* e-mail-data-session.c */

#include "libemail-engine/e-mail-session.h"
#include "e-mail-data-session.h"
#include "e-mail-data-operation.h"
#include "libemail-engine/e-mail-local.h"
#include "e-mail-data-store.h"
#include "e-gdbus-emailsession.h"
#include <camel/camel.h>
#include <gio/gio.h>
#include "libemail-engine/mail-ops.h"
#include "libemail-utils/e-account-utils.h"
#include "libemail-engine/mail-tools.h"
#include "mail-send-recv.h"
#include "utils.h"
#include <libedataserver/e-account-list.h>
#include <libedataserverui/e-passwords.h>
#include <string.h>

extern EMailSession *session;
extern EMailDataSession *data_session;
#define micro(x) if (mail_debug_log(EMAIL_DEBUG_SESSION|EMAIL_DEBUG_MICRO)) x;
#define ipc(x) if (mail_debug_log(EMAIL_DEBUG_SESSION|EMAIL_DEBUG_IPC)) x;


G_DEFINE_TYPE (EMailDataSession, e_mail_data_session, G_TYPE_OBJECT)

#define DATA_SESSION_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EMAIL_TYPE_DATA_SESSION, EMailDataSessionPrivate))

typedef struct _EMailDataSessionPrivate EMailDataSessionPrivate;

struct _EMailDataSessionPrivate
{
	EGdbusSession *gdbus_object;

	GMutex *stores_lock;
	GHashTable *stores;

	GMutex *datastores_lock;
	/* A hash of object paths for book URIs to EDataBooks */
	GHashTable *datastores;

	GMutex *connections_lock;
	/* This is a hash of client addresses to GList* of EDataBooks */
	GHashTable *connections;

	GMutex *cops_lock;
	GHashTable *cops;
	GMutex *mops_lock;
	GHashTable *mops;

	guint exit_timeout;
	
};

static gchar *
construct_mail_session_path (const char *uid)
{
	static volatile gint counter = 1;
	int i, len;
	char *path;
	
	path = g_strdup_printf (
		"/org/gnome/evolution/dataserver/mail/service/%s/%d/%u",
		uid, getpid (), g_atomic_int_exchange_and_add (&counter, 1));

	len = strlen(path);
	for (i=0; i<len ; i++)
		if (path[i] == '.')
			path[i] = '_';
		else if (path[i] == '#')
			path[i] = '_';
		else if (path[i] == '(')
			path[i] = '_';
		else if (path[i] == '-')
			path[i] = '_';
		else if (path[i] == '@')
			path[i] = '_';
		else if (path[i] == ')')
			path[i] = '_';
		else if (path[i] == ' ')
			path[i] = '_';
		else if (path[i] == '[' || path[i] == ']')
			path[i] = '_';
	
	

	return path;
}

static char *
process_service (EMailDataSession *msession, const char *uid, CamelService *service, EGdbusSession *object, GDBusMethodInvocation *invocation)
{
	char *path;
	EMailDataStore *estore;
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	const gchar *sender;
	GList *list = NULL;

	/* Hold store & datastore the lock when you are calling this function */
	path = construct_mail_session_path (uid);
	estore = e_mail_data_store_new (service, uid);

	g_hash_table_insert (priv->datastores, service, estore);
	e_mail_data_store_register_gdbus_object (estore, g_dbus_method_invocation_get_connection (invocation), path, NULL);
	
	g_mutex_lock (priv->connections_lock);
	sender = g_dbus_method_invocation_get_sender (invocation);

	list = g_hash_table_lookup (priv->connections, sender);
	list = g_list_prepend (list, estore);
	g_hash_table_insert (priv->connections, g_strdup (sender), list);
	g_mutex_unlock (priv->connections_lock);


	return path;
}

static gboolean
impl_Mail_getService (EGdbusSession *object, GDBusMethodInvocation *invocation, const gchar *uid, EMailDataSession *msession)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	CamelService *service;
	char *path;

	/* Remove a pending exit */
	if (priv->exit_timeout) {
		g_source_remove (priv->exit_timeout);
		priv->exit_timeout = 0;
	}

	if (!uid|| !*uid) {
		ipc(printf("GetService: empty uid passed\n"));
		return FALSE;
	}
	
	g_mutex_lock (priv->stores_lock);
	g_mutex_lock (priv->datastores_lock);
	service = g_hash_table_lookup (priv->stores, uid);
	
	if (service == NULL) {
		char *url;

		service = camel_session_get_service (CAMEL_SESSION(session), uid);
		url = mail_get_service_url (service);

		/* Hashtable owns the key's memory */
		g_hash_table_insert (priv->stores, g_strdup(uid), service);
		g_hash_table_insert (priv->stores, url, service);		

	}

	path = process_service (msession, uid, service, object, invocation);
	ipc (printf("EMailDataSession: Get Service: Success %s for uid %s\n", path, uid));
	egdbus_session_complete_get_service (object, invocation, path);

	g_mutex_unlock (priv->datastores_lock);
	g_mutex_unlock (priv->stores_lock);

	g_free (path);
	return TRUE;
}

static gboolean
impl_Mail_getServiceByUrl (EGdbusSession *object, GDBusMethodInvocation *invocation, const gchar *url, gboolean isstore, EMailDataSession *msession)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	CamelService *service;
	char *path;

	/* Remove a pending exit */
	if (priv->exit_timeout) {
		g_source_remove (priv->exit_timeout);
		priv->exit_timeout = 0;
	}

	if (!url|| !*url) {
		ipc(printf("GetServiceByUrl: empty url passed\n"));
		return FALSE;
	}
	
	g_mutex_lock (priv->stores_lock);
	g_mutex_lock (priv->datastores_lock);
	service = g_hash_table_lookup (priv->stores, url);
	
	if (service == NULL) {
		CamelURL *curl;

		curl = camel_url_new (url, NULL);
		service = camel_session_get_service_by_url (CAMEL_SESSION(session), curl, isstore ? CAMEL_PROVIDER_STORE : CAMEL_PROVIDER_TRANSPORT);

		/* Hashtable owns the key's memory */
		g_hash_table_insert (priv->stores, g_strdup(url), service);
		g_hash_table_insert (priv->stores, g_strdup(camel_service_get_uid(service)), service);

		camel_url_free (curl);

	}

	path = process_service (msession, camel_service_get_uid(service), service, object, invocation);
	ipc (printf("EMailDataSession: Get Service by Url : Success %s for url %s\n", path, url));

	if (!url)
		egdbus_session_complete_get_service (object, invocation, path);
	else
		egdbus_session_complete_get_service_by_url (object, invocation, path);

	g_mutex_unlock (priv->datastores_lock);
	g_mutex_unlock (priv->stores_lock);
	g_free (path);

	return TRUE;
}

static gboolean
impl_Mail_addService (EGdbusSession *object, GDBusMethodInvocation *invocation, const char *uid, const char *url, gboolean isstore, EMailDataSession *msession)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	CamelService *service;
	GError *error = NULL;
	char *path;

	g_mutex_lock (priv->stores_lock);
	g_mutex_lock (priv->datastores_lock);

	service = camel_session_add_service (CAMEL_SESSION(session), uid, url, isstore ? CAMEL_PROVIDER_STORE : CAMEL_PROVIDER_TRANSPORT, &error);

	if (!service || error) {
		ipc(printf("Unable to add  service %s/%s: %s\n", url, uid, error->message));
		g_dbus_method_invocation_return_gerror (invocation, error);
		g_mutex_unlock (priv->datastores_lock);
		g_mutex_unlock (priv->stores_lock);
		return TRUE;
	}
	
	/* Hashtable owns the key's memory */
	g_hash_table_insert (priv->stores, mail_get_service_url (service), service);
	g_hash_table_insert (priv->stores, g_strdup(camel_service_get_uid(service)), service);

	path = process_service (msession, camel_service_get_uid(service), service, object, invocation);
	ipc (printf("EMailDataSession: Add Service: Success %s  for uid/url %s/%s\n", path, uid, url));

	egdbus_session_complete_add_service (object, invocation, path);

	g_mutex_unlock (priv->datastores_lock);
	g_mutex_unlock (priv->stores_lock);
	g_free (path);

	return TRUE;
}

static gboolean
impl_Mail_removeService (EGdbusSession *object, GDBusMethodInvocation *invocation, const char *uid, EMailDataSession *msession)
{
	//EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	gboolean success;

	success = camel_session_remove_service (CAMEL_SESSION(session), uid);
	ipc (printf("EMailDataSession: Remove Service: Success %d  for uid %s\n", success, uid));

	egdbus_session_complete_remove_service (object, invocation, success);
	return TRUE;
}

static gboolean
impl_Mail_removeServices (EGdbusSession *object, GDBusMethodInvocation *invocation, EMailDataSession *msession)
{
	//EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);

	camel_session_remove_services (CAMEL_SESSION(session));

	egdbus_session_complete_remove_services (object, invocation);

	return TRUE;
}


static gboolean
impl_Mail_listServices (EGdbusSession *object, GDBusMethodInvocation *invocation, EMailDataSession *msession)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	GList *list, *iter;
	char **services = NULL;
	int len, i=0;

	list = camel_session_list_services (CAMEL_SESSION(session));
	iter = list;
	
	len = g_list_length (list);
	services = g_new0 (char *, len+1);

	g_mutex_lock (priv->stores_lock);
	g_mutex_lock (priv->datastores_lock);

	while (iter) {
		char *path;
		CamelService *service;
		const char *uid;

		service = CAMEL_SERVICE (iter->data);
		uid = camel_service_get_uid (service);
		ipc(printf("Sending Service: %s\n", uid));
		if (!g_hash_table_lookup (priv->stores, uid)) {
			char *url;

			url = mail_get_service_url (service);
			micro(printf("Sending Service Url: %s\n", url));

			/* Hashtable owns the key's memory */
			g_hash_table_insert (priv->stores, g_strdup(uid), service);
			g_hash_table_insert (priv->stores, url, service);
		}

		path = process_service (msession, uid, service, object, invocation);
		services[i] = path;
		ipc(printf("Sending Service Path : %s\n", path));

		i++;
		iter = iter->next;
	}

	g_mutex_unlock (priv->datastores_lock);
	g_mutex_unlock (priv->stores_lock);

	egdbus_session_complete_list_services (object, invocation, (const gchar * const*) services);

	g_free (services);
	return TRUE;
}

static gboolean
impl_Mail_getOnline (EGdbusSession *object, GDBusMethodInvocation *invocation, EMailDataSession *msession)
{
	//EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	gboolean online;

	online = camel_session_get_online (CAMEL_SESSION(session));
	egdbus_session_complete_get_online (object, invocation, online);

	return TRUE;
}

static gboolean
impl_Mail_setOnline (EGdbusSession *object, GDBusMethodInvocation *invocation, gboolean online, EMailDataSession *msession)
{
	//EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);

	camel_session_set_online (CAMEL_SESSION(session), online);
	egdbus_session_complete_set_online (object, invocation);
	return TRUE;
}

static gboolean
impl_Mail_getNetworkAvailable (EGdbusSession *object, GDBusMethodInvocation *invocation, EMailDataSession *msession)
{
	//EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	gboolean online;

	online = camel_session_get_network_available (CAMEL_SESSION(session));
	egdbus_session_complete_get_network_available  (object, invocation, online);

	return TRUE;
}

static gboolean
impl_Mail_setNetworkAvailable (EGdbusSession *object, GDBusMethodInvocation *invocation, gboolean online, EMailDataSession *msession)
{
	//EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);

	camel_session_set_network_available (CAMEL_SESSION(session), online);
	egdbus_session_complete_set_network_available (object, invocation);
	return TRUE;
}

#if 0
static gboolean
impl_Mail_(EGdbusSession *object, GDBusMethodInvocation *invocation, EMailDataSession *msession)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);

	egdbus_session_complete_ (object, invocation)
	return TRUE;
}
#endif

static gboolean
impl_Mail_getLocalStore (EGdbusSession *object, GDBusMethodInvocation *invocation, EMailDataSession *msession)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	CamelStore *store;
	EMailDataStore *estore;
	char *path = NULL;
	GList *list;
	const gchar *sender;

	/* Remove a pending exit */
	if (priv->exit_timeout) {
		g_source_remove (priv->exit_timeout);
		priv->exit_timeout = 0;
	}
	
	store = e_mail_local_get_store ();

	g_mutex_lock (priv->stores_lock);
	g_mutex_lock (priv->datastores_lock);
	estore = g_hash_table_lookup (priv->datastores, store);
	
	if (estore == NULL) {
		const char *uid;
		char *url;

		url = mail_get_service_url ((CamelService *)store);
		uid = camel_service_get_uid((CamelService *)store);
		path = construct_mail_session_path (uid);
		estore = e_mail_data_store_new ((CamelService *)store, uid);

		g_hash_table_insert (priv->datastores, store, estore);
		e_mail_data_store_register_gdbus_object (estore, g_dbus_method_invocation_get_connection (invocation), path, NULL);
	
		/* Hashtable owns the key's memory */
		g_hash_table_insert (priv->stores, g_strdup(uid), store);
		g_hash_table_insert (priv->stores, url, store);		

	} else 
		path = g_strdup (e_mail_data_store_get_path (estore));

	g_mutex_unlock (priv->datastores_lock);
	g_mutex_unlock (priv->stores_lock);

	g_mutex_lock (priv->connections_lock);
	sender = g_dbus_method_invocation_get_sender (invocation);

	list = g_hash_table_lookup (priv->connections, sender);
	list = g_list_prepend (list, estore);
	g_hash_table_insert (priv->connections, g_strdup (sender), list);
	g_mutex_unlock (priv->connections_lock);

	ipc (printf("EMailDataSession: Get Local Store: Success %s  for sender: '%s'\n", path, sender));

	egdbus_session_complete_get_local_store (object, invocation, path);

	g_free (path);
	
	return TRUE;
}

static gboolean
impl_Mail_getLocalFolder (EGdbusSession *object, GDBusMethodInvocation *invocation, const char *type, EMailDataSession *msession)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	CamelStore *store;
	CamelFolder *folder;
	EMailLocalFolder ftype;
	EMailDataStore *estore;
	const char *fpath;

	if (type[0] == 'i' || type[0] == 'I')
		ftype = E_MAIL_LOCAL_FOLDER_INBOX;
	else 	if (type[0] == 'd' || type[0] == 'D')
		ftype = E_MAIL_LOCAL_FOLDER_DRAFTS;
	else if (type[0] == 'o' || type[0] == 'O')
		ftype = E_MAIL_LOCAL_FOLDER_OUTBOX;
	else if (type[0] == 's' || type[0] == 'S')
		ftype = E_MAIL_LOCAL_FOLDER_SENT;
	else if (type[0] == 't' || type[0] == 'T')
		ftype = E_MAIL_LOCAL_FOLDER_TEMPLATES;
	else 
		ftype = E_MAIL_LOCAL_FOLDER_LOCAL_INBOX;
	
	folder = e_mail_local_get_folder (ftype);
	store = e_mail_local_get_store ();

	g_mutex_lock (priv->stores_lock);
	g_mutex_lock (priv->datastores_lock);
	estore = g_hash_table_lookup (priv->datastores, store);
	
	if (estore == NULL) {
		const char *uid;
		char *path;
		char *url;

		url = mail_get_service_url ((CamelService *)store);
		uid = camel_service_get_uid((CamelService *)store);
		path = construct_mail_session_path (uid);
		estore = e_mail_data_store_new ((CamelService *)store, uid);

		g_hash_table_insert (priv->datastores, store, estore);
		e_mail_data_store_register_gdbus_object (estore, g_dbus_method_invocation_get_connection (invocation), path, NULL);

		/* Hashtable owns the key's memory */
		g_hash_table_insert (priv->stores, g_strdup(uid), store);
		g_hash_table_insert (priv->stores, url, store);		

		g_free (path);
	}

	g_mutex_unlock (priv->datastores_lock);
	g_mutex_unlock (priv->stores_lock);

	fpath = e_mail_data_store_get_folder_path (estore, g_dbus_method_invocation_get_connection (invocation), folder);

	egdbus_session_complete_get_local_folder (object, invocation, fpath);

	return TRUE;
}

typedef struct _email_get_store_data {
	EMailDataSession *msession;
	EGdbusSession *object;
	GDBusMethodInvocation *invocation;
	
}EMailGetStoreData;

static void
get_folder_done (EMailSession *session, GAsyncResult *result, EMailGetStoreData *data)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(data->msession);
	CamelStore *store;
	const char *fpath;
	char *spath;
	EMailDataStore *estore;
	const gchar *sender;
	GList *list;
	GError *error = NULL;
	CamelFolder *folder;

	folder = e_mail_session_uri_to_folder_finish (
		session, result, &error);

	if (folder == NULL) {
		ipc(printf("Unable to get folder: %s\n", error->message));
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		return;
	}

	store = camel_folder_get_parent_store (folder);

	g_mutex_lock (priv->stores_lock);
	g_mutex_lock (priv->datastores_lock);
	estore = g_hash_table_lookup (priv->datastores, store);
	
	if (estore == NULL) {
		const char *uid;
		char *url;

		url = mail_get_service_url ((CamelService *)store);
		uid = camel_service_get_uid((CamelService *)store);
		spath = construct_mail_session_path (uid);
		estore = e_mail_data_store_new ((CamelService *)store, uid);

		g_hash_table_insert (priv->datastores, store, estore);
		e_mail_data_store_register_gdbus_object (estore, g_dbus_method_invocation_get_connection (data->invocation), spath, NULL);

		/* Hashtable owns the key's memory */
		g_hash_table_insert (priv->stores, g_strdup(uid), store);
		g_hash_table_insert (priv->stores, url, store);		

		g_free (url);
		g_free (spath);	
	}

	g_mutex_unlock (priv->datastores_lock);
	g_mutex_unlock (priv->stores_lock);

	g_mutex_lock (priv->connections_lock);
	sender = g_dbus_method_invocation_get_sender (data->invocation);

	list = g_hash_table_lookup (priv->connections, sender);
	list = g_list_prepend (list, estore);
	g_hash_table_insert (priv->connections, g_strdup (sender), list);
	g_mutex_unlock (priv->connections_lock);
	
	fpath = e_mail_data_store_get_folder_path (estore, g_dbus_method_invocation_get_connection (data->invocation), folder);

	egdbus_session_complete_get_folder_from_uri (data->object, data->invocation, fpath);

	ipc (printf("EMailDataSession: Get Folder from URI : Success %s  for sender: '%s'\n", fpath, sender));

	g_free (data);
}

static gboolean
impl_Mail_getFolderFromUri (EGdbusSession *object, GDBusMethodInvocation *invocation, const char *uri, const char *ops_path, EMailDataSession *msession)
{
	//EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	EMailGetStoreData *data = g_new0(EMailGetStoreData, 1);
	GCancellable *ops;

	ops = (GCancellable *)e_mail_data_session_get_camel_operation (ops_path);
	if (!ops) {
		g_warning ("Unable to get CamelOperation for path: %s\n", ops_path);
		ops = camel_operation_new ();
	}

	data->invocation = invocation;
	data->msession = msession;
	data->object = object;

	e_mail_session_uri_to_folder (
		session, uri, 0, G_PRIORITY_DEFAULT,
		ops, (GAsyncReadyCallback) get_folder_done,
		data);

	return TRUE;
}

static gboolean
impl_Mail_findPassword (EGdbusSession *object, GDBusMethodInvocation *invocation, const char *key, EMailDataSession *msession)
{
	//EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	char *password;

	ipc(printf("Finding Password for: %s\n", key));
	password = e_passwords_get_password ("Mail", key);

	if (g_getenv("EDS_SHOW_PASSWORDS")) {
		printf("findPass: %s: %s\n", key, password ? password : "EMPTY");
	}

	egdbus_session_complete_find_password (object, invocation, password ? password : "");
	g_free (password);

	return TRUE;
}


static gboolean
impl_Mail_addPassword (EGdbusSession *object, GDBusMethodInvocation *invocation, const char *key, const char *password, gboolean remember, EMailDataSession *msession)
{
	//EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);

	ipc(printf("Adding Password for: %s (remember: %d)\n", key, remember));
	if (g_getenv("EDS_SHOW_PASSWORDS")) {
		printf("Adding password: %s : %s\n", key, password);
	}
	e_passwords_add_password (key, password);
	if (remember)
		e_passwords_remember_password ("Mail", key);

	egdbus_session_complete_add_password (object, invocation);

	return TRUE;
}

static gboolean
impl_Mail_sendReceive (EGdbusSession *object, GDBusMethodInvocation *invocation, EMailDataSession *msession)
{
	ipc(printf("Initiating Send/Receive\n"));

	mail_send_receive (session);

	egdbus_session_complete_send_receive (object, invocation);
	return TRUE;
}

static gboolean
impl_Mail_sendMailsFromOutbox (EGdbusSession *object, GDBusMethodInvocation *invocation, EMailDataSession *msession)
{
	GCancellable *ops;
	EMailDataOperation *mops;
	char *mops_path;

	ipc(printf("Initiating Send which flushes mails from outbox\n"));

	ops = mail_send (session);
	mops = e_mail_data_operation_new ((CamelOperation *)ops);
	mops_path = e_mail_data_operation_register_gdbus_object (mops, g_dbus_method_invocation_get_connection(invocation), NULL);

	egdbus_session_complete_send_mails_from_outbox (object, invocation, mops_path);
	g_free (mops_path);

	return TRUE;
}


static gboolean
impl_Mail_fetchAccount (EGdbusSession *object, GDBusMethodInvocation *invocation, char *uid, EMailDataSession *msession)
{
	EIterator *iter;
	EAccountList *accounts;
	EAccount *account;
	GCancellable *ops;
	EMailDataOperation *mops;
	char *mops_path = NULL;
	
	accounts = e_get_account_list ();
	for (iter = e_list_get_iterator ((EList *)accounts);
	     e_iterator_is_valid (iter);
	     e_iterator_next (iter)) {
		account = (EAccount *) e_iterator_get (iter);
		if (account->uid && strcmp (account->uid, uid) == 0) {
			ops = mail_receive_account (session, account);
			mops = e_mail_data_operation_new ((CamelOperation *)ops);
			mops_path = e_mail_data_operation_register_gdbus_object (mops, g_dbus_method_invocation_get_connection(invocation), NULL);
		}
	}

	egdbus_session_complete_fetch_account (object, invocation, mops_path ? mops_path : "");
	return TRUE;
}

#if 0
static void
fetch_old_messages_done (gboolean still_more, EMailGetStoreData *data)
{
	ipc(printf("Done: Fetch old messages in POP: %d\n", still_more));
	egdbus_session_complete_fetch_old_messages (data->object, data->invocation, still_more);

	g_free (data);
}

static gboolean
impl_Mail_fetchOldMessages (EGdbusSession *object, GDBusMethodInvocation *invocation, char *uid, int count, EMailDataSession *msession)
{
	EIterator *iter;
	EAccountList *accounts;
	EAccount *account;
	EMailGetStoreData *data = g_new0(EMailGetStoreData, 1);

	data->invocation = invocation;
	data->msession = msession;
	data->object = object;
	
	accounts = e_get_account_list ();
	for (iter = e_list_get_iterator ((EList *)accounts);
	     e_iterator_is_valid (iter);
	     e_iterator_next (iter)) {
		account = (EAccount *) e_iterator_get (iter);
		if (account->uid && strcmp (account->uid, uid) == 0) {
			const gchar *uri;
			gboolean keep_on_server;

			uri = e_account_get_string (
				account, E_ACCOUNT_SOURCE_URL);
			keep_on_server = e_account_get_bool (
				account, E_ACCOUNT_SOURCE_KEEP_ON_SERVER);
			mail_fetch_mail (uri, keep_on_server,
				 E_FILTER_SOURCE_INCOMING,
				 NULL, count,
				 NULL, NULL,
				 NULL, NULL,
				 (void (*)(const gchar *, void *)) fetch_old_messages_done, data);
		}
	}

	return TRUE;
}
#endif 

static gboolean
impl_Mail_cancelOperations (EGdbusSession *object, GDBusMethodInvocation *invocation, EMailDataSession *msession)
{
	ipc(printf("Canceling all Mail Operations\n"));

	/* This is the only known reliable way to cancel an issued operation. No harm in canceling this. */
	mail_cancel_all ();

	egdbus_session_complete_cancel_operations (object, invocation);
	return TRUE;
}

static gboolean
impl_Mail_createMailOperation (EGdbusSession *object, GDBusMethodInvocation *invocation, EMailDataSession *msession)
{
	GCancellable *ops;
	EMailDataOperation *mops;
	char *mops_path;
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);

	ipc(printf("Creating Mail Operation handler\n"));
	ops = camel_operation_new ();
	mops = e_mail_data_operation_new ((CamelOperation *)ops);
	mops_path = e_mail_data_operation_register_gdbus_object (mops, g_dbus_method_invocation_get_connection(invocation), NULL);
	g_mutex_lock (priv->cops_lock);
	g_hash_table_insert (priv->cops, ops, mops);
	g_mutex_unlock (priv->cops_lock);
	g_mutex_lock (priv->mops_lock);
	g_hash_table_insert (priv->mops, mops_path, mops);
	g_mutex_unlock (priv->mops_lock);

	g_object_unref (ops);
	egdbus_session_complete_create_mail_operation (object, invocation, mops_path);

	return TRUE;
}

static void
e_mail_data_session_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
e_mail_data_session_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
e_mail_data_session_dispose (GObject *object)
{
  G_OBJECT_CLASS (e_mail_data_session_parent_class)->dispose (object);
}

static void
e_mail_data_session_finalize (GObject *object)
{
  G_OBJECT_CLASS (e_mail_data_session_parent_class)->finalize (object);
}

static void
e_mail_data_session_class_init (EMailDataSessionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (EMailDataSessionPrivate));

  object_class->get_property = e_mail_data_session_get_property;
  object_class->set_property = e_mail_data_session_set_property;
  object_class->dispose = e_mail_data_session_dispose;
  object_class->finalize = e_mail_data_session_finalize;
}

static void
e_mail_data_session_init (EMailDataSession *self)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(self);

	priv->gdbus_object = egdbus_session_skeleton_new ();
	g_signal_connect (priv->gdbus_object, "handle-create-mail-operation", G_CALLBACK (impl_Mail_createMailOperation), self);	
	g_signal_connect (priv->gdbus_object, "handle-add-service", G_CALLBACK (impl_Mail_addService), self);
	g_signal_connect (priv->gdbus_object, "handle-remove-service", G_CALLBACK (impl_Mail_removeService), self);
	
	g_signal_connect (priv->gdbus_object, "handle-get-service", G_CALLBACK (impl_Mail_getService), self);
	g_signal_connect (priv->gdbus_object, "handle-get-service-by-url", G_CALLBACK (impl_Mail_getServiceByUrl), self);
	g_signal_connect (priv->gdbus_object, "handle-remove-services", G_CALLBACK (impl_Mail_removeServices), self);
	g_signal_connect (priv->gdbus_object, "handle-list-services", G_CALLBACK (impl_Mail_listServices), self);
	g_signal_connect (priv->gdbus_object, "handle-get-online", G_CALLBACK (impl_Mail_getOnline), self);
	g_signal_connect (priv->gdbus_object, "handle-set-online", G_CALLBACK (impl_Mail_setOnline), self);
	g_signal_connect (priv->gdbus_object, "handle-get-network-available", G_CALLBACK (impl_Mail_getNetworkAvailable), self);
	g_signal_connect (priv->gdbus_object, "handle-set-network-available", G_CALLBACK (impl_Mail_setNetworkAvailable), self);
	

	g_signal_connect (priv->gdbus_object, "handle-get-local-store", G_CALLBACK (impl_Mail_getLocalStore), self);
	g_signal_connect (priv->gdbus_object, "handle-get-local-folder", G_CALLBACK (impl_Mail_getLocalFolder), self);
	g_signal_connect (priv->gdbus_object, "handle-get-folder-from-uri", G_CALLBACK (impl_Mail_getFolderFromUri), self);
	g_signal_connect (priv->gdbus_object, "handle-add-password", G_CALLBACK (impl_Mail_addPassword), self);
	g_signal_connect (priv->gdbus_object, "handle-find-password", G_CALLBACK (impl_Mail_findPassword), self);
	g_signal_connect (priv->gdbus_object, "handle-send-receive", G_CALLBACK (impl_Mail_sendReceive), self);
	g_signal_connect (priv->gdbus_object, "handle-send-mails-from-outbox", G_CALLBACK (impl_Mail_sendMailsFromOutbox), self);	
	g_signal_connect (priv->gdbus_object, "handle-fetch-account", G_CALLBACK (impl_Mail_fetchAccount), self);
//	g_signal_connect (priv->gdbus_object, "handle-fetch-old-messages", G_CALLBACK (impl_Mail_fetchOldMessages), self);
	g_signal_connect (priv->gdbus_object, "handle-cancel-operations", G_CALLBACK (impl_Mail_cancelOperations), self);

	priv->stores_lock = g_mutex_new ();
	priv->stores = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

	priv->datastores_lock = g_mutex_new ();
	priv->datastores = g_hash_table_new_full (
		g_direct_hash, g_direct_equal,
		(GDestroyNotify) NULL,
		(GDestroyNotify) NULL);

	priv->connections_lock = g_mutex_new ();
	priv->connections = g_hash_table_new_full (
		g_str_hash, g_str_equal,
		(GDestroyNotify) g_free,
		(GDestroyNotify) NULL);

	priv->mops_lock = g_mutex_new ();
	/* Key = object_path, Value = MailOperation */
	priv->mops = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

	priv->cops_lock = g_mutex_new ();
	/* Key = CamelOperation Value = MailOperation */
	priv->cops = g_hash_table_new_full (
		g_direct_hash, g_direct_equal,
		(GDestroyNotify) NULL,
		(GDestroyNotify) NULL);

}

EMailDataSession*
e_mail_data_session_new (void)
{
  return g_object_new (EMAIL_TYPE_DATA_SESSION, NULL);
}

guint
e_mail_data_session_register_gdbus_object (EMailDataSession *msession, GDBusConnection *connection, const gchar *object_path, GError **error)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	guint ret;

	g_return_val_if_fail (connection != NULL, 0);
	g_return_val_if_fail (object_path != NULL, 0);

 	ret = g_dbus_interface_skeleton_export ((GDBusInterfaceSkeleton *)priv->gdbus_object,
               	                     	connection,
                                    	object_path,
                                    	error);

	ipc (printf("EMailDataSession: Registering gdbus object %s\n", object_path));

	return ret;
}

void 
e_mail_data_session_release (EMailDataSession *session, GDBusConnection *connection, const char *name)
{
	
}

const char *
e_mail_data_session_get_path_from_store (EMailDataSession *msession, gpointer store)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	EMailDataStore *mstore;

	mstore = g_hash_table_lookup (priv->datastores, store);
	g_assert (mstore != NULL);
	
	micro(printf("e_mail_data_session_get_path_from_store: %p: %s\n", store, e_mail_data_store_get_path(mstore)));

	return e_mail_data_store_get_path (mstore);
}

CamelFolder *
e_mail_session_get_folder_from_path (EMailDataSession *msession, const char *path)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	GHashTableIter iter;
	gpointer key, value;
	
	g_hash_table_iter_init (&iter, priv->datastores);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		EMailDataStore *estore = (EMailDataStore *)value;
		CamelFolder *folder;

		folder = e_mail_data_store_get_camel_folder (estore, path);
		if (folder) {
			micro(printf("e_mail_session_get_folder_from_path: %s %p\n", path, folder));

			return folder;
		}
	}	

	micro(printf("e_mail_session_get_folder_from_path: %s %p\n", path, NULL));
	g_warning ("Unable to find CamelFolder from the object path\n");

	return NULL;	
}

void
e_mail_session_emit_ask_password (EMailDataSession *msession, const char *title, const gchar *prompt, const gchar *key)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	
	ipc(printf("Emitting for Ask Password: %s %s %s\n", title, prompt, key));
	egdbus_session_emit_get_password (priv->gdbus_object, title, prompt, key);
}

void
e_mail_session_emit_send_receive_completed (EMailDataSession *msession)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	
	ipc(printf("Emitting Send/Receive completed signal\n"));
	egdbus_session_emit_send_receive_complete (priv->gdbus_object);
}

void
e_mail_session_emit_account_added (EMailDataSession *msession, const char *uid)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	
	ipc(printf("Emitting Account added signal\n"));
	egdbus_session_emit_account_added (priv->gdbus_object, uid);
}

void
e_mail_session_emit_account_removed (EMailDataSession *msession, const char *uid)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	
	ipc(printf("Emitting Account removed signal\n"));
	egdbus_session_emit_account_removed (priv->gdbus_object, uid);
}

void
e_mail_session_emit_account_changed (EMailDataSession *msession, const char *uid)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(msession);
	
	ipc(printf("Emitting Account changed signal\n"));
	egdbus_session_emit_account_changed (priv->gdbus_object, uid);
}

CamelOperation *
e_mail_data_session_get_camel_operation (const char *mops_path)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(data_session);
	EMailDataOperation *mops;

	/* if the path isn't asked for, then lets create a operation for local usage. */
	if (!mops_path || !*mops_path)
		return (CamelOperation *)camel_operation_new ();

	mops = g_hash_table_lookup (priv->mops, mops_path);

	return e_mail_data_operation_get_camel_operation (mops);
	
}

EMailDataOperation *
e_mail_data_sessoin_get_mail_operation (CamelOperation *ops)
{
	EMailDataSessionPrivate *priv = DATA_SESSION_PRIVATE(data_session);

	return g_hash_table_lookup (priv->cops, ops);
}

