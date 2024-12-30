#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include<stdio.h>
//#include "prociscsid.h"
#include <dirent.h>

/**/
typedef uint16_t __be16;
typedef uint32_t __be32;

#define ISCSI_NAME_LENGTH 260
#define SECRET_LENGTH 20

typedef struct iscsi_target
{
    char name[ISCSI_NAME_LENGTH];
    unsigned int ip;
    int port;
    int pgtag;
    
}iscsi_target;

typedef struct session_handle
{
    uint64_t transport_handle;
    uint64_t handle;
    uint32_t session_id;
    char     device_name[ISCSI_NAME_LENGTH];
}session_handle;

/*
request and response structure
 Any change here should also reflect in mgmt_ipc.h
*/
typedef enum iscsid_err {
    MGMT_IPC_OK         = 0,
    MGMT_IPC_ERR            = 1,
    MGMT_IPC_ERR_NOT_FOUND      = 2,
    MGMT_IPC_ERR_NOMEM      = 3,
    MGMT_IPC_ERR_TCP_FAILURE    = 4,
    MGMT_IPC_ERR_LOGIN_FAILURE  = 5,
    MGMT_IPC_ERR_IDBM_FAILURE   = 6,
    MGMT_IPC_ERR_INVAL      = 7,
    MGMT_IPC_ERR_TCP_TIMEOUT    = 8,
    MGMT_IPC_ERR_INTERNAL       = 9,
    MGMT_IPC_ERR_LOGOUT_FAILURE = 10,
    MGMT_IPC_ERR_PDU_TIMEOUT    = 11,
    MGMT_IPC_ERR_TRANS_NOT_FOUND    = 12,
    MGMT_IPC_ERR_ACCESS     = 13,
    MGMT_IPC_ERR_TRANS_CAPS     = 14,
    MGMT_IPC_ERR_SESSON_EXIST       = 15,
    MGMT_IPC_ERR_DUPLICATE_RECORD    = 16,
    MGMT_IPC_ERR_RECORD_NOT_DELETED = 17,
} iscsid_err;

/*command type*/
#define X_MGMT_IPC_TARGET_ADD       11
#define X_MGMT_IPC_TARGET_SHOW      12
#define X_MGMT_IPC_SESSION_LOGIN    13
#define X_MGMT_IPC_TARGET_REMOVE    14
#define X_MGMT_IPC_DISCOVERY        15
#define X_MGMT_IPC_SESSION_CLOSE    16

typedef struct discovery_info {
    unsigned char initiator_name[ISCSI_NAME_LENGTH];
    unsigned int initiator_ip;
    unsigned int target_ip;
    int port;
    char chap_secret[SECRET_LENGTH];
}discovery_info;



typedef struct open_session {
    char target_name[ISCSI_NAME_LENGTH];
    int pgtag;
    char initiator_name[ISCSI_NAME_LENGTH];
    unsigned int initiator_ip;
    char chap_secret[SECRET_LENGTH];
}open_session;


typedef struct add_target_info
{
    unsigned char  name[ISCSI_NAME_LENGTH];
    int pgtag;
    unsigned int  ip;
    int port;
}add_target_info;

typedef struct request {
    uint32_t command;
    union {
        /* messages */
        discovery_info discovery;
        open_session o_session;
        add_target_info target;
        /*
        ** for close session and remove target 
        ** 1st 3 fields of open_session structure is used
        */ 
    } u;
} request;

typedef struct response {
    uint32_t command;
    uint32_t status;

    union {
        struct discovery_targetlist {
#define TARGETLIST_MAX        64
            iscsi_target target[TARGETLIST_MAX];
            int cnt;
        } targetlist;
        session_handle session;

    } u;
} response;


int do_iscsid(request *req, response *rsp);

#define ISCSIADM_NAMESPACE_PROC  "ISCSIADM_ABSTRACT_NAMESPACE"

struct iscsi_ipc *ipc = NULL; /* dummy */
static int ipc_fd = -1;

