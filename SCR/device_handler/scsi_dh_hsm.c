/*
 * Generic SCSI-3 HSM SCSI Device Handler
 *
 * Copyright (C) 2015-2020 Raghu Bilugu, Parsc Labs LLC.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <asm/unaligned.h>
#include <linux/kernel.h>
#include <scsi/scsi.h>
#include <scsi/scsi_eh.h>
#include <scsi/scsi_dh.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,32)
    /* Specifically for RedHat 2.6.32-642.el6.x86_64 */
    #define HSM_HANDLER_DATA(sdev)      ((struct hsm_dh_data *)((sdev->scsi_dh_data) ? &(sdev->scsi_dh_data->buf[0]) : NULL))
#else
    /* Specifically for Ubuntu 4.4.0-28 */
    #define HSM_HANDLER_DATA(sdev)      (sdev->handler_data)
#endif


#define HSM_DH_NAME "hsm"
#define HSM_DH_VER "0.3"

#define TPGS_STATE_OPTIMIZED		0x0
#define TPGS_STATE_NONOPTIMIZED		0x1
#define TPGS_STATE_STANDBY		0x2
#define TPGS_STATE_UNAVAILABLE		0x3
#define TPGS_STATE_LBA_DEPENDENT	0x4
#define TPGS_STATE_OFFLINE		0xe
#define TPGS_STATE_TRANSITIONING	0xf

#define TPGS_SUPPORT_NONE		0x00
#define TPGS_SUPPORT_OPTIMIZED		0x01
#define TPGS_SUPPORT_NONOPTIMIZED	0x02
#define TPGS_SUPPORT_STANDBY		0x04
#define TPGS_SUPPORT_UNAVAILABLE	0x08
#define TPGS_SUPPORT_LBA_DEPENDENT	0x10
#define TPGS_SUPPORT_OFFLINE		0x40
#define TPGS_SUPPORT_TRANSITION		0x80

#define RTPG_FMT_MASK			0x70
#define RTPG_FMT_EXT_HDR		0x10

#define TPGS_MODE_UNINITIALIZED		 -1
#define TPGS_MODE_NONE			0x0
#define TPGS_MODE_IMPLICIT		0x1
#define TPGS_MODE_EXPLICIT		0x2

#define HSM_RESP_BUFFERSIZE		128	// 1024
#define HSM_FAILOVER_TIMEOUT		60
#define HSM_FAILOVER_RETRIES		5
#define HSM_FAILOVER_RETRIES		5
#define HSM_RETRY_DELAY_MSECS		120
#define HSM_POLLING_DELAY_MSECS		1000

/* hsm_group level flags */
#define HSM_GRP_PASSIVEMODE		0x01
#define HSM_GRP_DIRECTIO		0x02

/* hsm_dh_data flags		*/
#define HSM_DH_PCN  			0x01
#define HSM_DH_SRC  			0x02
#define HSM_DH_TGT  			0x04
#define HSM_DH_BLACKLISTED              0x08
#define HSM_DH_FORCEUPDATE              0x10
#define HSM_DH_MONITORINPROGRESS        0x20
#define HSM_DH_DETACHINPROGRESS	        0x40

/*
** HSM SM states
*/
#define HSM_S_IDLE		0
#define HSM_S_READY_TO_START	1
#define HSM_S_INLINE		2
#define HSM_S_PREP_TO_SWITCH	3
#define HSM_S_WAIT_FOR_SWITCH	4
#define HSM_S_SWITCHED		5
#define HSM_S_MAX		5

/*
** HSM SM events
*/
#define HSM_E_INIT		0
#define HSM_E_START		1
#define HSM_E_GOLIVE		2
#define HSM_E_READY_TO_SWITCH	3
#define HSM_E_SWITCH		4
#define HSM_E_CANCEL		5
#define HSM_E_MAX		5

struct hsm_group {
	struct kref		kref;
	struct rcu_head		rcu;
	struct list_head	node;
	struct list_head	members;
	unsigned char		scsiid[64];
	int			scsiid_len;
	unsigned char		s_uuid[64];
	int			s_len;
	unsigned char		t_uuid[64];
	int			t_len;
	int			hsm_state;
	unsigned		flags;
	spinlock_t		lock;
};

struct hsm_dh_data {
	struct delayed_work	work;
	struct scsi_device	*sdev;
	struct hsm_group	*grp;
	spinlock_t		grp_lock;
	unsigned char		*buff;
	int			bufflen;
	unsigned		flags; 
	int			tpgs;
	int			path_state;
	int			tpgs_state;
	struct list_head	entry;
	int			init_error;
	struct mutex		attach_mutex;
	struct mutex		detach_mutex;
	unsigned char		resp[HSM_RESP_BUFFERSIZE];
	unsigned char		sense[SCSI_SENSE_BUFFERSIZE];
	int			senselen;
	unsigned char		uuid[64];
	int			uuid_len;
};

static uint passivemode;
module_param(passivemode, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(passivemode, "Host in passive mode does not trigger hms state transitions (0=No, 1=Yes). Default is 0.");

static struct workqueue_struct *hsm_wq;
static LIST_HEAD(hsm_group_list);
static DEFINE_SPINLOCK(hsm_group_lock);


static void dump_group(struct hsm_group *grp)
{
	struct hsm_dh_data *th = NULL;

	if (grp) {
		printk(KERN_INFO "[hsm:%s] grp=(%p): flags=0x%x\thsm_state=%d\n",
					__func__, grp,
					grp->flags, grp->hsm_state);
		printk(KERN_INFO "\tscsiid(len=%u)=%s\n\ts_uuid(len=%u)=%s\n\tt_uuid(len=%u)=%s\n",
					grp->scsiid_len, grp->scsiid,
					grp->s_len, grp->s_uuid,
					grp->t_len, grp->t_uuid);
		list_for_each_entry(th, &grp->members, entry)
			sdev_printk(KERN_INFO, th->sdev,
					"[path(0x%p) : %.8s]:flg=0x%x ts=%u uuid(len=%u)=%s\n",
					th, th->sdev->vendor,
					th->flags, th->tpgs_state,
					th->uuid_len, th->uuid);
	}
}

static void release_hsm_group(struct kref *kref)
{
	struct hsm_group *grp;

	printk(KERN_NOTICE "[hsm:%s] Enter\n", __func__);
	grp = container_of(kref, struct hsm_group, kref);
	dump_group(grp);
	spin_lock(&hsm_group_lock);
	list_del(&grp->node);
	spin_unlock(&hsm_group_lock);
	kfree_rcu(grp, rcu);
	printk(KERN_NOTICE "[hsm:%s] Exit\n", __func__);
}

static int realloc_buffer(struct hsm_dh_data *h, unsigned len)
{
	// sdev_printk(KERN_INFO, h->sdev, "[hsm:%s] Enter\n", __func__);
	if (h->buff && h->buff != h->resp)
		kfree(h->buff);

	h->buff = kmalloc(len, GFP_NOIO);
	if (!h->buff) {
		sdev_printk(KERN_WARNING, h->sdev, "%s kmalloc failed\n", __func__);
		h->buff = h->resp;
		h->bufflen = HSM_RESP_BUFFERSIZE;
		return 1;
	}
	h->bufflen = len;
	// sdev_printk(KERN_INFO, h->sdev, "[hsm:%s] Exit\n", __func__);
	return 0;
}

static struct request *get_hsm_req(struct scsi_device *sdev,
				    void *buffer, unsigned buflen, int rw)
{
	struct request *rq;
	struct request_queue *q = sdev->request_queue;

	// sdev_printk(KERN_DEBUG, sdev, "[hsm:%s] Enter\n", __func__);
	rq = blk_get_request(q, rw, GFP_NOIO);

	if (IS_ERR(rq)) {
		sdev_printk(KERN_WARNING, sdev, "[hsm:%s] blk_get_request failed\n", __func__);
		rq = NULL;
		goto out;
	}
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,32)
        /* Specifically for RedHat 2.6.32-642.el6.x86_64 */
        rq->cmd_type = REQ_TYPE_BLOCK_PC;
#else
        /* Specifically for Ubuntu 4.4.0-28 */
        blk_rq_set_block_pc(rq);
#endif


	if (buflen && blk_rq_map_kern(q, rq, buffer, buflen, GFP_NOIO)) {
		sdev_printk(KERN_WARNING, sdev, "[hsm:%s] blk_rq_map_kern failed\n", __func__);
		blk_put_request(rq);
		rq = NULL;
		goto out;
	}

	rq->cmd_flags |= REQ_FAILFAST_DEV | REQ_FAILFAST_TRANSPORT |
			 REQ_FAILFAST_DRIVER;
	rq->retries = HSM_FAILOVER_RETRIES;
	rq->timeout = HSM_FAILOVER_TIMEOUT * HZ;

out:
	// sdev_printk(KERN_DEBUG, sdev, "[hsm:%s] Exit\n", __func__);
	return rq;
}

