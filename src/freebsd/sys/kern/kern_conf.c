/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 1999-2002 Poul-Henning Kamp
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/bio.h>
#include <sys/devctl.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/module.h>
#include <sys/malloc.h>
#include <sys/conf.h>
#include <sys/vnode.h>
#include <sys/queue.h>
#include <sys/poll.h>
#include <sys/sx.h>
#include <sys/ctype.h>
#include <sys/ucred.h>
#include <sys/taskqueue.h>
#include <machine/stdarg.h>
#define	dev_lock_assert_locked()	mtx_assert(&devmtx, MA_OWNED)
#define	dev_lock_assert_unlocked()	mtx_assert(&devmtx, MA_NOTOWNED)

#include <fs/devfs/devfs_int.h>
#include <vm/vm.h>

static MALLOC_DEFINE(M_DEVT, "cdev", "cdev storage");

struct mtx devmtx;
static void destroy_devl(struct cdev* dev);
static int destroy_dev_sched_cbl(struct cdev* dev,
	void (*cb)(void*), void* arg);
static void destroy_dev_tq(void* ctx, int pending);
static int make_dev_credv(int flags, struct cdev** dres, struct cdevsw* devsw,
	int unit, struct ucred* cr, uid_t uid, gid_t gid, int mode, const char* fmt,
	va_list ap);

void
dev_lock(void)
{

	mtx_lock(&devmtx);
}

/*
 * Free all the memory collected while the cdev mutex was
 * locked. Since devmtx is after the system map mutex, free() cannot
 * be called immediately and is postponed until cdev mutex can be
 * dropped.
 */
static void
dev_unlock_and_free(void)
{

}

static void
dev_free_devlocked(struct cdev* cdev)
{

}

static void
cdevsw_free_devlocked(struct cdevsw* csw)
{

}

void
dev_unlock(void)
{

	mtx_unlock(&devmtx);
}

void
dev_ref(struct cdev* dev)
{

	dev_lock_assert_unlocked();
	mtx_lock(&devmtx);
	dev->si_refcount++;
	mtx_unlock(&devmtx);
}

void
dev_refl(struct cdev* dev)
{

	dev_lock_assert_locked();
	dev->si_refcount++;
}

void
dev_rel(struct cdev* dev)
{
	int flag = 0;

	dev_lock_assert_unlocked();
	dev_lock();
	dev->si_refcount--;
	KASSERT(dev->si_refcount >= 0,
		("dev_rel(%s) gave negative count", devtoname(dev)));
	if (dev->si_devsw == NULL && dev->si_refcount == 0) {
		LIST_REMOVE(dev, si_list);
		flag = 1;
	}
	dev_unlock();
	if (flag)
		devfs_free(dev);
}

struct cdevsw*
	dev_refthread(struct cdev* dev, int* ref)
{
	return NULL;
}

struct cdevsw*
	devvn_refthread(struct vnode* vp, struct cdev** devp, int* ref)
{
	return NULL;
}

void
dev_relthread(struct cdev* dev, int ref)
{

	dev_lock_assert_unlocked();
	if (!ref)
		return;
	KASSERT(dev->si_threadcount > 0,
		("%s threadcount is wrong", dev->si_name));
	atomic_subtract_rel_long(&dev->si_threadcount, 1);
}

int
nullop(void)
{

	return (0);
}

int
eopnotsupp(void)
{

	return (EOPNOTSUPP);
}

static int
enxio(void)
{
	return (ENXIO);
}

static int
enodev(void)
{
	return (ENODEV);
}

/* Define a dead_cdevsw for use when devices leave unexpectedly. */

#define dead_open	(d_open_t *)enxio
#define dead_close	(d_close_t *)enxio
#define dead_read	(d_read_t *)enxio
#define dead_write	(d_write_t *)enxio
#define dead_ioctl	(d_ioctl_t *)enxio
#define dead_poll	(d_poll_t *)enodev
#define dead_mmap	(d_mmap_t *)enodev

static void
dead_strategy(struct bio* bp)
{

	biofinish(bp, NULL, ENXIO);
}

