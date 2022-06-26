#ifndef __SERVER_H__
#define __SERVER_H__

/******************************************************************************
 Local Constant definitions
 *****************************************************************************/
#define SERVER_PORT 		(9001)
#define BUFFER_SIZE 		(10000)
#define LISTEN_BACKLOG 		(5)
#define SV_SUCCESS			(0)
#define SV_FAILED			(-1)
#define DELIMITER			("END\n\n")

#define REQ_CMD			    ("HANDLE")
#define SEND_CMD			("SEND")
#define IMG_SIZE_LABEL		("BYTEUSED")
#define RESP_CMD		    ("OK")
#define RESP_MOTION_LABEL	("MOTION")
#define RESP_CODE_OK		(200)
#define RESP_CODE_FAILE 	(400)
#define RESP_CODE_SV_ERROR	(500)
#define IMGNUM_SEND			(15)
#define IMG_BUFFER_LEN		(786432)

/******************************************************************************
 Local Data type definitions
 *****************************************************************************/
typedef struct {
	int numImgs; 
} capture_imgs_t;

/******************************************************************************
 Function definitions
 *****************************************************************************/

#endif


