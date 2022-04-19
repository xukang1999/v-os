/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2000-2004
 *	Poul-Henning Kamp.  All rights reserved.
 * Copyright (c) 1989, 1992-1993, 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software donated to Berkeley by
 * Jan-Simon Pendry.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)kernfs_vnops.c	8.15 (Berkeley) 5/21/95
 * From: FreeBSD: src/sys/miscfs/kernfs/kernfs_vnops.c 1.43
 *
 * $FreeBSD$
 */

/*
 * TODO:
 *	mkdir: want it ?
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/dirent.h>
#include <sys/eventhandler.h>
#include <sys/fcntl.h>
#include <sys/file.h>
#include <sys/filedesc.h>
#include <sys/filio.h>
#include <sys/jail.h>
#include <sys/kernel.h>
#include <sys/limits.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/namei.h>
#include <sys/priv.h>
#include <sys/proc.h>
#include <sys/stat.h>
#include <sys/sx.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/ttycom.h>
#include <sys/unistd.h>
#include <sys/vnode.h>

static struct vop_vector devfs_vnodeops;
static struct vop_vector devfs_specops;
static struct fileops devfs_ops_f;

#include <security/mac/mac_framework.h>

#include <vm/vm.h>
#include <vm/vm_extern.h>
#include <vm/vm_object.h>

static MALLOC_DEFINE(M_CDEVPDATA, "DEVFSP", "Metainfo for cdev-fp data");

struct mtx	devfs_de_interlock;
MTX_SYSINIT(devfs_de_interlock, &devfs_de_interlock, "devfs interlock", MTX_DEF);
struct sx	clone_drain_lock;
SX_SYSINIT(clone_drain_lock, &clone_drain_lock, "clone events drain lock");
struct mtx	cdevpriv_mtx;
MTX_SYSINIT(cdevpriv_mtx, &cdevpriv_mtx, "cdevpriv lock", MTX_DEF);

SYSCTL_DECL(_vfs_devfs);

static int devfs_dotimes;
SYSCTL_INT(_vfs_devfs, OID_AUTO, dotimes, CTLFLAG_RW,
    &devfs_dotimes, 0, "Update timestamps on DEVFS with default precision");

/*
 * Update devfs node timestamp.  Note that updates are unlocked and
 * stat(2) could see partially updated times.
 */
static void
devfs_timestamp(struct timespec *tsp)
{
	time_t ts;

	if (devfs_dotimes) {
		vfs_timestamp(tsp);
	} else {
		ts = time_second;
		if (tsp->tv_sec != ts) {
			tsp->tv_sec = ts;
			tsp->tv_nsec = 0;
		}
	}
}

static int
devfs_fp_check(struct file *fp, struct cdev **devp, struct cdevsw **dswp,
    int *ref)
{

	*dswp = devvn_refthread(fp->f_vnode, devp, ref);
	if (*devp != fp->f_data) {
		if (*dswp != NULL)
			dev_relthread(*devp, *ref);
		return (ENXIO);
	}
	KASSERT((*devp)->si_refcount > 0,
	    ("devfs: un-referenced struct cdev *(%s)", devtoname(*devp)));
	if (*dswp == NULL)
		return (ENXIO);
	curthread->td_fpop = fp;
	return (0);
}

int
devfs_get_cdevpriv(void **datap)
{
	return 0;
}

int
devfs_set_cdevpriv(void *priv, d_priv_dtor_t *priv_dtr)
{
	return 0;
}

void
devfs_destroy_cdevpriv(struct cdev_privdata *p)
{
	return;
}

static void
devfs_fpdrop(struct file *fp)
{
	return;
}

void
devfs_clear_cdevpriv(void)
{
	struct file *fp;

	fp = curthread->td_fpop;
	if (fp == NULL)
		return;
	devfs_fpdrop(fp);
}

static void
devfs_usecount_add(struct vnode *vp)
{

}

static void
devfs_usecount_subl(struct vnode *vp)
{

}