/*
 * submit_vpd - Issue an INQUIRY VPD page 0x83 command
 * @sdev: sdev the command should be sent to
 */
static int submit_vpd(struct hsm_dh_data *h)
{
	struct request *rq;
	struct scsi_device *sdev = h->sdev;
	int err = SCSI_DH_RES_TEMP_UNAVAIL;

	// sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Enter\n", __func__);
	rq = get_hsm_req(sdev, h->buff, h->bufflen, READ);
	if (!rq) {
		sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] get_hsm_req returned NULL\n", __func__);
		err = SCSI_DH_IO;
		goto done;
	}

	/* Prepare the command. */
	rq->cmd[0] = INQUIRY;
	rq->cmd[1] = 1;
	rq->cmd[2] = 0x83;
	rq->cmd[3] = (h->bufflen >>  8) & 0xff;
	rq->cmd[4] = h->bufflen & 0xff;
	rq->cmd_len = COMMAND_SIZE(INQUIRY);

	rq->sense = h->sense;
	memset(rq->sense, 0, SCSI_SENSE_BUFFERSIZE);
	rq->sense_len = h->senselen = 0;

	err = blk_execute_rq(rq->q, NULL, rq, 1);
	if (err == -EIO) {
		sdev_printk(KERN_WARNING, h->sdev, "[hsm:%s] evpd inquiry failed with %x\n",
			    __func__, rq->errors);
		h->senselen = rq->sense_len;
		err = SCSI_DH_IO;
	}
	blk_put_request(rq);
done:
	// sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Exit\n", __func__);
	return err;
}

/*
 * submit_min - Issue a SCSI MAINTENANCE IN command
 */
static unsigned submit_min(struct hsm_dh_data *h)
{
	struct request *rq;
	int err = SCSI_DH_RES_TEMP_UNAVAIL;
	struct scsi_device *sdev = h->sdev;

	// sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Enter\n", __func__);
	memset(h->buff, 0, h->bufflen);
	rq = get_hsm_req(sdev, h->buff, h->bufflen, READ);
	if (!rq) {
		sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] get_hsm_req returned NULL\n", __func__);
		err = SCSI_DH_IO;
		goto done;
	}

	/* Prepare the command. */
	rq->cmd[0] = MAINTENANCE_IN;
	rq->cmd[1] = 0x1f;
	rq->cmd[2] = 0x01;
	put_unaligned_be32(h->bufflen, &rq->cmd[6]);
	rq->cmd_len = COMMAND_SIZE(MAINTENANCE_IN);

	rq->sense = h->sense;
	memset(rq->sense, 0, SCSI_SENSE_BUFFERSIZE);
	rq->sense_len = h->senselen = 0;

	err = blk_execute_rq(rq->q, NULL, rq, 1);
	if (err == -EIO) {
		sdev_printk(KERN_WARNING, h->sdev, "[hsm:%s] submit_min failed with %x\n",
			    __func__, rq->errors);
		h->senselen = rq->sense_len;
		err = SCSI_DH_IO;
	}
	blk_put_request(rq);
done:
	// sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Exit\n", __func__);
	return err;
}

/*
 * submit_mout - Issue a SCSI MAINTENANCE OUT command
 */
static unsigned submit_mout(struct hsm_dh_data *h, int ev)
{
	int mout_len = 4;
	struct request *rq;
	struct hsm_group *grp = NULL;
	int err = SCSI_DH_RES_TEMP_UNAVAIL;
	struct scsi_device *sdev = h->sdev;

	// sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Enter\n", __func__);
	grp = rcu_dereference(h->grp);
	if (grp->flags & HSM_GRP_PASSIVEMODE)
		return SCSI_DH_OK;

	/* Prepare the data buffer */
	memset(h->buff, 0, mout_len);
	put_unaligned_be32(ev, h->buff);

	rq = get_hsm_req(sdev, h->buff, mout_len, WRITE);
	if (!rq) {
		sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] get_hsm_req returned NULL\n", __func__);
		return SCSI_DH_IO;
	}

	/* Prepare the command. */
	rq->cmd[0] = MAINTENANCE_OUT;
	rq->cmd[1] = 0x1f;
	rq->cmd[2] = 0x01;
	put_unaligned_be32(mout_len, &rq->cmd[6]);
	rq->cmd_len = COMMAND_SIZE(MAINTENANCE_OUT);

	rq->sense = h->sense;
	memset(rq->sense, 0, SCSI_SENSE_BUFFERSIZE);
	rq->sense_len = h->senselen = 0;

	err = blk_execute_rq(rq->q, NULL, rq, 1);
	blk_put_request(rq);
	if (err == -EIO) {
		sdev_printk(KERN_WARNING, h->sdev, "[hsm:%s] submit_mout failed with %x\n",
			    __func__, rq->errors);
		h->senselen = rq->sense_len;
		return SCSI_DH_IO;
	}
	// sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Exit\n", __func__);
	return SCSI_DH_OK;
}

/*
 * Functions for sysfs attribute 'scsi_id'
 */

static ssize_t
show_scsi_id(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t rval = 0;
	struct scsi_device *sdev = to_scsi_device(dev);
	struct hsm_dh_data *h = HSM_HANDLER_DATA(sdev);
	struct hsm_group __rcu *grp = NULL;

	sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Enter\n", __func__);
	rcu_read_lock();
	grp = rcu_dereference(h->grp);
	if (grp && grp->scsiid_len)
		rval = sprintf(buf, "%s", grp->scsiid);
	rcu_read_unlock();
	sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Exit\n", __func__);
	return (rval);
}

/*
 * Functions for sysfs attribute 'scsi_id'
 */
