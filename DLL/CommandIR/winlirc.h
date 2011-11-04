#define NULL 0

/* Import CommandIR Utils interface functions from C world */
typedef int lirc_t;

//extern int hardware_scan(void);
extern void addWinLIRCRxData(int snum);
// // // extern void initForWinLIRC(lirc_t * cppDataBufferStart, unsigned char * cppBufferEnd, unsigned char * cppBufferStart);
// // // extern int check_commandir_rec(void);
//	extern void commandir_sendir(struct sendir * tx);
//	extern void add_to_tx_pipeline(unsigned char *buffer, int bytes, unsigned int frequency);
// // // void send_lirc_buffer(unsigned char *buffer, int bytes, unsigned int frequency);
extern struct commandir_device *first_commandir_device = NULL;
extern void commandir_send_specific(struct sendir * tx, struct commandir_device * pcd);
void setup_libcmdir_winlirc(void);
void lirc_pipe_write( int * data );

#define DEFAULT_FREQ 38000


