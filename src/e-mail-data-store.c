/* e-mail-data-store.c */

#include <string.h>
#include "e-mail-data-store.h"
#include "e-mail-data-folder.h"
#include "e-gdbus-emailstore.h"
#include "libemail-engine/mail-ops.h"
#include "utils.h"
#include <glib/gi18n.h>

#define micro(x) if (mail_debug_log(EMAIL_DEBUG_STORE|EMAIL_DEBUG_MICRO)) x;
#define ipc(x) if (mail_debug_log(EMAIL_DEBUG_STORE|EMAIL_DEBUG_IPC)) x;


G_DEFINE_TYPE (EMailDataStore, e_mail_data_store, G_TYPE_OBJECT)

#define DATA_STORE_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EMAIL_TYPE_DATA_STORE, EMailDataStorePrivate))

typedef struct _EMailDataStorePrivate EMailDataStorePrivate;

struct _EMailDataStorePrivate
{
	EGdbusStore *gdbus_object;

	CamelService *store;
	char *url;

	GMutex *folders_lock;
	/* 'uri' -> EBookBackend */
	GHashTable *folders;

	GMutex *datafolders_lock;
	/* A hash of object paths for book URIs to EDataBooks */
	GHashTable *datafolders;

	char *object_path;

	
};

static void
e_mail_data_store_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
e_mail_data_store_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
e_mail_data_store_dispose (GObject *object)
{
  G_OBJECT_CLASS (e_mail_data_store_parent_class)->dispose (object);
}

static void
e_mail_data_store_finalize (GObject *object)
{
  G_OBJECT_CLASS (e_mail_data_store_parent_class)->finalize (object);
}

static void
e_mail_data_store_class_init (EMailDataStoreClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (EMailDataStorePrivate));

  object_class->get_property = e_mail_data_store_get_property;
  object_class->set_property = e_mail_data_store_set_property;
  object_class->dispose = e_mail_data_store_dispose;
  object_class->finalize = e_mail_data_store_finalize;
}

static gboolean
check_if_ascii (const char *name)
{
	int i=0, len;

	len = strlen (name);

	for (i=0; i< len; i++) {
		if (!g_ascii_isalnum(name[i]))
			return FALSE;
	}

	return TRUE;
}