static ssize_t
store_scsi_id(struct device *dev, struct device_attribute *attr,
	       const char *buf, size_t count)
{
	struct scsi_device *sdev = to_scsi_device(dev);
	struct hsm_dh_data *h = HSM_HANDLER_DATA(sdev);
	struct hsm_group __rcu *grp;
	int scsiid_len = count;
	unsigned char scsiid[64];

	sprintf(scsiid, "%s", buf);

	if (count == 33) {
		scsiid_len = count - 1;
		scsiid[count] = '\0';
	}


	sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Enter - scsiid=%s\n", __func__, scsiid);

	rcu_read_lock();
	grp = rcu_dereference(h->grp);
	rcu_read_unlock();
	if (grp) {
		sdev_printk(KERN_INFO, h->sdev, "[hsm:%s] assigned group(%p)\n", __func__, grp);
                if (!(strncmp(grp->scsiid, scsiid, scsiid_len))) {
			sdev_printk(KERN_WARNING, h->sdev, "[hsm:%s] grp with different scsiid (%s : new=%s) exists! \n",
                                                        __func__, grp->scsiid, scsiid);
                        spin_lock(&grp->lock);
                        strncpy(grp->scsiid, scsiid, scsiid_len);
                        grp->scsiid_len = count;
                        spin_unlock(&grp->lock);
                }
	}
	else {
		list_for_each_entry(grp, &hsm_group_list, node) {
			if (!(strncmp(grp->scsiid, scsiid, scsiid_len))) {
				sdev_printk(KERN_INFO, h->sdev, "[hsm:%s] found group(%p)\n", __func__, grp);
				spin_lock(&grp->lock);
				kref_get(&grp->kref);
				list_add(&h->entry, &grp->members);
				spin_unlock(&grp->lock);
				rcu_assign_pointer(h->grp, grp);
				break;
			}
		}
		if (!(h->grp)) {
			grp = kzalloc(sizeof(struct hsm_group), GFP_KERNEL);
			if (!grp) {
				sdev_printk(KERN_WARNING, h->sdev, "[hsm:%s] alloc grp failed! \n", __func__);
				goto out;
			}
			sdev_printk(KERN_INFO, h->sdev, "[hsm:%s] new group(%p)\n", __func__, grp);
			strncpy(grp->scsiid, scsiid, scsiid_len);
			grp->scsiid_len = scsiid_len;
			kref_init(&grp->kref);
			INIT_LIST_HEAD(&grp->node);
			INIT_LIST_HEAD(&grp->members);
			spin_lock_init(&grp->lock);

			/*
			** Add hsm_dh_data handle to group
			*/
			spin_lock(&grp->lock);
			list_add(&h->entry, &grp->members);
			spin_unlock(&grp->lock);

			/*
			** link group to hsm_dh_data handle
			*/
			rcu_assign_pointer(h->grp, grp);

			spin_lock(&hsm_group_lock);
			list_add(&grp->node, &hsm_group_list);
			spin_unlock(&hsm_group_lock);
		}
		
	}
	sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Exit grp(%p) scsiid(len=%u)=%s\n",
				__func__, grp, grp->scsiid_len, grp->scsiid);
out:
	return count;
}
DEVICE_ATTR(scsi_id, S_IRUGO | S_IWUSR, show_scsi_id, store_scsi_id);

static ssize_t
show_helper(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct hsm_group __rcu *grp = NULL;
	struct scsi_device *sdev = to_scsi_device(dev);
	struct hsm_dh_data *h = HSM_HANDLER_DATA(sdev);

	sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Enter (Debug)\n", __func__);
	sdev_printk(KERN_INFO, h->sdev, "[hsm:%s] Enter (Info)\n", __func__);
	sdev_printk(KERN_NOTICE, h->sdev, "[hsm:%s] Enter (Notice)\n", __func__);
	rcu_read_lock();
	grp = rcu_dereference(h->grp);
	if (grp)
		dump_group(grp);
	rcu_read_unlock();

	sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Exit\n", __func__);
	return (0);
}

DEVICE_ATTR(helper, S_IRUGO, show_helper, NULL);

static const char *tpgs_string[] = {
	[TPGS_STATE_OPTIMIZED]         = "active/optimized",
	[TPGS_STATE_NONOPTIMIZED]      = "active/non-optimized",
	[TPGS_STATE_STANDBY]           = "standby",
	[TPGS_STATE_UNAVAILABLE]       = "unavailable",
};

static ssize_t
show_access_state(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t rval = 0;
	struct scsi_device *sdev = to_scsi_device(dev);
	struct hsm_dh_data *h = HSM_HANDLER_DATA(sdev);

	if (h->tpgs_state <= TPGS_STATE_UNAVAILABLE)
		rval = sprintf(buf, "%s", tpgs_string[h->tpgs_state & 0x03]);
	else
		rval = sprintf(buf, "offline");
	// sdev_printk(KERN_DEBUG, sdev, "[hsm:%s] access_state = [%u, %s]\n", __func__, h->tpgs_state, buf);
	return (rval);
}
DEVICE_ATTR(access_state, S_IRUGO, show_access_state, NULL);

#define HSM_UUID_SRC	1
#define HSM_UUID_TGT	2

static int extract_deviceid(struct hsm_dh_data *h)
{
	int len, err = SCSI_DH_OK;
	unsigned char *p = NULL;
	struct scsi_device *sdev = h->sdev;

	sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Enter\n", __func__);
	h->tpgs = scsi_device_tpgs(sdev);
retry_vpd:
	err = submit_vpd(h);
	if (err != SCSI_DH_OK)
		goto out;

	/* Check if vpd page exceeds initial buffer */
	len = (h->buff[2] << 8) + h->buff[3] + 4;
	if (len > h->bufflen) {
		/* Resubmit with the correct length */
		if (realloc_buffer(h, len)) {
			sdev_printk(KERN_WARNING, h->sdev, "[hsm:%s] kmalloc buffer failed\n", __func__);
			/* Temporary failure, bypass */
			err = SCSI_DH_DEV_TEMP_BUSY;
			goto out;
		}
		goto retry_vpd;
	}
	p = h->buff + 4;

	while (p < h->buff + len) {
		switch (p[1] & 0x3f) {
		case 0x2:
		case 0x3:
			sprintf(h->uuid, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%c",
					p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],
					p[12],p[13],p[14],p[15], p[16],p[17],p[18],p[19],'\0');
			h->uuid_len = strlen(h->uuid);
			goto out;
		default:
			break;
		}
		p += p[3] + 4;
	}
out:
	sdev_printk(KERN_INFO, h->sdev, "[hsm:%s] Exit - uuid(len=%u)=%s\n", __func__, h->uuid_len, h->uuid);
	return err;
} 

static inline int extract_uuid(struct hsm_dh_data *h, unsigned char type, unsigned char *buf)
{
	int rval = 0;
	unsigned char *p = h->buff + 4;
	int len = (h->buff[2] << 8) + h->buff[3] + 4;

	sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Enter - hdr: %02x %02x %02x %02x, len:%u\n",
			__func__,h->buff[0], h->buff[1], h->buff[2], h->buff[3], len);
	while (p < h->buff + len) {
		sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] descriptor: %02x %02x %02x %02x\n",
				__func__, p[0], p[1], p[2], p[3]);
		if ((p[0] & 0x3f) == type) {
			//rval = p[3] & 0xff;
			sprintf(buf, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%c",
					p[4],p[5],p[6],p[7], p[8],p[9],p[10],p[11],
					p[12],p[13],p[14],p[15], p[16],p[17],p[18],p[19],'\0');
			rval = strlen(buf);
			goto out;
		}
		p += p[3] + 4;
	}
out:
	sdev_printk(KERN_INFO, h->sdev, "[hsm:%s] Exit - uuid(type=%u, len=%u)=%s\n", __func__, type, rval, buf);
	return rval;
} 