#define dead_dump	(dumper_t *)enxio
#define dead_kqfilter	(d_kqfilter_t *)enxio
#define dead_mmap_single (d_mmap_single_t *)enodev

static struct cdevsw dead_cdevsw = {
	.d_version = D_VERSION,
	.d_open = dead_open,
	.d_close = dead_close,
	.d_read = dead_read,
	.d_write = dead_write,
	.d_ioctl = dead_ioctl,
	.d_poll = dead_poll,
	.d_mmap = dead_mmap,
	.d_strategy = dead_strategy,
	.d_name = "dead",
	.d_dump = dead_dump,
	.d_kqfilter = dead_kqfilter,
	.d_mmap_single = dead_mmap_single
};

/* Default methods if driver does not specify method */

#define null_open	(d_open_t *)nullop
#define null_close	(d_close_t *)nullop
#define no_read		(d_read_t *)enodev
#define no_write	(d_write_t *)enodev
#define no_ioctl	(d_ioctl_t *)enodev
#define no_mmap		(d_mmap_t *)enodev
#define no_kqfilter	(d_kqfilter_t *)enodev
#define no_mmap_single	(d_mmap_single_t *)enodev

static void
no_strategy(struct bio* bp)
{

	biofinish(bp, NULL, ENODEV);
}

static int
no_poll(struct cdev* dev __unused, int events, struct thread* td __unused)
{

	return (poll_no_poll(events));
}

#define no_dump		(dumper_t *)enodev

static int
giant_open(struct cdev* dev, int oflags, int devtype, struct thread* td)
{
	struct cdevsw* dsw;
	int ref, retval;

	dsw = dev_refthread(dev, &ref);
	if (dsw == NULL)
		return (ENXIO);
	mtx_lock(&Giant);
	retval = dsw->d_gianttrick->d_open(dev, oflags, devtype, td);
	mtx_unlock(&Giant);
	dev_relthread(dev, ref);
	return (retval);
}

static int
giant_fdopen(struct cdev* dev, int oflags, struct thread* td, struct file* fp)
{
	struct cdevsw* dsw;
	int ref, retval;

	dsw = dev_refthread(dev, &ref);
	if (dsw == NULL)
		return (ENXIO);
	mtx_lock(&Giant);
	retval = dsw->d_gianttrick->d_fdopen(dev, oflags, td, fp);
	mtx_unlock(&Giant);
	dev_relthread(dev, ref);
	return (retval);
}

static int
giant_close(struct cdev* dev, int fflag, int devtype, struct thread* td)
{
	struct cdevsw* dsw;
	int ref, retval;

	dsw = dev_refthread(dev, &ref);
	if (dsw == NULL)
		return (ENXIO);
	mtx_lock(&Giant);
	retval = dsw->d_gianttrick->d_close(dev, fflag, devtype, td);
	mtx_unlock(&Giant);
	dev_relthread(dev, ref);
	return (retval);
}

static void
giant_strategy(struct bio* bp)
{
	struct cdevsw* dsw;
	struct cdev* dev;
	int ref;

	dev = bp->bio_dev;
	dsw = dev_refthread(dev, &ref);
	if (dsw == NULL) {
		biofinish(bp, NULL, ENXIO);
		return;
	}
	mtx_lock(&Giant);
	dsw->d_gianttrick->d_strategy(bp);
	mtx_unlock(&Giant);
	dev_relthread(dev, ref);
}

static int
giant_ioctl(struct cdev* dev, u_long cmd, caddr_t data, int fflag, struct thread* td)
{
	struct cdevsw* dsw;
	int ref, retval;

	dsw = dev_refthread(dev, &ref);
	if (dsw == NULL)
		return (ENXIO);
	mtx_lock(&Giant);
	retval = dsw->d_gianttrick->d_ioctl(dev, cmd, data, fflag, td);
	mtx_unlock(&Giant);
	dev_relthread(dev, ref);
	return (retval);
}

