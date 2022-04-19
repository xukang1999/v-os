#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_hwpmc_hooks.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/disk.h>
#include <sys/fail.h>
#include <sys/fcntl.h>
#include <sys/file.h>
#include <sys/kdb.h>
#include <sys/ktr.h>
#include <sys/stat.h>
#include <sys/priv.h>
#include <sys/proc.h>
#include <sys/limits.h>
#include <sys/lock.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/mutex.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/bio.h>
#include <sys/buf.h>
#include <sys/filio.h>
#include <sys/resourcevar.h>
#include <sys/rwlock.h>
#include <sys/prng.h>
#include <sys/sx.h>
#include <sys/sleepqueue.h>
#include <sys/sysctl.h>
#include <sys/ttycom.h>
#include <sys/conf.h>
#include <sys/syslog.h>
#include <sys/unistd.h>
#include <sys/user.h>

#include <security/audit/audit.h>
#include <security/mac/mac_framework.h>

#include <vm/vm.h>
#include <vm/vm_extern.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <vm/vm_pager.h>
#include "sys/vnode.h"
struct vnodeop_desc vop_default_desc;
struct vnodeop_desc vop_islocked_desc;
struct vnodeop_desc vop_lookup_desc;
struct vnodeop_desc vop_cachedlookup_desc;
struct vnodeop_desc vop_create_desc;
struct vnodeop_desc vop_whiteout_desc;
struct vnodeop_desc vop_mknod_desc;
struct vnodeop_desc vop_open_desc;
struct vnodeop_desc vop_close_desc;
struct vnodeop_desc vop_fplookup_vexec_desc;
struct vnodeop_desc vop_fplookup_symlink_desc;
struct vnodeop_desc vop_access_desc;
struct vnodeop_desc vop_accessx_desc;
struct vnodeop_desc vop_stat_desc;
struct vnodeop_desc vop_getattr_desc;
struct vnodeop_desc vop_setattr_desc;
struct vnodeop_desc vop_mmapped_desc;
struct vnodeop_desc vop_read_desc;
struct vnodeop_desc vop_read_pgcache_desc;
 struct vnodeop_desc vop_write_desc;
struct vnodeop_desc vop_ioctl_desc;
struct vnodeop_desc vop_poll_desc;
struct vnodeop_desc vop_kqfilter_desc;
struct vnodeop_desc vop_revoke_desc;
struct vnodeop_desc vop_fsync_desc;
struct vnodeop_desc vop_remove_desc;
struct vnodeop_desc vop_link_desc;
 struct vnodeop_desc vop_rename_desc;