static void
devfs_usecount_sub(struct vnode *vp)
{

}

static int
devfs_usecountl(struct vnode *vp)
{

	VNPASS(vp->v_type == VCHR, vp);
	mtx_assert(&devfs_de_interlock, MA_OWNED);
	ASSERT_VI_LOCKED(vp, __func__);
	return (vp->v_rdev->si_usecount);
}

int
c_usecount(struct vnode *vp)
{
	int count;

	VNPASS(vp->v_type == VCHR, vp);
	mtx_lock(&devfs_de_interlock);
	VI_LOCK(vp);
	count = devfs_usecountl(vp);
	VI_UNLOCK(vp);
	mtx_unlock(&devfs_de_interlock);
	return (count);
}

void
devfs_ctty_ref(struct vnode *vp)
{

	vrefact(vp);
	devfs_usecount_add(vp);
}

void
devfs_ctty_unref(struct vnode *vp)
{

	devfs_usecount_sub(vp);
	vrele(vp);
}

/*
 * On success devfs_populate_vp() returns with dmp->dm_lock held.
 */
static int
devfs_populate_vp(struct vnode *vp)
{


	return (0);
}

static int
devfs_vptocnp(struct vop_vptocnp_args *ap)
{
	return -1;
}

/*
 * Construct the fully qualified path name relative to the mountpoint.
 * If a NULL cnp is provided, no '/' is appended to the resulting path.
 */
char *
devfs_fqpn(char *buf, struct devfs_mount *dmp, struct devfs_dirent *dd,
    struct componentname *cnp)
{
	return NULL;
}

static int
devfs_allocv_drop_refs(int drop_dm_lock, struct devfs_mount *dmp,
	struct devfs_dirent *de)
{
	return 0;
}

static void
devfs_insmntque_dtr(struct vnode *vp, void *arg)
{

}

/*
 * devfs_allocv shall be entered with dmp->dm_lock held, and it drops
 * it on return.
 */
int
devfs_allocv(struct devfs_dirent *de, struct mount *mp, int lockmode,
    struct vnode **vpp)
{

	return (0);
}

static int
devfs_access(struct vop_access_args *ap)
{
	return 0;
}

_Static_assert(((FMASK | FCNTLFLAGS) & (FLASTCLOSE | FREVOKE)) == 0,
    "devfs-only flag reuse failed");

static int
devfs_close(struct vop_close_args *ap)
{
	return 0;
}

static int
devfs_close_f(struct file *fp, struct thread *td)
{
	return 0;
}

static int
devfs_getattr(struct vop_getattr_args *ap)
{
	return 0;
}

/* ARGSUSED */
static int
devfs_ioctl_f(struct file *fp, u_long com, void *data, struct ucred *cred, struct thread *td)
{
	struct file *fpop;
	int error;

	fpop = td->td_fpop;
	td->td_fpop = fp;
	error = vnops.fo_ioctl(fp, com, data, cred, td);
	td->td_fpop = fpop;
	return (error);
}

void *
fiodgname_buf_get_ptr(void *fgnp, u_long com)
{
	union {
		struct fiodgname_arg	fgn;
#ifdef COMPAT_FREEBSD32
		struct fiodgname_arg32	fgn32;
#endif
	} *fgnup;

	fgnup = fgnp;
	switch (com) {
	case FIODGNAME:
		return (fgnup->fgn.buf);
#ifdef COMPAT_FREEBSD32
	case FIODGNAME_32:
		return ((void *)(uintptr_t)fgnup->fgn32.buf);
#endif
	default:
		panic("Unhandled ioctl command %ld", com);
	}
}

static int
devfs_ioctl(struct vop_ioctl_args *ap)
{
	return -1;
}

/* ARGSUSED */
static int
devfs_kqfilter_f(struct file *fp, struct knote *kn)
{
	struct cdev *dev;
	struct cdevsw *dsw;
	int error, ref;
	struct file *fpop;
	struct thread *td;

	td = curthread;
	fpop = td->td_fpop;
	error = devfs_fp_check(fp, &dev, &dsw, &ref);
	if (error)
		return (error);
	error = dsw->d_kqfilter(dev, kn);
	td->td_fpop = fpop;
	dev_relthread(dev, ref);
	return (error);
}