static inline void set_states(struct hsm_dh_data *h, int tstate)
{
	int ps, ots, nts;

	ots = h->tpgs_state;
	ps = h->path_state;
	if (h->flags & HSM_DH_BLACKLISTED)
		nts = TPGS_STATE_OFFLINE;
	else if ((ps != TPGS_STATE_OPTIMIZED) &&
		(ps != TPGS_STATE_NONOPTIMIZED))
		nts = ps;
	else
		nts = tstate;
	spin_lock(&h->grp_lock);
	h->tpgs_state = nts;
	spin_unlock(&h->grp_lock);
	if (ots != nts)
		sdev_printk(KERN_INFO, h->sdev,
			"[hsm:%s] tstate=%u - ps=%u ts(%u => %u)\n",
			__func__, tstate,  ps, ots, nts);
}

static void entry_add (struct hsm_dh_data *h, struct hsm_group *grp)
{
	int rval;
	char *argv[5];
	struct hsm_group *tg = NULL, *tmp_tg = NULL;
	struct hsm_dh_data *th = NULL, *tmp_th = NULL;
	static char *hsmadmpath = "/sbin/hsmadm";
	static char *envp[] = {"HOME=/", "TERM=linux", "PATH=/usr/bin:/bin", NULL};
	unsigned char s_uuid[64], t_uuid[64];

	// sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Enter\n", __func__);

	spin_lock(&grp->lock);
	grp->s_len = extract_uuid(h, HSM_UUID_SRC, grp->s_uuid);
	grp->t_len = extract_uuid(h, HSM_UUID_TGT, grp->t_uuid);
	spin_unlock(&grp->lock);

	sprintf(t_uuid, "-u %s", grp->t_uuid);
	sprintf(s_uuid, "-s %s", grp->scsiid);
	sdev_printk(KERN_INFO, h->sdev, "[hsm:%s] %s %s\n", __func__, s_uuid, t_uuid);
	/*
	** add entry in persist store - TBD
	*/
	argv[0] = hsmadmpath;
	argv[1] = "add";
	argv[2] = t_uuid;
	argv[3] = s_uuid;
	argv[4] = NULL;
	rval = call_usermodehelper(hsmadmpath, argv, envp, UMH_WAIT_PROC);

	spin_lock(&hsm_group_lock);
	list_for_each_entry_safe(tg, tmp_tg, &hsm_group_list, node) {
		if (tg == grp)
			continue;
		list_for_each_entry_safe(th, tmp_th, &tg->members, entry) {
			if (strncmp(&t_uuid[3], th->uuid, th->uuid_len))
				continue;
			spin_lock(&tg->lock);
			list_del(&th->entry);
			spin_unlock(&tg->lock);

			spin_unlock(&hsm_group_lock);
			kref_put(&tg->kref, release_hsm_group);
			spin_lock(&hsm_group_lock);

			spin_lock(&grp->lock);
			kref_get(&grp->kref);
			list_add(&th->entry, &grp->members);
			rcu_assign_pointer(th->grp, grp);
			spin_unlock(&grp->lock);
		}
	}
	spin_unlock(&hsm_group_lock);
	spin_lock(&grp->lock);
	list_for_each_entry(th, &grp->members, entry) {
		if (th->flags & HSM_DH_PCN)
			continue;
		else if (!(strncmp(&t_uuid[3], th->uuid, th->uuid_len)))
			th->flags |= HSM_DH_TGT;
		else if (!(strncmp(&s_uuid[3], th->uuid, th->uuid_len)))
			th->flags |= HSM_DH_SRC;
	}
	spin_unlock(&grp->lock);
	// sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Exit\n", __func__);
}

static void entry_blacklist (struct hsm_dh_data *h)
{
	int rval;
	char *argv[5];
	static char *hsmadmpath = "/sbin/hsmadm";
	static char *envp[] = {"HOME=/", "TERM=linux", "PATH=/usr/bin:/bin", NULL};
	unsigned char uuid[64];

	sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Enter\n", __func__);

	sprintf(uuid, "-u %s", h->uuid);
	/*
	** blacklist the entry
	*/
	argv[0] = hsmadmpath;
	argv[1] = "blacklist";
	argv[2] = uuid;
	argv[3] = NULL;
	argv[4] = NULL;
	rval = call_usermodehelper(hsmadmpath, argv, envp, UMH_WAIT_PROC);
	spin_lock(&h->grp_lock);
	h->flags &= HSM_DH_BLACKLISTED;
	spin_unlock(&h->grp_lock);
	sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Exit\n", __func__);
}

static int update_portstate(struct hsm_dh_data *h, struct hsm_group *grp)
{
	int retval = SCSI_DH_OK;
	struct request *rq;
	int ps, ops, ohs, ots;
	struct hsm_dh_data *th = NULL;
	struct scsi_sense_hdr sense_hdr;
	struct scsi_device *sdev = h->sdev;

	// sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Enter\n", __func__);
	ps = ops = h->path_state;
	ots = h->tpgs_state;
	ohs = grp->hsm_state;

	rq = get_hsm_req(sdev, NULL, 0, READ);
	if (rq) {
		rq->cmd[0] = TEST_UNIT_READY;
		rq->sense = &sense_hdr;
		memset(rq->sense, 0, sizeof(sense_hdr));
		rq->sense_len = 0;
		retval = blk_execute_rq(rq->q, NULL, rq, 1);
		blk_put_request(rq);
		if (h->flags & HSM_DH_DETACHINPROGRESS) {
			sdev_printk(KERN_WARNING, h->sdev,
				"[hsm:%s] path lost - detected with TUR\n", __func__);
			retval = SCSI_DH_IO;
			goto _out;
		}
                if ((retval ==  SCSI_DH_OK)
                        && (ps != TPGS_STATE_OPTIMIZED)) {
                        spin_lock(&h->grp_lock);
                        h->path_state = TPGS_STATE_OPTIMIZED;
                        spin_unlock(&h->grp_lock);
                }
                ps = h->path_state;
        }
	else {
		sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] get_hsm_req returned NULL\n", __func__);
		ps = TPGS_STATE_OFFLINE;
		if (h->flags & HSM_DH_DETACHINPROGRESS) {
			sdev_printk(KERN_WARNING, h->sdev,
				"[hsm:%s] path disconnected\n", __func__);
			retval = SCSI_DH_IO;
			goto _out;
		}
	}

	if ((h->flags & HSM_DH_FORCEUPDATE) ||  (ops != ps)) {
		unsigned long flags;

		h->path_state = ps;
		set_states(h, ots);
		/*
		** If no active PCN paths - failback to source path
		*/
		if ((ohs == HSM_S_INLINE) ||
			(ohs == HSM_S_PREP_TO_SWITCH)) {
			list_for_each_entry(th, &grp->members, entry) {
				if (th->flags & HSM_DH_DETACHINPROGRESS)
					continue;
				if ((th->flags & HSM_DH_PCN) &&
					(th->tpgs_state == TPGS_STATE_OPTIMIZED)) {
					goto _out;
				}
			}
			sdev_printk(KERN_NOTICE, h->sdev,
				"[hsm:%s] all inline paths down!!! reverting to src paths\n",
				__func__);
			spin_lock_irqsave(&grp->lock, flags);
			grp->hsm_state = HSM_S_IDLE;
			spin_unlock_irqrestore(&grp->lock, flags);
			list_for_each_entry(th, &grp->members, entry) {
				if (th->flags & HSM_DH_DETACHINPROGRESS)
					continue;
				if (th->flags & HSM_DH_SRC)
					set_states(h, TPGS_STATE_OPTIMIZED);
			}
		}
	}
