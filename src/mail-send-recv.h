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

#ifndef MAIL_SEND_RECV_H
#define MAIL_SEND_RECV_H

#include <camel/camel.h>
#include <libemail-engine/e-mail-session.h>

G_BEGIN_DECLS

/* send/receive all uri's */
void 		mail_send_receive		(EMailSession *session);

void		mail_receive			(EMailSession *session);

/* receive a single account */
//FIXME: SRINI
/*GCancellable *	mail_receive_account		(EMailSession *session,
						 EAccount *account);*/
GCancellable *	mail_receive_service		(CamelService *service);

GCancellable *	mail_send			(EMailSession *session);

/* setup auto receive stuff */
void		mail_autoreceive_init		(EMailSession *session);

G_END_DECLS

#endif /* MAIL_SEND_RECV_H */