static gchar *
construct_mail_store_path (const char *full_name)
{
	static volatile gint counter = 1;
	int i, len;
	char *path;
	char *name;

	if (check_if_ascii(full_name))
		name = g_strdup(full_name);
	else
		name = g_strdup_printf ("%lu_%d", time(NULL), g_atomic_int_exchange_and_add (&counter, 1));
	
	path = g_strdup_printf (
		       "/org/gnome/evolution/dataserver/mail/folder/%s/%d/%u",
		       name, getpid (), g_atomic_int_exchange_and_add (&counter, 1));
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


static void
convert_folder_info (CamelFolderInfo *info, GVariantBuilder *builder)
{
	while (info) {
		g_variant_builder_add (builder, "(ssuii)", 
					info->display_name,
					info->full_name,
					info->flags,
					info->unread,
					info->total);

		convert_folder_info (info->child, builder);
		info = info->next;
	}
}

typedef struct _get_folder_info_data {
	EGdbusStore *object; 
	GDBusMethodInvocation *invocation; 
	EMailDataStore *mstore;
}GFIData;

static void
handle_get_folder_info_cb (CamelStore *store,
                	   GAsyncResult *result,
		    	   GFIData *gfi_data)

{
	EMailDataStore *mstore = gfi_data->mstore;
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	CamelFolderInfo *info;
	GVariantBuilder *builder;
	GVariant *variant;
	GError *error = NULL;

	info = camel_store_get_folder_info_finish (store, result, &error);
	if (!info) {
		g_warning ("Unable to get folder info on Store %p: %s\n", store, error ? error->message: "");
		if (error)
			g_dbus_method_invocation_return_gerror (gfi_data->invocation, error);
		else
			g_dbus_method_invocation_return_error (gfi_data->invocation, CAMEL_ERROR, CAMEL_STORE_ERROR_NO_FOLDER, _("Unable to fetch requested folder info"));
			
		ipc (printf("EMailDataStore: get folder info failed : %s - %s\n", priv->object_path, error ? error->message : ""));

		return;
	}

	builder = g_variant_builder_new (G_VARIANT_TYPE_ARRAY);

	convert_folder_info (info, builder);
	/* Added a empty entry */
	g_variant_builder_add (builder, "(ssuii)",  "", "", 0, -1, -1);
	
	variant = g_variant_builder_end (builder);
	g_variant_ref (variant);
	g_variant_builder_unref (builder);
	
	micro(printf("%s\n", g_variant_print (variant, TRUE)));
	egdbus_store_complete_get_folder_info (gfi_data->object, gfi_data->invocation, variant);
	g_variant_unref (variant);

	g_free (gfi_data);
	ipc (printf("EMailDataStore: get folder info success: %s\n", priv->object_path));

	return;
}

static gboolean
impl_Mail_getFolderInfo (EGdbusStore *object, GDBusMethodInvocation *invocation, char *top, guint32 flags, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	GFIData *gfi_data = g_new0 (GFIData, 1);
	GCancellable *ops;

	ops = camel_operation_new ();
	gfi_data->object = object;
	gfi_data->invocation = invocation;
	gfi_data->mstore = mstore;
	
	ipc (printf("EMailDataStore: get folder info: %s - %s: %d\n", priv->object_path, top ? top : "", flags));
	camel_store_get_folder_info ((CamelStore *)priv->store, 
		top, flags, G_PRIORITY_DEFAULT, ops, 
		(GAsyncReadyCallback) handle_get_folder_info_cb, gfi_data);

	return TRUE;
}

typedef struct _email_get_folder_data {
	EMailDataStore *mstore;
	EGdbusStore *object;
	GDBusMethodInvocation *invocation;
	char *folder_name;
	gboolean junk;
	gboolean trash;
	gboolean inbox;
}EMailGetFolderData;

static void
handle_mail_get_folder (CamelStore *store,
			GAsyncResult *result,
			EMailGetFolderData *send_data)
{
	EMailDataStore *mstore = send_data->mstore;
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	EMailDataFolder *efolder = NULL;
	char *path;
	CamelFolder *folder;
	GError *error = NULL;

	if (send_data->folder_name)
		folder = camel_store_get_folder_finish (store, result, &error);
	else if (send_data->inbox)
		folder = camel_store_get_inbox_folder_finish (store, result, &error);
	else if (send_data->junk)
		folder = camel_store_get_junk_folder_finish (store, result, &error);
	else
		folder = camel_store_get_trash_folder_finish (store, result, &error);

	if (folder == NULL) {
		g_mutex_unlock (priv->folders_lock);
		g_mutex_unlock (priv->datafolders_lock);
		g_warning ("Unable to get folder : %s\n", error ? error->message : "No Error");
		ipc (printf("EMailDataStore: get folder failed : %s - %s\n", priv->object_path, error ? error->message : "No error"));
	
		if (error)
			g_dbus_method_invocation_return_gerror (send_data->invocation, error);
		else {
			g_dbus_method_invocation_return_error (send_data->invocation,
								CAMEL_ERROR,
								CAMEL_ERROR_GENERIC,
								"No Error");
		}

		return;
	}

	g_mutex_lock (priv->folders_lock);
	g_mutex_lock (priv->datafolders_lock);

	g_hash_table_insert (priv->folders, g_strdup(camel_folder_get_full_name(folder)), folder);
	efolder = e_mail_data_folder_new (folder);

	path = construct_mail_store_path (camel_folder_get_full_name(folder));
	e_mail_data_folder_register_gdbus_object (efolder, g_dbus_method_invocation_get_connection (send_data->invocation), path, NULL);
	g_hash_table_insert (priv->datafolders, g_strdup (camel_folder_get_full_name(folder)), efolder);

	if (send_data->folder_name)
		egdbus_store_complete_get_folder (send_data->object, send_data->invocation, path);
	else if (send_data->inbox)
		egdbus_store_complete_get_inbox (send_data->object, send_data->invocation, path);
	else if (send_data->junk)
		egdbus_store_complete_get_junk (send_data->object, send_data->invocation, path);
	else
		egdbus_store_complete_get_trash (send_data->object, send_data->invocation, path);

	ipc (printf("EMailDataStore: get folder : %s %s: %s\n", priv->object_path, send_data->folder_name, path));

	g_mutex_unlock (priv->folders_lock);
	g_mutex_unlock (priv->datafolders_lock);

	g_free (send_data->folder_name);
	g_free (send_data);
	g_free (path);
}

static gboolean
impl_Mail_getFolder (EGdbusStore *object, GDBusMethodInvocation *invocation, const gchar *full_name, guint32 flags, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	CamelFolder *folder;
	gchar *path=NULL;
	EMailGetFolderData *send_data;
	EMailDataFolder *efolder = NULL;

	folder = g_hash_table_lookup (priv->folders, full_name);

	ipc (printf("EMailDataStore: get folder %s - %s\n", priv->object_path, full_name));

	if (folder == NULL) {
		char *new_name = g_strdup (full_name);
		GCancellable *ops = camel_operation_new ();

		send_data = g_new0 (EMailGetFolderData, 1);
		send_data->mstore = mstore;
		send_data->object = object;
		send_data->folder_name = new_name;
		send_data->invocation = invocation;

		camel_store_get_folder ((CamelStore *)priv->store, full_name, flags, 
			G_PRIORITY_DEFAULT, ops,
			(GAsyncReadyCallback) handle_mail_get_folder, send_data);

		return TRUE;
	}


	efolder = g_hash_table_lookup (priv->datafolders, full_name);
	path = (char *) e_mail_data_folder_get_path (efolder);

	ipc (printf("EMailDataStore: get folder success: %s %s: %s d\n", priv->object_path, full_name, path));

	egdbus_store_complete_get_folder (object, invocation, path);

	return TRUE;
}

static gboolean
impl_Mail_getInbox (EGdbusStore *object, GDBusMethodInvocation *invocation, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	EMailGetFolderData *send_data;
	GCancellable *ops = camel_operation_new ();

	send_data = g_new0 (EMailGetFolderData, 1);
	send_data->mstore = mstore;
	send_data->object = object;
	send_data->inbox = TRUE;
	send_data->invocation = invocation;

	ipc (printf("EMailDataStore: get inbox: %s\n", priv->object_path));
	camel_store_get_inbox_folder ((CamelStore *)priv->store, 
		G_PRIORITY_DEFAULT, ops,
		(GAsyncReadyCallback) handle_mail_get_folder, send_data);

	return TRUE;

}

static gboolean
impl_Mail_getTrash (EGdbusStore *object, GDBusMethodInvocation *invocation, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	EMailGetFolderData *send_data;
	GCancellable *ops = camel_operation_new ();

	send_data = g_new0 (EMailGetFolderData, 1);
	send_data->mstore = mstore;
	send_data->object = object;
	send_data->trash = TRUE;
	send_data->invocation = invocation;

	ipc (printf("EMailDataStore: get trash: %s\n", priv->object_path));
	camel_store_get_trash_folder ((CamelStore *)priv->store, 
		G_PRIORITY_DEFAULT, ops,
		(GAsyncReadyCallback) handle_mail_get_folder, send_data);
	
	return TRUE;

}

static gboolean
impl_Mail_getJunk (EGdbusStore *object, GDBusMethodInvocation *invocation, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	EMailGetFolderData *send_data;
	GCancellable *ops = camel_operation_new ();

	send_data = g_new0 (EMailGetFolderData, 1);
	send_data->mstore = mstore;
	send_data->object = object;
	send_data->junk = TRUE;
	send_data->invocation = invocation;

	ipc (printf("EMailDataStore: get junk: %s\n", priv->object_path));
	camel_store_get_junk_folder ((CamelStore *)priv->store, 
		G_PRIORITY_DEFAULT, ops,
		(GAsyncReadyCallback) handle_mail_get_folder, send_data);
	
	return TRUE;

}

typedef struct _email_cdr_folder_data {
	EMailDataStore *mstore;
	EGdbusStore *object;
	GDBusMethodInvocation *invocation;
}EMailCDRFolderData;

static void 
handle_create_folder_cb (CamelStore *store,
			 GAsyncResult *result,
			 EMailCDRFolderData *send_data)
{
	EMailDataStore *mstore = send_data->mstore;
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);	
	GVariantBuilder *builder;
	GVariant *variant;
	CamelFolderInfo *fi;
	GError *error=NULL;

	fi = camel_store_create_folder_finish (store, result, &error);
	if (!fi) {
		/* Handle error */
		g_warning ("Unable to create folder: %s\n", error->message);
		ipc (printf("EMailDataStore: folder create failed : %s - %s\n", priv->object_path, error->message));

		g_dbus_method_invocation_return_gerror (send_data->invocation, error);
		return;
	}
	
	builder = g_variant_builder_new (G_VARIANT_TYPE_ARRAY);

	convert_folder_info (fi, builder);
	/* Added a empty entry */
	g_variant_builder_add (builder, "(ssuii)", "", "", 0, -1, -1);
	
	variant = g_variant_builder_end (builder);
	g_variant_ref (variant);
	g_variant_builder_unref (builder);
	
	ipc (printf("EMailDataStore: folder create success: %s\n", priv->object_path));

	egdbus_store_complete_create_folder (send_data->object, send_data->invocation, variant);
	g_variant_unref (variant);

	g_free (send_data);
}