static inline int
devfs_prison_check(struct devfs_dirent *de, struct thread *td)
{
	return 0;
}

static int
devfs_lookupx(struct vop_lookup_args *ap, int *dm_unlock)
{
	return 0;
}

static int
devfs_lookup(struct vop_lookup_args *ap)
{
	return 0;
}

static int
devfs_mknod(struct vop_mknod_args *ap)
{
	return 0;
}

/* ARGSUSED */
static int
devfs_open(struct vop_open_args *ap)
{
	return 0;
}

static int
devfs_pathconf(struct vop_pathconf_args *ap)
{

	switch (ap->a_name) {
	case _PC_FILESIZEBITS:
		*ap->a_retval = 64;
		return (0);
	case _PC_NAME_MAX:
		*ap->a_retval = NAME_MAX;
		return (0);
	case _PC_LINK_MAX:
		*ap->a_retval = INT_MAX;
		return (0);
	case _PC_SYMLINK_MAX:
		*ap->a_retval = MAXPATHLEN;
		return (0);
	case _PC_MAX_CANON:
		if (ap->a_vp->v_vflag & VV_ISTTY) {
			*ap->a_retval = MAX_CANON;
			return (0);
		}
		return (EINVAL);
	case _PC_MAX_INPUT:
		if (ap->a_vp->v_vflag & VV_ISTTY) {
			*ap->a_retval = MAX_INPUT;
			return (0);
		}
		return (EINVAL);
	case _PC_VDISABLE:
		if (ap->a_vp->v_vflag & VV_ISTTY) {
			*ap->a_retval = _POSIX_VDISABLE;
			return (0);
		}
		return (EINVAL);
	case _PC_MAC_PRESENT:
#ifdef MAC
		/*
		 * If MAC is enabled, devfs automatically supports
		 * trivial non-persistent label storage.
		 */
		*ap->a_retval = 1;
#else
		*ap->a_retval = 0;
#endif
		return (0);
	case _PC_CHOWN_RESTRICTED:
		*ap->a_retval = 1;
		return (0);
	default:
		return (vop_stdpathconf(ap));
	}
	/* NOTREACHED */
}

/* ARGSUSED */
static int
devfs_poll_f(struct file *fp, int events, struct ucred *cred, struct thread *td)
{
	struct cdev *dev;
	struct cdevsw *dsw;
	int error, ref;
	struct file *fpop;

	fpop = td->td_fpop;
	error = devfs_fp_check(fp, &dev, &dsw, &ref);
	if (error != 0) {
		error = vnops.fo_poll(fp, events, cred, td);
		return (error);
	}
	error = dsw->d_poll(dev, events, td);
	td->td_fpop = fpop;
	dev_relthread(dev, ref);
	return(error);
}

/*
 * Print out the contents of a special device vnode.
 */
static int
devfs_print(struct vop_print_args *ap)
{

	printf("\tdev %s\n", devtoname(ap->a_vp->v_rdev));
	return (0);
}

static int
devfs_read_f(struct file *fp, struct uio *uio, struct ucred *cred,
    int flags, struct thread *td)
{
	struct cdev *dev;
	int ioflag, error, ref;
	ssize_t resid;
	struct cdevsw *dsw;
	struct file *fpop;

	if (uio->uio_resid > DEVFS_IOSIZE_MAX)
		return (EINVAL);
	fpop = td->td_fpop;
	error = devfs_fp_check(fp, &dev, &dsw, &ref);
	if (error != 0) {
		error = vnops.fo_read(fp, uio, cred, flags, td);
		return (error);
	}
	resid = uio->uio_resid;
	ioflag = fp->f_flag & (O_NONBLOCK | O_DIRECT);
	if (ioflag & O_DIRECT)
		ioflag |= IO_DIRECT;

	foffset_lock_uio(fp, uio, flags | FOF_NOLOCK);
	error = dsw->d_read(dev, uio, ioflag);
	if (uio->uio_resid != resid || (error == 0 && resid != 0))
		devfs_timestamp(&dev->si_atime);
	td->td_fpop = fpop;
	dev_relthread(dev, ref);

	foffset_unlock_uio(fp, uio, flags | FOF_NOLOCK | FOF_NEXTOFF_R);
	return (error);
}

