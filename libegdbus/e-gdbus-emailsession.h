/*
 * Generated by gdbus-codegen 2.30.0. DO NOT EDIT.
 *
 * The license of this code is the same as for the source it was derived from.
 */

#ifndef __E_GDBUS_EMAILSESSION_H__
#define __E_GDBUS_EMAILSESSION_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for org.gnome.evolution.dataserver.mail.Session */

#define EGDBUS_TYPE_SESSION (egdbus_session_get_type ())
#define EGDBUS_SESSION(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), EGDBUS_TYPE_SESSION, EGdbusSession))
#define EGDBUS_IS_SESSION(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), EGDBUS_TYPE_SESSION))
#define EGDBUS_SESSION_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), EGDBUS_TYPE_SESSION, EGdbusSessionIface))

struct _EGdbusSession;
typedef struct _EGdbusSession EGdbusSession;
typedef struct _EGdbusSessionIface EGdbusSessionIface;

struct _EGdbusSessionIface
{
  GTypeInterface parent_iface;


  gboolean (*handle_add_password) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_key,
    const gchar *arg_password,
    gboolean arg_remember);

  gboolean (*handle_add_service) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_uid,
    const gchar *arg_uri,
    gboolean arg_isstore);

  gboolean (*handle_cancel_operations) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_fetch_account) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_uid);

  gboolean (*handle_fetch_old_messages) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_uid,
    gint arg_count);

  gboolean (*handle_find_password) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_key);

  gboolean (*handle_get_folder_from_uri) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_uri);

  gboolean (*handle_get_local_folder) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_type);

  gboolean (*handle_get_local_store) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_get_network_available) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_get_online) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_get_service) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_uid);

  gboolean (*handle_get_service_by_url) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_url,
    gboolean arg_isstore);

  gboolean (*handle_list_services) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_remove_service) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_uid);

  gboolean (*handle_remove_services) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_send_mails_from_outbox) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_send_receive) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_set_network_available) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    gboolean arg_online);

  gboolean (*handle_set_online) (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    gboolean arg_online);

  void (*account_added) (
    EGdbusSession *object,
    const gchar *arg_uid);

  void (*account_changed) (
    EGdbusSession *object,
    const gchar *arg_uid);

  void (*account_removed) (
    EGdbusSession *object,
    const gchar *arg_uid);

  void (*get_password) (
    EGdbusSession *object,
    const gchar *arg_title,
    const gchar *arg_prompt,
    const gchar *arg_key);

  void (*send_receive_complete) (
    EGdbusSession *object);

};

GType egdbus_session_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *egdbus_session_interface_info (void);
guint egdbus_session_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void egdbus_session_complete_add_service (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *service);

void egdbus_session_complete_remove_service (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    gboolean success);

void egdbus_session_complete_get_service (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *service);

void egdbus_session_complete_get_service_by_url (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *service);

void egdbus_session_complete_remove_services (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation);

void egdbus_session_complete_list_services (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *const *services);

void egdbus_session_complete_get_online (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    gboolean online);

void egdbus_session_complete_set_online (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation);

void egdbus_session_complete_get_network_available (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    gboolean online);

void egdbus_session_complete_set_network_available (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation);

void egdbus_session_complete_get_local_store (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *store);

void egdbus_session_complete_add_password (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation);

void egdbus_session_complete_find_password (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *password);

void egdbus_session_complete_get_local_folder (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *folder);

void egdbus_session_complete_get_folder_from_uri (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *folder);

void egdbus_session_complete_send_receive (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation);

void egdbus_session_complete_send_mails_from_outbox (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *operation);

void egdbus_session_complete_fetch_account (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    const gchar *operation);

void egdbus_session_complete_fetch_old_messages (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation,
    gboolean success);

void egdbus_session_complete_cancel_operations (
    EGdbusSession *object,
    GDBusMethodInvocation *invocation);



/* D-Bus signal emissions functions: */
void egdbus_session_emit_send_receive_complete (
    EGdbusSession *object);

void egdbus_session_emit_account_added (
    EGdbusSession *object,
    const gchar *arg_uid);

void egdbus_session_emit_account_removed (
    EGdbusSession *object,
    const gchar *arg_uid);

void egdbus_session_emit_account_changed (
    EGdbusSession *object,
    const gchar *arg_uid);

