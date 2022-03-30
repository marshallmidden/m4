Make "1", which is supposedly like boot line addition of printk.devkmsg=pn.

/* Keep both the 'on' and 'off' bits clear, i.e. ratelimit by default: */
// #define DEVKMSG_LOG_MASK_DEFAULT     0
#define DEVKMSG_LOG_MASK_DEFAULT        1

