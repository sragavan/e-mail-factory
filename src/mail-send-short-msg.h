/* mail-send-short-msg.h */

#ifndef _MAIL_SEND_SHORT_MSG_H
#define _MAIL_SEND_SHORT_MSG_H

#include <e-gdbus-emailsession.h>
#include <e-mail-data-session.h>

gboolean
mail_send_short_message (EGdbusSession *object,
		GDBusMethodInvocation *invocation, const char *account_uid,
		const char *text, const char **to,
		EMailDataSession *msession, GError **ret_error);

#endif /* _MAIL_SEND_SHORT_MSG_H */