_out:
	if (ops != ps)
		sdev_printk(KERN_INFO, h->sdev,
			"[hsm:%s] flags (h=0x%x g=0x%x) hs=%d ots=%d ps(%d->%d)\n",
			__func__, h->flags, grp->flags, ohs, ots, ops, ps);
	// sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Exit\n", __func__);
	return retval;
}

static bool update_hsmstate(struct hsm_dh_data *h, struct hsm_group *grp)
{
	int err, hs=0xff, nhs, ohs;
	struct hsm_dh_data *th = NULL;

	// sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Enter\n", __func__);
	if (h->flags & HSM_DH_DETACHINPROGRESS) {
		sdev_printk(KERN_WARNING, h->sdev,
			"[hsm:%s] path lost\n", __func__);
		return SCSI_DH_IO;
	}

	nhs = ohs = grp->hsm_state;

	if (!(h->flags & HSM_DH_PCN) ||
		(h->path_state != TPGS_STATE_OPTIMIZED))
		goto _out;

	err = submit_min(h);
	if (err !=  SCSI_DH_OK)  {
		sdev_printk(KERN_WARNING, h->sdev, "[hsm:%s] submit_min failed (%u)\n", __func__, err);
		goto _out;
	}
	hs = (h->buff[0] & 0xff);
	if ((ohs == hs) && !(h->flags & HSM_DH_FORCEUPDATE))
		goto _out;
	switch (ohs) {
		case HSM_S_IDLE :
			switch (hs) {
				case HSM_S_READY_TO_START:
					entry_add(h, grp);
					nhs = hs;
					break;
				case HSM_S_INLINE:
				case HSM_S_PREP_TO_SWITCH:
					list_for_each_entry(th, &grp->members, entry) {
						if (th->flags & HSM_DH_DETACHINPROGRESS)
							continue;
						if (!(th->flags & HSM_DH_PCN)) {
							/*
							** Jumping directly from IDLE to INLINE
							** with direct paths active. Cancel
							** the migration
							*/
							sdev_printk(KERN_NOTICE, h->sdev,
									"[hsm:%s] cancel migration (%u, %u)\n",
									__func__, ohs, hs);
							submit_mout(h, HSM_E_CANCEL);
							goto _out;
						}
					}
					sdev_printk(KERN_DEBUG, h->sdev,
								"[hsm:%s] correcting hsm state (%u, %u) to INLINE\n",
								__func__, ohs, hs);
					entry_add(h, grp);
					nhs = HSM_S_INLINE;
					break;
				case HSM_S_WAIT_FOR_SWITCH:
				case HSM_S_SWITCHED:
					list_for_each_entry(th, &grp->members, entry) {
						if (th->flags & HSM_DH_DETACHINPROGRESS)
							continue;
						if (!(th->flags & HSM_DH_PCN)) {
							/*
							** Jumping directly from IDLE to SWITCHED
							** with direct paths active.
							** Should never come here - this is BAD!!!
							** TBD
							*/
							sdev_printk(KERN_WARNING, h->sdev,
								"[hsm:%s] Data integrity compromised!!! (%u, %u)\n",
								__func__, ohs, hs);
						}
					}
					entry_add(h, grp);
					nhs = hs;
					break;
				default:
					nhs = hs;
					break;
			}
		break;
		case HSM_S_INLINE :
		case HSM_S_PREP_TO_SWITCH:
			switch (hs) {
				case HSM_S_PREP_TO_SWITCH:
				case HSM_S_WAIT_FOR_SWITCH:
					list_for_each_entry(th, &grp->members, entry) {
						if (th->flags & HSM_DH_DETACHINPROGRESS)
							continue;
						if (th->flags & HSM_DH_TGT) {
							nhs = hs;
							break;
						}
					}
				default:
					nhs = hs;
					break;
			}
		break;
		case HSM_S_WAIT_FOR_SWITCH:
			if (hs == HSM_S_SWITCHED)
				entry_blacklist(h);
			// fall thru....
		default:
			nhs = hs;
			break;
	}
_out:
	if (!(h->flags & HSM_DH_DETACHINPROGRESS) && (ohs != nhs)) {
		unsigned long flags;

		spin_lock_irqsave(&grp->lock, flags);
		grp->hsm_state = nhs;
		spin_unlock_irqrestore(&grp->lock, flags);
		sdev_printk(KERN_INFO, h->sdev,
			"[hsm:%s] hs=%u - ohs=%u nhs=%u\n",
			__func__, hs, ohs, nhs);
	}
	// sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Exit\n", __func__);
	return ((ohs != hs) || (h->flags & HSM_DH_FORCEUPDATE));
}

static void update_tpgsstate(struct hsm_group *grp)
{
	int hs, ts, ots;
	struct hsm_dh_data *th = NULL;

	// sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Enter\n", __func__);
	hs = grp->hsm_state;

	list_for_each_entry(th, &grp->members, entry) {
		if (th->flags & HSM_DH_DETACHINPROGRESS)
			continue;
		ts = TPGS_STATE_OFFLINE;
		ots = th->tpgs_state;
		switch(hs) {
			case HSM_S_IDLE:
				if (!(th->flags & HSM_DH_TGT))
					ts = TPGS_STATE_OPTIMIZED;
			break;
			case HSM_S_READY_TO_START:
				if (th->flags & HSM_DH_PCN) {
					ts = TPGS_STATE_OPTIMIZED;
					/* Send start for each PCN path */
					submit_mout(th, HSM_E_START);
				}
			break;
			case HSM_S_PREP_TO_SWITCH:
				if (th->flags & HSM_DH_PCN) {
					ts = TPGS_STATE_OPTIMIZED;
					submit_mout(th, HSM_E_READY_TO_SWITCH);
				}
			break;
			case HSM_S_WAIT_FOR_SWITCH:
				if (th->flags & HSM_DH_PCN) {
					ts = TPGS_STATE_OPTIMIZED;
					submit_mout(th, HSM_E_SWITCH);
				}
			break;
			case HSM_S_SWITCHED:
				if (th->flags & HSM_DH_TGT)
					ts = TPGS_STATE_OPTIMIZED;
			break;
			default:
				if (th->flags & HSM_DH_PCN)
					ts = TPGS_STATE_OPTIMIZED;
			break;
		}
		set_states(th, ts);
		ts = th->tpgs_state;
		if (ots != ts)
			sdev_printk(KERN_INFO, th->sdev,
				"[hsm:%s] hs=%d ts(%d->%d)\n",
				__func__, hs, ots, ts);
	}
	// sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Exit\n", __func__);
	return;
}

