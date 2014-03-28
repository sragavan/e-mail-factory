

#ifndef UTILS_H
#define UTILS_H

#include <camel/camel.h>

G_BEGIN_DECLS

typedef enum {
	EMAIL_DEBUG_FOLDER=1,
	EMAIL_DEBUG_STORE=2,
	EMAIL_DEBUG_SESSION=3,
	EMAIL_DEBUG_OPERATION=4,
	EMAIL_DEBUG_IPC=5,
	EMAIL_DEBUG_MICRO=6
} EMailDebugFlag;

void 		mail_debug_init 		(void);
gboolean 	mail_debug_log 			(EMailDebugFlag flag);
char * 		mail_get_service_url 		(CamelService *service);
void 		mail_operate_on_object		(GObject *object, 
						GCancellable *cancellable,
		       	    			gboolean (*do_op) (GObject *object, gpointer data, GError **error),
		            			void (*done) (gboolean ret, gpointer data, GError *error), 
		            			gpointer data);
void		mail_provider_fetch_lock 	(const char *source);
void		mail_provider_fetch_unlock 	(const char *source);
CamelFolder *	mail_provider_fetch_inbox_folder
						(const char *source,
						 GCancellable *cancellable, 
						 GError **error);
gboolean	mail_get_keep_on_server 	(CamelService *service);
GList *		mail_get_all_accounts		(void);
GList *		mail_get_store_accounts		(void);
GList *		mail_get_transport_accounts	(void);

G_END_DECLS

#endif /* UTILS_H */

