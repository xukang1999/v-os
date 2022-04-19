#ifndef VOS_SO_DEFINES_H
#define VOS_SO_DEFINES_H

#define VOS_SOL_SOCKET      1

#define VOS_SO_DEBUG        1
#define VOS_SO_REUSEADDR    2
#define VOS_SO_ERROR        4
#define VOS_SO_DONTROUTE    5
#define VOS_SO_BROADCAST    6
#define VOS_SO_SNDBUF       7
#define VOS_SO_RCVBUF       8
#define VOS_SO_KEEPALIVE    9
#define VOS_SO_OOBINLINE    10
#define VOS_SO_LINGER       13
#define VOS_SO_REUSEPORT    15
#define VOS_SO_RCVLOWAT     18
#define VOS_SO_SNDLOWAT     19
#define VOS_SO_RCVTIMEO     20
#define VOS_SO_SNDTIMEO     21
#define VOS_SO_ACCEPTCONN   30
#define VOS_SO_PROTOCOL     38


#define VOS_IP_TOS        1
#define VOS_IP_TTL        2
#define VOS_IP_HDRINCL    3
#define VOS_IP_OPTIONS    4

#define VOS_IP_MULTICAST_IF       32
#define VOS_IP_MULTICAST_TTL      33
#define VOS_IP_MULTICAST_LOOP     34
#define VOS_IP_ADD_MEMBERSHIP     35
#define VOS_IP_DROP_MEMBERSHIP    36

#define VOS_IPV6_V6ONLY           26
#define VOS_IPV6_RECVPKTINFO      49  

#define VOS_TCP_NODELAY     1
#define VOS_TCP_MAXSEG      2
#define VOS_TCP_KEEPIDLE    4
#define VOS_TCP_KEEPINTVL   5
#define VOS_TCP_KEEPCNT     6
#define VOS_TCP_INFO        11
#define VOS_TCP_MD5SIG      14

/* setsockopt/getsockopt define end */


/* ioctl define start */

#define VOS_TIOCEXCL    0x540C
#define VOS_TIOCNXCL    0x540D
#define VOS_TIOCSCTTY   0x540E
#define VOS_TIOCGPGRP   0x540F
#define VOS_TIOCSPGRP   0x5410
#define VOS_TIOCOUTQ    0x5411
#define VOS_TIOCSTI     0x5412
#define VOS_TIOCGWINSZ  0x5413
#define VOS_TIOCSWINSZ  0x5414
#define VOS_TIOCMGET    0x5415
#define VOS_TIOCMBIS    0x5416
#define VOS_TIOCMBIC    0x5417
#define VOS_TIOCMSET    0x5418

#define VOS_FIONREAD    0x541B
#define VOS_TIOCCONS    0x541D
#define VOS_TIOCPKT     0x5420
#define VOS_FIONBIO     0x5421
#define VOS_TIOCNOTTY   0x5422
#define VOS_TIOCSETD    0x5423
#define VOS_TIOCGETD    0x5424
#define VOS_TIOCSBRK    0x5427
#define VOS_TIOCCBRK    0x5428
#define VOS_TIOCGSID    0x5429

#define VOS_FIONCLEX    0x5450
#define VOS_FIOCLEX     0x5451
#define VOS_FIOASYNC    0x5452

#define VOS_TIOCPKT_DATA          0
#define VOS_TIOCPKT_FLUSHREAD     1
#define VOS_TIOCPKT_FLUSHWRITE    2
#define VOS_TIOCPKT_STOP          4
#define VOS_TIOCPKT_START         8
#define VOS_TIOCPKT_NOSTOP        16
#define VOS_TIOCPKT_DOSTOP        32
#define VOS_TIOCPKT_IOCTL         64

#define VOS_SIOCGIFCONF     0x8912
#define VOS_SIOCGIFFLAGS    0x8913
#define VOS_SIOCSIFFLAGS    0x8914
#define VOS_SIOCGIFADDR     0x8915
#define VOS_SIOCSIFADDR     0x8916
#define VOS_SIOCGIFDSTADDR  0x8917
#define VOS_SIOCSIFDSTADDR  0x8918
#define VOS_SIOCGIFBRDADDR  0x8919
#define VOS_SIOCSIFBRDADDR  0x891a
#define VOS_SIOCGIFNETMASK  0x891b
#define VOS_SIOCSIFNETMASK  0x891c
#define VOS_SIOCGIFMETRIC   0x891d
#define VOS_SIOCSIFMETRIC   0x891e
#define VOS_SIOCGIFMTU      0x8921
#define VOS_SIOCSIFMTU      0x8922
#define VOS_SIOCSIFNAME     0x8923
#define VOS_SIOCADDMULTI    0x8931
#define VOS_SIOCDELMULTI    0x8932
#define VOS_SIOCGIFINDEX    0x8933
#define VOS_SIOCDIFADDR     0x8936