static void proc_statusmonitor(struct work_struct *work)
{
	int interval = 10;
	int rval = SCSI_DH_OK;
	struct hsm_dh_data *h = NULL;
	struct hsm_group __rcu *grp = NULL;

	h = container_of(work, struct hsm_dh_data, work.work);
	// sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Enter\n", __func__);
	
	if (h->flags & HSM_DH_DETACHINPROGRESS)
		goto out;
        spin_lock(&h->grp_lock);
        h->flags |= HSM_DH_MONITORINPROGRESS;
        spin_unlock(&h->grp_lock);

	rcu_read_lock();
	grp = rcu_dereference(h->grp);
	rcu_read_unlock();
	if (!grp) {
		sdev_printk(KERN_WARNING, h->sdev, "[hsm:%s] grp is null!!!\n", __func__);
		goto out;
	}
	rval = update_portstate(h,grp);
	if (h->flags & HSM_DH_DETACHINPROGRESS)
		goto out;
	switch (rval) {
		case SCSI_DH_OK:
			if (update_hsmstate(h,grp))
				update_tpgsstate(grp);
		break;
		case SCSI_DH_RETRY:
			sdev_printk(KERN_INFO, h->sdev, "[hsm:%s] TUR retry\n", __func__);
			interval = 1;
		break;
		case SCSI_DH_IO:
			sdev_printk(KERN_INFO, h->sdev, "[hsm:%s] TUR failed with IO error\n", __func__);
			// h->flags |= HSM_DH_DETACHINPROGRESS;
		break;
		default:
		break;
	}
out:
	if (h->flags & HSM_DH_DETACHINPROGRESS) {
		sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Exit - detach\n", __func__);
		kref_put(&grp->kref, release_hsm_group);
		mutex_unlock(&h->detach_mutex);
		kobject_uevent(&h->sdev->sdev_gendev.kobj, KOBJ_CHANGE);
	}
	else {
                spin_lock(&h->grp_lock);
                h->flags &= ~(HSM_DH_FORCEUPDATE | HSM_DH_MONITORINPROGRESS);
                spin_unlock(&h->grp_lock);
		queue_delayed_work(hsm_wq, &h->work, msecs_to_jiffies(interval*1000)); 
		// sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Exit\n", __func__);
	}
}

static void proc_attachhsm(struct work_struct *work)
{
	int rval;
	char *argv[5];
	char devarg[64];
	char devid[64];
	struct hsm_dh_data *h = NULL;
	struct scsi_device *sdev = NULL;
	struct hsm_group __rcu *grp = NULL;
	static char *hsmadmpath = "/sbin/hsmadm";
	static char *envp[] = {"HOME=/", "TERM=linux", "PATH=/usr/bin:/bin", NULL};

	h = container_of(work, struct hsm_dh_data, work.work);

	mutex_lock(&h->attach_mutex);
	sdev = h->sdev;
	sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Enter\n", __func__);

	sprintf(devarg, "--device=%s", dev_name(&sdev->sdev_gendev));
	sprintf(devid, "-u %s", h->uuid);

	argv[0] = hsmadmpath;
	argv[1] = "lookup";
	argv[2] = devid;
	argv[3] = devarg;
	argv[4] = NULL;
	rval = call_usermodehelper(hsmadmpath, argv, envp, UMH_WAIT_PROC);
	if (rval) {
		h->flags |= HSM_DH_BLACKLISTED;
		h->tpgs_state = TPGS_STATE_OFFLINE;
		sdev_printk(KERN_INFO, h->sdev, "[hsm:%s] Blacklisted\n", __func__);
	}
	else {
		h->flags &= ~HSM_DH_BLACKLISTED;
		rcu_read_lock();
		grp = rcu_dereference(h->grp);
		if (grp) {
			if (h->flags & HSM_DH_PCN)
				sdev_printk(KERN_INFO, h->sdev,
						"[hsm:%s] HyperQ Inline Path\n", __func__);
			else if ((grp->t_len == h->uuid_len) &&
				(strncmp(grp->t_uuid, h->uuid, h->uuid_len) == 0)) {
				sdev_printk(KERN_INFO, h->sdev,
						"[hsm:%s] Target Direct Path\n", __func__);
				h->flags |= HSM_DH_TGT;
			}
			else if ((grp->s_len == h->uuid_len) &&
				(strncmp(grp->s_uuid, h->uuid, h->uuid_len) == 0)) {
				sdev_printk(KERN_INFO, h->sdev,
						"[hsm:%s] Source Direct Path\n", __func__);
				h->flags |= HSM_DH_SRC;
			}
			else
				sdev_printk(KERN_INFO, h->sdev,
						"[hsm:%s] Unknown Path flgs=0x%x\n",
						__func__, h->flags);
			h->flags |= HSM_DH_FORCEUPDATE;
			INIT_DELAYED_WORK(&h->work, proc_statusmonitor);
			kref_get(&grp->kref);
			// queue_delayed_work(hsm_wq, &h->work, msecs_to_jiffies(1*1000)); 
			mutex_lock(&h->detach_mutex);
			queue_delayed_work(hsm_wq, &h->work, 0);
		}
		else
			sdev_printk(KERN_WARNING, h->sdev,
					"[hsm:%s] Lookup (rval=%d) completed with NULL grp\n",
					__func__, rval);
		rcu_read_unlock();

	}
	mutex_unlock(&h->attach_mutex);
	sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Exit\n", __func__);
}

/*
 * hsm_check - check path status
 * @sdev: device on the path to be checked
 *
 * Check the device status
 */
static void hsm_check(struct hsm_dh_data *h)
{
	if(h) {
		if(cancel_delayed_work(&h->work)) {
			sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] cancel successful\n", __func__);
			queue_delayed_work(hsm_wq, &h->work, 0);
		}
		else {
			sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Nothing to cancel - queue empty\n", __func__);
		}
	}
}

