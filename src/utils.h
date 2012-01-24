

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
G_END_DECLS

#endif /* UTILS_H */

