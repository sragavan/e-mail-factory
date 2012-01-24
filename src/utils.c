
#include <string.h>
#include <glib.h>
#include "utils.h"

/* 
 * EDS_MAIL_DEBUG should be a CSV
 * export EDS_MAIL_DEBUG=folder,store,session,micro,ipc
 * */

static int mail_debug_flag = 0;

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

