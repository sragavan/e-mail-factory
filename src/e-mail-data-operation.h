/* e-mail-data-operation.h */

#ifndef _E_MAIL_DATA_OPERATION_H
#define _E_MAIL_DATA_OPERATION_H

#include <glib-object.h>
#include <gio/gio.h>
#include <camel/camel.h>

G_BEGIN_DECLS

#define EMAIL_TYPE_DATA_OPERATION e_mail_data_operation_get_type()

#define EMAIL_DATA_OPERATION(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  EMAIL_TYPE_DATA_OPERATION, EMailDataOperation))

#define EMAIL_DATA_OPERATION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  EMAIL_TYPE_DATA_OPERATION, EMailDataOperationClass))

#define EMAIL_IS_DATA_OPERATION(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  EMAIL_TYPE_DATA_OPERATION))

#define EMAIL_IS_DATA_OPERATION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  EMAIL_TYPE_DATA_OPERATION))

#define EMAIL_DATA_OPERATION_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  EMAIL_TYPE_DATA_OPERATION, EMailDataOperationClass))

typedef struct {
  GObject parent;
} EMailDataOperation;

typedef struct {
  GObjectClass parent_class;
} EMailDataOperationClass;

GType e_mail_data_operation_get_type (void);

EMailDataOperation* e_mail_data_operation_new (CamelOperation *operation);
char *e_mail_data_operation_register_gdbus_object (EMailDataOperation *moperation, GDBusConnection *connection, GError **error);
const char * e_mail_data_operation_get_path (EMailDataOperation *moperation);
CamelOperation * e_mail_data_operation_get_camel_operation (EMailDataOperation *moperation);

void e_mail_operation_emit_cancelled (EMailDataOperation *moperation);
void e_mail_operation_emit_status (EMailDataOperation *moperation, const char *msg, int percentage);

G_END_DECLS

#endif /* _E_MAIL_DATA_OPERATION_H */