struct vnodeop_desc vop_mkdir_desc;
struct vnodeop_desc vop_rmdir_desc;
struct vnodeop_desc vop_symlink_desc;
struct vnodeop_desc vop_readdir_desc;
struct vnodeop_desc vop_readlink_desc;
struct vnodeop_desc vop_inactive_desc;
struct vnodeop_desc vop_need_inactive_desc;
struct vnodeop_desc vop_reclaim_desc;
struct vnodeop_desc vop_lock1_desc;
struct vnodeop_desc vop_unlock_desc;
struct vnodeop_desc vop_bmap_desc;
struct vnodeop_desc vop_strategy_desc;
struct vnodeop_desc vop_getwritemount_desc;
struct vnodeop_desc vop_print_desc;
struct vnodeop_desc vop_pathconf_desc;
struct vnodeop_desc vop_advlock_desc;
struct vnodeop_desc vop_advlockasync_desc;
struct vnodeop_desc vop_advlockpurge_desc;
struct vnodeop_desc vop_reallocblks_desc;
struct vnodeop_desc vop_getpages_desc;
struct vnodeop_desc vop_getpages_async_desc;
struct vnodeop_desc vop_putpages_desc;
struct vnodeop_desc vop_getacl_desc;
struct vnodeop_desc vop_setacl_desc;
struct vnodeop_desc vop_aclcheck_desc;
struct vnodeop_desc vop_closeextattr_desc;
struct vnodeop_desc vop_getextattr_desc;
struct vnodeop_desc vop_listextattr_desc;
struct vnodeop_desc vop_openextattr_desc;
struct vnodeop_desc vop_deleteextattr_desc;
struct vnodeop_desc vop_setextattr_desc;
struct vnodeop_desc vop_setlabel_desc;
struct vnodeop_desc vop_vptofh_desc;
struct vnodeop_desc vop_vptocnp_desc;
struct vnodeop_desc vop_allocate_desc;
struct vnodeop_desc vop_advise_desc;
struct vnodeop_desc vop_unp_bind_desc;
struct vnodeop_desc vop_unp_connect_desc;
struct vnodeop_desc vop_unp_detach_desc;
struct vnodeop_desc vop_is_text_desc;
struct vnodeop_desc vop_set_text_desc;
struct vnodeop_desc vop_unset_text_desc;
struct vnodeop_desc vop_add_writecount_desc;
struct vnodeop_desc vop_fdatasync_desc;
struct vnodeop_desc vop_copy_file_range_desc;
struct vnodeop_desc vop_vput_pair_desc;
struct vnodeop_desc vop_spare1_desc;
struct vnodeop_desc vop_spare2_desc;
struct vnodeop_desc vop_spare3_desc;
struct vnodeop_desc vop_spare4_desc;
struct vnodeop_desc vop_spare5_desc;
int VOP_ISLOCKED_AP(struct vop_islocked_args *args){return -1;}
int VOP_ISLOCKED_APV(struct vop_vector *vop, struct vop_islocked_args *args){return -1;}
int VOP_LOOKUP_AP(struct vop_lookup_args *args){return -1;}
int VOP_LOOKUP_APV(struct vop_vector *vop, struct vop_lookup_args *args){return -1;}
int VOP_CACHEDLOOKUP_AP(struct vop_cachedlookup_args *args){return -1;}
int VOP_CACHEDLOOKUP_APV(struct vop_vector *vop, struct vop_cachedlookup_args *args){return -1;}
int VOP_CREATE_AP(struct vop_create_args *args){return -1;}
int VOP_CREATE_APV(struct vop_vector *vop, struct vop_create_args *args){return -1;}
int VOP_WHITEOUT_AP(struct vop_whiteout_args *args){return -1;}
int VOP_WHITEOUT_APV(struct vop_vector *vop, struct vop_whiteout_args *args){return -1;}
int VOP_MKNOD_AP(struct vop_mknod_args *args){return -1;}
int VOP_MKNOD_APV(struct vop_vector *vop, struct vop_mknod_args *args){return -1;}
int VOP_OPEN_AP(struct vop_open_args *args){return -1;}
int VOP_OPEN_APV(struct vop_vector *vop, struct vop_open_args *args){return -1;}
int VOP_CLOSE_AP(struct vop_close_args *args){return -1;}
int VOP_CLOSE_APV(struct vop_vector *vop, struct vop_close_args *args){return -1;}
int VOP_FPLOOKUP_VEXEC_AP(struct vop_fplookup_vexec_args *args){return -1;}
int VOP_FPLOOKUP_VEXEC_APV(struct vop_vector *vop, struct vop_fplookup_vexec_args *args){return -1;}
int VOP_FPLOOKUP_SYMLINK_AP(struct vop_fplookup_symlink_args *args){return -1;}
int VOP_FPLOOKUP_SYMLINK_APV(struct vop_vector *vop, struct vop_fplookup_symlink_args *args){return -1;}
int VOP_ACCESS_AP(struct vop_access_args *args){return -1;}
int VOP_ACCESS_APV(struct vop_vector *vop, struct vop_access_args *args){return -1;}
int VOP_ACCESSX_AP(struct vop_accessx_args *args){return -1;}
int VOP_ACCESSX_APV(struct vop_vector *vop, struct vop_accessx_args *args){return -1;}
int VOP_STAT_AP(struct vop_stat_args *args){return -1;}
int VOP_STAT_APV(struct vop_vector *vop, struct vop_stat_args *args){return -1;}
int VOP_GETATTR_AP(struct vop_getattr_args *args){return -1;}
int VOP_GETATTR_APV(struct vop_vector *vop, struct vop_getattr_args *args){return -1;}

