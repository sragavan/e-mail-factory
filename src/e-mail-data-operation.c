/* e-mail-data-operation.c */

#include "e-mail-data-operation.h"
#include "e-gdbus-emailoperation.h"
#include <camel/camel.h>
#include <gio/gio.h>
#include "utils.h"
#include <string.h>

#define micro(x) if (mail_debug_log(EMAIL_DEBUG_OPERATION|EMAIL_DEBUG_MICRO)) x;
#define ipc(x) if (mail_debug_log(EMAIL_DEBUG_OPERATION|EMAIL_DEBUG_IPC)) x;


G_DEFINE_TYPE (EMailDataOperation, e_mail_data_operation, G_TYPE_OBJECT)

#define DATA_OPERATION_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EMAIL_TYPE_DATA_OPERATION, EMailDataOperationPrivate))

typedef struct _EMailDataOperationPrivate EMailDataOperationPrivate;

struct _EMailDataOperationPrivate
{	
	char *object_path;
	EGdbusOperation *gdbus_object;
	CamelOperation *operation;
};

static gboolean
impl_Mail_cancel (EGdbusOperation *object, GDBusMethodInvocation *invocation, EMailDataOperation *moperation)
{
	EMailDataOperationPrivate *priv = DATA_OPERATION_PRIVATE(moperation);

	ipc(printf("Canceling mail Operation: %s\n", priv->object_path));

	g_cancellable_cancel ((GCancellable *)priv->operation);

	egdbus_operation_complete_cancel (object, invocation);
	return TRUE;
}

static gboolean
impl_Mail_isCancelled (EGdbusOperation *object, GDBusMethodInvocation *invocation, EMailDataOperation *moperation)
{
	EMailDataOperationPrivate *priv = DATA_OPERATION_PRIVATE(moperation);
	gboolean cancelled;

	ipc(printf("Checking if cancelled: %s\n", priv->object_path));

	cancelled = g_cancellable_is_cancelled ((GCancellable *)priv->operation);

	egdbus_operation_complete_is_cancelled (object, invocation, cancelled);

	return TRUE;
}


static void
e_mail_data_operation_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
e_mail_data_operation_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
e_mail_data_operation_dispose (GObject *object)
{
  G_OBJECT_CLASS (e_mail_data_operation_parent_class)->dispose (object);
}

static void
e_mail_data_operation_finalize (GObject *object)
{
  G_OBJECT_CLASS (e_mail_data_operation_parent_class)->finalize (object);
}

static void
e_mail_data_operation_class_init (EMailDataOperationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (EMailDataOperationPrivate));

  object_class->get_property = e_mail_data_operation_get_property;
  object_class->set_property = e_mail_data_operation_set_property;
  object_class->dispose = e_mail_data_operation_dispose;
  object_class->finalize = e_mail_data_operation_finalize;
}

static void
e_mail_data_operation_init (EMailDataOperation *self)
{
	EMailDataOperationPrivate *priv = DATA_OPERATION_PRIVATE(self);

	priv->gdbus_object = egdbus_operation_skeleton_new ();
	g_signal_connect (priv->gdbus_object, "handle-cancel", G_CALLBACK (impl_Mail_cancel), self);
	g_signal_connect (priv->gdbus_object, "handle-is-cancelled", G_CALLBACK (impl_Mail_isCancelled), self);
}

static gchar *
construct_mail_operation_path ()
{
	static volatile gint counter = 1;
	int i, len;
	char *path;
	char *name;

	name = g_strdup_printf ("%lu_%d", time(NULL), g_atomic_int_exchange_and_add (&counter, 1));
	
	path = g_strdup_printf (
		       "/org/gnome/evolution/dataserver/mail/operation/%s/%d/%u",
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

EMailDataOperation*
e_mail_data_operation_new (CamelOperation *operation)
{
  EMailDataOperation *moperation;
  EMailDataOperationPrivate *priv;

  moperation = g_object_new (EMAIL_TYPE_DATA_OPERATION, NULL);
  priv = DATA_OPERATION_PRIVATE (moperation);
  priv->operation = g_object_ref (operation);

  return moperation;
}

CamelOperation *
e_mail_data_operation_get_camel_operation (EMailDataOperation *mops)
{
	EMailDataOperationPrivate *priv = DATA_OPERATION_PRIVATE(mops);

	return priv->operation;
}

const char *
e_mail_data_operation_get_path (EMailDataOperation *mops)
{
	EMailDataOperationPrivate *priv = DATA_OPERATION_PRIVATE(mops);

	return priv->object_path;
}


static void
op_status_cb (CamelOperation *operation,
	      const gchar *what,
	      gint pc,
	      EMailDataOperation *moperation)
{
	e_mail_operation_emit_status (moperation, what, pc);
}

static void
op_cancelled_cb (GCancellable *cancellable,
		 EMailDataOperation *moperation)
{
	e_mail_operation_emit_cancelled (moperation);
}

char *
e_mail_data_operation_register_gdbus_object (EMailDataOperation *moperation, GDBusConnection *connection, GError **error)
{
	EMailDataOperationPrivate *priv = DATA_OPERATION_PRIVATE(moperation);
	guint ret;
	char *object_path;

	g_return_val_if_fail (connection != NULL, 0);

	g_signal_connect (priv->operation, "status", G_CALLBACK(op_status_cb), moperation);
	g_signal_connect (priv->operation, "cancelled", G_CALLBACK(op_cancelled_cb), moperation);

	priv->object_path = object_path = construct_mail_operation_path ();
 	ret = g_dbus_interface_skeleton_export ((GDBusInterfaceSkeleton *)priv->gdbus_object,
               	                     	connection,
                                    	object_path,
                                    	error);

	ipc (printf("EMailDataOperation: Registering gdbus object %s\n", object_path));

	return object_path;
}


void
e_mail_operation_emit_cancelled (EMailDataOperation *moperation)
{
	EMailDataOperationPrivate *priv = DATA_OPERATION_PRIVATE(moperation);
	
	ipc(printf("Emitting Cancelled \n"));
	egdbus_operation_emit_cancelled (priv->gdbus_object);
}

void
e_mail_operation_emit_status (EMailDataOperation *moperation, const char *what, int pc)
{
	EMailDataOperationPrivate *priv = DATA_OPERATION_PRIVATE(moperation);
	
	ipc(printf("Emitting Status signal\n"));
	egdbus_operation_emit_status (priv->gdbus_object, what, pc);
}


