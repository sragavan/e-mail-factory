/* e-mail-data-folder.c */

#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#include "libemail-engine/e-mail-local.h"
#include "e-mail-data-folder.h"
#include "e-mail-data-session.h"
#include "e-gdbus-emailfolder.h"
#include "mail-send-recv.h"
#include <camel/camel.h>
#include "libemail-engine/mail-ops.h"
#include "utils.h"

#define micro(x) if (mail_debug_log(EMAIL_DEBUG_FOLDER|EMAIL_DEBUG_MICRO)) x;
#define ipc(x) if (mail_debug_log(EMAIL_DEBUG_FOLDER|EMAIL_DEBUG_IPC)) x;

extern EMailSession *session;
extern EMailDataSession *data_session;

G_DEFINE_TYPE (EMailDataFolder, e_mail_data_folder, G_TYPE_OBJECT)

#define DATA_FOLDER_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EMAIL_TYPE_DATA_FOLDER, EMailDataFolderPrivate))

extern EMailDataSession *data_session;
	
typedef struct _EMailDataFolderPrivate EMailDataFolderPrivate;

struct _EMailDataFolderPrivate
{
	char *path;
	EGdbusFolder *gdbus_object;
	
	CamelFolder *folder;
};

static void
e_mail_data_folder_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
e_mail_data_folder_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
e_mail_data_folder_dispose (GObject *object)
{
  EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(object);
  
  g_object_unref (priv->folder);

  G_OBJECT_CLASS (e_mail_data_folder_parent_class)->dispose (object);
}

static void
e_mail_data_folder_finalize (GObject *object)
{
  G_OBJECT_CLASS (e_mail_data_folder_parent_class)->finalize (object);
}

static void
e_mail_data_folder_class_init (EMailDataFolderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (EMailDataFolderPrivate));

  object_class->get_property = e_mail_data_folder_get_property;
  object_class->set_property = e_mail_data_folder_set_property;
  object_class->dispose = e_mail_data_folder_dispose;
  object_class->finalize = e_mail_data_folder_finalize;
}

typedef struct _email_folder_std_data {
	EMailDataFolder *mfolder;
	EGdbusFolder *object;
	GDBusMethodInvocation *invocation;
}EMailFolderStdData;

static void
handle_refresh_info_done (CamelFolder *folder,
			  GAsyncResult *result,
			  EMailFolderStdData *data)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	GError *error = NULL;
	gboolean status;

	//status = camel_folder_refresh_info_finish (folder, result, &error);
	if (error && error->message) {
		g_warning ("Unable to refresh folder: %s\n", error->message);

		ipc(printf("Refresh info failed %s : %s \n", priv->path, error->message));
		/* FIXME: There should be a signal to pass out such generic errors */
		return;
	}
	ipc(printf("Refresh info success: %s \n", priv->path));

	/* For email clients that need downloand completed signal, this should do */
	e_mail_session_emit_send_receive_completed (data_session);
	g_free (data);
}

static gboolean
impl_Mail_refreshInfo (EGdbusFolder *object, GDBusMethodInvocation *invocation, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderStdData *data;
	GCancellable *ops = camel_operation_new ();

	data = g_new0 (EMailFolderStdData, 1);
	data->mfolder = mfolder;
	data->invocation = invocation;
	data->object = object;

	ipc(printf("Mail refresh info %s\n", priv->path));

//	camel_folder_refresh_info (priv->folder,
//		G_PRIORITY_DEFAULT, ops,
//		(GAsyncReadyCallback) handle_refresh_info_done, data);

	/* NOTE: We don't want refreshInfo to be a blocking dbus call. */
	egdbus_folder_complete_refresh_info (object, invocation, TRUE);

	return TRUE;
}

static void
handle_sync_done (CamelFolder *folder,
 		  GAsyncResult *result,
		  EMailFolderStdData *data)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	GError *error = NULL;
	gboolean status;

	status = camel_folder_synchronize_finish (folder, result, &error);

	if (error && error->message) {
		g_warning ("Unable to sync folder: %s\n", error->message);

		g_dbus_method_invocation_return_gerror (data->invocation, error);
		ipc(printf("Sync failed: %s : %s \n", priv->path, error->message));
	
		return;
	}

	ipc(printf("Folder sync success: %s\n", priv->path));
	egdbus_folder_complete_sync (data->object, data->invocation, status);
	g_free (data);
}

static gboolean
impl_Mail_sync (EGdbusFolder *object, GDBusMethodInvocation *invocation, gboolean expunge, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderStdData *data;
	GCancellable *ops = camel_operation_new ();

	data = g_new0 (EMailFolderStdData, 1);
	data->mfolder = mfolder;
	data->invocation = invocation;
	data->object = object;

	ipc(printf("Sync folder %s: exp:%d\n", priv->path, expunge));
	camel_folder_synchronize (priv->folder, expunge,
		G_PRIORITY_DEFAULT, ops,
		(GAsyncReadyCallback) handle_sync_done, data);

	return TRUE;
}  

static void
handle_sync_msg_done (CamelFolder *folder,
 		      GAsyncResult *result,
		      EMailFolderStdData *data)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	GError *error = NULL;
	gboolean status;

	status = camel_folder_synchronize_message_finish (folder, result, &error);
	if (error && error->message) {
		g_warning ("Unable to sync message: %s\n", error->message);

		g_dbus_method_invocation_return_gerror (data->invocation, error);
		ipc(printf("Sync message failed: %s : %s \n", priv->path, error->message));
	
		return;
	}

	ipc(printf("Message sync success: %s\n", priv->path));
	egdbus_folder_complete_sync_message (data->object, data->invocation, status);
	g_free (data);
}

static gboolean
impl_Mail_syncMessage (EGdbusFolder *object, GDBusMethodInvocation *invocation, const char *uid, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderStdData *data;
	GCancellable *ops = camel_operation_new ();

	data = g_new0 (EMailFolderStdData, 1);
	data->mfolder = mfolder;
	data->invocation = invocation;
	data->object = object;

	ipc(printf("Sync message %s: \n", priv->path));
	camel_folder_synchronize_message (priv->folder, uid,
		G_PRIORITY_DEFAULT, ops,
		(GAsyncReadyCallback) handle_sync_msg_done, data);

	return TRUE;
}  

static void
handle_expunge_done (CamelFolder *folder,
 		     GAsyncResult *result,
 		     EMailFolderStdData *data)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	GError *error = NULL;
	gboolean success;

	success = camel_folder_expunge_finish (folder, result, &error);
	if (error && error->message) {
		g_warning ("Unable to expunge folder: %s\n", error->message);

		g_dbus_method_invocation_return_gerror (data->invocation, error);
		ipc(printf("Expunge folder failed: %s : %s \n", priv->path, error->message));

		return;
	}

	egdbus_folder_complete_expunge (data->object, data->invocation, success);
	ipc(printf("Expunge folder success: %s\n", priv->path));
	
	g_free (data);
}

static gboolean
impl_Mail_expunge (EGdbusFolder *object, GDBusMethodInvocation *invocation, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderStdData *data;
	GCancellable *ops = camel_operation_new ();

	data = g_new0 (EMailFolderStdData, 1);
	data->mfolder = mfolder;
	data->invocation = invocation;
	data->object = object;

	ipc(printf("Expunge folder %s\n", priv->path));
	camel_folder_expunge (priv->folder, 
		G_PRIORITY_DEFAULT, ops,
		(GAsyncReadyCallback) handle_expunge_done, data);

	return TRUE;
}

static gboolean
impl_Mail_getDisplayName (EGdbusFolder *object, GDBusMethodInvocation *invocation, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);

	egdbus_folder_complete_get_display_name (object, invocation, camel_folder_get_display_name (priv->folder));
	ipc(printf("Get Name %s : %s \n", priv->path, camel_folder_get_display_name (priv->folder)));

	return TRUE;
}

static gboolean
impl_Mail_setDisplayName (EGdbusFolder *object, GDBusMethodInvocation *invocation, const char *name, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);

	camel_folder_set_display_name (priv->folder, name);
	ipc(printf("Set name %s : %s \n", priv->path, name));

	egdbus_folder_complete_set_display_name (object, invocation);

	return TRUE;
}