static int
devfs_readdir(struct vop_readdir_args *ap)
{
	return 0;
}

static int
devfs_readlink(struct vop_readlink_args *ap)
{
	return 0;
}

static void
devfs_reclaiml(struct vnode *vp)
{
	return;
}

static int
devfs_reclaim(struct vop_reclaim_args *ap)
{
	struct vnode *vp;

	vp = ap->a_vp;
	mtx_lock(&devfs_de_interlock);
	devfs_reclaiml(vp);
	mtx_unlock(&devfs_de_interlock);
	return (0);
}

static int
devfs_reclaim_vchr(struct vop_reclaim_args *ap)
{
	struct vnode *vp;
	struct cdev *dev;

	vp = ap->a_vp;
	MPASS(vp->v_type == VCHR);

	mtx_lock(&devfs_de_interlock);
	VI_LOCK(vp);
	devfs_usecount_subl(vp);
	devfs_reclaiml(vp);
	mtx_unlock(&devfs_de_interlock);
	dev_lock();
	dev = vp->v_rdev;
	vp->v_rdev = NULL;
	dev_unlock();
	VI_UNLOCK(vp);
	if (dev != NULL)
		dev_rel(dev);
	return (0);
}

static int
devfs_remove(struct vop_remove_args *ap)
{

	return (0);
}

/*
 * Revoke is called on a tty when a terminal session ends.  The vnode
 * is orphaned by setting v_op to deadfs so we need to let go of it
 * as well so that we create a new one next time around.
 *
 */
static int
devfs_revoke(struct vop_revoke_args *ap)
{

	return (0);
}

static int
devfs_rioctl(struct vop_ioctl_args *ap)
{
	return 0;
}

static int
devfs_rread(struct vop_read_args *ap)
{

	if (ap->a_vp->v_type != VDIR)
		return (EINVAL);
	return (VOP_READDIR(ap->a_vp, ap->a_uio, ap->a_cred, NULL, NULL, NULL));
}

static int
devfs_setattr(struct vop_setattr_args *ap)
{
	return 0;
}

#ifdef MAC
static int
devfs_setlabel(struct vop_setlabel_args *ap)
{
	struct vnode *vp;
	struct devfs_dirent *de;

	vp = ap->a_vp;
	de = vp->v_data;

	mac_vnode_relabel(ap->a_cred, vp, ap->a_label);
	mac_devfs_update(vp->v_mount, de, vp);

	return (0);
}
#endif

static int
devfs_stat_f(struct file *fp, struct stat *sb, struct ucred *cred, struct thread *td)
{

	return (vnops.fo_stat(fp, sb, cred, td));
}

static int
devfs_symlink(struct vop_symlink_args *ap)
{
	return 0;
}

static int
devfs_truncate_f(struct file *fp, off_t length, struct ucred *cred, struct thread *td)
{

	return (vnops.fo_truncate(fp, length, cred, td));
}

