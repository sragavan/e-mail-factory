/*
 * Generated by gdbus-codegen 2.30.0. DO NOT EDIT.
 *
 * The license of this code is the same as for the source it was derived from.
 */

#ifndef __E_GDBUS_EMAILOPERATION_H__
#define __E_GDBUS_EMAILOPERATION_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for org.gnome.evolution.dataserver.mail.Operation */

#define EGDBUS_TYPE_OPERATION (egdbus_operation_get_type ())
#define EGDBUS_OPERATION(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), EGDBUS_TYPE_OPERATION, EGdbusOperation))
#define EGDBUS_IS_OPERATION(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), EGDBUS_TYPE_OPERATION))
#define EGDBUS_OPERATION_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), EGDBUS_TYPE_OPERATION, EGdbusOperationIface))

struct _EGdbusOperation;
typedef struct _EGdbusOperation EGdbusOperation;
typedef struct _EGdbusOperationIface EGdbusOperationIface;

struct _EGdbusOperationIface
{
  GTypeInterface parent_iface;


  gboolean (*handle_cancel) (
    EGdbusOperation *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_is_cancelled) (
    EGdbusOperation *object,
    GDBusMethodInvocation *invocation);

  void (*cancelled) (
    EGdbusOperation *object);

  void (*status) (
    EGdbusOperation *object,
    const gchar *arg_description,
    gint arg_percentage);

};

GType egdbus_operation_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *egdbus_operation_interface_info (void);
guint egdbus_operation_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void egdbus_operation_complete_cancel (
    EGdbusOperation *object,
    GDBusMethodInvocation *invocation);

void egdbus_operation_complete_is_cancelled (
    EGdbusOperation *object,
    GDBusMethodInvocation *invocation,
    gboolean cancelled);



/* D-Bus signal emissions functions: */
void egdbus_operation_emit_status (
    EGdbusOperation *object,
    const gchar *arg_description,
    gint arg_percentage);

void egdbus_operation_emit_cancelled (
    EGdbusOperation *object);



/* D-Bus method calls: */
void egdbus_operation_call_cancel (
    EGdbusOperation *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_operation_call_cancel_finish (
    EGdbusOperation *proxy,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_operation_call_cancel_sync (
    EGdbusOperation *proxy,
    GCancellable *cancellable,
    GError **error);

void egdbus_operation_call_is_cancelled (
    EGdbusOperation *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_operation_call_is_cancelled_finish (
    EGdbusOperation *proxy,
    gboolean *out_cancelled,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_operation_call_is_cancelled_sync (
    EGdbusOperation *proxy,
    gboolean *out_cancelled,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define EGDBUS_TYPE_OPERATION_PROXY (egdbus_operation_proxy_get_type ())
#define EGDBUS_OPERATION_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), EGDBUS_TYPE_OPERATION_PROXY, EGdbusOperationProxy))
#define EGDBUS_OPERATION_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), EGDBUS_TYPE_OPERATION_PROXY, EGdbusOperationProxyClass))
#define EGDBUS_OPERATION_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), EGDBUS_TYPE_OPERATION_PROXY, EGdbusOperationProxyClass))
#define EGDBUS_IS_OPERATION_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), EGDBUS_TYPE_OPERATION_PROXY))
#define EGDBUS_IS_OPERATION_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), EGDBUS_TYPE_OPERATION_PROXY))

typedef struct _EGdbusOperationProxy EGdbusOperationProxy;
typedef struct _EGdbusOperationProxyClass EGdbusOperationProxyClass;
typedef struct _EGdbusOperationProxyPrivate EGdbusOperationProxyPrivate;

struct _EGdbusOperationProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  EGdbusOperationProxyPrivate *priv;
};

struct _EGdbusOperationProxyClass
{
  GDBusProxyClass parent_class;
};

GType egdbus_operation_proxy_get_type (void) G_GNUC_CONST;

void egdbus_operation_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
EGdbusOperation *egdbus_operation_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
EGdbusOperation *egdbus_operation_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void egdbus_operation_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
EGdbusOperation *egdbus_operation_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
EGdbusOperation *egdbus_operation_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define EGDBUS_TYPE_OPERATION_SKELETON (egdbus_operation_skeleton_get_type ())
#define EGDBUS_OPERATION_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), EGDBUS_TYPE_OPERATION_SKELETON, EGdbusOperationSkeleton))
#define EGDBUS_OPERATION_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), EGDBUS_TYPE_OPERATION_SKELETON, EGdbusOperationSkeletonClass))
#define EGDBUS_OPERATION_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), EGDBUS_TYPE_OPERATION_SKELETON, EGdbusOperationSkeletonClass))
#define EGDBUS_IS_OPERATION_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), EGDBUS_TYPE_OPERATION_SKELETON))
#define EGDBUS_IS_OPERATION_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), EGDBUS_TYPE_OPERATION_SKELETON))

typedef struct _EGdbusOperationSkeleton EGdbusOperationSkeleton;
typedef struct _EGdbusOperationSkeletonClass EGdbusOperationSkeletonClass;
typedef struct _EGdbusOperationSkeletonPrivate EGdbusOperationSkeletonPrivate;

struct _EGdbusOperationSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  EGdbusOperationSkeletonPrivate *priv;
};

struct _EGdbusOperationSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType egdbus_operation_skeleton_get_type (void) G_GNUC_CONST;

EGdbusOperation *egdbus_operation_skeleton_new (void);


G_END_DECLS

#endif /* __E_GDBUS_EMAILOPERATION_H__ */