static gboolean
impl_Mail_getFullName (EGdbusFolder *object, GDBusMethodInvocation *invocation, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);

	egdbus_folder_complete_get_full_name (object, invocation, camel_folder_get_full_name (priv->folder));
	ipc(printf("Get Fullname %s : %s \n", priv->path, camel_folder_get_full_name (priv->folder)));

	return TRUE;
}

static gboolean
impl_Mail_setFullName (EGdbusFolder *object, GDBusMethodInvocation *invocation, const char *fullname, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);

	camel_folder_set_full_name (priv->folder, fullname);
	ipc(printf("Set full name %s : %s \n", priv->path, fullname));

	egdbus_folder_complete_set_full_name (object, invocation);

	return TRUE;
}

static gboolean
impl_Mail_getDescription (EGdbusFolder *object, GDBusMethodInvocation *invocation, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);

	egdbus_folder_complete_get_description (object, invocation, camel_folder_get_description (priv->folder));
	ipc(printf("Get description %s : %s \n", priv->path, camel_folder_get_description (priv->folder)));

	return TRUE;
}

static gboolean
impl_Mail_setDescription (EGdbusFolder *object, GDBusMethodInvocation *invocation, const char *description, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);

	camel_folder_set_description (priv->folder, description);
	ipc(printf("Set description %s : %s \n", priv->path, description));

	egdbus_folder_complete_set_description (object, invocation);

	return TRUE;
}

static gboolean
impl_Mail_totalMessageCount (EGdbusFolder *object, GDBusMethodInvocation *invocation, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);

	ipc(printf("Get total message count %s : %d \n", priv->path, camel_folder_get_message_count (priv->folder)));

	egdbus_folder_complete_total_message_count (object, invocation, camel_folder_get_message_count (priv->folder));

	return TRUE;
}

static gboolean
impl_Mail_unreadMessageCount (EGdbusFolder *object, GDBusMethodInvocation *invocation, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);

	ipc(printf("Get unread message count %s : %d \n", priv->path, camel_folder_get_unread_message_count (priv->folder)));

	egdbus_folder_complete_unread_message_count (object, invocation, camel_folder_get_unread_message_count (priv->folder));

	return TRUE;
}

static gboolean
impl_Mail_deletedMessageCount (EGdbusFolder *object, GDBusMethodInvocation *invocation, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);

	ipc(printf("Get deleted message count %s : %d \n", priv->path, camel_folder_get_deleted_message_count (priv->folder)));

	egdbus_folder_complete_deleted_message_count (object, invocation, camel_folder_get_deleted_message_count (priv->folder));

	return TRUE;
}

/* Get Permanent flags */
typedef struct _email_folder_gpf_data {
	EMailDataFolder *mfolder;
	EGdbusFolder *object;
	GDBusMethodInvocation *invocation;
	guint32 flags;
}EMailFolderGPFData;

static gboolean
gpf_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	EMailFolderGPFData *data = (EMailFolderGPFData *)sdata;

	data->flags = camel_folder_get_permanent_flags (folder);

	return TRUE;
}

static void
gpf_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderGPFData *data = (EMailFolderGPFData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	
	if (error && error->message) {
		g_warning ("get permanent failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Get Permanent flags: %s failed: %s\n", priv->path, error->message));
		return;
	}
	
	egdbus_folder_complete_get_message_flags (data->object, data->invocation, data->flags);
	ipc(printf("Get permanent flags success %s\n", priv->path));
	
	g_free (data);
}

static gboolean
impl_Mail_getPermanentFlags (EGdbusFolder *object, GDBusMethodInvocation *invocation, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderGPFData *data = g_new0 (EMailFolderGPFData, 1);

	data->object = object;
	data->invocation = invocation;
	data->mfolder = mfolder;

	ipc(printf("Get Permanent flags %s\n", priv->path));

	mail_operate_on_folder (priv->folder, gpf_operate, gpf_done, data);

	return TRUE;
}

/* Has Summary capability */
static gboolean
hsuc_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	/* EMailFolderStdData *data = (EMailFolderStdData *)sdata; */

	return camel_folder_has_summary_capability (folder);
}

static void
hsuc_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderStdData *data = (EMailFolderStdData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	
	if (error && error->message) {
		g_warning ("Has summary capability failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Has summary capability: %s failed: %s\n", priv->path, error->message));
		return;
	}	
	egdbus_folder_complete_has_summary_capability (data->object, data->invocation, success);
	ipc(printf("Has Search capability success%s : %d \n", priv->path, success));
	
	g_free (data);
}

static gboolean
impl_Mail_hasSummaryCapability (EGdbusFolder *object, GDBusMethodInvocation *invocation, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderStdData *data = g_new0 (EMailFolderStdData, 1);

	data->object = object;
	data->invocation = invocation;
	data->mfolder = mfolder;

	ipc(printf("Has Summary capability %s : \n", priv->path));

	mail_operate_on_folder (priv->folder, hsuc_operate, hsuc_done, data);

	return TRUE;
}

/* Has Search Capability */
static gboolean
hsec_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	/* EMailFolderStdData *data = (EMailFolderStdData *)sdata; */

	return camel_folder_has_search_capability (folder);
}

static void
hsec_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderStdData *data = (EMailFolderStdData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	
	if (error && error->message) {
		g_warning ("Has search capability failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Has search capability: %s failed: %s\n", priv->path, error->message));
		return;
	}

	ipc(printf("Has Search capability success%s : %d \n", priv->path, success));
	
	egdbus_folder_complete_has_search_capability (data->object, data->invocation, success);
	g_free (data);
}

static gboolean
impl_Mail_hasSearchCapability (EGdbusFolder *object, GDBusMethodInvocation *invocation, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderStdData *data = g_new0 (EMailFolderStdData, 1);

	data->object = object;
	data->invocation = invocation;
	data->mfolder = mfolder;

	ipc(printf("Has search capability : %s \n", priv->path));

	mail_operate_on_folder (priv->folder, hsec_operate, hsec_done, data);

	return TRUE;
}

/* Get Message Flags */
typedef struct _email_folder_gmf_data {
	EMailDataFolder *mfolder;
	EGdbusFolder *object;
	GDBusMethodInvocation *invocation;
	guint32 flags;
	guint32 set;
	char *uid;
	char *name;
	char *value;
}EMailFolderGMFData;

static gboolean
gmf_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	EMailFolderGMFData *data = (EMailFolderGMFData *)sdata;

	data->flags = camel_folder_get_message_flags (folder, data->uid);

	return TRUE;
}

static void
gmf_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderGMFData *data = (EMailFolderGMFData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	
	if (error && error->message) {
		g_warning ("Get message flags failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Get message flags: %s failed: %s\n", priv->path, error->message));
		return;
	}
	
	ipc(printf("Set message user flag success: %s : %s \n", priv->path, data->uid));
	
	egdbus_folder_complete_get_message_flags (data->object, data->invocation, data->flags);
	g_free (data->uid);
	g_free (data);
}

static gboolean
impl_Mail_getMessageFlags (EGdbusFolder *object, GDBusMethodInvocation *invocation, const char *uid, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderGMFData *data = g_new0 (EMailFolderGMFData, 1);

	data->object = object;
	data->invocation = invocation;
	data->mfolder = mfolder;
	data->uid = g_strdup (uid);

	ipc(printf("Get message flags: %s : %s \n", priv->path, data->uid));
	
	mail_operate_on_folder (priv->folder, gmf_operate, gmf_done, data);

	return TRUE;
}

/* Set Message Flags */
static gboolean
smf_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	EMailFolderGMFData *data = (EMailFolderGMFData *)sdata;

	return camel_folder_set_message_flags (folder, data->uid, data->flags, data->set);

}