int VOP_SETATTR_AP(struct vop_setattr_args *args){return -1;}
int VOP_SETATTR_APV(struct vop_vector *vop, struct vop_setattr_args *args){return -1;}
int VOP_MMAPPED_AP(struct vop_mmapped_args *args){return -1;}
int VOP_MMAPPED_APV(struct vop_vector *vop, struct vop_mmapped_args *args){return -1;}
int VOP_READ_AP(struct vop_read_args *args){return -1;}
int VOP_READ_APV(struct vop_vector *vop, struct vop_read_args *args){return -1;}
int VOP_READ_PGCACHE_AP(struct vop_read_pgcache_args *args){return -1;}
int VOP_READ_PGCACHE_APV(struct vop_vector *vop, struct vop_read_pgcache_args *args){return -1;}
int VOP_WRITE_AP(struct vop_write_args *args){return -1;}
int VOP_WRITE_APV(struct vop_vector *vop, struct vop_write_args *args){return -1;}
int VOP_IOCTL_AP(struct vop_ioctl_args *args){return -1;}
int VOP_IOCTL_APV(struct vop_vector *vop, struct vop_ioctl_args *args){return -1;}
int VOP_POLL_AP(struct vop_poll_args *args){return -1;}
int VOP_POLL_APV(struct vop_vector *vop, struct vop_poll_args *args){return -1;}
int VOP_KQFILTER_AP(struct vop_kqfilter_args *args){return -1;}
int VOP_KQFILTER_APV(struct vop_vector *vop, struct vop_kqfilter_args *args){return -1;}
int VOP_REVOKE_AP(struct vop_revoke_args *args){return -1;}
int VOP_REVOKE_APV(struct vop_vector *vop, struct vop_revoke_args *args){return -1;}
int VOP_FSYNC_AP(struct vop_fsync_args *args){return -1;}
int VOP_FSYNC_APV(struct vop_vector *vop, struct vop_fsync_args *args){return -1;}
int VOP_REMOVE_AP(struct vop_remove_args *args){return -1;}
int VOP_REMOVE_APV(struct vop_vector *vop, struct vop_remove_args *args){return -1;}
int VOP_LINK_AP(struct vop_link_args *args){return -1;}
int VOP_LINK_APV(struct vop_vector *vop, struct vop_link_args *args){return -1;}
int VOP_RENAME_AP(struct vop_rename_args *args){return -1;}
int VOP_RENAME_APV(struct vop_vector *vop, struct vop_rename_args *args){return -1;}
int VOP_MKDIR_AP(struct vop_mkdir_args *args){return -1;}
int VOP_MKDIR_APV(struct vop_vector *vop, struct vop_mkdir_args *args){return -1;}
int VOP_RMDIR_AP(struct vop_rmdir_args *args){return -1;}
int VOP_RMDIR_APV(struct vop_vector *vop, struct vop_rmdir_args *args){return -1;}