static int
giant_read(struct cdev* dev, struct uio* uio, int ioflag)
{
	struct cdevsw* dsw;
	int ref, retval;

	dsw = dev_refthread(dev, &ref);
	if (dsw == NULL)
		return (ENXIO);
	mtx_lock(&Giant);
	retval = dsw->d_gianttrick->d_read(dev, uio, ioflag);
	mtx_unlock(&Giant);
	dev_relthread(dev, ref);
	return (retval);
}

static int
giant_write(struct cdev* dev, struct uio* uio, int ioflag)
{
	struct cdevsw* dsw;
	int ref, retval;

	dsw = dev_refthread(dev, &ref);
	if (dsw == NULL)
		return (ENXIO);
	mtx_lock(&Giant);
	retval = dsw->d_gianttrick->d_write(dev, uio, ioflag);
	mtx_unlock(&Giant);
	dev_relthread(dev, ref);
	return (retval);
}

static int
giant_poll(struct cdev* dev, int events, struct thread* td)
{
	struct cdevsw* dsw;
	int ref, retval;

	dsw = dev_refthread(dev, &ref);
	if (dsw == NULL)
		return (ENXIO);
	mtx_lock(&Giant);
	retval = dsw->d_gianttrick->d_poll(dev, events, td);
	mtx_unlock(&Giant);
	dev_relthread(dev, ref);
	return (retval);
}

static int
giant_kqfilter(struct cdev* dev, struct knote* kn)
{
	struct cdevsw* dsw;
	int ref, retval;

	dsw = dev_refthread(dev, &ref);
	if (dsw == NULL)
		return (ENXIO);
	mtx_lock(&Giant);
	retval = dsw->d_gianttrick->d_kqfilter(dev, kn);
	mtx_unlock(&Giant);
	dev_relthread(dev, ref);
	return (retval);
}

static int
giant_mmap(struct cdev* dev, vm_ooffset_t offset, vm_paddr_t* paddr, int nprot,
	vm_memattr_t* memattr)
{
	struct cdevsw* dsw;
	int ref, retval;

	dsw = dev_refthread(dev, &ref);
	if (dsw == NULL)
		return (ENXIO);
	mtx_lock(&Giant);
	retval = dsw->d_gianttrick->d_mmap(dev, offset, paddr, nprot,
		memattr);
	mtx_unlock(&Giant);
	dev_relthread(dev, ref);
	return (retval);
}

static int
giant_mmap_single(struct cdev* dev, vm_ooffset_t* offset, vm_size_t size,
	vm_object_t* object, int nprot)
{
	struct cdevsw* dsw;
	int ref, retval;

	dsw = dev_refthread(dev, &ref);
	if (dsw == NULL)
		return (ENXIO);
	mtx_lock(&Giant);
	retval = dsw->d_gianttrick->d_mmap_single(dev, offset, size, object,
		nprot);
	mtx_unlock(&Giant);
	dev_relthread(dev, ref);
	return (retval);
}

static void
notify(struct cdev* dev, const char* ev, int flags)
{
	static const char prefix[] = "cdev=";
	char* data;
	int namelen, mflags;

	if (cold)
		return;
	mflags = (flags & MAKEDEV_NOWAIT) ? M_NOWAIT : M_WAITOK;
	namelen = strlen(dev->si_name);
	data = malloc(namelen + sizeof(prefix), M_TEMP, mflags);
	if (data == NULL)
		return;
	memcpy(data, prefix, sizeof(prefix) - 1);
	memcpy(data + sizeof(prefix) - 1, dev->si_name, namelen + 1);
	devctl_notify("DEVFS", "CDEV", ev, data);
	free(data, M_TEMP);
}

static void
notify_create(struct cdev* dev, int flags)
{

	notify(dev, "CREATE", flags);
}

static void
notify_destroy(struct cdev* dev)
{

	notify(dev, "DESTROY", MAKEDEV_WAITOK);
}