static gboolean
impl_Mail_createFolder (EGdbusStore *object, GDBusMethodInvocation *invocation, const char *parent, const char *folder_name, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	EMailCDRFolderData *send_data;
	GCancellable *ops = camel_operation_new ();

	send_data = g_new0 (EMailCDRFolderData, 1);
	send_data->mstore = mstore;
	send_data->object = object;
	send_data->invocation = invocation;
	
	ipc (printf("EMailDataStore: folder create: %s\n", folder_name));
	camel_store_create_folder ((CamelStore *)priv->store, parent, folder_name, 
		G_PRIORITY_DEFAULT, ops,
		(GAsyncReadyCallback) handle_create_folder_cb, send_data);

	return TRUE;

}

static void 
handle_delete_folder_cb (CamelStore *store,
			 GAsyncResult *result,
			 EMailCDRFolderData *send_data)
{
	EMailDataStore *mstore = send_data->mstore;
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	GError *error = NULL;
	gboolean success;

	success = camel_store_delete_folder_finish (store, result, &error);
	if (error && error->message) {
		/* Handle error */
		g_warning ("Unable to delete folder: %s\n", error->message);
		ipc (printf("EMailDataStore: folder delete failed : %s - %s\n", priv->object_path, error->message));

		g_dbus_method_invocation_return_gerror (send_data->invocation, error);
		return;
	}
	
	ipc (printf("EMailDataStore: folder delete success: %s: %d\n", priv->object_path, success));

	egdbus_store_complete_delete_folder (send_data->object, send_data->invocation, success);

	g_free (send_data);
}

static gboolean
impl_Mail_deleteFolder (EGdbusStore *object, GDBusMethodInvocation *invocation, const char *folder_name, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	EMailCDRFolderData *send_data;
	GCancellable *ops = camel_operation_new ();

	send_data = g_new0 (EMailCDRFolderData, 1);
	send_data->mstore = mstore;
	send_data->object = object;
	send_data->invocation = invocation;

	ipc (printf("EMailDataStore: folder delete: %s - %s\n", priv->object_path, folder_name));
	camel_store_delete_folder ((CamelStore *)priv->store, folder_name, 
		G_PRIORITY_DEFAULT, ops,
		(GAsyncReadyCallback) handle_delete_folder_cb, send_data);

	return TRUE;

}

static void 
handle_rename_folder_cb (CamelStore *store,
			 GAsyncResult *result,
			 EMailCDRFolderData *send_data)
{
	EMailDataStore *mstore = send_data->mstore;
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	GError *error = NULL;
	gboolean success;

	success = camel_store_rename_folder_finish (store, result, &error);
	if (error && error->message) {
		/* Handle error */
		g_warning ("Unable to rename folder: %s\n", error->message);
		ipc (printf("EMailDataStore: folder rename failed : %s - %s\n", priv->object_path, error->message));

		g_dbus_method_invocation_return_gerror (send_data->invocation, error);
		return;
	}
	ipc (printf("EMailDataStore: folder rename success: %s: %d\n", priv->object_path, success));
	
	egdbus_store_complete_rename_folder (send_data->object, send_data->invocation, success);

	g_free (send_data);
}


static gboolean
impl_Mail_renameFolder (EGdbusStore *object, GDBusMethodInvocation *invocation, const char *old_name, const char *new_name, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	EMailCDRFolderData *send_data;
	GCancellable *ops = camel_operation_new ();

	send_data = g_new0 (EMailCDRFolderData, 1);
	send_data->mstore = mstore;
	send_data->object = object;
	send_data->invocation = invocation;

	ipc (printf("EMailDataStore: folder rename: %s: %s to %s\n", priv->object_path, old_name, new_name));
	camel_store_rename_folder ((CamelStore *)priv->store, old_name, new_name,
		G_PRIORITY_DEFAULT, ops,
		(GAsyncReadyCallback) handle_rename_folder_cb, send_data);

	return TRUE;

}