static int iscsid_connect(void)
{
    int fd, status;
    struct sockaddr_un addr;

    fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (fd < 0) {
        fprintf(stderr,"can not create IPC socket!");
        return fd;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_LOCAL;
    memcpy((char *) &addr.sun_path + 1, ISCSIADM_NAMESPACE_PROC, strlen(ISCSIADM_NAMESPACE_PROC));

    if ((status = connect(fd, (struct sockaddr *) &addr, sizeof(addr))) < 0) {
        fprintf(stderr,"can not connect to iSCSI daemon!");
        fd = status;
    }

    return fd;
}

static int iscsid_request(int fd, request *req)
{
    int status;
    fprintf(stderr,"size of request = %d\n",sizeof(*req));
    if ((status = write(fd, req, sizeof(*req))) != sizeof(*req)) {
        fprintf(stderr,"got write statusor (%d) on cmd %d, daemon died?", status, req->command);
        if (status >= 0)
            status = -EIO;
    }
    return status;
}

static int iscsid_response(int fd, response *rsp)
{
    int status;

    if ((status = read(fd, rsp, sizeof(*rsp))) != sizeof(*rsp)) {
        fprintf(stderr,"got read statusor (%d), daemon died?", status);
        if (status >= 0)
            status = -EIO;
    } else
        status = rsp->status;

    return status;
}

int do_iscsid(request *req, response *rsp)
{
    int status;

    fprintf(stderr,"do_iscsid: try connect\n");
    if ((ipc_fd = iscsid_connect()) < 0) {
        status = ipc_fd;
        fprintf(stderr,"do_iscsid: connect failure\n");
        goto out;
    }

    fprintf(stderr,"do_iscsid: call iscsid_request\n");
    if ((status = iscsid_request(ipc_fd, req)) < 0)
    {
        fprintf(stderr,"do_iscsid: iscsid_request failure\n");
        goto out;
    }

    status = iscsid_response(ipc_fd, rsp);
    fprintf(stderr,"iscsid_response returns %d\n",status);
    if (!status && req->command != rsp->command)
        status = -EIO;
out:
    if (ipc_fd > 0)
        close(ipc_fd);
    ipc_fd = -1;

    return status;
}

iscsi_target* iscsi_discover_targets(char *initiator_name,unsigned int initiator_ip, unsigned int target_ip, int port, char* chap_secret, int *count)
{
    int rc;
    int i=0;
    request req;
    response rsp;
    iscsi_target *targetlist = NULL;

    memset(&req, 0, sizeof(req));
    req.command = X_MGMT_IPC_DISCOVERY;
    fprintf(stderr,"iscsi_discover_targets: i_name=%s i_ip=0x%x t_ip=0x%x \n",initiator_name,initiator_ip,target_ip);
    strcpy(req.u.discovery.initiator_name,initiator_name);
    fprintf(stderr,"req discovery size = %d\n",sizeof(req.u.discovery));
    fprintf(stderr,"req open_session size = %d\n",sizeof(req.u.o_session));
    fprintf(stderr,"req add_target size = %d\n",sizeof(req.u.target));
    fprintf(stderr,"req size = %d\n",sizeof(req));
    if(chap_secret)
        strcpy(req.u.discovery.chap_secret,chap_secret);

    req.u.discovery.port         = port;
    req.u.discovery.initiator_ip = initiator_ip;
    req.u.discovery.target_ip    = target_ip;

    rc = do_iscsid(&req, &rsp);
    /*
     *  we have got the iscsi_target array in response 
     *  allocate memory and fill it. Memory should be freed by caller
     */
    if(rsp.status != 0)
    {
        /*
        ** error case. count will hold the errno
        **
        */
        *count = rsp.status;
        return NULL;
    }
    fprintf(stderr,"discovery returns %d targets\n",rsp.u.targetlist.cnt);

    if(rsp.u.targetlist.cnt > 0)
    {
        targetlist = malloc(sizeof(iscsi_target) * rsp.u.targetlist.cnt);    

        if(targetlist)
        {
            for(i=0; i< rsp.u.targetlist.cnt; i++)
            {
                *(targetlist + i) = rsp.u.targetlist.target[i];
            }
        }

    }
    *count = rsp.u.targetlist.cnt;
    return targetlist;
}

int iscsi_add_target(char *target_name, unsigned int target_ip, int port, int pgtag)
{
    int rc;
    request req;
    response rsp;

    memset(&req, 0, sizeof(req));
    req.command = X_MGMT_IPC_TARGET_ADD;
    strcpy(req.u.target.name,target_name);

    req.u.target.pgtag = pgtag;
    req.u.target.port  = port;
    req.u.target.ip    = target_ip;

    rc = do_iscsid(&req, &rsp);
    return rc;
}

int iscsi_remove_target(char *target_name,int pgtag)
{
    int rc;
    request req;
    response rsp;

    memset(&req, 0, sizeof(req));
    req.command = X_MGMT_IPC_TARGET_REMOVE;
    strcpy(req.u.target.name,target_name);
    req.u.target.pgtag = pgtag;

    rc = do_iscsid(&req, &rsp);
    return rc;
}

int iscsi_open_session(char *initiator_name,unsigned int initiator_ip, char *target_name, int pgtag, char *chap_secret,session_handle *handle )
{
    int rc;
    request req;
    response rsp;

    memset(&req, 0, sizeof(req));
    req.command = X_MGMT_IPC_SESSION_LOGIN;
    strcpy(req.u.o_session.target_name,target_name);
    strcpy(req.u.o_session.initiator_name,initiator_name);

    if(chap_secret)
        strcpy(req.u.o_session.chap_secret,chap_secret);

    req.u.o_session.pgtag = pgtag;
    req.u.o_session.initiator_ip = initiator_ip;

    rc = do_iscsid(&req, &rsp);
    fprintf(stderr,"do_iscsid returns %d\n",rc);
    fprintf(stderr,"response = %d \ntransport_handle %llx \nsession_handle = %llx\n",rsp.status,
                                    rsp.u.session.transport_handle,rsp.u.session.handle);
    *handle = rsp.u.session; 
    return rc;
}

int iscsi_close_session(char *target_name, int pgtag, char *initiator_name)
{
    int rc;
    request req;
    response rsp;
    memset(&req, 0, sizeof(req));

    req.command = X_MGMT_IPC_SESSION_CLOSE;
    strcpy(req.u.o_session.target_name,target_name);
    req.u.o_session.pgtag = pgtag;
    strcpy(req.u.o_session.initiator_name,initiator_name);

    rc = do_iscsid(&req, &rsp);
    return rc;
}



/**/


void print_usage(void)
{
     printf("\nTest program to check interface between FE and daemon\n");
     printf("\n./test discover initiator_ip  target_ip\n"); 
     printf("./test login initiator_name  target_name target_portal_group_tag\n"); 
     printf("./test logout initiator_name  target_name target_portal_group_tag\n");
}


int main(int argc, char *argv[])

{
    if(argc < 2 )
    {
         print_usage();
         return 1;
    }
	if(!strcmp(argv[1],"add"))
	{

		//target_add(target);
		iscsi_add_target("naa.2012121212121",0x0b01a8c0,3260,6);

	}else if(!strcmp(argv[1],"login"))
	{

		session_handle handle;
                char initiator_name[256];
                char target_name[256]; 
                int pgtag=3;
                int rc;
                if(argv[2])
                {
                        strcpy(initiator_name,argv[2]);
                }else
                {
                        strcpy(initiator_name,"iqn.202300D0B2027C60");
                       
                }
                if(argv[3])
                {
                        strcpy(target_name,argv[3]);
                }else
                {
                        strcpy(target_name,"naa.202100D0B2027C60");
                }
                if(argv[4])
                {
                    pgtag = atoi(argv[4]); 
                }

		rc = iscsi_open_session(initiator_name,0xabcd, target_name,pgtag, "Ihavenothing",&handle);
                printf("return value= %d\n",rc);
                printf("handle->transport=0x%llx\n",handle.transport_handle);
                printf("handle->session=0x%llx\n",handle.handle);


	}else if(!strcmp(argv[1],"discover"))
	{
		int i=0;
		iscsi_target *target = NULL,*tmp;
		//unsigned int i_ip = 0x0c01a8c0;
		//unsigned int t_ip = 0x0b01a8c0;

		unsigned int i_ip = ntohl(0xC0A8010C);
		unsigned int t_ip = ntohl(0xC0A8010B);
		if(argv[2])
		{
			inet_pton(AF_INET,argv[2],&i_ip);
		}
		if(argv[3])
		{
			inet_pton(AF_INET,argv[3],&t_ip);
		}
		printf("sending initiator_ip=0x%x target_ip=0x%x\n",i_ip,t_ip);
		int count=0;
		target = iscsi_discover_targets("iqn.2022111111111",i_ip,t_ip,3260,NULL,&count);
		for(i=0;i<count;i++)
		{
			tmp = (target + i);
			printf("target name=%s\n",tmp->name);
			printf("target ip=%x\n",tmp->ip);
			printf("target port=%x\n",tmp->port);
			printf("target pgtag=%x\n",tmp->pgtag);
			printf("\n\n");
		}

	}else if(!strcmp(argv[1],"logout"))
	{

                char initiator_name[256];
                char target_name[256]; 
                int pgtag=3;
                int rc;
                if(argv[2])
                {
                        strcpy(initiator_name,argv[2]);
                }else
                {
                        strcpy(initiator_name,"iqn.202300D0B2027C60");
                       
                }
                if(argv[3])
                {
                        strcpy(target_name,argv[3]);
                }else
                {
                        strcpy(target_name,"naa.202100D0B2027C60");
                }
                if(argv[4])
                {
                    pgtag = atoi(argv[4]); 
                }

		rc = iscsi_close_session(target_name,pgtag, initiator_name);
                printf("return value= %d\n",rc);


	}else if(!strcmp(argv[1],"sg"))
        {
            int sid=0;
            char filename[256];
            DIR *dir;
            struct dirent *entry;
            if(argv[2])
            {
                sid = atoi(argv[2]);
            }
            sprintf(filename,"/sys/class/scsi_host/host%d/"
                        "device/target%d:0:0/%d:0:0:%d",sid,sid,sid,0);
            dir = opendir(filename) ;
            if(dir == NULL)
            {
               printf("The file %s does not exist\n",filename);
               return 1;
            }
           while((entry = readdir(dir))!=NULL)
           {
               if(memcmp(entry->d_name,"scsi_generic:",13) == 0)
               {
                   sid = atoi(entry->d_name + 15);
                   printf("sg device is %d\n",sid);
               }
               printf("\n%s\n",entry->d_name); 
           }
        }else
        {
                print_usage();
        }

	return 1;
}