static void
smf_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderGMFData *data = (EMailFolderGMFData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	
	if (error && error->message) {
		g_warning ("set message flags failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Set message flags: %s failed: %s\n", priv->path, error->message));
		return;
	}

	ipc(printf("Set message flags success: %s : %s \n", priv->path, data->uid));
	
	egdbus_folder_complete_set_message_flags (data->object, data->invocation, success);
	g_free (data->uid);
	g_free (data);
}

static gboolean
impl_Mail_setMessageFlags (EGdbusFolder *object, GDBusMethodInvocation *invocation, const char *uid, guint32 flags, guint32 set, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderGMFData *data = g_new0 (EMailFolderGMFData, 1);

	data->object = object;
	data->invocation = invocation;
	data->mfolder = mfolder;
	data->uid = g_strdup (uid);
	data->flags = flags;
	data->set = set;

	ipc(printf("Set message flags: %s : %s \n", priv->path, data->uid));
	mail_operate_on_folder (priv->folder, smf_operate, smf_done, data);

	return TRUE;
}

/* Get Message User Flag */
static gboolean
gmuf_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	EMailFolderGMFData *data = (EMailFolderGMFData *)sdata;

	return camel_folder_get_message_user_flag (folder, data->uid, data->name);
}

static void
gmuf_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderGMFData *data = (EMailFolderGMFData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	
	if (error && error->message) {
		g_warning ("Get message user flag failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Get messgae user flag: %s failed: %s\n", priv->path, error->message));
		return;
	}
	
	egdbus_folder_complete_get_message_user_flag (data->object, data->invocation, success);
	ipc(printf("Set message user flag success: %s : %s %s\n", priv->path, data->uid, data->name));
	
	g_free (data->name);
	g_free (data->uid);
	g_free (data);
}

static gboolean
impl_Mail_getMessageUserFlag (EGdbusFolder *object, GDBusMethodInvocation *invocation, const char *uid, char *name, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderGMFData *data = g_new0 (EMailFolderGMFData, 1);

	data->object = object;
	data->invocation = invocation;
	data->mfolder = mfolder;
	data->uid = g_strdup (uid);
	data->name = g_strdup (name);

	ipc(printf("Get message user flag: %s : %s %s\n", priv->path, data->uid, name));
	
	mail_operate_on_folder (priv->folder, gmuf_operate, gmuf_done, data);

	return TRUE;
}

/* Set Message User Flag */
static gboolean
smuf_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	EMailFolderGMFData *data = (EMailFolderGMFData *)sdata;

	camel_folder_set_message_user_flag (folder, data->uid, data->name, (data->flags != 0));

	return TRUE;
}

static void
smuf_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderGMFData *data = (EMailFolderGMFData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	
	if (error && error->message) {
		g_warning ("Set message user flag failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Set message user flag: %s failed: %s\n", priv->path, error->message));
		return;
	}
	
	egdbus_folder_complete_set_message_user_flag (data->object, data->invocation);
	ipc(printf("Set message user fag success: %s : %s:%s \n", priv->path, data->uid, data->name));

	g_free (data->name);
	g_free (data->uid);
	g_free (data);
}

static gboolean
impl_Mail_setMessageUserFlag (EGdbusFolder *object, GDBusMethodInvocation *invocation, const char *uid, const char *name, gboolean flag, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderGMFData *data = g_new0 (EMailFolderGMFData, 1);

	data->object = object;
	data->invocation = invocation;
	data->mfolder = mfolder;
	data->uid = g_strdup (uid);
	data->name = g_strdup (name);
	data->flags = flag != FALSE;

	ipc(printf("Set message user flag: %s : %s %s\n", priv->path, data->uid, name));
	
	mail_operate_on_folder (priv->folder, smuf_operate, smuf_done, data);

	return TRUE;
}

/* Get Message User Tag */
static gboolean
gmut_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	EMailFolderGMFData *data = (EMailFolderGMFData *)sdata;

	data->value = g_strdup (camel_folder_get_message_user_tag (folder, data->uid, data->name));

	return TRUE;
}

static void
gmut_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderGMFData *data = (EMailFolderGMFData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	
	
	if (error && error->message) {
		g_warning ("Get message user tag failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Get message user tag: %s failed: %s\n", priv->path, error->message));
		return;
	}
	
	egdbus_folder_complete_get_message_user_tag (data->object, data->invocation, data->value ? data->value : "");
	ipc(printf("Get message user tag success: %s : %s %s:%s\n", priv->path, data->uid, data->name, data->value));
	
	g_free (data->value);
	g_free (data->name);
	g_free (data->uid);
	g_free (data);
}

static gboolean
impl_Mail_getMessageUserTag (EGdbusFolder *object, GDBusMethodInvocation *invocation, const char *uid, char *name, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderGMFData *data = g_new0 (EMailFolderGMFData, 1);

	data->object = object;
	data->invocation = invocation;
	data->mfolder = mfolder;
	data->uid = g_strdup (uid);
	data->name = g_strdup (name);

	ipc(printf("Get message user tag: %s : %s %s\n", priv->path, data->uid, name));
	
	mail_operate_on_folder (priv->folder, gmut_operate, gmut_done, data);

	return TRUE;
}

/* Set Message User Tags */
static gboolean
smut_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	EMailFolderGMFData *data = (EMailFolderGMFData *)sdata;

	camel_folder_set_message_user_tag (folder, data->uid, data->name, data->value);

	return TRUE;
}

