/*
 * pru_rc_lib.h
 *
 *  Created on: 06 apr 2018
 *      Author: andrea
 */

#ifndef PRU_RC_LIB_H_
#define PRU_RC_LIB_H_
#include <stdint.h>

/*
 * ECAP
 */
#define PRU_RC_LIB_ECCTL1_CFG       0xC1EE /* DIV1,ENABLED,DELTA_MODE, RISING/FALLING */
#define PRU_RC_LIB_ECCTL2_CFG       0x00DE /* RE-ARM, ECAP_MODE, RUN, SYNCO/I DISABLED, CONTINUOUS */
#define PRU_RC_LIB_ECEINT_CFG       0x0000 /* none */
#define PRU_RC_LIB_ECFLG_MSK        0x00FF /* none */
#define PRU_RC_LIB_ECCLR_MSK        0x00FF /* clear all */
#define PRU_RC_LIB_EC_STOP_MSK      0xFFEF /* stop ecap */
#define PRU_RC_LIB_NUM_CHANNELS     0x08   /* PPM 8 channels */
#define PRU_RC_LIB_MIN_RISE         90000  /* 200MHz => 200 cycles per us, 450 us => 200*450 = 90000 */
#define PRU_RC_LIB_MAX_RISE         500000 /* 200MHz => 200 cycles per us, 2500 us => 200*2500 = 500000 */

#define PRU_RC_LIB_CMD_ID             0xFA
#define PRU_RC_LIB_CMD_START          0x01
#define PRU_RC_LIB_CMD_START_RSP      0x02
#define PRU_RC_LIB_CMD_STOP           0x03
#define PRU_RC_LIB_CMD_STOP_RSP       0x04
#define PRU_RC_LIB_CMD_GET_DATA       0x05
#define PRU_RC_LIB_CMD_GET_DATA_RSP   0x06
#define PRU_RC_LIB_CMD_DATA_BUFF_SIZE 0x22 /* 34 bytes: 4 bytes * 8 channels + 2 bytes (id and cmd) */
#define PRU_RC_LIB_CMD_DATA_BUFF_OFF  0x2

typedef struct {
    uint8_t (*onStart)(unsigned char* data, uint8_t dataBytes);
    uint8_t (*onStop)(unsigned char* data, uint8_t dataBytes);
    uint8_t (*onGetData)(unsigned char* data, uint8_t dataBytes);
} PruRCLibConfig;

uint8_t pru_rc_lib_IsConfigured();
uint8_t pru_rc_lib_IsInitialized();
uint8_t pru_rc_lib_IsRunning();
uint8_t pru_rc_lib_Init();
uint8_t pru_rc_lib_Pulse();
uint8_t pru_rc_lib_Conf(PruRCLibConfig* config);
uint32_t* pru_rc_lib_GetData();
uint8_t pru_rc_lib_Start();
uint8_t pru_rc_lib_Stop();

uint8_t pru_rc_lib_IsCmdSupported(unsigned char* cmd, uint8_t numBytes);
uint8_t pru_rc_lib_ExecCmd(unsigned char* cmd, uint8_t numBytes);

#endif /* PRU_RC_LIB_H_ */