void egdbus_session_emit_get_password (
    EGdbusSession *object,
    const gchar *arg_title,
    const gchar *arg_prompt,
    const gchar *arg_key);



/* D-Bus method calls: */
void egdbus_session_call_add_service (
    EGdbusSession *proxy,
    const gchar *arg_uid,
    const gchar *arg_uri,
    gboolean arg_isstore,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_add_service_finish (
    EGdbusSession *proxy,
    gchar **out_service,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_add_service_sync (
    EGdbusSession *proxy,
    const gchar *arg_uid,
    const gchar *arg_uri,
    gboolean arg_isstore,
    gchar **out_service,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_remove_service (
    EGdbusSession *proxy,
    const gchar *arg_uid,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_remove_service_finish (
    EGdbusSession *proxy,
    gboolean *out_success,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_remove_service_sync (
    EGdbusSession *proxy,
    const gchar *arg_uid,
    gboolean *out_success,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_get_service (
    EGdbusSession *proxy,
    const gchar *arg_uid,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_get_service_finish (
    EGdbusSession *proxy,
    gchar **out_service,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_get_service_sync (
    EGdbusSession *proxy,
    const gchar *arg_uid,
    gchar **out_service,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_get_service_by_url (
    EGdbusSession *proxy,
    const gchar *arg_url,
    gboolean arg_isstore,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_get_service_by_url_finish (
    EGdbusSession *proxy,
    gchar **out_service,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_get_service_by_url_sync (
    EGdbusSession *proxy,
    const gchar *arg_url,
    gboolean arg_isstore,
    gchar **out_service,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_remove_services (
    EGdbusSession *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_remove_services_finish (
    EGdbusSession *proxy,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_remove_services_sync (
    EGdbusSession *proxy,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_list_services (
    EGdbusSession *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_list_services_finish (
    EGdbusSession *proxy,
    gchar ***out_services,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_list_services_sync (
    EGdbusSession *proxy,
    gchar ***out_services,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_get_online (
    EGdbusSession *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_get_online_finish (
    EGdbusSession *proxy,
    gboolean *out_online,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_get_online_sync (
    EGdbusSession *proxy,
    gboolean *out_online,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_set_online (
    EGdbusSession *proxy,
    gboolean arg_online,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_set_online_finish (
    EGdbusSession *proxy,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_set_online_sync (
    EGdbusSession *proxy,
    gboolean arg_online,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_get_network_available (
    EGdbusSession *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_get_network_available_finish (
    EGdbusSession *proxy,
    gboolean *out_online,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_get_network_available_sync (
    EGdbusSession *proxy,
    gboolean *out_online,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_set_network_available (
    EGdbusSession *proxy,
    gboolean arg_online,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_set_network_available_finish (
    EGdbusSession *proxy,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_set_network_available_sync (
    EGdbusSession *proxy,
    gboolean arg_online,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_get_local_store (
    EGdbusSession *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_get_local_store_finish (
    EGdbusSession *proxy,
    gchar **out_store,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_get_local_store_sync (
    EGdbusSession *proxy,
    gchar **out_store,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_add_password (
    EGdbusSession *proxy,
    const gchar *arg_key,
    const gchar *arg_password,
    gboolean arg_remember,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_add_password_finish (
    EGdbusSession *proxy,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_add_password_sync (
    EGdbusSession *proxy,
    const gchar *arg_key,
    const gchar *arg_password,
    gboolean arg_remember,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_find_password (
    EGdbusSession *proxy,
    const gchar *arg_key,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_find_password_finish (
    EGdbusSession *proxy,
    gchar **out_password,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_find_password_sync (
    EGdbusSession *proxy,
    const gchar *arg_key,
    gchar **out_password,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_get_local_folder (
    EGdbusSession *proxy,
    const gchar *arg_type,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_get_local_folder_finish (
    EGdbusSession *proxy,
    gchar **out_folder,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_get_local_folder_sync (
    EGdbusSession *proxy,
    const gchar *arg_type,
    gchar **out_folder,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_get_folder_from_uri (
    EGdbusSession *proxy,
    const gchar *arg_uri,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_get_folder_from_uri_finish (
    EGdbusSession *proxy,
    gchar **out_folder,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_get_folder_from_uri_sync (
    EGdbusSession *proxy,
    const gchar *arg_uri,
    gchar **out_folder,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_send_receive (
    EGdbusSession *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_send_receive_finish (
    EGdbusSession *proxy,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_send_receive_sync (
    EGdbusSession *proxy,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_send_mails_from_outbox (
    EGdbusSession *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_send_mails_from_outbox_finish (
    EGdbusSession *proxy,
    gchar **out_operation,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_send_mails_from_outbox_sync (
    EGdbusSession *proxy,
    gchar **out_operation,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_fetch_account (
    EGdbusSession *proxy,
    const gchar *arg_uid,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_fetch_account_finish (
    EGdbusSession *proxy,
    gchar **out_operation,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_fetch_account_sync (
    EGdbusSession *proxy,
    const gchar *arg_uid,
    gchar **out_operation,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_fetch_old_messages (
    EGdbusSession *proxy,
    const gchar *arg_uid,
    gint arg_count,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_fetch_old_messages_finish (
    EGdbusSession *proxy,
    gboolean *out_success,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_fetch_old_messages_sync (
    EGdbusSession *proxy,
    const gchar *arg_uid,
    gint arg_count,
    gboolean *out_success,
    GCancellable *cancellable,
    GError **error);

void egdbus_session_call_cancel_operations (
    EGdbusSession *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_session_call_cancel_operations_finish (
    EGdbusSession *proxy,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_session_call_cancel_operations_sync (
    EGdbusSession *proxy,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define EGDBUS_TYPE_SESSION_PROXY (egdbus_session_proxy_get_type ())
#define EGDBUS_SESSION_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), EGDBUS_TYPE_SESSION_PROXY, EGdbusSessionProxy))
#define EGDBUS_SESSION_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), EGDBUS_TYPE_SESSION_PROXY, EGdbusSessionProxyClass))
#define EGDBUS_SESSION_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), EGDBUS_TYPE_SESSION_PROXY, EGdbusSessionProxyClass))
#define EGDBUS_IS_SESSION_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), EGDBUS_TYPE_SESSION_PROXY))
#define EGDBUS_IS_SESSION_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), EGDBUS_TYPE_SESSION_PROXY))

typedef struct _EGdbusSessionProxy EGdbusSessionProxy;
typedef struct _EGdbusSessionProxyClass EGdbusSessionProxyClass;
typedef struct _EGdbusSessionProxyPrivate EGdbusSessionProxyPrivate;

struct _EGdbusSessionProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  EGdbusSessionProxyPrivate *priv;
};

struct _EGdbusSessionProxyClass
{
  GDBusProxyClass parent_class;
};

GType egdbus_session_proxy_get_type (void) G_GNUC_CONST;

void egdbus_session_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
EGdbusSession *egdbus_session_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
EGdbusSession *egdbus_session_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void egdbus_session_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
EGdbusSession *egdbus_session_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
EGdbusSession *egdbus_session_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define EGDBUS_TYPE_SESSION_SKELETON (egdbus_session_skeleton_get_type ())
#define EGDBUS_SESSION_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), EGDBUS_TYPE_SESSION_SKELETON, EGdbusSessionSkeleton))
#define EGDBUS_SESSION_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), EGDBUS_TYPE_SESSION_SKELETON, EGdbusSessionSkeletonClass))
#define EGDBUS_SESSION_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), EGDBUS_TYPE_SESSION_SKELETON, EGdbusSessionSkeletonClass))
#define EGDBUS_IS_SESSION_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), EGDBUS_TYPE_SESSION_SKELETON))
#define EGDBUS_IS_SESSION_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), EGDBUS_TYPE_SESSION_SKELETON))

typedef struct _EGdbusSessionSkeleton EGdbusSessionSkeleton;
typedef struct _EGdbusSessionSkeletonClass EGdbusSessionSkeletonClass;
typedef struct _EGdbusSessionSkeletonPrivate EGdbusSessionSkeletonPrivate;

struct _EGdbusSessionSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  EGdbusSessionSkeletonPrivate *priv;
};

struct _EGdbusSessionSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType egdbus_session_skeleton_get_type (void) G_GNUC_CONST;

EGdbusSession *egdbus_session_skeleton_new (void);


G_END_DECLS

#endif /* __E_GDBUS_EMAILSESSION_H__ */