static void
handle_mail_sync (CamelStore *store,
		  GAsyncResult *result,
		  EMailCDRFolderData *send_data)
{
	EMailDataStore *mstore = send_data->mstore;
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	GError *error=NULL;
	gboolean success;

	success = camel_store_synchronize_finish (store, result, &error);
	if (error && error->message) {
		g_warning ("Error while syncing store: %s\n", error->message);
		ipc (printf("EMailDataStore: sync: failed: %s %s\n", priv->object_path, error->message));

		g_dbus_method_invocation_return_gerror (send_data->invocation, error);
		return;
	}
	ipc (printf("EMailDataStore: sync: success: %s\n", priv->object_path));

	egdbus_store_complete_sync (send_data->object, send_data->invocation, success);
}

static gboolean
impl_Mail_sync (EGdbusStore *object, GDBusMethodInvocation *invocation, gboolean expunge, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	EMailCDRFolderData *send_data;
	GCancellable *ops = camel_operation_new ();

	send_data = g_new0 (EMailCDRFolderData, 1);
	send_data->mstore = mstore;
	send_data->object = object;
	send_data->invocation = invocation;
	
	ipc (printf("EMailDataStore: sync: %s\n", priv->object_path));
	camel_store_synchronize ((CamelStore *)priv->store, expunge,
		G_PRIORITY_DEFAULT, ops,
		(GAsyncReadyCallback) handle_mail_sync, send_data);

	return TRUE;
}

static void
handle_mail_noop (CamelStore *store,
 		  GAsyncResult *result,
		  EMailCDRFolderData *send_data)
{
	EMailDataStore *mstore = send_data->mstore;
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	GError *error = NULL;
	gboolean success;

	success = camel_store_noop_finish ((CamelStore *)priv->store, result, &error);

	if (error && error->message) {
		g_warning ("Error while noop for store: %s\n", error->message);
		ipc (printf("EMailDataStore: noop: failed: %s %s\n", priv->object_path, error->message));

		g_dbus_method_invocation_return_gerror (send_data->invocation, error);
		return;
	}

	ipc (printf("EMailDataStore: noop: success: %s\n", priv->object_path));
	egdbus_store_complete_noop (send_data->object, send_data->invocation, success);
}


static gboolean
impl_Mail_noop (EGdbusStore *object, GDBusMethodInvocation *invocation, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	EMailCDRFolderData *send_data;
	GCancellable *ops = camel_operation_new ();

	send_data = g_new0 (EMailCDRFolderData, 1);
	send_data->mstore = mstore;
	send_data->object = object;
	send_data->invocation = invocation;

	ipc (printf("EMailDataStore: noop: %s\n", priv->object_path));
	camel_store_noop ((CamelStore *)priv->store, 
		G_PRIORITY_DEFAULT, ops,
		(GAsyncReadyCallback) handle_mail_noop, send_data);

	return TRUE;
}

static gboolean
impl_Mail_canRefreshFolder (EGdbusStore *object, GDBusMethodInvocation *invocation, GVariant *variant, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	CamelFolderInfo *info = NULL;
	GVariantIter iter;
	char *full_name, *name;
	guint32 flag;
	int total, unread;
	gboolean can_refresh;
	GError *error = NULL;

	g_variant_iter_init (&iter, variant);
	while (g_variant_iter_next (&iter, "(ssuii)", &name, &full_name, &flag, &unread, &total)) {
		if (name && *name) {
			info = camel_folder_info_new ();
			info->display_name = name;
			info->full_name = full_name;
			info->flags = flag;
			info->unread = unread;
			info->total = total;
			ipc (printf("EMailDataStore: can refresh %s\n", info->full_name));

			break;
		}
	}

	can_refresh = camel_store_can_refresh_folder ((CamelStore *)priv->store, info, &error);
	if (error && error->message) {
		g_warning ("Error while  can refresh folder : %s\n", error->message);
		ipc (printf("EMailDataStore: Refresh Folder: failed: %s\n", error->message));
		g_dbus_method_invocation_return_gerror (invocation, error);
		return TRUE;
	}

	ipc (printf("EMailDataStore: Can Refresh Folder: success\n"));

	egdbus_store_complete_can_refresh_folder (object, invocation, can_refresh);
	camel_folder_info_free (info);

	return TRUE;
}

/* Subscriptions */

static gboolean
impl_Mail_supportsSubscriptions (EGdbusStore *object, GDBusMethodInvocation *invocation, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	gboolean support;

	support = CAMEL_IS_SUBSCRIBABLE (priv->store);
	
	ipc (printf("EMailDataStore: supports subscription: %s - %d\n", priv->object_path, support));

	egdbus_store_complete_supports_subscriptions (object, invocation, support);

	return TRUE;
}

typedef struct _email_folder_sub_data {
	EMailDataStore *mstore;
	EGdbusStore *object;
	GDBusMethodInvocation *invocation;
	gboolean subscribe;
}EMailFolderSubData;

static void 
handle_folder_subscriptions (CamelStore *store,
 			     GAsyncResult *result,
			     EMailFolderSubData *send_data)
{
	EMailDataStore *mstore = send_data->mstore;
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	GError *error=NULL;
	gboolean state;

	if (send_data->subscribe)
		state = camel_subscribable_subscribe_folder_finish (CAMEL_SUBSCRIBABLE(priv->store), result, &error);
	else
		state = camel_subscribable_unsubscribe_folder_finish (CAMEL_SUBSCRIBABLE(priv->store), result, &error);

	if (error && error->message) {
		g_warning ("folder subscription: %s\n", error->message);
		ipc (printf("EMailDataStore: folder-sub failed : %s - %d: %s\n", priv->object_path, send_data->subscribe, error->message));
		
		g_dbus_method_invocation_return_gerror (send_data->invocation, error);
		return;
	}

	if (send_data->subscribe)
		egdbus_store_complete_subscribe_folder (send_data->object, send_data->invocation, state);
	else
		egdbus_store_complete_unsubscribe_folder (send_data->object, send_data->invocation, state);

	ipc (printf("EMailDataStore: folder-sub success: %s - %d: %d\n", priv->object_path, send_data->subscribe, state));

	g_free (send_data);
	return;
}