static struct cdev*
newdev(struct make_dev_args* args, struct cdev* si)
{
	struct cdev* si2;
	struct cdevsw* csw;

	dev_lock_assert_locked();
	csw = args->mda_devsw;
	si2 = NULL;
	if (csw->d_flags & D_NEEDMINOR) {
		/* We may want to return an existing device */
		LIST_FOREACH(si2, &csw->d_devs, si_list) {
			if (dev2unit(si2) == args->mda_unit) {
				dev_free_devlocked(si);
				si = si2;
				break;
			}
		}

		/*
		 * If we're returning an existing device, we should make sure
		 * it isn't already initialized.  This would have been caught
		 * in consumers anyways, but it's good to catch such a case
		 * early.  We still need to complete initialization of the
		 * device, and we'll use whatever make_dev_args were passed in
		 * to do so.
		 */
		KASSERT(si2 == NULL || (si2->si_flags & SI_NAMED) == 0,
			("make_dev() by driver %s on pre-existing device (min=%x, name=%s)",
				args->mda_devsw->d_name, dev2unit(si2), devtoname(si2)));
	}
	si->si_drv0 = args->mda_unit;
	si->si_drv1 = args->mda_si_drv1;
	si->si_drv2 = args->mda_si_drv2;
	/* Only push to csw->d_devs if it's not a cloned device. */
	if (si2 == NULL) {
		si->si_devsw = csw;
		LIST_INSERT_HEAD(&csw->d_devs, si, si_list);
	}
	else {
		KASSERT(si->si_devsw == csw,
			("%s: inconsistent devsw between clone_create() and make_dev()",
				__func__));
	}
	return (si);
}

static void
fini_cdevsw(struct cdevsw* devsw)
{
	struct cdevsw* gt;

	if (devsw->d_gianttrick != NULL) {
		gt = devsw->d_gianttrick;
		memcpy(devsw, gt, sizeof * devsw);
		cdevsw_free_devlocked(gt);
		devsw->d_gianttrick = NULL;
	}
	devsw->d_flags &= ~D_INIT;
}

static int
prep_cdevsw(struct cdevsw* devsw, int flags)
{

	return (0);
}

static int
prep_devname(struct cdev* dev, const char* fmt, va_list ap)
{
	int len;
	char* from, * q, * s, * to;

	dev_lock_assert_locked();

	len = vsnrprintf(dev->si_name, sizeof(dev->si_name), 32, fmt, ap);
	if (len > sizeof(dev->si_name) - 1)
		return (ENAMETOOLONG);

	/* Strip leading slashes. */
	for (from = dev->si_name; *from == '/'; from++)
		;

	for (to = dev->si_name; *from != '\0'; from++, to++) {
		/*
		 * Spaces and double quotation marks cause
		 * problems for the devctl(4) protocol.
		 * Reject names containing those characters.
		 */
		if (isspace(*from) || *from == '"')
			return (EINVAL);
		/* Treat multiple sequential slashes as single. */
		while (from[0] == '/' && from[1] == '/')
			from++;
		/* Trailing slash is considered invalid. */
		if (from[0] == '/' && from[1] == '\0')
			return (EINVAL);
		*to = *from;
	}
	*to = '\0';

	if (dev->si_name[0] == '\0')
		return (EINVAL);

	/* Disallow "." and ".." components. */
	for (s = dev->si_name;;) {
		for (q = s; *q != '/' && *q != '\0'; q++)
			;
		if (q - s == 1 && s[0] == '.')
			return (EINVAL);
		if (q - s == 2 && s[0] == '.' && s[1] == '.')
			return (EINVAL);
		if (*q != '/')
			break;
		s = q + 1;
	}

	if (devfs_dev_exists(dev->si_name) != 0)
		return (EEXIST);

	return (0);
}

void
make_dev_args_init_impl(struct make_dev_args* args, size_t sz)
{

	bzero(args, sz);
	args->mda_size = sz;
}

static int
make_dev_sv(struct make_dev_args* args1, struct cdev** dres,
	const char* fmt, va_list ap)
{

	return (0);
}

int
make_dev_s(struct make_dev_args* args, struct cdev** dres,
	const char* fmt, ...)
{
	va_list ap;
	int res;

	va_start(ap, fmt);
	res = make_dev_sv(args, dres, fmt, ap);
	va_end(ap);
	return (res);
}

