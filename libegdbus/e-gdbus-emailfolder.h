/*
 * Generated by gdbus-codegen 2.30.0. DO NOT EDIT.
 *
 * The license of this code is the same as for the source it was derived from.
 */

#ifndef __E_GDBUS_EMAILFOLDER_H__
#define __E_GDBUS_EMAILFOLDER_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for org.gnome.evolution.dataserver.mail.Folder */

#define EGDBUS_TYPE_FOLDER (egdbus_folder_get_type ())
#define EGDBUS_FOLDER(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), EGDBUS_TYPE_FOLDER, EGdbusFolder))
#define EGDBUS_IS_FOLDER(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), EGDBUS_TYPE_FOLDER))
#define EGDBUS_FOLDER_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), EGDBUS_TYPE_FOLDER, EGdbusFolderIface))

struct _EGdbusFolder;
typedef struct _EGdbusFolder EGdbusFolder;
typedef struct _EGdbusFolderIface EGdbusFolderIface;

struct _EGdbusFolderIface
{
  GTypeInterface parent_iface;


  gboolean (*handle_append_message) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_info,
    const gchar *arg_message,
    const gchar *arg_ops);

  gboolean (*handle_deleted_message_count) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_expunge) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_ops);

  gboolean (*handle_fetch_messages) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_type,
    gint arg_limit,
    const gchar *arg_ops);

  gboolean (*handle_freeze_folder) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_ops);

  gboolean (*handle_get_description) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_get_display_name) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_get_full_name) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_get_message) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_uid,
    const gchar *arg_ops);

  gboolean (*handle_get_message_by_fd) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    GUnixFDList *fd_list,
    const gchar *arg_uid,
    const gchar *arg_ops,
    GVariant *arg_message);

  gboolean (*handle_get_message_flags) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_uid);

  gboolean (*handle_get_message_info) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_uid);

  gboolean (*handle_get_message_user_flag) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_uid,
    const gchar *arg_flagname);

  gboolean (*handle_get_message_user_tag) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_uid,
    const gchar *arg_param);

  gboolean (*handle_get_parent_store) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_get_permanent_flags) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_get_quota_info) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_ops);

  gboolean (*handle_get_uids) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_has_search_capability) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_has_summary_capability) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_is_vee_folder) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_prepare_summary) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_ops);

  gboolean (*handle_refresh_info) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_ops);

  gboolean (*handle_search_by_expression) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_expression,
    const gchar *arg_ops);

  gboolean (*handle_search_by_uids) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_expression,
    const gchar *const *arg_searchuids,
    const gchar *arg_ops);

  gboolean (*handle_search_sort_by_expression) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_expression,
    const gchar *arg_sort,
    gboolean arg_ascending,
    const gchar *arg_ops);

  gboolean (*handle_set_description) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_desc);

  gboolean (*handle_set_display_name) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_name);

  gboolean (*handle_set_full_name) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_name);

  gboolean (*handle_set_message_flags) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_uid,
    guint arg_flags,
    guint arg_set);

  gboolean (*handle_set_message_user_flag) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_uid,
    const gchar *arg_flagname,
    guint arg_set);

  gboolean (*handle_set_message_user_tag) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_uid,
    const gchar *arg_param,
    const gchar *arg_value);

  gboolean (*handle_sync) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    gboolean arg_expunge,
    const gchar *arg_ops);

  gboolean (*handle_sync_message) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_uid,
    const gchar *arg_ops);

  gboolean (*handle_thaw_folder) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_ops);

  gboolean (*handle_total_message_count) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_transfer_messages_to) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *const *arg_uids,
    const gchar *arg_destfolder,
    gboolean arg_deleteoriginals,
    const gchar *arg_ops);

  gboolean (*handle_unread_message_count) (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

  void (*folder_changed) (
    EGdbusFolder *object,
    const gchar *const *arg_uids_added,
    const gchar *const *arg_uids_removed,
    const gchar *const *arg_uids_changed,
    const gchar *const *arg_uids_recent);

};

GType egdbus_folder_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *egdbus_folder_interface_info (void);
guint egdbus_folder_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void egdbus_folder_complete_refresh_info (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    gboolean success);

void egdbus_folder_complete_sync (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    gboolean success);

void egdbus_folder_complete_sync_message (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    gboolean success);