static void
smut_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderGMFData *data = (EMailFolderGMFData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	
	if (error && error->message) {
		g_warning ("set message user tag failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("set message user tag: %s failed: %s\n", priv->path, error->message));
		return;
	}
	
	ipc(printf("Set message user tag: %s success: %s\n", priv->path, data->uid));
	
	egdbus_folder_complete_set_message_user_tag (data->object, data->invocation);

	g_free (data->value);
	g_free (data->name);
	g_free (data->uid);
	g_free (data);
}

static gboolean
impl_Mail_setMessageUserTag (EGdbusFolder *object, GDBusMethodInvocation *invocation, const char *uid, const char *name, const char *value, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderGMFData *data = g_new0 (EMailFolderGMFData, 1);

	data->object = object;
	data->invocation = invocation;
	data->mfolder = mfolder;
	data->uid = g_strdup (uid);
	data->name = g_strdup (name);
	data->value = g_strdup (value);

	ipc(printf("set message user tag: %s : %s %s:%s\n", priv->path, uid, name, value));

	mail_operate_on_folder (priv->folder, smut_operate, smut_done, data);

	return TRUE;
}

/* Get Parent Store */
static gboolean
impl_Mail_getParentStore (EGdbusFolder *object, GDBusMethodInvocation *invocation, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	CamelStore *store;
	const char *object_path;

	store = camel_folder_get_parent_store (priv->folder);
	object_path = g_object_get_data ((GObject *)store, "object-path");
	if (!object_path) {
		object_path = e_mail_data_session_get_path_from_store (data_session, (gpointer)store);
		if (object_path && *object_path)
			g_object_set_data ((GObject *)store, "object-path", (gpointer)object_path);
	}

	if (object_path == NULL) {
		/* We are having a strange situation. */
		ipc(printf("Get ParentStore : %s failed\n", priv->path));
		
		g_dbus_method_invocation_return_dbus_error (invocation, G_DBUS_ERROR_FAILED, _("Unable to find the object path of the parent store"));
		return TRUE;
	}
	ipc(printf("Get Parent Store: %s success: %s\n", priv->path, object_path));

	egdbus_folder_complete_get_parent_store (object, invocation, object_path);

	return TRUE;
}

/* Append Message */
static CamelMessageInfoBase *
info_from_variant (CamelFolder *folder, GVariant *vinfo) 
{
	CamelMessageInfoBase *info;
	GVariantIter iter, aiter;
	GVariant *item, *aitem;
	int count, i;

	info = (CamelMessageInfoBase *) camel_message_info_new (folder->summary);

         /* Structure of CamelMessageInfoBase
         ssssss - uid, sub, from, to, cc, mlist
	 uu - flags, size
	 tt - date_sent, date_received
	 t  - message_id
	 iat - references
	 as - userflags
	 a(ss) - usertags
	 a(ss) - header 
         NOTE: We aren't now sending content_info*/

	g_variant_iter_init (&iter, vinfo);

	/* Uid, Subject, From, To, CC, mlist */
	item = g_variant_iter_next_value (&iter);
	info->uid = camel_pstring_strdup (g_variant_get_string(item, NULL));

	item = g_variant_iter_next_value (&iter);
	info->subject = camel_pstring_strdup (g_variant_get_string(item, NULL));

	item = g_variant_iter_next_value (&iter);
	info->from = camel_pstring_strdup (g_variant_get_string(item, NULL));

	item = g_variant_iter_next_value (&iter);
	info->to = camel_pstring_strdup (g_variant_get_string(item, NULL));

	item = g_variant_iter_next_value (&iter);
	info->cc = camel_pstring_strdup (g_variant_get_string(item, NULL));

	item = g_variant_iter_next_value (&iter);
	info->mlist = camel_pstring_strdup (g_variant_get_string(item, NULL));

	item = g_variant_iter_next_value (&iter);
	info->preview = g_strdup (g_variant_get_string(item, NULL));
	
	/* Flags & size */
	item = g_variant_iter_next_value (&iter);
	info->flags = g_variant_get_uint32 (item);

	item = g_variant_iter_next_value (&iter);
	info->size = g_variant_get_uint32 (item);

	/* Date: Sent/Received */
	item = g_variant_iter_next_value (&iter);
	info->date_sent = g_variant_get_uint64 (item);

	item = g_variant_iter_next_value (&iter);
	info->date_received = g_variant_get_uint64 (item);

	/* Message Id */
	item = g_variant_iter_next_value (&iter);	
	info->message_id.id.id = g_variant_get_uint64 (item);

	/* References */
	item = g_variant_iter_next_value (&iter);	
	count = g_variant_get_int32 (item);
	if (count) {
		item = g_variant_iter_next_value (&iter);	
      		g_variant_iter_init (&aiter, item);
		
		info->references = g_malloc(sizeof(*info->references) + ((count-1) * sizeof(info->references->references[0])));
		i=0;
      		while ((aitem = g_variant_iter_next_value (&aiter))) {
			info->references->references[i].id.id = g_variant_get_uint64 (aitem);
			i++;
       	 	}
		info->references->size = count;
	} else {
		item = g_variant_iter_next_value (&iter);	

		info->references = NULL;
	}
	/* UserFlags */
	item = g_variant_iter_next_value (&iter);	
      	g_variant_iter_init (&aiter, item);
	
      	while ((aitem = g_variant_iter_next_value (&aiter))) {
		const char *str = g_variant_get_string (aitem, NULL);
		if (str && *str)
			camel_flag_set (&info->user_flags, str, TRUE);
        }
	
	/* User Tags */
	item = g_variant_iter_next_value (&iter);	
      	g_variant_iter_init (&aiter, item);
	
      	while ((aitem = g_variant_iter_next_value (&aiter))) {
		GVariantIter siter;
		GVariant *sitem;
		char *tagname, *tagvalue;
		
		g_variant_iter_init (&siter, aitem);
		sitem = g_variant_iter_next_value (&siter);
		tagname = g_strdup (g_variant_get_string (sitem, NULL));
		sitem = g_variant_iter_next_value (&siter);
		tagvalue = g_strdup (g_variant_get_string (sitem, NULL));
		if (tagname && *tagname && tagvalue && *tagvalue)
			camel_tag_set (&info->user_tags, tagname, tagvalue);
		g_free (tagname);
		g_free (tagvalue);
        }

	return info;
}

typedef struct _email_folder_msg_data {
	EMailDataFolder *mfolder;
	EGdbusFolder *object;
	GDBusMethodInvocation *invocation;
	CamelMessageInfoBase *info;
	CamelMimeMessage *message;
	char *uid;
	char *msg_buf;
}EMailFolderMessageData;


static void
app_msg_done (CamelFolder *folder,
 	      GAsyncResult *result,
	      EMailFolderMessageData *data)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	CamelFolder *outbox;
	GError *error = NULL;
	gboolean success;
	char *uid=NULL;

	success = camel_folder_append_message_finish (folder, result, &uid, &error);
	if (error && error->message) {
		g_warning ("Append message failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Append message: %s failed: %s\n", priv->path, error->message));
		return;
	}

	egdbus_folder_complete_append_message (data->object, data->invocation, uid ? uid : "", success);
	
	ipc(printf("Apppend message: %s success: %s\n", priv->path, data->uid ? data->uid : ""));

	outbox = e_mail_local_get_folder (E_MAIL_LOCAL_FOLDER_OUTBOX);
	if (priv->folder == outbox) {
		/* We just appended to OUTBOX. Issue a Send command */
		micro(printf("Append to Outbox, so issuing a send command\n"));
		mail_send(session);
	} else
		micro(printf("Append to %s\n", camel_folder_get_full_name (priv->folder)));
		
	g_object_unref (data->message);
	camel_message_info_free (data->info);
	g_free (uid);
	g_free (data);
}


static gboolean
impl_Mail_appendMessage (EGdbusFolder *object, GDBusMethodInvocation *invocation, GVariant *vinfo, const char *message, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderMessageData *data;
	CamelStream *stream;
	GCancellable *ops = camel_operation_new ();
	GError *error = NULL;

	data = g_new0 (EMailFolderMessageData, 1);
	data->mfolder = mfolder;	
	data->object = object;
	data->invocation = invocation;
	data->info = info_from_variant (priv->folder, vinfo);
	data->message = camel_mime_message_new ();

	stream = camel_stream_mem_new_with_buffer (message, strlen (message));
	camel_data_wrapper_construct_from_stream_sync ((CamelDataWrapper *)data->message, stream, ops, &error);
	g_object_unref (stream);
	
	ipc(printf("Append message: %s: %s\n", priv->path, data->info->uid));

	camel_folder_append_message (priv->folder, data->message, (CamelMessageInfo *)data->info,
		G_PRIORITY_DEFAULT, ops,
		(GAsyncReadyCallback) app_msg_done, data);
	
	return TRUE;
}

/* Get UIDs */
static gboolean
impl_Mail_getUids (EGdbusFolder *object, GDBusMethodInvocation *invocation, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	GPtrArray *uids;

	uids = camel_folder_get_uids (priv->folder);
	g_ptr_array_add (uids, NULL);

	ipc(printf("Get uids: %s: success :%d\n", priv->path, uids->len-1));
	
	egdbus_folder_complete_get_uids (object, invocation, (const gchar *const *) uids->pdata);

	g_ptr_array_remove_index_fast (uids, uids->len-1);
	camel_folder_free_uids (priv->folder, uids);

	return TRUE;
}

/* Get Message */
/* FIXME: We should get this passed via FD than as a string */
static gboolean
app_getmsg_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	EMailFolderMessageData *data = (EMailFolderMessageData *)sdata;
	CamelMimeMessage *msg;
	CamelStream *stream;
	GByteArray *array;
	CamelStream *filter_stream = NULL;
	CamelMimeFilter *charenc = NULL;
	static const char *charset = NULL;
	GConfClient *gconf;
	GCancellable *ops = camel_operation_new ();

	/* FIXME we should somehow get the right operation and pass it */
	msg = camel_folder_get_message_sync (folder, data->uid, ops, error);

	if (!msg) {
		g_warning ("Unable to fetch message: %s in %s\n", data->uid, camel_folder_get_full_name (folder));
		return FALSE;
	}

	stream = camel_stream_mem_new ();
	filter_stream = camel_stream_filter_new (stream);

	if (!charset)  {
		gboolean ret = FALSE;
		gconf = gconf_client_get_default ();
		charset = gconf_client_get_string (gconf, "/apps/evolution/mail/display/charset",NULL);
		g_object_unref (gconf);
		if (!charset || !*charset) {
			char *lcharset = NULL;
			ret = g_get_charset ((const char **)&lcharset);
			if (lcharset)
				charset = g_strdup(lcharset);
		}
		micro(printf("Got Charset? %d: %s\n", ret, charset ? charset : ""));
	}
	
	charenc = camel_mime_filter_charset_new (charset, "UTF-8");
	camel_stream_filter_add (CAMEL_STREAM_FILTER (filter_stream), charenc);
	g_object_unref (charenc);
	g_object_unref (stream);

	camel_data_wrapper_decode_to_stream_sync ((CamelDataWrapper *)msg, filter_stream, ops, NULL);
	array = camel_stream_mem_get_byte_array ((CamelStreamMem *)stream);
	data->msg_buf = g_strndup ((gchar *) array->data, array->len);

	g_object_unref (filter_stream);
	g_object_unref (msg);

	return TRUE;
}