static int
make_dev_credv(int flags, struct cdev** dres, struct cdevsw* devsw, int unit,
	struct ucred* cr, uid_t uid, gid_t gid, int mode, const char* fmt,
	va_list ap)
{
	struct make_dev_args args;

	make_dev_args_init(&args);
	args.mda_flags = flags;
	args.mda_devsw = devsw;
	args.mda_cr = cr;
	args.mda_uid = uid;
	args.mda_gid = gid;
	args.mda_mode = mode;
	args.mda_unit = unit;
	return (make_dev_sv(&args, dres, fmt, ap));
}

struct cdev*
	make_dev(struct cdevsw* devsw, int unit, uid_t uid, gid_t gid, int mode,
		const char* fmt, ...)
{
	struct cdev* dev;
	va_list ap;
	int res __unused;

	va_start(ap, fmt);
	res = make_dev_credv(0, &dev, devsw, unit, NULL, uid, gid, mode, fmt,
		ap);
	va_end(ap);
	KASSERT(res == 0 && dev != NULL,
		("make_dev: failed make_dev_credv (error=%d)", res));
	return (dev);
}

struct cdev*
	make_dev_cred(struct cdevsw* devsw, int unit, struct ucred* cr, uid_t uid,
		gid_t gid, int mode, const char* fmt, ...)
{
	struct cdev* dev;
	va_list ap;
	int res __unused;

	va_start(ap, fmt);
	res = make_dev_credv(0, &dev, devsw, unit, cr, uid, gid, mode, fmt, ap);
	va_end(ap);

	KASSERT(res == 0 && dev != NULL,
		("make_dev_cred: failed make_dev_credv (error=%d)", res));
	return (dev);
}

struct cdev*
	make_dev_credf(int flags, struct cdevsw* devsw, int unit, struct ucred* cr,
		uid_t uid, gid_t gid, int mode, const char* fmt, ...)
{
	struct cdev* dev;
	va_list ap;
	int res;

	va_start(ap, fmt);
	res = make_dev_credv(flags, &dev, devsw, unit, cr, uid, gid, mode,
		fmt, ap);
	va_end(ap);

	KASSERT(((flags & MAKEDEV_NOWAIT) != 0 && res == ENOMEM) ||
		((flags & MAKEDEV_CHECKNAME) != 0 && res != ENOMEM) || res == 0,
		("make_dev_credf: failed make_dev_credv (error=%d)", res));
	return (res == 0 ? dev : NULL);
}

int
make_dev_p(int flags, struct cdev** cdev, struct cdevsw* devsw,
	struct ucred* cr, uid_t uid, gid_t gid, int mode, const char* fmt, ...)
{
	va_list ap;
	int res;

	va_start(ap, fmt);
	res = make_dev_credv(flags, cdev, devsw, 0, cr, uid, gid, mode,
		fmt, ap);
	va_end(ap);

	KASSERT(((flags & MAKEDEV_NOWAIT) != 0 && res == ENOMEM) ||
		((flags & MAKEDEV_CHECKNAME) != 0 && res != ENOMEM) || res == 0,
		("make_dev_p: failed make_dev_credv (error=%d)", res));
	return (res);
}

static void
dev_dependsl(struct cdev* pdev, struct cdev* cdev)
{

	cdev->si_parent = pdev;
	cdev->si_flags |= SI_CHILD;
	LIST_INSERT_HEAD(&pdev->si_children, cdev, si_siblings);
}

void
dev_depends(struct cdev* pdev, struct cdev* cdev)
{

	dev_lock();
	dev_dependsl(pdev, cdev);
	dev_unlock();
}

static int
make_dev_alias_v(int flags, struct cdev** cdev, struct cdev* pdev,
	const char* fmt, va_list ap)
{


	return (0);
}