static gboolean
impl_Mail_isFolderSubscribed (EGdbusStore *object, GDBusMethodInvocation *invocation, char *folder, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	gboolean state;

	ipc (printf("EMailDataStore: folder issubscribed: %s\n", priv->object_path));

	state = camel_subscribable_folder_is_subscribed (CAMEL_SUBSCRIBABLE(priv->store), folder);
	egdbus_store_complete_is_folder_subscribed (object, invocation, state);

	return TRUE;
}

static gboolean
impl_Mail_subscribeFolder (EGdbusStore *object, GDBusMethodInvocation *invocation, char *folder, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	EMailFolderSubData *send_data;
	GCancellable *ops = camel_operation_new ();

	send_data = g_new0 (EMailFolderSubData, 1);
	send_data->mstore = mstore;
	send_data->object = object;
	send_data->invocation = invocation;
	send_data->subscribe = TRUE;

	ipc (printf("EMailDataStore: folder subscribe: %s\n", priv->object_path));
	camel_subscribable_subscribe_folder (CAMEL_SUBSCRIBABLE(priv->store), folder,
		G_PRIORITY_DEFAULT, ops,
		(GAsyncReadyCallback) handle_folder_subscriptions, send_data);

	return TRUE;
}

static gboolean
impl_Mail_unsubscribeFolder (EGdbusStore *object, GDBusMethodInvocation *invocation, char *folder, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	EMailFolderSubData *send_data;
	GCancellable *ops = camel_operation_new ();

	send_data = g_new0 (EMailFolderSubData, 1);
	send_data->mstore = mstore;
	send_data->object = object;
	send_data->invocation = invocation;
	send_data->subscribe = FALSE;

	ipc (printf("EMailDataStore: folder unsubscribe: %s\n", priv->object_path));
	camel_subscribable_unsubscribe_folder (CAMEL_SUBSCRIBABLE(priv->store), folder,
		G_PRIORITY_DEFAULT, ops,
		(GAsyncReadyCallback) handle_folder_subscriptions, send_data);
	return TRUE;
}

/* Camel Service APIs */
static gboolean
impl_Mail_getDisplayName (EGdbusStore *object, GDBusMethodInvocation *invocation, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	const char *display_name;

	display_name = camel_service_get_display_name (priv->store);
	egdbus_store_complete_get_display_name (object, invocation, display_name);

	return TRUE;
}

static gboolean
impl_Mail_setDisplayName (EGdbusStore *object, GDBusMethodInvocation *invocation, const char *full_name, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	
	camel_service_set_display_name (priv->store, full_name);
	egdbus_store_complete_set_display_name (object, invocation);

	return TRUE;
}

static gboolean
impl_Mail_getPassword (EGdbusStore *object, GDBusMethodInvocation *invocation, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	const char *password;

	password = camel_service_get_password (priv->store);
	egdbus_store_complete_get_password (object, invocation, password);

	return TRUE;
}

static gboolean
impl_Mail_setPassword (EGdbusStore *object, GDBusMethodInvocation *invocation, const char *password, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);

	camel_service_set_password (priv->store, password);
	egdbus_store_complete_set_password (object, invocation);

	return TRUE;
}

static gboolean
impl_Mail_getUserDataDir (EGdbusStore *object, GDBusMethodInvocation *invocation, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	const char *dir;

	dir = camel_service_get_user_data_dir (priv->store);
	egdbus_store_complete_get_user_data_dir (object, invocation, dir);

	return TRUE;
}

static gboolean
impl_Mail_getUserCacheDir (EGdbusStore *object, GDBusMethodInvocation *invocation, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	const char *dir;

	dir = camel_service_get_user_cache_dir (priv->store);
	egdbus_store_complete_get_user_cache_dir (object, invocation, dir);

	return TRUE;
}

static gboolean
impl_Mail_getName (EGdbusStore *object, GDBusMethodInvocation *invocation, gboolean brief, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	char *name;

	name = camel_service_get_name (priv->store, brief);
	egdbus_store_complete_get_name (object, invocation, name);
	g_free (name);

	return TRUE;
}

static gboolean
impl_Mail_getProviderFlags (EGdbusStore *object, GDBusMethodInvocation *invocation, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	CamelProvider *provider;
	guint flags;

	provider = camel_service_get_provider (priv->store);
	flags = provider->flags;
	egdbus_store_complete_get_provider_flags (object, invocation, flags);

	return TRUE;
}

static gboolean
impl_Mail_getProviderUrlFlags (EGdbusStore *object, GDBusMethodInvocation *invocation, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	CamelProvider *provider;
	guint32 flags;

	provider = camel_service_get_provider (priv->store);
	flags = provider->url_flags;
	egdbus_store_complete_get_provider_url_flags (object, invocation, flags);

	return TRUE;
}


static gboolean
impl_Mail_getUid (EGdbusStore *object, GDBusMethodInvocation *invocation, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	const char *uid;

	uid = camel_service_get_uid (priv->store);
	egdbus_store_complete_get_uid (object, invocation, uid);

	return TRUE;
}