static void
app_getmsg_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderMessageData *data = (EMailFolderMessageData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);

	if (error && error->message) {
		g_warning ("Get message failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Get message: %s failed: %s\n", priv->path, error->message));
		return;
	}
	
	egdbus_folder_complete_get_message (data->object, data->invocation, data->msg_buf);
	
	ipc(printf("Get Message: %s success: %s\n", priv->path, data->uid));

	g_free (data->msg_buf);
	g_free (data->uid);
	g_free (data);
}

static gboolean
impl_Mail_getMessage (EGdbusFolder *object, GDBusMethodInvocation *invocation, const char *uid, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderMessageData *data;

	data = g_new0 (EMailFolderMessageData, 1);
	data->mfolder = mfolder;	
	data->object = object;
	data->invocation = invocation;
	data->uid = g_strdup (uid);
	
	ipc(printf("Get message: %s : %s\n", priv->path, uid));

	mail_operate_on_folder (priv->folder, app_getmsg_operate, app_getmsg_done, data);
	

	return TRUE;
}

/* Get Quota Info */

static void
app_getquota_done (CamelFolder *folder,
 		 GAsyncResult *result,
		 EMailFolderMessageData *data)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	GError *error = NULL;
	CamelFolderQuotaInfo *quota, *iter;
	GVariant *variant;
	GVariantBuilder *builder;

	quota = camel_folder_get_quota_info_finish (folder, result, &error);
	if (error && error->message) {
		g_warning ("Get Quota failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Get Quota: %s failed: %s\n", priv->path, error->message));
		return;
	}

	builder = g_variant_builder_new (G_VARIANT_TYPE_ARRAY);
	iter = quota;
	while (iter) {
		g_variant_builder_add (builder, "(stt)", iter->name, iter->used, iter->total);
		iter = iter->next;
	}

	/* Append a empty entry*/
	g_variant_builder_add (builder, "(stt)", "", 0, 0);
	variant = g_variant_builder_end (builder);
	g_variant_ref (variant);
	g_variant_builder_unref (builder);

	egdbus_folder_complete_get_quota_info(data->object, data->invocation, variant);
	g_variant_unref (variant);
	
	ipc(printf("Get Quota: %s success\n", priv->path));

	g_free (data);
}

static gboolean
impl_Mail_getQuotaInfo (EGdbusFolder *object, GDBusMethodInvocation *invocation, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderMessageData *data;
	GCancellable *ops = camel_operation_new ();

	data = g_new0 (EMailFolderMessageData, 1);
	data->mfolder = mfolder;	
	data->object = object;
	data->invocation = invocation;
	
	ipc(printf("Get Quota Info : %s\n", priv->path));

	camel_folder_get_quota_info (priv->folder, 
		G_PRIORITY_DEFAULT, ops,
		(GAsyncReadyCallback) app_getquota_done, data);

	return TRUE;
}

/* Search by Expression */
typedef struct _email_folder_search_data {
	EMailDataFolder *mfolder;
	EGdbusFolder *object;
	GDBusMethodInvocation *invocation;
	char *query;
	char *sort;
	gboolean ascending;
	GPtrArray *query_uids;
	GPtrArray *result_uids;
}EMailFolderSearchData;

static gboolean
search_expr_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	EMailFolderSearchData *data = (EMailFolderSearchData *)sdata;
	
	data->result_uids = camel_folder_search_by_expression (folder, data->query, error);

	return TRUE;
}

static void
search_expr_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderSearchData *data = (EMailFolderSearchData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	
	g_ptr_array_add (data->result_uids, NULL);

	if (error && error->message) {
		g_warning ("Search by expr failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Search by expr : %s failed: %s\n", priv->path, error->message));
		return;
	}

	egdbus_folder_complete_search_by_expression (data->object, data->invocation, (const gchar *const *)data->result_uids->pdata);

	g_ptr_array_remove_index_fast (data->result_uids, data->result_uids->len-1);
	ipc(printf("Search messages by expr: %s success: %d results\n", priv->path, data->result_uids->len));

	camel_folder_search_free (priv->folder, data->result_uids);
	g_free (data->query);
	g_free (data);
}

static gboolean
impl_Mail_searchByExpression (EGdbusFolder *object, GDBusMethodInvocation *invocation, const char *expression, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderSearchData *data;


	data = g_new0 (EMailFolderSearchData, 1);
	data->object = object;
	data->mfolder = mfolder;	
	data->invocation = invocation;
	data->query = g_strdup (expression);
	
	ipc(printf("Search by expr : %s : %s\n", priv->path, expression));

	mail_operate_on_folder (priv->folder, search_expr_operate, search_expr_done, data);
	

	return TRUE;
}

struct _sort_data {
	CamelFolder *folder;
	char sort; /* u- subject, e- sender, r-datereceived */
	gboolean ascending;
};

static gint
compare_uids (gconstpointer a,
              gconstpointer b,
              gpointer user_data)
{
	const gchar *uid1 = *(const gchar **) a;
	const gchar *uid2 = *(const gchar **) b;
	struct _sort_data *data = (struct _sort_data *) user_data;
	CamelFolder *folder = data->folder;
	CamelMessageInfoBase *info1, *info2;
	gint ret=0;

	info1 = (CamelMessageInfoBase *)camel_folder_get_message_info (folder, uid1);
	info2 = (CamelMessageInfoBase *)camel_folder_get_message_info (folder, uid2);

	if (data->sort == 'u') {
		ret = g_ascii_strcasecmp (info1->subject ? info1->subject : "", info2->subject ? info2->subject : "");
	} else if (data->sort == 'e') {
		ret = g_ascii_strcasecmp (info1->from ? info1->from : "" , info2->from ? info2->from : "");
	} else if (data->sort == 'r') {
		ret = info1->date_received - info2->date_received;
	}

	if (!data->ascending)
		ret = -ret;

	camel_message_info_free (info1);
	camel_message_info_free (info2);

	return ret;
}

static gboolean
search_sort_expr_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	EMailFolderSearchData *data = (EMailFolderSearchData *)sdata;
	struct _sort_data *sort = g_new0(struct _sort_data, 1);

	sort->folder = folder;
	if (g_strcmp0 (data->sort, "subject") == 0)
		sort->sort = 'u';
	else if (g_strcmp0 (data->sort, "sender") == 0)
		sort->sort = 'e';
	else /* Date received*/
		sort->sort = 'r';
	
	sort->ascending = data->ascending;

	data->result_uids = camel_folder_search_by_expression (folder, data->query, error);
	micro(printf("Search returned: %d\n", data->result_uids->len));
	g_qsort_with_data (data->result_uids->pdata, data->result_uids->len, sizeof (gpointer), compare_uids, sort);
	micro(printf("Sorting completed\n"));
	g_free (sort);

	return TRUE;
}

static void
search_sort_expr_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderSearchData *data = (EMailFolderSearchData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	
	g_ptr_array_add (data->result_uids, NULL);

	if (error && error->message) {
		g_warning ("Search Sort by expr failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Search Sort by expr : %s failed: %s\n", priv->path, error->message));
		return;
	}

	egdbus_folder_complete_search_sort_by_expression (data->object, data->invocation, (const gchar *const *)data->result_uids->pdata);

	g_ptr_array_remove_index_fast (data->result_uids, data->result_uids->len-1);
	ipc(printf("Search Sort messages by expr: %s success: %d results\n", priv->path, data->result_uids->len));

	camel_folder_search_free (priv->folder, data->result_uids);
	g_free (data->query);
	g_free (data->sort);
	g_free (data);
}