struct cdev*
	make_dev_alias(struct cdev* pdev, const char* fmt, ...)
{
	struct cdev* dev;
	va_list ap;
	int res __unused;

	va_start(ap, fmt);
	res = make_dev_alias_v(MAKEDEV_WAITOK, &dev, pdev, fmt, ap);
	va_end(ap);

	KASSERT(res == 0 && dev != NULL,
		("make_dev_alias: failed make_dev_alias_v (error=%d)", res));
	return (dev);
}

int
make_dev_alias_p(int flags, struct cdev** cdev, struct cdev* pdev,
	const char* fmt, ...)
{
	va_list ap;
	int res;

	va_start(ap, fmt);
	res = make_dev_alias_v(flags, cdev, pdev, fmt, ap);
	va_end(ap);
	return (res);
}

int
make_dev_physpath_alias(int flags, struct cdev** cdev, struct cdev* pdev,
	struct cdev* old_alias, const char* physpath)
{
	char* devfspath;
	int physpath_len;
	int max_parentpath_len;
	int parentpath_len;
	int devfspathbuf_len;
	int mflags;
	int ret;

	*cdev = NULL;
	devfspath = NULL;
	physpath_len = strlen(physpath);
	ret = EINVAL;
	if (physpath_len == 0)
		goto out;

	if (strncmp("id1,", physpath, 4) == 0) {
		physpath += 4;
		physpath_len -= 4;
		if (physpath_len == 0)
			goto out;
	}

	max_parentpath_len = SPECNAMELEN - physpath_len - /*/*/1;
	parentpath_len = strlen(pdev->si_name);
	if (max_parentpath_len < parentpath_len) {
		if (bootverbose)
			printf("WARNING: Unable to alias %s "
				"to %s/%s - path too long\n",
				pdev->si_name, physpath, pdev->si_name);
		ret = ENAMETOOLONG;
		goto out;
	}

	mflags = (flags & MAKEDEV_NOWAIT) ? M_NOWAIT : M_WAITOK;
	devfspathbuf_len = physpath_len + /*/*/1 + parentpath_len + /*NUL*/1;
	devfspath = malloc(devfspathbuf_len, M_DEVBUF, mflags);
	if (devfspath == NULL) {
		ret = ENOMEM;
		goto out;
	}

	sprintf(devfspath, "%s/%s", physpath, pdev->si_name);
	if (old_alias != NULL && strcmp(old_alias->si_name, devfspath) == 0) {
		/* Retain the existing alias. */
		*cdev = old_alias;
		old_alias = NULL;
		ret = 0;
	}
	else {
		ret = make_dev_alias_p(flags, cdev, pdev, "%s", devfspath);
	}
out:
	if (old_alias != NULL)
		destroy_dev(old_alias);
	if (devfspath != NULL)
		free(devfspath, M_DEVBUF);
	return (ret);
}

static void
destroy_devl(struct cdev* dev)
{

}

static void
delist_dev_locked(struct cdev* dev)
{

}

/*
 * This function will delist a character device and its children from
 * the directory listing and create a destroy event without waiting
 * for all character device references to go away. At some later point
 * destroy_dev() must be called to complete the character device
 * destruction. After calling this function the character device name
 * can instantly be re-used.
 */
void
delist_dev(struct cdev* dev)
{

	WITNESS_WARN(WARN_GIANTOK | WARN_SLEEPOK, NULL, "delist_dev");
	dev_lock();
	delist_dev_locked(dev);
	dev_unlock();
}

void
destroy_dev(struct cdev* dev)
{

	WITNESS_WARN(WARN_GIANTOK | WARN_SLEEPOK, NULL, "destroy_dev");
	dev_lock();
	destroy_devl(dev);
	dev_unlock_and_free();
}

const char*
devtoname(struct cdev* dev)
{

	return (dev->si_name);
}

int
dev_stdclone(char* name, char** namep, const char* stem, int* unit)
{
	int u, i;

	i = strlen(stem);
	if (strncmp(stem, name, i) != 0)
		return (0);
	if (!isdigit(name[i]))
		return (0);
	u = 0;
	if (name[i] == '0' && isdigit(name[i + 1]))
		return (0);
	while (isdigit(name[i])) {
		u *= 10;
		u += name[i++] - '0';
	}
	if (u > 0xffffff)
		return (0);
	*unit = u;
	if (namep)
		*namep = &name[i];
	if (name[i])
		return (2);
	return (1);
}