static int
devfs_write_f(struct file *fp, struct uio *uio, struct ucred *cred,
    int flags, struct thread *td)
{
	struct cdev *dev;
	int error, ioflag, ref;
	ssize_t resid;
	struct cdevsw *dsw;
	struct file *fpop;

	if (uio->uio_resid > DEVFS_IOSIZE_MAX)
		return (EINVAL);
	fpop = td->td_fpop;
	error = devfs_fp_check(fp, &dev, &dsw, &ref);
	if (error != 0) {
		error = vnops.fo_write(fp, uio, cred, flags, td);
		return (error);
	}
	KASSERT(uio->uio_td == td, ("uio_td %p is not td %p", uio->uio_td, td));
	ioflag = fp->f_flag & (O_NONBLOCK | O_DIRECT | O_FSYNC);
	if (ioflag & O_DIRECT)
		ioflag |= IO_DIRECT;
	foffset_lock_uio(fp, uio, flags | FOF_NOLOCK);

	resid = uio->uio_resid;

	error = dsw->d_write(dev, uio, ioflag);
	if (uio->uio_resid != resid || (error == 0 && resid != 0)) {
		devfs_timestamp(&dev->si_ctime);
		dev->si_mtime = dev->si_ctime;
	}
	td->td_fpop = fpop;
	dev_relthread(dev, ref);

	foffset_unlock_uio(fp, uio, flags | FOF_NOLOCK | FOF_NEXTOFF_W);
	return (error);
}

static int
devfs_mmap_f(struct file *fp, vm_map_t map, vm_offset_t *addr, vm_size_t size,
    vm_prot_t prot, vm_prot_t cap_maxprot, int flags, vm_ooffset_t foff,
    struct thread *td)
{
	struct cdev *dev;
	struct cdevsw *dsw;
	struct mount *mp;
	struct vnode *vp;
	struct file *fpop;
	vm_object_t object;
	vm_prot_t maxprot;
	int error, ref;

	vp = fp->f_vnode;

	/*
	 * Ensure that file and memory protections are
	 * compatible.
	 */
	mp = vp->v_mount;
	if (mp != NULL && (mp->mnt_flag & MNT_NOEXEC) != 0) {
		maxprot = VM_PROT_NONE;
		if ((prot & VM_PROT_EXECUTE) != 0)
			return (EACCES);
	} else
		maxprot = VM_PROT_EXECUTE;
	if ((fp->f_flag & FREAD) != 0)
		maxprot |= VM_PROT_READ;
	else if ((prot & VM_PROT_READ) != 0)
		return (EACCES);

	/*
	 * If we are sharing potential changes via MAP_SHARED and we
	 * are trying to get write permission although we opened it
	 * without asking for it, bail out.
	 *
	 * Note that most character devices always share mappings.
	 * The one exception is that D_MMAP_ANON devices
	 * (i.e. /dev/zero) permit private writable mappings.
	 *
	 * Rely on vm_mmap_cdev() to fail invalid MAP_PRIVATE requests
	 * as well as updating maxprot to permit writing for
	 * D_MMAP_ANON devices rather than doing that here.
	 */
	if ((flags & MAP_SHARED) != 0) {
		if ((fp->f_flag & FWRITE) != 0)
			maxprot |= VM_PROT_WRITE;
		else if ((prot & VM_PROT_WRITE) != 0)
			return (EACCES);
	}
	maxprot &= cap_maxprot;

	fpop = td->td_fpop;
	error = devfs_fp_check(fp, &dev, &dsw, &ref);
	if (error != 0)
		return (error);

	error = vm_mmap_cdev(td, size, prot, &maxprot, &flags, dev, dsw, &foff,
	    &object);
	td->td_fpop = fpop;
	dev_relthread(dev, ref);
	if (error != 0)
		return (error);

	error = vm_mmap_object(map, addr, size, prot, maxprot, flags, object,
	    foff, FALSE, td);
	if (error != 0)
		vm_object_deallocate(object);
	return (error);
}

dev_t
dev2udev(struct cdev *x)
{
	return 0;
}

static struct fileops devfs_ops_f = {
	.fo_read =	devfs_read_f,
	.fo_write =	devfs_write_f,
	.fo_truncate =	devfs_truncate_f,
	.fo_ioctl =	devfs_ioctl_f,
	.fo_poll =	devfs_poll_f,
	.fo_kqfilter =	devfs_kqfilter_f,
	.fo_stat =	devfs_stat_f,
	.fo_close =	devfs_close_f,
	.fo_chmod =	vn_chmod,
	.fo_chown =	vn_chown,
	.fo_sendfile =	vn_sendfile,
	.fo_seek =	vn_seek,
	.fo_fill_kinfo = vn_fill_kinfo,
	.fo_mmap =	devfs_mmap_f,
	.fo_flags =	DFLAG_PASSABLE | DFLAG_SEEKABLE
};