static gboolean
impl_Mail_getUrl (EGdbusStore *object, GDBusMethodInvocation *invocation, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	char *url;	

	url = mail_get_service_url (priv->store);
	egdbus_store_complete_get_url (object, invocation, url);
	g_free (url);

	return TRUE;
}
#define CHECK_NULL(data) (data && *data)?data:""
static void
handle_get_auth_types (CamelService *service, GAsyncResult *result, EMailCDRFolderData *send_data)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(send_data->mstore);
	GList *list, *iter;
	GError *error = NULL;
	GVariantBuilder *builder;
	GVariant *variant;

	list = camel_service_query_auth_types_finish (service, result, &error);
	if (error && error->message) {
		g_warning ("Get AuthTypes failed: %s\n", error->message);
		ipc (printf("EMailDataStore: Get Auth Types failed : %s : %s\n", priv->object_path, error->message));
		
		g_dbus_method_invocation_return_gerror (send_data->invocation, error);
		return;
	}

	builder = g_variant_builder_new (G_VARIANT_TYPE_ARRAY);

	iter = list;
	while (iter) {
		CamelServiceAuthType *atype = (CamelServiceAuthType *)iter->data;

		g_variant_builder_add (builder, "(sssb)", CHECK_NULL(atype->name), CHECK_NULL(atype->description), CHECK_NULL(atype->authproto), atype->need_password);	
		iter = iter->next;
	}
	/* Added a empty entry */
	g_variant_builder_add (builder, "(sssb)", "", "", "", FALSE);
	
	variant = g_variant_builder_end (builder);
	g_variant_ref (variant);
	g_variant_builder_unref (builder);
	
	ipc (printf("EMailDataStore: get auth types success: %s\n", priv->object_path));
	egdbus_store_complete_get_auth_types (send_data->object, send_data->invocation, variant);
	g_variant_unref (variant);
	g_list_free (list);

}

static gboolean
impl_Mail_getAuthTypes (EGdbusStore *object, GDBusMethodInvocation *invocation, EMailDataStore *mstore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);
	GCancellable *ops = camel_operation_new ();
	EMailCDRFolderData *send_data;

	send_data = g_new0 (EMailCDRFolderData, 1);
	send_data->mstore = mstore;
	send_data->invocation = invocation;
	send_data->object = object;

	camel_service_query_auth_types (priv->store, 
		G_PRIORITY_DEFAULT, ops,
		(GAsyncReadyCallback) handle_get_auth_types, send_data);

	return TRUE;
}

typedef struct _email_store_std_data {
	EMailDataStore *mstore;
	EGdbusStore *object;
	GDBusMethodInvocation *invocation;
	char *command;
	GPtrArray *uids;
	GPtrArray *folder_names;
}EMailStoreStdData;

static gint
read_folder_names (gpointer ref_hash,
		   gint ncol,
		   gchar **cols,
		   gchar **name)
{
	GList **list = ref_hash;

	g_return_val_if_fail (ncol == 1, 0);

	if (cols[0]) {
		if (g_ascii_strncasecmp (cols[0], ".#evo", 5) != 0) /* Ignore vtrash & vjunk */
			*list = g_list_prepend (*list, g_strdup (cols[0]));
	}

	return 0;
}

static gint
read_uids_to_array_callback (gpointer sdata,
                            gint ncol,
                            gchar **cols,
                            gchar **name)
{
	EMailStoreStdData *data = (EMailStoreStdData *)sdata;

	g_return_val_if_fail (ncol == 2, 0);

	if (cols[0] && cols[1]) {
		g_ptr_array_add (data->folder_names, g_strdup(cols[0]));
		g_ptr_array_add (data->uids, g_strdup(cols[1]));
	}

	return 0;
}


static gboolean
sbs_operate (GObject *object, gpointer sdata, GError **error)
{
	EMailStoreStdData *data = (EMailStoreStdData *)sdata;
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(data->mstore);
	CamelStore *store = (CamelStore *) object;
	CamelDB *db;
	GList *list = NULL;
	GString *create_query;
	

	create_query = g_string_new ("CREATE VIEW AllFoldersView AS ");

	/* Find the folder names first. */
	db = store->cdb_r;
	camel_db_select (db, "SELECT folder_name FROM folders", read_folder_names, &list, error);
	if (list) {
		gboolean once_tmp = FALSE;
		gchar *select_query;

		GList *tmp = list;
		while (tmp) {
			gchar *tbl_query;
	
			if (once_tmp)
				g_string_append (create_query, "UNION ");

			tbl_query = sqlite3_mprintf ("SELECT %Q AS folder_name, uid, subject, mail_from, mail_to, mail_cc, flags, part FROM %Q", tmp->data, tmp->data);

			g_string_append (create_query, tbl_query);
			if (!once_tmp)
				once_tmp = TRUE;

			sqlite3_free (tbl_query);
			tmp=tmp->next;
		}
		camel_db_command (db, create_query->str, error);
		g_string_free (create_query, TRUE);

		/* Now that we have the view, just execute the command */
		data->folder_names = g_ptr_array_new ();
		data->uids = g_ptr_array_new ();
		select_query = g_strconcat("SELECT folder_name, uid from AllFoldersView WHERE ", data->command, NULL);
		camel_db_select (db, select_query, read_uids_to_array_callback, data, error);
		g_free (select_query);

		/* Drop the VIEW */
		camel_db_command (db, "DROP VIEW AllFoldersView", error);
	}

	return TRUE;
}