static gboolean
impl_Mail_searchSortByExpression (EGdbusFolder *object, GDBusMethodInvocation *invocation, const char *expression, const char *sort, gboolean ascending, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderSearchData *data;


	data = g_new0 (EMailFolderSearchData, 1);
	data->object = object;
	data->mfolder = mfolder;	
	data->invocation = invocation;
	data->query = g_strdup (expression);
	data->sort = g_strdup (sort);
	data->ascending = ascending;
	ipc(printf("Search Sort by expr : %s : %s: %s: %d\n", priv->path, expression, sort, ascending));

	mail_operate_on_folder (priv->folder, search_sort_expr_operate, search_sort_expr_done, data);
	

	return TRUE;
}

/* Search by UIDs */
static gboolean
search_uids_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	EMailFolderSearchData *data = (EMailFolderSearchData *)sdata;
	
	data->result_uids = camel_folder_search_by_uids (folder, data->query, data->query_uids, error);

	return TRUE;
}

static void
search_uids_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderSearchData *data = (EMailFolderSearchData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	
	if (error && error->message) {
		g_warning ("Search by uids failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Search by uids : %s failed: %s\n", priv->path, error->message));
		return;
	}
	
	g_ptr_array_add (data->result_uids, NULL);

	egdbus_folder_complete_search_by_uids (data->object, data->invocation, (const gchar *const *)data->result_uids->pdata);

	g_ptr_array_remove_index_fast (data->result_uids, data->result_uids->len-1);
	ipc(printf("Search messages in  uids: %s success: %d results\n", priv->path, data->result_uids->len));

	camel_folder_search_free (priv->folder, data->query_uids);	
	camel_folder_search_free (priv->folder, data->result_uids);
	g_free (data->query);
	g_free (data);
}

static gboolean
impl_Mail_searchByUids (EGdbusFolder *object, GDBusMethodInvocation *invocation, const char *expression, const char **uids, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderSearchData *data;
	int i;

	
	data = g_new0 (EMailFolderSearchData, 1);
	data->object = object;
	data->mfolder = mfolder;	
	data->invocation = invocation;
	data->query = g_strdup (expression);
	data->query_uids = g_ptr_array_new ();
	for (i=0; uids && uids[i]; i++) {
		g_ptr_array_add (data->query_uids, (gpointer) camel_pstring_strdup(uids[i]));
	}
	
	ipc(printf("Search in uids: %s: %s in %d\n", priv->path, expression, data->query_uids->len));

	mail_operate_on_folder (priv->folder, search_uids_operate, search_uids_done, data);
	

	return TRUE;
}

/* Get Message Info */
static gboolean
gmi_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	EMailFolderMessageData *data = (EMailFolderMessageData *)sdata;
	
	data->info = (CamelMessageInfoBase *)camel_folder_get_message_info (folder, data->uid);

	return TRUE;
}

#define VALUE_OR_NULL(x) x?x:""
static GVariant *
variant_from_info (CamelMessageInfoBase *info)
{
	GVariant *v, *v1;
	GVariantBuilder *builder, *b1, *b2;
	int i;
	CamelFlag *flags;
	CamelTag *tags;

	builder = g_variant_builder_new (G_VARIANT_TYPE_TUPLE);
	
	g_variant_builder_add (builder, "s", info->uid);
	g_variant_builder_add (builder, "s", VALUE_OR_NULL(info->subject));
	g_variant_builder_add (builder, "s", VALUE_OR_NULL(info->from));
	g_variant_builder_add (builder, "s", VALUE_OR_NULL(info->to));
	g_variant_builder_add (builder, "s", VALUE_OR_NULL(info->cc));
	g_variant_builder_add (builder, "s", VALUE_OR_NULL(info->mlist));
	g_variant_builder_add (builder, "s", VALUE_OR_NULL(info->preview));


	g_variant_builder_add (builder, "u", info->flags);
	g_variant_builder_add (builder, "u", info->size);

	g_variant_builder_add (builder, "t", info->date_sent);
	g_variant_builder_add (builder, "t", info->date_received);

	g_variant_builder_add (builder, "t", info->message_id.id.id);


	
	/* references */

	if (info->references) {
		g_variant_builder_add (builder, "i", info->references->size);

		b1 = g_variant_builder_new (G_VARIANT_TYPE_ARRAY);
		for (i=0; i<info->references->size; i++) {
			g_variant_builder_add (b1, "t", info->references->references[i].id.id);
		}
		v1 = g_variant_builder_end (b1);
		g_variant_ref (v1);
		g_variant_builder_unref (b1);
	
		g_variant_builder_add_value (builder, v1);
		g_variant_unref (v1);
	} else {
		g_variant_builder_add (builder, "i", 0);
		b1 = g_variant_builder_new (G_VARIANT_TYPE_ARRAY);
		g_variant_builder_add (b1, "t", 0);
		v1 = g_variant_builder_end (b1);
		g_variant_ref (v1);
		g_variant_builder_unref (b1);
	
		g_variant_builder_add_value (builder, v1);
		g_variant_unref (v1);		
	}

	/* User Flags */
	b1 = g_variant_builder_new (G_VARIANT_TYPE_ARRAY);
	flags = info->user_flags;
	while (flags) {
		g_variant_builder_add (b1, "s", flags->name);
		flags = flags->next;
	}
	g_variant_builder_add (b1, "s", "");
	v1 = g_variant_builder_end (b1);
	g_variant_ref (v1);
	g_variant_builder_unref (b1);
	
	g_variant_builder_add_value (builder, v1);
	g_variant_unref (v1);
	/* User Tags */
	b1 = g_variant_builder_new (G_VARIANT_TYPE_ARRAY);
	tags = info->user_tags;
	while (tags) {
		b2 = g_variant_builder_new (G_VARIANT_TYPE_TUPLE);
		g_variant_builder_add (b2, "s", tags->name);
		g_variant_builder_add (b2, "s", tags->value);
		
		v1 = g_variant_builder_end (b2);
		g_variant_ref (v1);
		g_variant_builder_unref (b2);

		/* FIXME: Should we handle empty tags? Can it be empty? If it potential crasher ahead*/
		g_variant_builder_add_value (b1, v1);
		g_variant_unref (v1);		

		tags = tags->next;
	}

	b2 = g_variant_builder_new (G_VARIANT_TYPE_TUPLE);
	g_variant_builder_add (b2, "s", "");
	g_variant_builder_add (b2, "s", "");	
	v1 = g_variant_builder_end (b2);
	g_variant_ref (v1);

	g_variant_builder_unref (b2);
	g_variant_builder_add_value (b1, v1);
	g_variant_unref (v1);

	v1 = g_variant_builder_end (b1);
	g_variant_ref (v1);

	g_variant_builder_unref (b1);
	
	g_variant_builder_add_value (builder, v1);
	g_variant_unref (v1);

	v = g_variant_builder_end (builder);
	g_variant_ref (v);

	g_variant_builder_unref (builder);

	return v;
}

static void
gmi_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderMessageData *data = (EMailFolderMessageData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);
	
	GVariant *variant;
	
	if (error && error->message) {
		g_warning ("Get Message info failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Get message info: %s failed: %s\n", priv->path, error->message));
		return;
	}

	variant = variant_from_info (data->info);
	micro(printf("MessageInfo: %s %p\n", data->info->uid, data->info));
	egdbus_folder_complete_get_message_info (data->object, data->invocation, variant);
	g_variant_unref (variant);

	camel_message_info_free (data->info);
	g_free (data->uid);
	g_free (data);
}

static gboolean
impl_Mail_getMessageInfo (EGdbusFolder *object, GDBusMethodInvocation *invocation, const char *uid, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	EMailFolderMessageData *data;

	
	data = g_new0 (EMailFolderMessageData, 1);
	data->mfolder = mfolder;
	data->object = object;
	data->invocation = invocation;
	data->uid = g_strdup (uid);

	ipc(printf("Mail get message info: %s uid: %s\n", priv->path, uid));

	mail_operate_on_folder (priv->folder, gmi_operate, gmi_done, data);
	

	return TRUE;
}