/* Vops for non-CHR vnodes in /dev. */
static struct vop_vector devfs_vnodeops = {
	.vop_default =		&default_vnodeops,

	.vop_access =		devfs_access,
	.vop_getattr =		devfs_getattr,
	.vop_ioctl =		devfs_rioctl,
	.vop_lookup =		devfs_lookup,
	.vop_mknod =		devfs_mknod,
	.vop_pathconf =		devfs_pathconf,
	.vop_read =		devfs_rread,
	.vop_readdir =		devfs_readdir,
	.vop_readlink =		devfs_readlink,
	.vop_reclaim =		devfs_reclaim,
	.vop_remove =		devfs_remove,
	.vop_revoke =		devfs_revoke,
	.vop_setattr =		devfs_setattr,
#ifdef MAC
	.vop_setlabel =		devfs_setlabel,
#endif
	.vop_symlink =		devfs_symlink,
	.vop_vptocnp =		devfs_vptocnp,
	.vop_lock1 =		vop_lock,
	.vop_unlock =		vop_unlock,
	.vop_islocked =		vop_islocked,
};
VFS_VOP_VECTOR_REGISTER(devfs_vnodeops);

/* Vops for VCHR vnodes in /dev. */
static struct vop_vector devfs_specops = {
	.vop_default =		&default_vnodeops,

	.vop_access =		devfs_access,
	.vop_bmap =		VOP_PANIC,
	.vop_close =		devfs_close,
	.vop_create =		VOP_PANIC,
	.vop_fsync =		vop_stdfsync,
	.vop_getattr =		devfs_getattr,
	.vop_ioctl =		devfs_ioctl,
	.vop_link =		VOP_PANIC,
	.vop_mkdir =		VOP_PANIC,
	.vop_mknod =		VOP_PANIC,
	.vop_open =		devfs_open,
	.vop_pathconf =		devfs_pathconf,
	.vop_poll =		dead_poll,
	.vop_print =		devfs_print,
	.vop_read =		dead_read,
	.vop_readdir =		VOP_PANIC,
	.vop_readlink =		VOP_PANIC,
	.vop_reallocblks =	VOP_PANIC,
	.vop_reclaim =		devfs_reclaim_vchr,
	.vop_remove =		devfs_remove,
	.vop_rename =		VOP_PANIC,
	.vop_revoke =		devfs_revoke,
	.vop_rmdir =		VOP_PANIC,
	.vop_setattr =		devfs_setattr,
#ifdef MAC
	.vop_setlabel =		devfs_setlabel,
#endif
	.vop_strategy =		VOP_PANIC,
	.vop_symlink =		VOP_PANIC,
	.vop_vptocnp =		devfs_vptocnp,
	.vop_write =		dead_write,
	.vop_lock1 =		vop_lock,
	.vop_unlock =		vop_unlock,
	.vop_islocked =		vop_islocked,
};
VFS_VOP_VECTOR_REGISTER(devfs_specops);

/*
 * Our calling convention to the device drivers used to be that we passed
 * vnode.h IO_* flags to read()/write(), but we're moving to fcntl.h O_ 
 * flags instead since that's what open(), close() and ioctl() takes and
 * we don't really want vnode.h in device drivers.
 * We solved the source compatibility by redefining some vnode flags to
 * be the same as the fcntl ones and by sending down the bitwise OR of
 * the respective fcntl/vnode flags.  These CTASSERTS make sure nobody
 * pulls the rug out under this.
 */
CTASSERT(O_NONBLOCK == IO_NDELAY);
CTASSERT(O_FSYNC == IO_SYNC);