int VOP_SYMLINK_AP(struct vop_symlink_args *args){return -1;}
int VOP_SYMLINK_APV(struct vop_vector *vop, struct vop_symlink_args *args){return -1;}
int VOP_READDIR_AP(struct vop_readdir_args *args){return -1;}
int VOP_READDIR_APV(struct vop_vector *vop, struct vop_readdir_args *args){return -1;}
int VOP_READLINK_AP(struct vop_readlink_args *args){return -1;}
int VOP_READLINK_APV(struct vop_vector *vop, struct vop_readlink_args *args){return -1;}
int VOP_INACTIVE_AP(struct vop_inactive_args *args){return -1;}
int VOP_INACTIVE_APV(struct vop_vector *vop, struct vop_inactive_args *args){return -1;}
int VOP_NEED_INACTIVE_AP(struct vop_need_inactive_args *args){return -1;}
int VOP_NEED_INACTIVE_APV(struct vop_vector *vop, struct vop_need_inactive_args *args){return -1;}
int VOP_RECLAIM_AP(struct vop_reclaim_args *args){return -1;}
int VOP_RECLAIM_APV(struct vop_vector *vop, struct vop_reclaim_args *args){return -1;}
int VOP_LOCK1_AP(struct vop_lock1_args *args){return -1;}
int VOP_LOCK1_APV(struct vop_vector *vop, struct vop_lock1_args *args){return -1;}
int VOP_UNLOCK_AP(struct vop_unlock_args *args){return -1;}
int VOP_UNLOCK_APV(struct vop_vector *vop, struct vop_unlock_args * args){return -1;}
int VOP_BMAP_AP(struct vop_bmap_args * args){return -1;}
int VOP_BMAP_APV(struct vop_vector *vop, struct vop_bmap_args * args){return -1;}
int VOP_STRATEGY_AP(struct vop_strategy_args *args){return -1;}
int VOP_STRATEGY_APV(struct vop_vector *vop, struct vop_strategy_args *args){return -1;}
int VOP_GETWRITEMOUNT_AP(struct vop_getwritemount_args *args){return -1;}
int VOP_GETWRITEMOUNT_APV(struct vop_vector *vop, struct vop_getwritemount_args *args){return -1;}
int VOP_PRINT_AP(struct vop_print_args *args){return -1;}
int VOP_PRINT_APV(struct vop_vector *vop, struct vop_print_args *args){return -1;}
int VOP_PATHCONF_AP(struct vop_pathconf_args *args){return -1;}
int VOP_PATHCONF_APV(struct vop_vector *vop, struct vop_pathconf_args *args){return -1;}
int VOP_ADVLOCK_AP(struct vop_advlock_args *args){return -1;}
int VOP_ADVLOCK_APV(struct vop_vector *vop, struct vop_advlock_args *args){return -1;}
int VOP_ADVLOCKASYNC_AP(struct vop_advlockasync_args *args){return -1;}
int VOP_ADVLOCKASYNC_APV(struct vop_vector *vop, struct vop_advlockasync_args *args){return -1;}
int VOP_ADVLOCKPURGE_AP(struct vop_advlockpurge_args *args){return -1;}
int VOP_ADVLOCKPURGE_APV(struct vop_vector *vop, struct vop_advlockpurge_args *args){return -1;}
int VOP_REALLOCBLKS_AP(struct vop_reallocblks_args *args){return -1;}
int VOP_REALLOCBLKS_APV(struct vop_vector *vop, struct vop_reallocblks_args *args){return -1;}
int VOP_GETPAGES_AP(struct vop_getpages_args * args){return -1;}
int VOP_GETPAGES_APV(struct vop_vector *vop, struct vop_getpages_args * args){return -1;}
int VOP_GETPAGES_ASYNC_AP(struct vop_getpages_async_args *args){return -1;}
int VOP_GETPAGES_ASYNC_APV(struct vop_vector *vop, struct vop_getpages_async_args *args){return -1;}
int VOP_PUTPAGES_AP(struct vop_putpages_args *args){return -1;}
int VOP_PUTPAGES_APV(struct vop_vector *vop, struct vop_putpages_args *args){return -1;}
int VOP_GETACL_AP(struct vop_getacl_args *args){return -1;}
int VOP_GETACL_APV(struct vop_vector *vop, struct vop_getacl_args *args){return -1;}
int VOP_SETACL_AP(struct vop_setacl_args *args){return -1;}
int VOP_SETACL_APV(struct vop_vector *vop, struct vop_setacl_args *args){return -1;}
int VOP_ACLCHECK_AP(struct vop_aclcheck_args *args){return -1;}
int VOP_ACLCHECK_APV(struct vop_vector *vop, struct vop_aclcheck_args *args){return -1;}
int VOP_CLOSEEXTATTR_AP(struct vop_closeextattr_args *args){return -1;}
int VOP_CLOSEEXTATTR_APV(struct vop_vector *vop, struct vop_closeextattr_args *args){return -1;}
int VOP_GETEXTATTR_AP(struct vop_getextattr_args *args){return -1;}
int VOP_GETEXTATTR_APV(struct vop_vector *vop, struct vop_getextattr_args *args){return -1;}
int VOP_LISTEXTATTR_AP(struct vop_listextattr_args *args){return -1;}
int VOP_LISTEXTATTR_APV(struct vop_vector *vop, struct vop_listextattr_args *args){return -1;}
int VOP_OPENEXTATTR_AP(struct vop_openextattr_args *args){return -1;}
int VOP_OPENEXTATTR_APV(struct vop_vector *vop, struct vop_openextattr_args *args){return -1;}
int VOP_DELETEEXTATTR_AP(struct vop_deleteextattr_args *args){return -1;}
int VOP_DELETEEXTATTR_APV(struct vop_vector *vop, struct vop_deleteextattr_args *args){return -1;}
int VOP_SETEXTATTR_AP(struct vop_setextattr_args *args){return -1;}
int VOP_SETEXTATTR_APV(struct vop_vector *vop, struct vop_setextattr_args *args){return -1;}
int VOP_SETLABEL_AP(struct vop_setlabel_args *args){return -1;}
int VOP_SETLABEL_APV(struct vop_vector *vop, struct vop_setlabel_args *args){return -1;}
int VOP_VPTOFH_AP(struct vop_vptofh_args *args){return -1;}
int VOP_VPTOFH_APV(struct vop_vector *vop, struct vop_vptofh_args *args){return -1;}
int VOP_VPTOCNP_AP(struct vop_vptocnp_args *args){return -1;}
int VOP_VPTOCNP_APV(struct vop_vector *vop, struct vop_vptocnp_args *args){return -1;}
int VOP_ALLOCATE_AP(struct vop_allocate_args *args){return -1;}
int VOP_ALLOCATE_APV(struct vop_vector *vop, struct vop_allocate_args *args){return -1;}
int VOP_ADVISE_AP(struct vop_advise_args *args){return -1;}
int VOP_ADVISE_APV(struct vop_vector *vop, struct vop_advise_args *args){return -1;}

