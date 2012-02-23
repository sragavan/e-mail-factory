/* mail-send-short-msg.h */

#ifndef _MAIL_SEND_SHORT_MSG_H
#define _MAIL_SEND_SHORT_MSG_H

#include <glib-object.h>
#include <gio/gio.h>
#include <libemail-engine/e-mail-session.h>

GCancellable *
mail_send_short_message (EMailSession *session, const char *account_uid,
			const char *text, const char **to, GError **ret_error);

#endif /* _MAIL_SEND_SHORT_MSG_H */