static void
sbs_done (gboolean success, gpointer sdata, GError *error)
{
	EMailStoreStdData *data = (EMailStoreStdData *)sdata;
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(data->mstore);

	if (error && error->message) {
		g_warning ("SearchBySQL failed: %s: %s\n", priv->object_path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Search by SQL : %s failed: %s\n", priv->object_path, error->message));
		return;
	}

	g_ptr_array_add (data->folder_names , NULL);
	g_ptr_array_add (data->uids, NULL);

	egdbus_store_complete_search_by_sql (data->object, data->invocation, (const gchar *const *)data->uids->pdata, (const gchar *const *)data->folder_names->pdata);

	g_ptr_array_remove_index_fast (data->uids, data->uids->len-1);
	g_ptr_array_remove_index_fast (data->folder_names, data->folder_names->len-1);

	g_ptr_array_foreach (data->uids, (GFunc)g_free, NULL);
	g_ptr_array_foreach (data->folder_names, (GFunc)g_free, NULL);

	g_free (data->command);
	g_ptr_array_free (data->folder_names, TRUE);
	g_ptr_array_free (data->uids, TRUE);
	g_free (data);
}

static gboolean
impl_Mail_searchBySql (EGdbusStore *object, GDBusMethodInvocation *invocation, const char *sql, EMailDataStore *mstore)
{
	EMailStoreStdData *data;
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(mstore);

	ipc(printf("Executing SQL command: %s\n", sql));

	data = g_new0 (EMailStoreStdData, 1);
	data->mstore = mstore;
	data->invocation = invocation;
	data->object = object;
	data->command = g_strdup(sql);

	mail_operate_on_object ((GObject *)priv->store, sbs_operate, sbs_done, data);

	return TRUE;
}

static void
e_mail_data_store_init (EMailDataStore *self)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(self);

	priv->object_path = NULL;

	priv->folders_lock = g_mutex_new ();
	priv->folders = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

	priv->datafolders_lock = g_mutex_new ();
	priv->datafolders = g_hash_table_new_full (
		g_str_hash, g_str_equal,
		(GDestroyNotify) g_free,
		(GDestroyNotify) NULL);

	priv->gdbus_object = egdbus_store_skeleton_new ();
	g_signal_connect (priv->gdbus_object, "handle-get-folder", G_CALLBACK (impl_Mail_getFolder), self);
	g_signal_connect (priv->gdbus_object, "handle-get-folder-info", G_CALLBACK (impl_Mail_getFolderInfo), self);
	g_signal_connect (priv->gdbus_object, "handle-get-inbox", G_CALLBACK (impl_Mail_getInbox), self);
	g_signal_connect (priv->gdbus_object, "handle-get-trash", G_CALLBACK (impl_Mail_getTrash), self);
	g_signal_connect (priv->gdbus_object, "handle-get-junk", G_CALLBACK (impl_Mail_getJunk), self);
	g_signal_connect (priv->gdbus_object, "handle-create-folder", G_CALLBACK (impl_Mail_createFolder), self);
	g_signal_connect (priv->gdbus_object, "handle-delete-folder", G_CALLBACK (impl_Mail_deleteFolder), self);
	g_signal_connect (priv->gdbus_object, "handle-rename-folder", G_CALLBACK (impl_Mail_renameFolder), self);
	g_signal_connect (priv->gdbus_object, "handle-sync", G_CALLBACK (impl_Mail_sync), self);
	g_signal_connect (priv->gdbus_object, "handle-noop", G_CALLBACK (impl_Mail_noop), self);
	g_signal_connect (priv->gdbus_object, "handle-can-refresh-folder", G_CALLBACK (impl_Mail_canRefreshFolder), self);

	g_signal_connect (priv->gdbus_object, "handle-supports-subscriptions", G_CALLBACK (impl_Mail_supportsSubscriptions), self);
	g_signal_connect (priv->gdbus_object, "handle-is-folder-subscribed", G_CALLBACK (impl_Mail_isFolderSubscribed), self);
	g_signal_connect (priv->gdbus_object, "handle-subscribe-folder", G_CALLBACK (impl_Mail_subscribeFolder), self);
	g_signal_connect (priv->gdbus_object, "handle-unsubscribe-folder", G_CALLBACK (impl_Mail_unsubscribeFolder), self);

	g_signal_connect (priv->gdbus_object, "handle-get-display-name", G_CALLBACK (impl_Mail_getDisplayName), self);
	g_signal_connect (priv->gdbus_object, "handle-set-display-name", G_CALLBACK (impl_Mail_setDisplayName), self);
	g_signal_connect (priv->gdbus_object, "handle-get-password", G_CALLBACK (impl_Mail_getPassword), self);
	g_signal_connect (priv->gdbus_object, "handle-set-password", G_CALLBACK (impl_Mail_setPassword), self);
	g_signal_connect (priv->gdbus_object, "handle-get-user-data-dir", G_CALLBACK (impl_Mail_getUserDataDir), self);
	g_signal_connect (priv->gdbus_object, "handle-get-user-cache-dir", G_CALLBACK (impl_Mail_getUserCacheDir), self);
	g_signal_connect (priv->gdbus_object, "handle-get-name", G_CALLBACK (impl_Mail_getName), self);
	g_signal_connect (priv->gdbus_object, "handle-get-provider-flags", G_CALLBACK (impl_Mail_getProviderFlags), self);
	g_signal_connect (priv->gdbus_object, "handle-get-provider-url-flags", G_CALLBACK (impl_Mail_getProviderUrlFlags), self);		
	g_signal_connect (priv->gdbus_object, "handle-get-uid", G_CALLBACK (impl_Mail_getUid), self);
	g_signal_connect (priv->gdbus_object, "handle-get-url", G_CALLBACK (impl_Mail_getUrl), self);
	g_signal_connect (priv->gdbus_object, "handle-get-auth-types", G_CALLBACK (impl_Mail_getAuthTypes), self);
	g_signal_connect (priv->gdbus_object, "handle-search-by-sql", G_CALLBACK (impl_Mail_searchBySql), self);

}

EMailDataStore*
e_mail_data_store_new (CamelService *store, const char *url)
{
	EMailDataStore *estore;
	EMailDataStorePrivate *priv;

  	estore = g_object_new (EMAIL_TYPE_DATA_STORE, NULL);
	priv = DATA_STORE_PRIVATE(estore);
	priv->store = g_object_ref(store);
	priv->url = g_strdup (url);

	return estore;
}

static GVariant *
variant_from_info (CamelFolderInfo *info)
{
	GVariantBuilder *builder;
	GVariant *variant;

	builder = g_variant_builder_new (G_VARIANT_TYPE_ARRAY);

	convert_folder_info (info, builder);
	/* Added a empty entry */
	g_variant_builder_add (builder, "(ssuii)", "", "", 0, -1, -1);
	
	variant = g_variant_builder_end (builder);
	g_variant_ref (variant);
	g_variant_builder_unref (builder);

	return variant;
}

static void
store_folder_subscribed_cb (CamelStore *store,
                            CamelFolderInfo *info,
                            EMailDataStore *estore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(estore);
	GVariant *variant;

	variant = variant_from_info (info);
	ipc(printf("Emitting Folder subscribed: %s\n", info->full_name));		
	egdbus_store_emit_folder_subscribed (priv->gdbus_object, variant);

	g_variant_unref (variant);
}