/*
 * Helper functions for cloning device drivers.
 *
 * The objective here is to make it unnecessary for the device drivers to
 * use rman or similar to manage their unit number space.  Due to the way
 * we do "on-demand" devices, using rman or other "private" methods
 * will be very tricky to lock down properly once we lock down this file.
 *
 * Instead we give the drivers these routines which puts the struct cdev *'s
 * that are to be managed on their own list, and gives the driver the ability
 * to ask for the first free unit number or a given specified unit number.
 *
 * In addition these routines support paired devices (pty, nmdm and similar)
 * by respecting a number of "flag" bits in the minor number.
 *
 */

struct clonedevs {
	LIST_HEAD(, cdev)	head;
};

void
clone_setup(struct clonedevs** cdp)
{

	*cdp = malloc(sizeof * *cdp, M_DEVBUF, M_WAITOK | M_ZERO);
	LIST_INIT(&(*cdp)->head);
}

int
clone_create(struct clonedevs** cdp, struct cdevsw* csw, int* up,
	struct cdev** dp, int extra)
{
	struct clonedevs* cd;
	struct cdev* dev, * ndev, * dl, * de;
	struct make_dev_args args;
	int unit, low, u;

	KASSERT(*cdp != NULL,
		("clone_setup() not called in driver \"%s\"", csw->d_name));
	KASSERT(!(extra & CLONE_UNITMASK),
		("Illegal extra bits (0x%x) in clone_create", extra));
	KASSERT(*up <= CLONE_UNITMASK,
		("Too high unit (0x%x) in clone_create", *up));
	KASSERT(csw->d_flags & D_NEEDMINOR,
		("clone_create() on cdevsw without minor numbers"));

	/*
	 * Search the list for a lot of things in one go:
	 *   A preexisting match is returned immediately.
	 *   The lowest free unit number if we are passed -1, and the place
	 *	 in the list where we should insert that new element.
	 *   The place to insert a specified unit number, if applicable
	 *       the end of the list.
	 */
	unit = *up;
	ndev = devfs_alloc(MAKEDEV_WAITOK);
	dev_lock();
	prep_cdevsw(csw, MAKEDEV_WAITOK);
	low = extra;
	de = dl = NULL;
	cd = *cdp;
	LIST_FOREACH(dev, &cd->head, si_clone) {
		KASSERT(dev->si_flags & SI_CLONELIST,
			("Dev %p(%s) should be on clonelist", dev, dev->si_name));
		u = dev2unit(dev);
		if (u == (unit | extra)) {
			*dp = dev;
			dev_unlock();
			devfs_free(ndev);
			return (0);
		}
		if (unit == -1 && u == low) {
			low++;
			de = dev;
			continue;
		}
		else if (u < (unit | extra)) {
			de = dev;
			continue;
		}
		else if (u > (unit | extra)) {
			dl = dev;
			break;
		}
	}
	if (unit == -1)
		unit = low & CLONE_UNITMASK;
	make_dev_args_init(&args);
	args.mda_unit = unit | extra;
	args.mda_devsw = csw;
	dev = newdev(&args, ndev);
	if (dev->si_flags & SI_CLONELIST) {
		printf("dev %p (%s) is on clonelist\n", dev, dev->si_name);
		printf("unit=%d, low=%d, extra=0x%x\n", unit, low, extra);
		LIST_FOREACH(dev, &cd->head, si_clone) {
			printf("\t%p %s\n", dev, dev->si_name);
		}
		panic("foo");
	}
	KASSERT(!(dev->si_flags & SI_CLONELIST),
		("Dev %p(%s) should not be on clonelist", dev, dev->si_name));
	if (dl != NULL)
		LIST_INSERT_BEFORE(dl, dev, si_clone);
	else if (de != NULL)
		LIST_INSERT_AFTER(de, dev, si_clone);
	else
		LIST_INSERT_HEAD(&cd->head, dev, si_clone);
	dev->si_flags |= SI_CLONELIST;
	*up = unit;
	dev_unlock_and_free();
	return (1);
}