/* Transfer Messages to */
typedef struct _email_folder_transfer_data {
	EMailDataFolder *mfolder;
	EGdbusFolder *object;
	GDBusMethodInvocation *invocation;
	CamelFolder *dest_folder;
	char *object_path;
	GPtrArray *uids;
	GPtrArray *return_uids;
	gboolean delete_originals;
}EMailFolderTransferData;

static gboolean
transfer_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	EMailFolderTransferData *data = (EMailFolderTransferData *)sdata;
	GCancellable *ops = camel_operation_new ();

	/* FIXME we should somehow get the right operation and pass it */
	data->dest_folder = e_mail_session_get_folder_from_path (data_session, data->object_path);
	
	return camel_folder_transfer_messages_to_sync (folder, data->uids, data->dest_folder, data->delete_originals, &data->return_uids, ops, error);
}

static void
transfer_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderTransferData *data = (EMailFolderTransferData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);	
	
	if (error && error->message) {
		g_warning ("Transfer messages failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Mail transfer messages: %s failed: %s\n", priv->path, error->message));
		return;
	}


	if (!data->return_uids) {
		/* It is possible that some providers don't return this.*/
		data->return_uids = g_ptr_array_new ();
	}
	g_ptr_array_add (data->return_uids, NULL);


	egdbus_folder_complete_transfer_messages_to (data->object, data->invocation, (const gchar *const *) data->return_uids->pdata);

	ipc(printf("Mail transfer messages: %s success: %d\n", priv->path, data->return_uids->len-1));
	g_ptr_array_remove_index_fast (data->uids, data->uids->len-1);

	micro(printf("Arrays %p %p: success: %d\n", data->uids, data->return_uids, success));

	g_ptr_array_foreach (data->uids, (GFunc)g_free, NULL);
	g_ptr_array_free (data->uids, TRUE);
	g_ptr_array_foreach (data->return_uids, (GFunc)g_free, NULL);
	g_ptr_array_free (data->return_uids, TRUE);
	g_free (data->object_path);
	g_object_unref (data->dest_folder);
	g_free (data);
}



static gboolean
impl_Mail_transferMessagesTo (EGdbusFolder *object, GDBusMethodInvocation *invocation, const char **uids, const char *object_path, gboolean delete_originals, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);	
	EMailFolderTransferData *data = g_new0 (EMailFolderTransferData, 1);
	int i;

	data->object = object;
	data->invocation = invocation;
	data->mfolder = mfolder;
	data->object_path = g_strdup (object_path);
	data->uids = g_ptr_array_new ();
	for (i=0; uids && uids[i]; i++) {
		g_ptr_array_add (data->uids, g_strdup(uids[i]));
	}
	data->delete_originals = delete_originals;

	ipc(printf("Transfer messages from %s: to %s: len %d\n", priv->path, object_path, data->uids->len));

	mail_operate_on_folder (priv->folder, transfer_operate, transfer_done, data);

	return TRUE;
}

#if 0
/* Fetch Old messages */
typedef struct _email_folder_fetch_data {
	EMailDataFolder *mfolder;
	EGdbusFolder *object;
	GDBusMethodInvocation *invocation;
	gint count;
}EMailFolderFetchData;


static gboolean
fetch_old_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	EMailFolderFetchData *data = (EMailFolderFetchData *)sdata;

	return camel_folder_fetch_old_messages (folder, data->count, error);
}

static void
fetch_old_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderFetchData *data = (EMailFolderFetchData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);	
	
	if (error && error->message) {
		g_warning ("Fetch old messages failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Fetch old messages: %s failed: %s\n", priv->path, error->message));
		return;
	}


	egdbus_folder_complete_fetch_old_messages (data->object, data->invocation, success);

	ipc(printf("Fetch old messages: %s success: %d\n", priv->path, success));

	g_free (data);
}

static 
gboolean
impl_Mail_fetchOldMessages (EGdbusFolder *object, GDBusMethodInvocation *invocation, int count, EMailDataFolder *mfolder)
{
	EMailFolderFetchData *data;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);	

	data = g_new0 (EMailFolderFetchData, 1);
	data->mfolder = mfolder;
	data->invocation = invocation;
	data->object = object;
	data->count = count;

	mail_operate_on_folder (priv->folder, fetch_old_operate, fetch_old_done, data);

	return TRUE;
}
#endif

/* Prepare Summary */
static gboolean
ps_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	//EMailFolderTransferData *data = (EMailFolderTransferData *)sdata;

	camel_folder_summary_set_need_preview (folder->summary, TRUE);
	camel_folder_summary_prepare_fetch_all (folder->summary, error);

	return TRUE;
}

static void
ps_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderTransferData *data = (EMailFolderTransferData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);	
	
	if (error && error->message) {
		g_warning ("Preparing summary failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Preparing summary : %s failed: %s\n", priv->path, error->message));
		return;
	}

	egdbus_folder_complete_prepare_summary (data->object, data->invocation);

	ipc(printf("Preparing summary: %s success\n", priv->path));

	g_free (data);
}

static gboolean
impl_Mail_prepareSummary (EGdbusFolder *object, GDBusMethodInvocation *invocation, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);	
	EMailFolderTransferData *data = g_new0 (EMailFolderTransferData, 1);

	data->object = object;
	data->invocation = invocation;
	data->mfolder = mfolder;

	ipc(printf("Prepare folder summary %s: \n", priv->path));

	mail_operate_on_folder (priv->folder, ps_operate, ps_done, data);

	return TRUE;
}

/* Freeze */
static gboolean
freeze_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	//EMailFolderTransferData *data = (EMailFolderTransferData *)sdata;

	camel_folder_freeze (folder);
	return TRUE;
}

static void
freeze_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderTransferData *data = (EMailFolderTransferData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);	
	
	if (error && error->message) {
		g_warning ("Folder Freeze failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Folder Freeze : %s failed: %s\n", priv->path, error->message));
		return;
	}

	egdbus_folder_complete_freeze_folder (data->object, data->invocation);

	ipc(printf("Folder Freeze : %s success\n", priv->path));

	g_free (data);
}

static gboolean
impl_Mail_freezeFolder (EGdbusFolder *object, GDBusMethodInvocation *invocation, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);	
	EMailFolderTransferData *data = g_new0 (EMailFolderTransferData, 1);

	data->object = object;
	data->invocation = invocation;
	data->mfolder = mfolder;

	ipc(printf("Freeze Folder %s: \n", priv->path));

	mail_operate_on_folder (priv->folder, freeze_operate, freeze_done, data);

	return TRUE;
}
/* Thaw */
static gboolean
thaw_operate (CamelFolder *folder, gpointer sdata, GError **error)
{
	//EMailFolderTransferData *data = (EMailFolderTransferData *)sdata;

	camel_folder_thaw (folder);

	return TRUE;
}

static void
thaw_done (gboolean success, gpointer sdata, GError *error)
{
	EMailFolderTransferData *data = (EMailFolderTransferData *)sdata;
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(data->mfolder);	
	
	if (error && error->message) {
		g_warning ("Folder Thaw failed: %s: %s\n", priv->path, error->message);
		g_dbus_method_invocation_return_gerror (data->invocation, error);		
		ipc(printf("Folder Thaw : %s failed: %s\n", priv->path, error->message));
		return;
	}

	egdbus_folder_complete_thaw_folder (data->object, data->invocation);

	ipc(printf("Folder Thaw : %s success\n", priv->path));

	g_free (data);
}

static gboolean
impl_Mail_thawFolder (EGdbusFolder *object, GDBusMethodInvocation *invocation, EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);	
	EMailFolderTransferData *data = g_new0 (EMailFolderTransferData, 1);

	data->object = object;
	data->invocation = invocation;
	data->mfolder = mfolder;

	ipc(printf("Folder Thaw %s: \n", priv->path));

	mail_operate_on_folder (priv->folder, thaw_operate, thaw_done, data);

	return TRUE;
}


