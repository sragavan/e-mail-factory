/* mail-send-short-msg.c */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n-lib.h>

#include <libemail-engine/e-mail-session.h>
#include <libemail-engine/e-mail-session-utils.h>

#include "mail-send-short-msg.h"
#include "e-mail-data-session.h"
#include "e-mail-data-operation.h"

#define MAX_SUBJECT_LENGTH 20

extern EMailDataSession *data_session;
extern EMailSession *session;

typedef struct _SendAsyncContext SendAsyncContext;

struct _SendAsyncContext {
	CamelFolder *sent_folder;
	gchar *sent_folder_uri;

	CamelMimeMessage *message;
	CamelMessageInfo *info;
	CamelAddress *recipients;
	CamelAddress *from;

	CamelFilterDriver *driver;
	CamelService *transport;

	GVariantBuilder *result;

	GCancellable *cancellable;
	gchar *ops_path;
	gint io_priority;
};

static void
async_context_free (SendAsyncContext *context)
{
	if (context->sent_folder != NULL)
		g_object_unref (context->sent_folder);
	if (context->message != NULL)
		g_object_unref (context->message);
	if (context->info != NULL)
		camel_message_info_free (context->info);
	if (context->recipients != NULL)
		g_object_unref (context->recipients);
	if (context->from != NULL)
		g_object_unref (context->from);
	if (context->driver != NULL)
		g_object_unref (context->driver);
	if (context->result != NULL)
		g_variant_builder_unref(context->result);
	if (context->sent_folder_uri)
		g_free (context->sent_folder_uri);
	if (context->ops_path)
		g_free (context->ops_path);
	if (context->cancellable != NULL) {
		camel_operation_pop_message (context->cancellable);
		g_object_unref (context->cancellable);
	}

	g_slice_free (SendAsyncContext, context);
}

static void
mail_send_short_connection_fail(SendAsyncContext *context)
{
	const gchar *addr;
	gint i, len;

	len = camel_address_length(context->recipients);
	for (i = 0; i < len; i++) {
		if (camel_internet_address_get(
				CAMEL_INTERNET_ADDRESS(context->recipients),
				i, NULL, &addr))
			g_variant_builder_add (context->result, "(sssi)",
					addr, _("Connection failed"),
					g_quark_to_string(CAMEL_SERVICE_ERROR),
					CAMEL_SERVICE_ERROR_NOT_CONNECTED);
	}
}