static int hsm_check_sense(struct scsi_device *sdev, struct scsi_sense_hdr *sense_hdr)
{
	switch (sense_hdr->sense_key) {
	case NOT_READY:
		sdev_printk(KERN_INFO, sdev, "[hsm:%s] Enter - sense data (%u / 0x%x / 0x%x) \n",
				__func__ , sense_hdr->sense_key, sense_hdr->asc, sense_hdr->ascq);
		if (sense_hdr->asc == 0x04 && sense_hdr->ascq == 0x0a)
			/*
			 * LUN Not Accessible - ALUA state transition
			 */
			return ADD_TO_MLQUEUE;
		if (sense_hdr->asc == 0x04 && sense_hdr->ascq == 0x0b) {
			/*
			 * LUN Not Accessible -- Target port in standby state
			 */
                        struct hsm_dh_data *h = HSM_HANDLER_DATA(sdev);
                        if (!(h->flags & HSM_DH_DETACHINPROGRESS) &&
                                (h->path_state != TPGS_STATE_STANDBY)) {
                                spin_lock(&h->grp_lock);
                                h->flags |=  HSM_DH_FORCEUPDATE;
                                h->path_state = TPGS_STATE_STANDBY;
                                spin_unlock(&h->grp_lock);
                                if (!(h->flags & HSM_DH_MONITORINPROGRESS)) {
                                        if(cancel_delayed_work(&h->work))
                                                queue_delayed_work(hsm_wq, &h->work, 0);
                                }
                        }
			return SUCCESS;
		}
		if (sense_hdr->asc == 0x04 && sense_hdr->ascq == 0x0c)
			/*
			 * LUN Not Accessible -- Target port in unavailable state
			 */
			return SUCCESS;
		if (sense_hdr->asc == 0x04 && sense_hdr->ascq == 0x12)
			/*
			 * LUN Not Ready -- Offline
			 */
			return SUCCESS;
		if (sdev->allow_restart &&
		    sense_hdr->asc == 0x04 && sense_hdr->ascq == 0x02)
			/*
			 * if the device is not started, we need to wake
			 * the error handler to start the motor
			 */
			return FAILED;
		break;
	case UNIT_ATTENTION:
		sdev_printk(KERN_INFO, sdev, "[hsm:%s] Enter - sense data (%u / 0x%x / 0x%x) \n",
				__func__ , sense_hdr->sense_key, sense_hdr->asc, sense_hdr->ascq);
		if (sense_hdr->asc == 0x29 && sense_hdr->ascq == 0x00)
			/*
			 * Power On, Reset, or Bus Device Reset, just retry.
			 */
			return ADD_TO_MLQUEUE;
		if (sense_hdr->asc == 0x29 && sense_hdr->ascq == 0x04)
			/*
			 * Device internal reset
			 */
			return ADD_TO_MLQUEUE;
		if (sense_hdr->asc == 0x2a && sense_hdr->ascq == 0x01)
			/*
			 * Mode Parameters Changed
			 */
			return ADD_TO_MLQUEUE;
		if (sense_hdr->asc == 0x2a && sense_hdr->ascq == 0x06) {
			/*
			 * ALUA state changed
			 */
			hsm_check(HSM_HANDLER_DATA(sdev));
			return ADD_TO_MLQUEUE;
		}
		if (sense_hdr->asc == 0x2a && sense_hdr->ascq == 0x07) {
			/*
			 * Implicit ALUA state transition failed
			 */
			hsm_check(HSM_HANDLER_DATA(sdev));
			return ADD_TO_MLQUEUE;
		}
		if (sense_hdr->asc == 0x3f && sense_hdr->ascq == 0x03) {
			/*
			 * Inquiry data has changed
			 */
			hsm_check(HSM_HANDLER_DATA(sdev));
			return ADD_TO_MLQUEUE;
		}
		if (sense_hdr->asc == 0x3f && sense_hdr->ascq == 0x0e)
			/*
			 * REPORTED_LUNS_DATA_HAS_CHANGED is reported
			 * when switching controllers on targets like
			 * Intel Multi-Flex. We can just retry.
			 */
			return ADD_TO_MLQUEUE;
		return ADD_TO_MLQUEUE;
	case ILLEGAL_REQUEST:
                if (sense_hdr->asc == 0x25 && sense_hdr->ascq == 0x00) {
                        /*
                        ** LOGICAL UNIT NOT SUPPORTED. 
                        ** Implies the LUN is removed from under us
                        ** proceed to cleanup
                        */
                        struct hsm_dh_data *h = HSM_HANDLER_DATA(sdev);
                        if (!(h->flags & HSM_DH_DETACHINPROGRESS)) {
                                sdev_printk(KERN_INFO, sdev,
                                        "[hsm:%s] ILLEGAL_REQUEST - sense data (%u / 0x%x / 0x%x) \n",
                                        __func__ , sense_hdr->sense_key,
                                        sense_hdr->asc, sense_hdr->ascq);
                                h->flags |= HSM_DH_DETACHINPROGRESS | HSM_DH_FORCEUPDATE;
                                if (!(h->flags & HSM_DH_MONITORINPROGRESS)) {
                                        if(cancel_delayed_work(&h->work))
                                                queue_delayed_work(hsm_wq, &h->work, 0);
                                }
                        }
                        return FAILED;
                }
                else  { 
                        sdev_printk(KERN_INFO, sdev, "[hsm:%s] ILLEGAL_REQUEST - sense data (%u / 0x%x / 0x%x) \n",
                                __func__ , sense_hdr->sense_key, sense_hdr->asc, sense_hdr->ascq);
                        return ADD_TO_MLQUEUE;
                }
                break;
        default:
                sdev_printk(KERN_INFO, sdev, "[hsm:%s] default - sense data (%u / 0x%x / 0x%x) \n",
                        __func__ , sense_hdr->sense_key, sense_hdr->asc, sense_hdr->ascq);
                break;
        }

	return SCSI_RETURN_NOT_HANDLED;
}

/*
 * hsm_set_params - set/unset the optimize flag
 * @sdev: device on the path to be activated
 * params - parameters in the following format
 *      "no_of_params\0param1\0param2\0param3\0...\0"
 * For example, to set the flag pass the following parameters
 * from multipath.conf
 *     hardware_handler        "2 hsm 1"
 */
static int hsm_set_params(struct scsi_device *sdev, const char *params)
{
	int result = SCSI_DH_OK;
	const char *p = params;
	unsigned int argc;
	unsigned long flags;
	unsigned int passive = 0;
	struct hsm_dh_data *h = HSM_HANDLER_DATA(sdev);
	struct hsm_group __rcu *grp = NULL;

	sdev_printk(KERN_DEBUG, h->sdev, "[hsm:%s] Enter\n", __func__);
	if ((sscanf(params, "%u", &argc) != 1) || (argc != 1))
		return -EINVAL;

	while (*p++)
		;
	if ((sscanf(p, "%u", &passive) != 1) || (passive > 1))
		return -EINVAL;

	rcu_read_lock();
	grp = rcu_dereference(h->grp);
	if (!grp) {
		rcu_read_unlock();
		return -ENXIO;
	}
	spin_lock_irqsave(&grp->lock, flags);
	if (passive)
		grp->flags |= HSM_GRP_PASSIVEMODE;
	else
		grp->flags &= ~HSM_GRP_PASSIVEMODE;
	spin_unlock_irqrestore(&grp->lock, flags);
	rcu_read_unlock();
	sdev_printk(KERN_INFO, sdev, "[hsm:%s] Exit - host mode = %s\n",
			__func__, (passive == 0) ? "Passive" : "Active");
	return result;
}


/*
 * hsm_activate - activate a path
 * @sdev: device on the path to be activated
 *
 * We're currently switching the port group to be activated only and
 * let the array figure out the rest.
 * There may be other arrays which require us to switch all port groups
 * based on a certain policy. But until we actually encounter them it
 * should be okay.
 */
static int hsm_activate(struct scsi_device *sdev,
			activate_complete fn, void *data) // --- TBD ---
{
	struct hsm_dh_data *h = HSM_HANDLER_DATA(sdev);

	sdev_printk(KERN_INFO, sdev, "[hsm:%s] Enter\n", __func__);
	/*
	** Wait for attach to complete before activating
	*/
	mutex_lock(&h->attach_mutex);
	mutex_unlock(&h->attach_mutex);
	if (fn)
		fn(data, SCSI_DH_OK);
	sdev_printk(KERN_INFO, h->sdev, "[hsm:%s] Exit\n", __func__);
	return 0;
}

/*
 * hsm_prep_fn - request callback
 *
 * Fail I/O to all paths not in state
 * active/optimized or active/non-optimized.
 */
static int hsm_prep_fn(struct scsi_device *sdev, struct request *req)
{
	struct hsm_dh_data *h = HSM_HANDLER_DATA(sdev);
	int ret = BLKPREP_OK;

	if (h->tpgs_state == TPGS_STATE_UNAVAILABLE)
		ret = BLKPREP_DEFER;
	else if (h->tpgs_state != TPGS_STATE_OPTIMIZED &&
		 h->tpgs_state != TPGS_STATE_NONOPTIMIZED &&
		 h->tpgs_state != TPGS_STATE_LBA_DEPENDENT) {
		ret = BLKPREP_KILL;
		req->cmd_flags |= REQ_QUIET;
	}
#if 0
	if (req->cmd[0] != 0)
		sdev_printk(KERN_INFO, sdev,
			"[hsm:%s] cdb: %02x %02x %02x %02x %02x %02x\n",
			__func__, req->cmd[0], req->cmd[1], req->cmd[2],
			req->cmd[3], req->cmd[4], req->cmd[5]);
#endif
	return ret;
}

static int hsm_bus_attach(struct scsi_device *sdev);
static void hsm_bus_detach(struct scsi_device *sdev);

static struct scsi_device_handler hsm_dh = {
	.name = HSM_DH_NAME,
	.module = THIS_MODULE,
	.attach = hsm_bus_attach,
	.detach = hsm_bus_detach,
	.prep_fn = hsm_prep_fn,
	.check_sense = hsm_check_sense,
	.activate = hsm_activate,
	.set_params = hsm_set_params,
};