/*
 * Kill everything still on the list.  The driver should already have
 * disposed of any softc hung of the struct cdev *'s at this time.
 */
void
clone_cleanup(struct clonedevs** cdp)
{

}

static TAILQ_HEAD(, cdev_priv) dev_ddtr =
TAILQ_HEAD_INITIALIZER(dev_ddtr);
static struct task dev_dtr_task = TASK_INITIALIZER(0, destroy_dev_tq, NULL);

static void
destroy_dev_tq(void* ctx, int pending)
{


}

/*
 * devmtx shall be locked on entry. devmtx will be unlocked after
 * function return.
 */
static int
destroy_dev_sched_cbl(struct cdev* dev, void (*cb)(void*), void* arg)
{

	return (1);
}

int
destroy_dev_sched_cb(struct cdev* dev, void (*cb)(void*), void* arg)
{

	dev_lock();
	return (destroy_dev_sched_cbl(dev, cb, arg));
}

int
destroy_dev_sched(struct cdev* dev)
{

	return (destroy_dev_sched_cb(dev, NULL, NULL));
}

void
destroy_dev_drain(struct cdevsw* csw)
{

	dev_lock();
	while (!LIST_EMPTY(&csw->d_devs)) {
		msleep(&csw->d_devs, &devmtx, PRIBIO, "devscd", hz / 10);
	}
	dev_unlock();
}

void
drain_dev_clone_events(void)
{


}

#include "opt_ddb.h"
#ifdef DDB
#include <sys/kernel.h>

#include <ddb/ddb.h>

DB_SHOW_COMMAND(cdev, db_show_cdev)
{
	struct cdev_priv* cdp;
	struct cdev* dev;
	u_int flags;
	char buf[512];

	if (!have_addr) {
		TAILQ_FOREACH(cdp, &cdevp_list, cdp_list) {
			dev = &cdp->cdp_c;
			db_printf("%s %p\n", dev->si_name, dev);
			if (db_pager_quit)
				break;
		}
		return;
	}

	dev = (struct cdev*)addr;
	cdp = cdev2priv(dev);
	db_printf("dev %s ref %d use %ld thr %ld inuse %u fdpriv %p\n",
		dev->si_name, dev->si_refcount, dev->si_usecount,
		dev->si_threadcount, cdp->cdp_inuse, cdp->cdp_fdpriv.lh_first);
	db_printf("devsw %p si_drv0 %d si_drv1 %p si_drv2 %p\n",
		dev->si_devsw, dev->si_drv0, dev->si_drv1, dev->si_drv2);
	flags = dev->si_flags;
#define	SI_FLAG(flag)	do {						\
	if (flags & (flag)) {						\
		if (buf[0] != '\0')					\
			strlcat(buf, ", ", sizeof(buf));		\
		strlcat(buf, (#flag) + 3, sizeof(buf));			\
		flags &= ~(flag);					\
	}								\
} while (0)
	buf[0] = '\0';
	SI_FLAG(SI_ETERNAL);
	SI_FLAG(SI_ALIAS);
	SI_FLAG(SI_NAMED);
	SI_FLAG(SI_CHILD);
	SI_FLAG(SI_DUMPDEV);
	SI_FLAG(SI_CLONELIST);
	db_printf("si_flags %s\n", buf);

	flags = cdp->cdp_flags;
#define	CDP_FLAG(flag)	do {						\
	if (flags & (flag)) {						\
		if (buf[0] != '\0')					\
			strlcat(buf, ", ", sizeof(buf));		\
		strlcat(buf, (#flag) + 4, sizeof(buf));			\
		flags &= ~(flag);					\
	}								\
} while (0)
	buf[0] = '\0';
	CDP_FLAG(CDP_ACTIVE);
	CDP_FLAG(CDP_SCHED_DTR);
	db_printf("cdp_flags %s\n", buf);
}
#endif
