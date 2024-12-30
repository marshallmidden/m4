
int finish_io(int sg_fd, unsigned char **buffer, unsigned int *tag);
int inquiry(int handle, int vpd, int page, unsigned char *data);
void dispbuf(const unsigned char *buffer, int len);
int write_lba(int handle, unsigned int lba, unsigned int blckcount,
	unsigned char* datap, int tag);
int read_lba(int handle, unsigned int lba, unsigned int blckcount,
	unsigned char * datap, unsigned int tag);
unsigned int get_cdb_lba(unsigned char *cdb);
int readcap(int handle, unsigned char *data);
int log_sense(int handle, int page, unsigned char *data);
unsigned int ata_smart_enable(int handle);
unsigned int ata_smart_get_temp(int handle);
int read_buffer(int handle, unsigned char mode, unsigned char bufid,
	unsigned int offset, unsigned int length, unsigned char *datap,
	unsigned int tag);
int write_buffer(int handle, unsigned char mode, unsigned char bufid,
	unsigned int offset, unsigned int length, unsigned char *datap,
	unsigned int tag);
int do_scsi_io_wait(int sg_fd, const unsigned char *cdb, int cdblen,
	unsigned char *datap, int dir, int datalen);
 