/*
 * hsm_bus_attach - Attach device handler
 * @sdev: device to be attached to
 */
static int hsm_bus_attach(struct scsi_device *sdev)
{
	struct hsm_dh_data *h;
	int err = 0;
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,32)
        unsigned long flags;
	struct scsi_dh_data *s;

	sdev_printk(KERN_INFO, sdev, "[hsm:%s] Enter\n", __func__);
	s = kzalloc(sizeof(*s) + sizeof(*h) , GFP_KERNEL);
	if (!s)
		return -ENOMEM;
	s->scsi_dh = &hsm_dh;
	h = (struct hsm_dh_data *)s->buf;
#else
	sdev_printk(KERN_INFO, sdev, "[hsm:%s] Enter\n", __func__);
	h = kzalloc(sizeof(*h) , GFP_KERNEL);
	if (!h)
		return -ENOMEM;
#endif

	spin_lock_init(&h->grp_lock);
	mutex_init(&h->attach_mutex);
	mutex_init(&h->detach_mutex);
	h->sdev = sdev;
	h->buff = h->resp;
	h->init_error = SCSI_DH_OK;
	h->flags = (!strncmp(sdev->vendor, "Parsec", 6)) ? HSM_DH_PCN : 0;
	h->path_state = TPGS_STATE_OPTIMIZED;
	/*
	** To prevent IO errors when taking over
	** the device, need to keep it operational
	*/
	h->tpgs_state = TPGS_STATE_OPTIMIZED;
	h->bufflen = HSM_RESP_BUFFERSIZE;
	rcu_assign_pointer(h->grp, NULL);
	INIT_LIST_HEAD(&h->entry);
	INIT_DELAYED_WORK(&h->work, proc_attachhsm);
	err = extract_deviceid(h);
	if (err != SCSI_DH_OK)
		goto failed;

	err = device_create_file(&sdev->sdev_gendev, &dev_attr_scsi_id);
	err = device_create_file(&sdev->sdev_gendev, &dev_attr_access_state);
	err = device_create_file(&sdev->sdev_gendev, &dev_attr_helper);

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,32)
        /* Specifically for RedHat 2.6.32-642.el6.x86_64 */
        if (!try_module_get(THIS_MODULE))
                goto failed; 

        spin_lock_irqsave(sdev->request_queue->queue_lock, flags); 
        sdev->scsi_dh_data = s;
        spin_unlock_irqrestore(sdev->request_queue->queue_lock, flags); 
        sdev_printk(KERN_INFO, sdev, "%s: Attached\n", HSM_DH_NAME);
#else
        /* Specifically for Ubuntu 4.4.0-28 */
        sdev->handler_data = h;
#endif

	queue_delayed_work(hsm_wq, &h->work, 0);
	sdev_printk(KERN_INFO, sdev, "[hsm:%s] Exit\n", __func__);
	return 0;
failed:
	sdev_printk(KERN_WARNING, sdev, "[hsm:%s] Exit err=%u\n", __func__, err);
	if (h->buff && h->resp != h->buff)
		kfree(h->buff);
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,32)
	kfree(s);
#else
	kfree(h);
#endif
	return -EINVAL;
}

/*
 * hsm_bus_detach - Detach device handler
 * @sdev: device to be detached from
 */
static void hsm_bus_detach(struct scsi_device *sdev)
{
	struct hsm_group __rcu *grp = NULL;
	struct hsm_dh_data *h = HSM_HANDLER_DATA(sdev);
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,32)
        unsigned long flags;
#endif

	sdev_printk(KERN_INFO, sdev, "[hsm:%s] Enter\n", __func__);

	if (!(h->flags & HSM_DH_DETACHINPROGRESS)) {
		spin_lock(&h->grp_lock);
		h->flags |= HSM_DH_DETACHINPROGRESS;
		spin_unlock(&h->grp_lock);
	}

	rcu_read_lock();
	grp = rcu_dereference(h->grp);
	rcu_read_unlock();

	if(cancel_delayed_work_sync(&h->work)) {
		sdev_printk(KERN_DEBUG, sdev, "[hsm:%s] cancel successful\n", __func__);
		kref_put(&grp->kref, release_hsm_group);
		mutex_unlock(&h->detach_mutex);
	}
	else {
		sdev_printk(KERN_DEBUG, sdev, "[hsm:%s] Nothing to cancel - queue empty\n", __func__);
	}

	mutex_lock(&h->detach_mutex);


	if (grp) {
                struct hsm_dh_data *th = NULL;

		spin_lock(&grp->lock);
		list_del(&h->entry);
		rcu_assign_pointer(h->grp, NULL);
                list_for_each_entry(th, &grp->members, entry) {
			spin_lock(&th->grp_lock);
                        th->flags |= HSM_DH_FORCEUPDATE;
			spin_unlock(&th->grp_lock);
		}
		spin_unlock(&grp->lock);
		kref_put(&grp->kref, release_hsm_group);
	}

	if (h->buff && h->resp != h->buff)
		kfree(h->buff);
	mutex_unlock(&h->detach_mutex);

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,32)
        /* Specifically for RedHat 2.6.32-642.el6.x86_64 */
	spin_lock_irqsave(sdev->request_queue->queue_lock, flags);
        kfree(sdev->scsi_dh_data);
        sdev->scsi_dh_data = NULL;
	spin_unlock_irqrestore(sdev->request_queue->queue_lock, flags);
	module_put(THIS_MODULE);
#else
        /* Specifically for Ubuntu 4.4.0-28 */
        kfree(sdev->handler_data);
        sdev->handler_data = NULL;
#endif
	sdev_printk(KERN_INFO, sdev, "[hsm:%s] Exit\n", __func__);
}

static int __init hsm_init(void)
{
	int rval;

	printk(KERN_NOTICE "[hsm:%s] Enter\n", __func__);
	hsm_wq = create_singlethread_workqueue("hsm_wq");
	if (!hsm_wq) {
		/* Temporary failure, bypass */
		return SCSI_DH_DEV_TEMP_BUSY;
	}
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,32)
	/* Specifically for RedHat 2.6.32-642.el6.x86_64 */
	rval = scsi_register_device_handler(&hsm_dh, NULL);
	if (rval != 0) {
		printk(KERN_WARNING "[hsm:%s] Failed to register scsi device handler\n", __func__);
		destroy_workqueue(hsm_wq);
	}

#else
        /* Specifically for Ubuntu 4.4.0-28 */
        rval = scsi_register_device_handler(&hsm_dh);
	if (rval != 0) {
		printk(KERN_WARNING "[hsm:%s] Failed to register scsi device handler\n", __func__);
		destroy_workqueue(hsm_wq);
	}
#endif
	printk(KERN_NOTICE "[hsm:%s] Exit return=%u\n", __func__, rval);
	return rval;
}

static void __exit hsm_exit(void)
{
	printk(KERN_NOTICE "[hsm:%s] Enter\n", __func__);
	scsi_unregister_device_handler(&hsm_dh);
	destroy_workqueue(hsm_wq);
	printk(KERN_NOTICE "[hsm:%s] Exit\n", __func__);
}

module_init(hsm_init);
module_exit(hsm_exit);

MODULE_DESCRIPTION("DM Multipath HSM support");
MODULE_AUTHOR("Raghu Bilugu <raghu@parseclabs.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION(HSM_DH_VER);