/*sctp*/
#define VOS_SCTP_RTOINFO    0
#define VOS_SCTP_ASSOCINFO  1
#define VOS_SCTP_INITMSG    2
#define VOS_SCTP_NODELAY    3               /* Get/set nodelay option. */
#define VOS_SCTP_AUTOCLOSE  4
#define VOS_SCTP_SET_PEER_PRIMARY_ADDR 5
#define VOS_SCTP_PRIMARY_ADDR       6
#define VOS_SCTP_ADAPTATION_LAYER   7
#define VOS_SCTP_DISABLE_FRAGMENTS  8
#define VOS_SCTP_PEER_ADDR_PARAMS   9
#define VOS_SCTP_DEFAULT_SEND_PARAM 10
#define VOS_SCTP_EVENTS     11
#define VOS_SCTP_I_WANT_MAPPED_V4_ADDR 12   /* Turn on/off mapped v4 addresses  */
#define VOS_SCTP_MAXSEG     13              /* Get/set maximum fragment. */
#define VOS_SCTP_STATUS     14
#define VOS_SCTP_GET_PEER_ADDR_INFO 15
#define VOS_SCTP_DELAYED_ACK_TIME   16
#define VOS_SCTP_DELAYED_ACK LINUX_SCTP_DELAYED_ACK_TIME
#define VOS_SCTP_DELAYED_SACK LINUX_SCTP_DELAYED_ACK_TIME
#define VOS_SCTP_CONTEXT    17
#define VOS_SCTP_FRAGMENT_INTERLEAVE        18
#define VOS_SCTP_PARTIAL_DELIVERY_POINT     19 /* Set/Get partial delivery point */
#define VOS_SCTP_MAX_BURST  20              /* Set/Get max burst */
#define VOS_SCTP_AUTH_CHUNK 21      /* Set only: add a chunk type to authenticate */
#define VOS_SCTP_HMAC_IDENT 22
#define VOS_SCTP_AUTH_KEY   23
#define VOS_SCTP_AUTH_ACTIVE_KEY    24
#define VOS_SCTP_AUTH_DELETE_KEY    25
#define VOS_SCTP_PEER_AUTH_CHUNKS   26      /* Read only */
#define VOS_SCTP_LOCAL_AUTH_CHUNKS  27      /* Read only */
#define VOS_SCTP_GET_ASSOC_NUMBER   28      /* Read only */
#define VOS_SCTP_GET_ASSOC_ID_LIST  29      /* Read only */
#define VOS_SCTP_AUTO_ASCONF       30
#define VOS_SCTP_PEER_ADDR_THLDS    31
#define VOS_SCTP_RECVRCVINFO        32
#define VOS_SCTP_RECVNXTINFO        33
#define VOS_SCTP_DEFAULT_SNDINFO    34
#define VOS_SCTP_AUTH_DEACTIVATE_KEY        35
#define VOS_SCTP_REUSE_PORT         36

/* Internal Socket Options. Some of the sctp library functions are
 * implemented using these socket options.
 */
#define VOS_SCTP_SOCKOPT_BINDX_ADD  100     /* BINDX requests for adding addrs */
#define VOS_SCTP_SOCKOPT_BINDX_REM  101     /* BINDX requests for removing addrs. */
#define VOS_SCTP_SOCKOPT_PEELOFF    102     /* peel off association. */
/* Options 104-106 are deprecated and removed. Do not use this space */
#define VOS_SCTP_SOCKOPT_CONNECTX_OLD       107     /* CONNECTX old requests. */
#define VOS_SCTP_GET_PEER_ADDRS     108             /* Get all peer address. */
#define VOS_SCTP_GET_LOCAL_ADDRS    109             /* Get all local address. */
#define VOS_SCTP_SOCKOPT_CONNECTX   110             /* CONNECTX requests. */
#define VOS_SCTP_SOCKOPT_CONNECTX3  111     /* CONNECTX requests (updated) */
#define VOS_SCTP_GET_ASSOC_STATS    112     /* Read only */
#define VOS_SCTP_PR_SUPPORTED       113
#define VOS_SCTP_DEFAULT_PRINFO     114
#define VOS_SCTP_PR_ASSOC_STATUS    115
#define VOS_SCTP_PR_STREAM_STATUS   116
#define VOS_SCTP_RECONFIG_SUPPORTED 117
#define VOS_SCTP_ENABLE_STREAM_RESET        118
#define VOS_SCTP_RESET_STREAMS      119
#define VOS_SCTP_RESET_ASSOC        120
#define VOS_SCTP_ADD_STREAMS        121
#define VOS_SCTP_SOCKOPT_PEELOFF_FLAGS 122
#define VOS_SCTP_STREAM_SCHEDULER   123
#define VOS_SCTP_STREAM_SCHEDULER_VALUE     124
#define VOS_SCTP_INTERLEAVING_SUPPORTED     125
#define VOS_SCTP_SENDMSG_CONNECT    126
#define VOS_SCTP_EVENT      127
#define VOS_SCTP_ASCONF_SUPPORTED   128
#define VOS_SCTP_AUTH_SUPPORTED     129
#define VOS_SCTP_ECN_SUPPORTED      130


/* ioctl define end */

/* af define start */

#define VOS_AF_INET6        10


#define	VOS_F_GETFL		3		/* get file status flags */
#define	VOS_F_SETFL		4		/* set file status flags */
#define	VOS_O_NONBLOCK	0x0004		/* no delay */
#endif