int VOP_UNP_BIND_AP(struct vop_unp_bind_args *args){return -1;}
int VOP_UNP_BIND_APV(struct vop_vector *vop, struct vop_unp_bind_args *args){return -1;}
int VOP_UNP_CONNECT_AP(struct vop_unp_connect_args *args){return -1;}
int VOP_UNP_CONNECT_APV(struct vop_vector *vop, struct vop_unp_connect_args *args){return -1;}
int VOP_UNP_DETACH_AP(struct vop_unp_detach_args *args){return -1;}
int VOP_UNP_DETACH_APV(struct vop_vector *vop, struct vop_unp_detach_args *args){return -1;}
int VOP_IS_TEXT_AP(struct vop_is_text_args *args){return -1;}
int VOP_IS_TEXT_APV(struct vop_vector *vop, struct vop_is_text_args *args){return -1;}
int VOP_SET_TEXT_AP(struct vop_set_text_args * args){return -1;}
int VOP_SET_TEXT_APV(struct vop_vector *vop, struct vop_set_text_args * args){return -1;}
int VOP_UNSET_TEXT_AP(struct vop_unset_text_args *args){return -1;}
int VOP_UNSET_TEXT_APV(struct vop_vector *vop, struct vop_unset_text_args *args){return -1;}
int VOP_ADD_WRITECOUNT_AP(struct vop_add_writecount_args *args){return -1;}
int VOP_ADD_WRITECOUNT_APV(struct vop_vector *vop, struct vop_add_writecount_args *args){return -1;}
int VOP_FDATASYNC_AP(struct vop_fdatasync_args *args){return -1;}
int VOP_FDATASYNC_APV(struct vop_vector *vop, struct vop_fdatasync_args *args){return -1;}
int VOP_COPY_FILE_RANGE_AP(struct vop_copy_file_range_args *args){return -1;}
int VOP_COPY_FILE_RANGE_APV(struct vop_vector *vop, struct vop_copy_file_range_args *args){return -1;}
int VOP_VPUT_PAIR_AP(struct vop_vput_pair_args *args){return -1;}
int VOP_VPUT_PAIR_APV(struct vop_vector *vop, struct vop_vput_pair_args *args){return -1;}
int VOP_SPARE1_AP(struct vop_spare1_args *args){return -1;}
int VOP_SPARE1_APV(struct vop_vector *vop, struct vop_spare1_args *args){return -1;}
int VOP_SPARE2_AP(struct vop_spare2_args *args){return -1;}
int VOP_SPARE2_APV(struct vop_vector *vop, struct vop_spare2_args *args){return -1;}
int VOP_SPARE3_AP(struct vop_spare3_args *args){return -1;}
int VOP_SPARE3_APV(struct vop_vector *vop, struct vop_spare3_args *args){return -1;}
int VOP_SPARE4_AP(struct vop_spare4_args *args){return -1;}
int VOP_SPARE4_APV(struct vop_vector *vop, struct vop_spare4_args *args){return -1;}
int VOP_SPARE5_AP(struct vop_spare5_args *args){return -1;}
int VOP_SPARE5_APV(struct vop_vector *vop, struct vop_spare5_args *args){return -1;}
void vfs_vector_op_register(struct vop_vector *orig_vop){return;}