void egdbus_folder_complete_expunge (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    gboolean success);

void egdbus_folder_complete_get_display_name (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *name);

void egdbus_folder_complete_set_display_name (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

void egdbus_folder_complete_get_full_name (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *name);

void egdbus_folder_complete_set_full_name (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

void egdbus_folder_complete_get_description (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *desc);

void egdbus_folder_complete_set_description (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

void egdbus_folder_complete_get_permanent_flags (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    guint flags);

void egdbus_folder_complete_has_summary_capability (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    gboolean summary);

void egdbus_folder_complete_has_search_capability (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    gboolean search);

void egdbus_folder_complete_total_message_count (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    gint count);

void egdbus_folder_complete_unread_message_count (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    gint count);

void egdbus_folder_complete_deleted_message_count (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    gint count);

void egdbus_folder_complete_get_message_flags (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    guint flags);

void egdbus_folder_complete_set_message_flags (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    gboolean success);

void egdbus_folder_complete_get_message_user_flag (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    gboolean flag);

void egdbus_folder_complete_set_message_user_flag (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

void egdbus_folder_complete_get_message_user_tag (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *value);

void egdbus_folder_complete_set_message_user_tag (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

void egdbus_folder_complete_get_parent_store (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *store);

void egdbus_folder_complete_append_message (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *appendeduid,
    gboolean success);

void egdbus_folder_complete_get_uids (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *const *uids);

void egdbus_folder_complete_get_message (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *message);

void egdbus_folder_complete_get_message_by_fd (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    GUnixFDList *fd_list);

void egdbus_folder_complete_fetch_messages (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    gboolean more);

void egdbus_folder_complete_get_quota_info (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    GVariant *quotainfo);

void egdbus_folder_complete_search_by_expression (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *const *uids);

void egdbus_folder_complete_search_sort_by_expression (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *const *uids);

void egdbus_folder_complete_search_by_uids (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *const *resultuids);

void egdbus_folder_complete_get_message_info (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    GVariant *info);

void egdbus_folder_complete_transfer_messages_to (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    const gchar *const *returnuids);

void egdbus_folder_complete_prepare_summary (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

void egdbus_folder_complete_freeze_folder (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

void egdbus_folder_complete_thaw_folder (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation);

void egdbus_folder_complete_is_vee_folder (
    EGdbusFolder *object,
    GDBusMethodInvocation *invocation,
    gboolean vfolder);



/* D-Bus signal emissions functions: */
void egdbus_folder_emit_folder_changed (
    EGdbusFolder *object,
    const gchar *const *arg_uids_added,
    const gchar *const *arg_uids_removed,
    const gchar *const *arg_uids_changed,
    const gchar *const *arg_uids_recent);



/* D-Bus method calls: */
void egdbus_folder_call_refresh_info (
    EGdbusFolder *proxy,
    const gchar *arg_ops,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_refresh_info_finish (
    EGdbusFolder *proxy,
    gboolean *out_success,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_refresh_info_sync (
    EGdbusFolder *proxy,
    const gchar *arg_ops,
    gboolean *out_success,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_sync (
    EGdbusFolder *proxy,
    gboolean arg_expunge,
    const gchar *arg_ops,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_sync_finish (
    EGdbusFolder *proxy,
    gboolean *out_success,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_sync_sync (
    EGdbusFolder *proxy,
    gboolean arg_expunge,
    const gchar *arg_ops,
    gboolean *out_success,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_sync_message (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    const gchar *arg_ops,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_sync_message_finish (
    EGdbusFolder *proxy,
    gboolean *out_success,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_sync_message_sync (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    const gchar *arg_ops,
    gboolean *out_success,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_expunge (
    EGdbusFolder *proxy,
    const gchar *arg_ops,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_expunge_finish (
    EGdbusFolder *proxy,
    gboolean *out_success,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_expunge_sync (
    EGdbusFolder *proxy,
    const gchar *arg_ops,
    gboolean *out_success,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_get_display_name (
    EGdbusFolder *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_get_display_name_finish (
    EGdbusFolder *proxy,
    gchar **out_name,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_get_display_name_sync (
    EGdbusFolder *proxy,
    gchar **out_name,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_set_display_name (
    EGdbusFolder *proxy,
    const gchar *arg_name,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_set_display_name_finish (
    EGdbusFolder *proxy,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_set_display_name_sync (
    EGdbusFolder *proxy,
    const gchar *arg_name,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_get_full_name (
    EGdbusFolder *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_get_full_name_finish (
    EGdbusFolder *proxy,
    gchar **out_name,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_get_full_name_sync (
    EGdbusFolder *proxy,
    gchar **out_name,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_set_full_name (
    EGdbusFolder *proxy,
    const gchar *arg_name,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_set_full_name_finish (
    EGdbusFolder *proxy,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_set_full_name_sync (
    EGdbusFolder *proxy,
    const gchar *arg_name,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_get_description (
    EGdbusFolder *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_get_description_finish (
    EGdbusFolder *proxy,
    gchar **out_desc,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_get_description_sync (
    EGdbusFolder *proxy,
    gchar **out_desc,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_set_description (
    EGdbusFolder *proxy,
    const gchar *arg_desc,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_set_description_finish (
    EGdbusFolder *proxy,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_set_description_sync (
    EGdbusFolder *proxy,
    const gchar *arg_desc,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_get_permanent_flags (
    EGdbusFolder *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_get_permanent_flags_finish (
    EGdbusFolder *proxy,
    guint *out_flags,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_get_permanent_flags_sync (
    EGdbusFolder *proxy,
    guint *out_flags,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_has_summary_capability (
    EGdbusFolder *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_has_summary_capability_finish (
    EGdbusFolder *proxy,
    gboolean *out_summary,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_has_summary_capability_sync (
    EGdbusFolder *proxy,
    gboolean *out_summary,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_has_search_capability (
    EGdbusFolder *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_has_search_capability_finish (
    EGdbusFolder *proxy,
    gboolean *out_search,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_has_search_capability_sync (
    EGdbusFolder *proxy,
    gboolean *out_search,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_total_message_count (
    EGdbusFolder *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_total_message_count_finish (
    EGdbusFolder *proxy,
    gint *out_count,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_total_message_count_sync (
    EGdbusFolder *proxy,
    gint *out_count,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_unread_message_count (
    EGdbusFolder *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_unread_message_count_finish (
    EGdbusFolder *proxy,
    gint *out_count,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_unread_message_count_sync (
    EGdbusFolder *proxy,
    gint *out_count,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_deleted_message_count (
    EGdbusFolder *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_deleted_message_count_finish (
    EGdbusFolder *proxy,
    gint *out_count,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_deleted_message_count_sync (
    EGdbusFolder *proxy,
    gint *out_count,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_get_message_flags (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_get_message_flags_finish (
    EGdbusFolder *proxy,
    guint *out_flags,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_get_message_flags_sync (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    guint *out_flags,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_set_message_flags (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    guint arg_flags,
    guint arg_set,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_set_message_flags_finish (
    EGdbusFolder *proxy,
    gboolean *out_success,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_set_message_flags_sync (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    guint arg_flags,
    guint arg_set,
    gboolean *out_success,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_get_message_user_flag (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    const gchar *arg_flagname,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_get_message_user_flag_finish (
    EGdbusFolder *proxy,
    gboolean *out_flag,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_get_message_user_flag_sync (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    const gchar *arg_flagname,
    gboolean *out_flag,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_set_message_user_flag (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    const gchar *arg_flagname,
    guint arg_set,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_set_message_user_flag_finish (
    EGdbusFolder *proxy,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_set_message_user_flag_sync (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    const gchar *arg_flagname,
    guint arg_set,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_get_message_user_tag (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    const gchar *arg_param,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_get_message_user_tag_finish (
    EGdbusFolder *proxy,
    gchar **out_value,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_get_message_user_tag_sync (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    const gchar *arg_param,
    gchar **out_value,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_set_message_user_tag (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    const gchar *arg_param,
    const gchar *arg_value,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_set_message_user_tag_finish (
    EGdbusFolder *proxy,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_set_message_user_tag_sync (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    const gchar *arg_param,
    const gchar *arg_value,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_get_parent_store (
    EGdbusFolder *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_get_parent_store_finish (
    EGdbusFolder *proxy,
    gchar **out_store,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_get_parent_store_sync (
    EGdbusFolder *proxy,
    gchar **out_store,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_append_message (
    EGdbusFolder *proxy,
    GVariant *arg_info,
    const gchar *arg_message,
    const gchar *arg_ops,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_append_message_finish (
    EGdbusFolder *proxy,
    gchar **out_appendeduid,
    gboolean *out_success,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_append_message_sync (
    EGdbusFolder *proxy,
    GVariant *arg_info,
    const gchar *arg_message,
    const gchar *arg_ops,
    gchar **out_appendeduid,
    gboolean *out_success,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_get_uids (
    EGdbusFolder *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_get_uids_finish (
    EGdbusFolder *proxy,
    gchar ***out_uids,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_get_uids_sync (
    EGdbusFolder *proxy,
    gchar ***out_uids,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_get_message (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    const gchar *arg_ops,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_get_message_finish (
    EGdbusFolder *proxy,
    gchar **out_message,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_get_message_sync (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    const gchar *arg_ops,
    gchar **out_message,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_get_message_by_fd (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    const gchar *arg_ops,
    GVariant *arg_message,
    GUnixFDList *fd_list,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_get_message_by_fd_finish (
    EGdbusFolder *proxy,
    GUnixFDList **out_fd_list,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_get_message_by_fd_sync (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    const gchar *arg_ops,
    GVariant *arg_message,
    GUnixFDList  *fd_list,
    GUnixFDList **out_fd_list,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_fetch_messages (
    EGdbusFolder *proxy,
    const gchar *arg_type,
    gint arg_limit,
    const gchar *arg_ops,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_fetch_messages_finish (
    EGdbusFolder *proxy,
    gboolean *out_more,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_fetch_messages_sync (
    EGdbusFolder *proxy,
    const gchar *arg_type,
    gint arg_limit,
    const gchar *arg_ops,
    gboolean *out_more,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_get_quota_info (
    EGdbusFolder *proxy,
    const gchar *arg_ops,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_get_quota_info_finish (
    EGdbusFolder *proxy,
    GVariant **out_quotainfo,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_get_quota_info_sync (
    EGdbusFolder *proxy,
    const gchar *arg_ops,
    GVariant **out_quotainfo,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_search_by_expression (
    EGdbusFolder *proxy,
    const gchar *arg_expression,
    const gchar *arg_ops,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_search_by_expression_finish (
    EGdbusFolder *proxy,
    gchar ***out_uids,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_search_by_expression_sync (
    EGdbusFolder *proxy,
    const gchar *arg_expression,
    const gchar *arg_ops,
    gchar ***out_uids,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_search_sort_by_expression (
    EGdbusFolder *proxy,
    const gchar *arg_expression,
    const gchar *arg_sort,
    gboolean arg_ascending,
    const gchar *arg_ops,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_search_sort_by_expression_finish (
    EGdbusFolder *proxy,
    gchar ***out_uids,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_search_sort_by_expression_sync (
    EGdbusFolder *proxy,
    const gchar *arg_expression,
    const gchar *arg_sort,
    gboolean arg_ascending,
    const gchar *arg_ops,
    gchar ***out_uids,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_search_by_uids (
    EGdbusFolder *proxy,
    const gchar *arg_expression,
    const gchar *const *arg_searchuids,
    const gchar *arg_ops,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_search_by_uids_finish (
    EGdbusFolder *proxy,
    gchar ***out_resultuids,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_search_by_uids_sync (
    EGdbusFolder *proxy,
    const gchar *arg_expression,
    const gchar *const *arg_searchuids,
    const gchar *arg_ops,
    gchar ***out_resultuids,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_get_message_info (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_get_message_info_finish (
    EGdbusFolder *proxy,
    GVariant **out_info,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_get_message_info_sync (
    EGdbusFolder *proxy,
    const gchar *arg_uid,
    GVariant **out_info,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_transfer_messages_to (
    EGdbusFolder *proxy,
    const gchar *const *arg_uids,
    const gchar *arg_destfolder,
    gboolean arg_deleteoriginals,
    const gchar *arg_ops,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_transfer_messages_to_finish (
    EGdbusFolder *proxy,
    gchar ***out_returnuids,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_transfer_messages_to_sync (
    EGdbusFolder *proxy,
    const gchar *const *arg_uids,
    const gchar *arg_destfolder,
    gboolean arg_deleteoriginals,
    const gchar *arg_ops,
    gchar ***out_returnuids,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_prepare_summary (
    EGdbusFolder *proxy,
    const gchar *arg_ops,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_prepare_summary_finish (
    EGdbusFolder *proxy,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_prepare_summary_sync (
    EGdbusFolder *proxy,
    const gchar *arg_ops,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_freeze_folder (
    EGdbusFolder *proxy,
    const gchar *arg_ops,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_freeze_folder_finish (
    EGdbusFolder *proxy,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_freeze_folder_sync (
    EGdbusFolder *proxy,
    const gchar *arg_ops,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_thaw_folder (
    EGdbusFolder *proxy,
    const gchar *arg_ops,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_thaw_folder_finish (
    EGdbusFolder *proxy,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_thaw_folder_sync (
    EGdbusFolder *proxy,
    const gchar *arg_ops,
    GCancellable *cancellable,
    GError **error);

void egdbus_folder_call_is_vee_folder (
    EGdbusFolder *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean egdbus_folder_call_is_vee_folder_finish (
    EGdbusFolder *proxy,
    gboolean *out_vfolder,
    GAsyncResult *res,
    GError **error);

gboolean egdbus_folder_call_is_vee_folder_sync (
    EGdbusFolder *proxy,
    gboolean *out_vfolder,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define EGDBUS_TYPE_FOLDER_PROXY (egdbus_folder_proxy_get_type ())
#define EGDBUS_FOLDER_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), EGDBUS_TYPE_FOLDER_PROXY, EGdbusFolderProxy))
#define EGDBUS_FOLDER_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), EGDBUS_TYPE_FOLDER_PROXY, EGdbusFolderProxyClass))
#define EGDBUS_FOLDER_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), EGDBUS_TYPE_FOLDER_PROXY, EGdbusFolderProxyClass))
#define EGDBUS_IS_FOLDER_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), EGDBUS_TYPE_FOLDER_PROXY))
#define EGDBUS_IS_FOLDER_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), EGDBUS_TYPE_FOLDER_PROXY))

typedef struct _EGdbusFolderProxy EGdbusFolderProxy;
typedef struct _EGdbusFolderProxyClass EGdbusFolderProxyClass;
typedef struct _EGdbusFolderProxyPrivate EGdbusFolderProxyPrivate;

struct _EGdbusFolderProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  EGdbusFolderProxyPrivate *priv;
};

struct _EGdbusFolderProxyClass
{
  GDBusProxyClass parent_class;
};

GType egdbus_folder_proxy_get_type (void) G_GNUC_CONST;

void egdbus_folder_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
EGdbusFolder *egdbus_folder_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
EGdbusFolder *egdbus_folder_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void egdbus_folder_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
EGdbusFolder *egdbus_folder_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
EGdbusFolder *egdbus_folder_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define EGDBUS_TYPE_FOLDER_SKELETON (egdbus_folder_skeleton_get_type ())
#define EGDBUS_FOLDER_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), EGDBUS_TYPE_FOLDER_SKELETON, EGdbusFolderSkeleton))
#define EGDBUS_FOLDER_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), EGDBUS_TYPE_FOLDER_SKELETON, EGdbusFolderSkeletonClass))
#define EGDBUS_FOLDER_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), EGDBUS_TYPE_FOLDER_SKELETON, EGdbusFolderSkeletonClass))
#define EGDBUS_IS_FOLDER_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), EGDBUS_TYPE_FOLDER_SKELETON))
#define EGDBUS_IS_FOLDER_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), EGDBUS_TYPE_FOLDER_SKELETON))

typedef struct _EGdbusFolderSkeleton EGdbusFolderSkeleton;
typedef struct _EGdbusFolderSkeletonClass EGdbusFolderSkeletonClass;
typedef struct _EGdbusFolderSkeletonPrivate EGdbusFolderSkeletonPrivate;

struct _EGdbusFolderSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  EGdbusFolderSkeletonPrivate *priv;
};

struct _EGdbusFolderSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType egdbus_folder_skeleton_get_type (void) G_GNUC_CONST;

EGdbusFolder *egdbus_folder_skeleton_new (void);


G_END_DECLS

#endif /* __E_GDBUS_EMAILFOLDER_H__ */