static void
mail_send_short_to_thread (GSimpleAsyncResult *simple, EMailSession *session,
						GCancellable *cancellable)
{
	SendAsyncContext *context;
	CamelFolder *local_sent_folder;
	CamelProvider *provider;
	CamelService *service;
	CamelInternetAddress *cia;
	const gchar *addr;
	gint i, len;
	gboolean copy_to_sent = TRUE;
	GError *error = NULL;
	gboolean success = FALSE;
	gboolean did_connect = FALSE;
	gboolean cancelled = FALSE;

	context = g_simple_async_result_get_op_res_gpointer (simple);

	/* Connect transport service */

	service = context->transport;

	if (camel_service_get_connection_status (service) !=
						CAMEL_SERVICE_CONNECTED) {
		did_connect = TRUE;

		if (!camel_service_connect_sync (service, &error)) {
			mail_send_short_connection_fail (context);
			g_simple_async_result_take_error (simple, error);
			return;
		}
	}

	provider = camel_service_get_provider (service);

	if (provider->flags & CAMEL_PROVIDER_DISABLE_SENT_FOLDER)
		copy_to_sent = FALSE;

	/* Send the message to each individual recipient. */

	len = camel_address_length (context->recipients);
	for (i = 0; i < len; i++) {
		if (!cancelled) {
			if (!camel_internet_address_get(
				CAMEL_INTERNET_ADDRESS(context->recipients),
				i, NULL, &addr))
					continue;

			cia = camel_internet_address_new ();
			camel_internet_address_add (cia, NULL, addr);
			camel_transport_send_to_sync (
				CAMEL_TRANSPORT (service), context->message,
				context->from, CAMEL_ADDRESS(cia),
				cancellable, &error);
			g_object_unref(cia);
		}

		if (error) {
			g_variant_builder_add (context->result, "(sssi)", addr,
					error->message,
					g_quark_to_string(error->domain),
					error->code);
			if (g_error_matches (error, G_IO_ERROR,
							G_IO_ERROR_CANCELLED))
				cancelled = TRUE;
			else
				g_clear_error (&error);
		} else {
			g_variant_builder_add (context->result, "(sssi)", addr,
								"", "", 0);
			success = TRUE;
		}

		camel_operation_progress (cancellable, (i+1)*100/len);
	}

	g_clear_error (&error);

	if (did_connect)
		camel_service_disconnect_sync (service, FALSE, NULL);

	/*** Post Processing ***/

	if (!success) {
		g_simple_async_result_set_error (
					simple, E_MAIL_ERROR,
					E_MAIL_ERROR_POST_PROCESSING,
					_("All recipients failed"));
		return;
	}

	/* Run filters on the outgoing message. */
	if (context->driver != NULL) {
		camel_filter_driver_filter_message (
			context->driver, context->message, context->info,
			NULL, NULL, NULL, "", cancellable, &error);

		if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			goto exit;
		g_clear_error (&error);
	}

	if (!copy_to_sent)
		goto cleanup;

	/* Append the sent message to a Sent folder. */
	local_sent_folder = e_mail_session_get_local_folder (session,
						E_MAIL_LOCAL_FOLDER_SENT);

	/* Try to extract a CamelFolder from the Sent folder URI. */
	if (context->sent_folder_uri != NULL) {
		context->sent_folder = e_mail_session_uri_to_folder_sync (
					session, context->sent_folder_uri, 0,
					cancellable, &error);
		if (error != NULL) {
			g_warn_if_fail (context->sent_folder == NULL);
			g_clear_error (&error);
		}
	}

	/* Fall back to the local Sent folder. */
	if (context->sent_folder == NULL)
		context->sent_folder = g_object_ref (local_sent_folder);

	/* Append the message. */
	camel_folder_append_message_sync (
				context->sent_folder, context->message,
				context->info, NULL, cancellable, &error);

	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		goto exit;

	if (error == NULL)
		goto cleanup;

	/* If appending to a remote Sent folder failed,
	 * try appending to the local Sent folder. */
	if (context->sent_folder != local_sent_folder) {
		g_clear_error (&error);
		camel_folder_append_message_sync (local_sent_folder,
					context->message, context->info, NULL,
					cancellable, &error);
	}

	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		goto exit;

	/* We can't even append to the local Sent folder?
	 * In that case just leave the message in Outbox. */
	if (error != NULL) {
		g_clear_error (&error);
		goto exit;
	}

cleanup:

	/* The send operation was successful; ignore cleanup errors. */

	/* Mark the draft message for deletion, if present. */
	e_mail_session_handle_draft_headers_sync (session, context->message,
							cancellable, &error);
	if (error != NULL) {
		g_warning ("%s", error->message);
		g_clear_error (&error);
	}

	/* Set flags on the original source message, if present.
	 * Source message refers to the message being forwarded
	 * or replied to. */
	e_mail_session_handle_source_headers_sync (session, context->message,
							cancellable, &error);
	if (error != NULL) {
		g_warning ("%s", error->message);
		g_clear_error (&error);
	}

exit:

	/* If we were cancelled, disregard any other errors. */
	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
		g_simple_async_result_take_error (simple, error);

	/* Stuff the accumulated error messages in a GError. */
	} else if (error != NULL) {
		g_simple_async_result_set_error (simple, E_MAIL_ERROR,
			E_MAIL_ERROR_POST_PROCESSING, "%s", error->message);
	}

	/* Synchronize the Sent folder. */
	if (context->sent_folder != NULL)
		camel_folder_synchronize_sync (context->sent_folder, FALSE,
							cancellable, NULL);
}

static void
mail_send_short_message_completed (GObject *source, GAsyncResult *res,
							gpointer user_data)
{
	SendAsyncContext *context = (SendAsyncContext *) user_data;
	e_mail_session_emit_send_short_message_completed (data_session,
					context->ops_path,
					context->result);
}