static void
store_folder_unsubscribed_cb (CamelStore *store,
                              CamelFolderInfo *info,
                              EMailDataStore *estore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(estore);
	GVariant *variant;

	variant = variant_from_info (info);
	ipc(printf("Emitting Folder unsubscribed: %s\n", info->full_name));		
	egdbus_store_emit_folder_unsubscribed (priv->gdbus_object, variant);

	g_variant_unref (variant);
	
}

static void
store_folder_created_cb (CamelStore *store,
                         CamelFolderInfo *info,
                         EMailDataStore *estore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(estore);
	GVariant *variant;

	variant = variant_from_info (info);
	ipc(printf("Emitting Folder created: %s\n", info->full_name));		
	egdbus_store_emit_folder_created (priv->gdbus_object, variant);

	g_variant_unref (variant);
	
}

static void
store_folder_opened_cb (CamelStore *store,
                        CamelFolder *folder,
                        EMailDataStore *estore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(estore);
	const char *path;
	const char *full_name;
	EMailDataFolder *efolder;

	full_name = camel_folder_get_full_name (folder);
	efolder = g_hash_table_lookup (priv->datafolders, full_name);
	if (!efolder) /* Don't bother to return about folders that aren't being used by the client. */
		return;
	path = e_mail_data_folder_get_path (efolder);
	ipc(printf("Emitting Folder opened: %s\n", path));
	egdbus_store_emit_folder_opened (priv->gdbus_object, path);
}

static void
store_folder_deleted_cb (CamelStore *store,
                         CamelFolderInfo *info,
                         EMailDataStore *estore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(estore);
	GVariant *variant;

	variant = variant_from_info (info);
	ipc(printf("Emitting Folder deleted: %s\n", info->full_name));	
	egdbus_store_emit_folder_deleted (priv->gdbus_object, variant);

	g_variant_unref (variant);	
}

static void
store_folder_renamed_cb (CamelStore *store,
                         const gchar *old_name,
                         CamelFolderInfo *info,
                         EMailDataStore *estore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(estore);
	GVariant *variant;

	variant = variant_from_info (info);
	ipc(printf("Emitting Folder renamed: %s %s\n", old_name, info->full_name));	
	egdbus_store_emit_folder_renamed (priv->gdbus_object, old_name, variant);

	g_variant_unref (variant);	
}


guint 
e_mail_data_store_register_gdbus_object (EMailDataStore *estore, GDBusConnection *connection, const gchar *object_path, GError **error)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(estore);

	g_return_val_if_fail (connection != NULL, 0);
	g_return_val_if_fail (object_path != NULL, 0);

	priv->object_path = g_strdup (object_path);
	g_object_set_data ((GObject *)priv->store, "object-path", priv->object_path);

	ipc (printf("EMailDataStore: Registering gdbus path: %s: %p\n", object_path, priv->store));
	
	if (CAMEL_IS_STORE(priv->store)) {
		g_signal_connect (
			priv->store, "folder-opened",
			G_CALLBACK (store_folder_opened_cb), estore);
		g_signal_connect (
			priv->store, "folder-created",
			G_CALLBACK (store_folder_created_cb), estore);
		g_signal_connect (
			priv->store, "folder-deleted",
			G_CALLBACK (store_folder_deleted_cb), estore);
		g_signal_connect (
			priv->store, "folder-renamed",
			G_CALLBACK (store_folder_renamed_cb), estore);
		g_signal_connect (
			priv->store, "folder-subscribed",
			G_CALLBACK (store_folder_subscribed_cb), estore);
		g_signal_connect (
			priv->store, "folder-unsubscribed",
			G_CALLBACK (store_folder_unsubscribed_cb), estore);
	}

 	return g_dbus_interface_skeleton_export ((GDBusInterfaceSkeleton *) priv->gdbus_object,
               	                     		 connection,
						 object_path,
						 error);	
}

const char *
e_mail_data_store_get_path (EMailDataStore *estore)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(estore);

	return priv->object_path;
}

CamelFolder *
e_mail_data_store_get_camel_folder (EMailDataStore *estore, const char *path)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(estore);
	GHashTableIter iter;
	gpointer key, value;

	g_hash_table_iter_init (&iter, priv->datafolders);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		EMailDataFolder *efolder = (EMailDataFolder *)value;
		const char *opath = e_mail_data_folder_get_path (efolder);

		if (strcmp (opath, path) == 0) {
			CamelFolder *f = e_mail_data_folder_get_camel_folder (efolder);
			micro(printf("e_mail_data_store_get_camel_folder: %s %p\n", path, f));

			return f;
		}
	}

	micro(printf("e_mail_data_store_get_camel_folder: %s NULL\n", path));
	return NULL;
}

const char *
e_mail_data_store_get_folder_path (EMailDataStore *estore, GDBusConnection *connection, CamelFolder *folder)
{
	EMailDataStorePrivate *priv = DATA_STORE_PRIVATE(estore);
	const char *full_name;
	EMailDataFolder *efolder;
	gchar *path;

	full_name = camel_folder_get_full_name (folder);


	g_mutex_lock (priv->folders_lock);
	g_mutex_lock (priv->datafolders_lock);
	
	efolder = g_hash_table_lookup (priv->datafolders, full_name);
	if (!efolder) {

		g_hash_table_insert (priv->folders, g_strdup(full_name), folder);
		efolder = e_mail_data_folder_new (folder);
		path = construct_mail_store_path (full_name);
		e_mail_data_folder_register_gdbus_object (efolder, connection, path, NULL);
		g_hash_table_insert (priv->datafolders, g_strdup(full_name), efolder);
		micro (printf("EMailDataStore: Created object from folder : %s %s: %s\n", priv->object_path, full_name, path));
		g_free (path);
	}

	g_mutex_unlock (priv->folders_lock);
	g_mutex_unlock (priv->datafolders_lock);

	return e_mail_data_folder_get_path (efolder);
}