/* Class def */

static void
e_mail_data_folder_init (EMailDataFolder *self)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(self);

	priv->gdbus_object = egdbus_folder_skeleton_new ();
	g_signal_connect (priv->gdbus_object, "handle-refresh-info", G_CALLBACK (impl_Mail_refreshInfo), self);
	g_signal_connect (priv->gdbus_object, "handle-sync", G_CALLBACK (impl_Mail_sync), self);
	g_signal_connect (priv->gdbus_object, "handle-sync-message", G_CALLBACK (impl_Mail_syncMessage), self);
	g_signal_connect (priv->gdbus_object, "handle-expunge", G_CALLBACK (impl_Mail_expunge), self);

	g_signal_connect (priv->gdbus_object, "handle-get-display-name", G_CALLBACK (impl_Mail_getDisplayName), self);
	g_signal_connect (priv->gdbus_object, "handle-set-display-name", G_CALLBACK (impl_Mail_setDisplayName), self);
	g_signal_connect (priv->gdbus_object, "handle-get-full-name", G_CALLBACK (impl_Mail_getFullName ), self);
	g_signal_connect (priv->gdbus_object, "handle-set-full-name", G_CALLBACK (impl_Mail_setFullName ), self);
	g_signal_connect (priv->gdbus_object, "handle-get-description", G_CALLBACK (impl_Mail_getDescription), self);
	g_signal_connect (priv->gdbus_object, "handle-set-description", G_CALLBACK (impl_Mail_setDescription), self);

	g_signal_connect (priv->gdbus_object, "handle-get-permanent-flags", G_CALLBACK (impl_Mail_getPermanentFlags), self);
	g_signal_connect (priv->gdbus_object, "handle-has-summary-capability", G_CALLBACK (impl_Mail_hasSummaryCapability), self);
	g_signal_connect (priv->gdbus_object, "handle-has-search-capability", G_CALLBACK (impl_Mail_hasSearchCapability), self);

	g_signal_connect (priv->gdbus_object, "handle-total-message-count", G_CALLBACK (impl_Mail_totalMessageCount), self);
	g_signal_connect (priv->gdbus_object, "handle-unread-message-count", G_CALLBACK (impl_Mail_unreadMessageCount), self);
	g_signal_connect (priv->gdbus_object, "handle-deleted-message-count", G_CALLBACK (impl_Mail_deletedMessageCount), self);

	g_signal_connect (priv->gdbus_object, "handle-get-message-flags", G_CALLBACK (impl_Mail_getMessageFlags), self);
	g_signal_connect (priv->gdbus_object, "handle-set-message-flags", G_CALLBACK (impl_Mail_setMessageFlags), self);
	g_signal_connect (priv->gdbus_object, "handle-get-message-user-flag", G_CALLBACK (impl_Mail_getMessageUserFlag), self);
	g_signal_connect (priv->gdbus_object, "handle-set-message-user-flag", G_CALLBACK (impl_Mail_setMessageUserFlag), self);
	g_signal_connect (priv->gdbus_object, "handle-get-message-user-tag", G_CALLBACK (impl_Mail_getMessageUserTag), self);
	g_signal_connect (priv->gdbus_object, "handle-set-message-user-tag", G_CALLBACK (impl_Mail_setMessageUserTag), self);

	g_signal_connect (priv->gdbus_object, "handle-get-parent-store", G_CALLBACK (impl_Mail_getParentStore), self);
	g_signal_connect (priv->gdbus_object, "handle-append-message", G_CALLBACK (impl_Mail_appendMessage), self);
	g_signal_connect (priv->gdbus_object, "handle-get-uids", G_CALLBACK (impl_Mail_getUids), self);
	g_signal_connect (priv->gdbus_object, "handle-get-message", G_CALLBACK (impl_Mail_getMessage), self);
	g_signal_connect (priv->gdbus_object, "handle-get-quota-info", G_CALLBACK (impl_Mail_getQuotaInfo), self);	
	g_signal_connect (priv->gdbus_object, "handle-search-by-expression", G_CALLBACK (impl_Mail_searchByExpression), self);
	g_signal_connect (priv->gdbus_object, "handle-search-sort-by-expression", G_CALLBACK (impl_Mail_searchSortByExpression), self);	
	g_signal_connect (priv->gdbus_object, "handle-search-by-uids", G_CALLBACK (impl_Mail_searchByUids), self);
	g_signal_connect (priv->gdbus_object, "handle-get-message-info", G_CALLBACK (impl_Mail_getMessageInfo), self);
	g_signal_connect (priv->gdbus_object, "handle-transfer-messages-to", G_CALLBACK (impl_Mail_transferMessagesTo), self);
	g_signal_connect (priv->gdbus_object, "handle-prepare-summary", G_CALLBACK (impl_Mail_prepareSummary), self);
	g_signal_connect (priv->gdbus_object, "handle-freeze-folder", G_CALLBACK (impl_Mail_thawFolder), self);
	g_signal_connect (priv->gdbus_object, "handle-thaw-folder", G_CALLBACK (impl_Mail_freezeFolder), self);

	/* g_signal_connect (priv->gdbus_object, "handle-fetch-old-messages", G_CALLBACK (impl_Mail_fetchOldMessages), self); */
	
	
}

EMailDataFolder*
e_mail_data_folder_new (CamelFolder *folder)
{
	EMailDataFolder *efolder;
	EMailDataFolderPrivate *priv;

  	efolder = g_object_new (EMAIL_TYPE_DATA_FOLDER, NULL);
	priv = DATA_FOLDER_PRIVATE(efolder);
	priv->folder = g_object_ref(folder);

	return efolder;
}

static void
folder_changed_cb (CamelFolder *folder,
                   CamelFolderChangeInfo *changes,
                   EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);

	g_ptr_array_add (changes->uid_added, NULL);
	g_ptr_array_add (changes->uid_removed, NULL);
	g_ptr_array_add (changes->uid_changed, NULL);
	g_ptr_array_add (changes->uid_recent, NULL);

	ipc(printf("Emitting Folder Changedin %s: %d %d %d %d\n", camel_folder_get_full_name(folder), changes->uid_added->len-1, 
				changes->uid_removed->len-1, changes->uid_changed->len-1, changes->uid_recent->len -1));
	egdbus_folder_emit_folder_changed (priv->gdbus_object, 
					(const gchar *const *) changes->uid_added->pdata, 
					(const gchar *const *) changes->uid_removed->pdata, 
					(const gchar *const *) changes->uid_changed->pdata, 
					(const gchar *const *) changes->uid_recent->pdata);

	g_ptr_array_remove_index_fast (changes->uid_added, changes->uid_added->len-1);
	g_ptr_array_remove_index_fast (changes->uid_removed, changes->uid_removed->len-1);
	g_ptr_array_remove_index_fast (changes->uid_changed, changes->uid_changed->len-1);
	g_ptr_array_remove_index_fast (changes->uid_recent, changes->uid_recent->len-1);

}

guint 
e_mail_data_folder_register_gdbus_object (EMailDataFolder *mfolder, GDBusConnection *connection, const gchar *object_path, GError **error)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);
	gboolean ret; 
	g_return_val_if_fail (connection != NULL, 0);
	g_return_val_if_fail (object_path != NULL, 0);

	priv->path  = g_strdup (object_path);
	g_signal_connect (
		priv->folder, "changed",
		G_CALLBACK (folder_changed_cb), mfolder);
	
 	ret = g_dbus_interface_skeleton_export ((GDBusInterfaceSkeleton *) priv->gdbus_object,
               	                     	connection,
                                    	object_path,
                                    	error);	
	
	ipc(printf("Registering folder %s: %d\n", object_path, ret));

	return ret;
}

const char *
e_mail_data_folder_get_path (EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);

	return priv->path;
}

CamelFolder *
e_mail_data_folder_get_camel_folder (EMailDataFolder *mfolder)
{
	EMailDataFolderPrivate *priv = DATA_FOLDER_PRIVATE(mfolder);

	return priv->folder;
}