gboolean
mail_send_short_message (EGdbusSession *object,
		GDBusMethodInvocation *invocation, const char *account_uid,
		const char *text, const char **to,
		EMailDataSession *msession, GError **ret_error)
{
	EAccount *account;
	CamelMimeMessage *message;
	CamelService *service;
	gchar *transport_uid;
	CamelInternetAddress *recipients;
	CamelMessageInfo *info;
	GSimpleAsyncResult *simple;
	SendAsyncContext *context;
	CamelInternetAddress *from;
	GCancellable *ops;
	EMailDataOperation *mops;
	char *mops_path;
	gchar subject[MAX_SUBJECT_LENGTH + 4];
	GError *error = NULL;

	/* Check params. */

	if (account_uid == NULL || *account_uid == 0) {
		error = g_error_new (G_DBUS_ERROR,
						G_DBUS_ERROR_INVALID_ARGS,
						_("Invalid account"));
		goto on_error;
	}
	if (text == NULL || *text == 0) {
		error = g_error_new (G_DBUS_ERROR,
						G_DBUS_ERROR_INVALID_ARGS,
						_("Text is empty"));
		goto on_error;
	}
	if (to == NULL || *to == 0 || **to == 0) {
		error = g_error_new (G_DBUS_ERROR,
						G_DBUS_ERROR_INVALID_ARGS,
						_("No recipient"));
		goto on_error;
	}

	/* Get transport. */

	account = e_get_account_by_uid (account_uid);
	if (!account) {
		error = g_error_new (G_DBUS_ERROR,
						G_DBUS_ERROR_INVALID_ARGS,
						_("Invalid account %s"),
						account_uid);
		goto on_error;
	}

	transport_uid = g_strconcat (account->uid, "-transport", NULL);
	service = camel_session_ref_service (CAMEL_SESSION (session),
								transport_uid);
	if (!CAMEL_IS_TRANSPORT (service)) {
		error = g_error_new(G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
					_("Invalid account %s"), account_uid);
		g_object_unref (account);
		g_free (transport_uid);
		goto on_error;
	}

	/* Prepare message. */

	message = camel_mime_message_new ();
	strncpy (subject, text, MAX_SUBJECT_LENGTH + 1);
	if (strlen(text) > MAX_SUBJECT_LENGTH)
		strcpy (subject + MAX_SUBJECT_LENGTH, "...");
	camel_mime_message_set_subject (message, subject);
	from = camel_internet_address_new ();
	camel_internet_address_add (from, NULL, "sms");
	recipients = camel_internet_address_new ();
	while (*to) {
		camel_internet_address_add (recipients, NULL, *to);
		to++;
	}
	camel_mime_message_set_from (message, from);
	camel_mime_message_set_recipients (message, CAMEL_RECIPIENT_TYPE_TO,
								recipients);
	camel_mime_message_set_date (message, CAMEL_MESSAGE_DATE_CURRENT, 0);
	camel_mime_part_set_content_type (CAMEL_MIME_PART(message),
								"text/plain");
	camel_mime_part_set_content (CAMEL_MIME_PART(message), text,
						strlen(text), "text/plain");

	info = camel_message_info_new (NULL);
	camel_message_info_set_flags (info, CAMEL_MESSAGE_SEEN, ~0);

	/* Return the new operation */

	ops = camel_operation_new ();
	mops = e_mail_data_operation_new ((CamelOperation *) ops);

	mops_path = e_mail_data_operation_register_gdbus_object (mops,
			g_dbus_method_invocation_get_connection(invocation),
			NULL);
	egdbus_session_complete_send_short_message (object, invocation,
								mops_path);

	/* The rest of the processing happens in a thread. */

	context = g_slice_new0 (SendAsyncContext);
	context->message = message;
	context->io_priority = G_PRIORITY_DEFAULT;
	context->from = CAMEL_ADDRESS (from);
	context->recipients = CAMEL_ADDRESS (recipients);
	context->info = info;
	context->transport = service;
	context->sent_folder_uri = g_strdup (account->sent_folder_uri);
	context->cancellable = ops;
	context->ops_path = mops_path;
	context->result = g_variant_builder_new (G_VARIANT_TYPE_ARRAY);

	/* Failure here emits a runtime warning but is non-fatal. */
	context->driver = camel_session_get_filter_driver (
				CAMEL_SESSION (session), "outgoing", &error);
	if (error != NULL) {
		g_warn_if_fail (context->driver == NULL);
		g_warning ("%s", error->message);
		g_error_free (error);
	}

	/* This gets popped in async_context_free(). */
	camel_operation_push_message (context->cancellable,
							_("Sending message"));

	simple = g_simple_async_result_new (
			G_OBJECT (session), mail_send_short_message_completed,
			context, mail_send_short_message);

	g_simple_async_result_set_op_res_gpointer (
			simple, context, (GDestroyNotify) async_context_free);

	g_simple_async_result_run_in_thread (
			simple,
			(GSimpleAsyncThreadFunc) mail_send_short_to_thread,
			context->io_priority, context->cancellable);

	g_object_unref (simple);

	return TRUE;

on_error:

	*ret_error = error;

	return FALSE;
}

