/*
 * pru_rc_lib.c
 *
 *  Created on: 06 apr 2018
 *      Author: andrea
 */

#include <pru_rc_lib.h>
#include <pru_cfg.h>
#include <pru_ecap.h>

uint8_t pru_rc_lib_initialized = 0;
PruRCLibConfig* pru_rc_lib_config = 0;
int8_t pru_rc_lib_currentChannel = 0;
uint32_t pru_rc_lib_extractedData[8] = { 0 };
unsigned char pru_rc_lib_publishedBuffer[PRU_RC_LIB_CMD_DATA_BUFF_SIZE] = { 0 };
uint32_t* pru_rc_lib_publishedData =
        (uint32_t*) (pru_rc_lib_publishedBuffer + 2);
uint32_t pru_rc_lib_cap[4] = { 0 };
unsigned char pru_rc_lib_cmd_rsp[2] = { 0 };

uint8_t pru_rc_lib_IsConfigured()
{
    return (pru_rc_lib_config != 0);
}
uint8_t pru_rc_lib_IsInitialized()
{
    return pru_rc_lib_initialized;
}
uint8_t pru_rc_lib_IsRunning()
{
    return (CT_ECAP.ECCTL2 & 0x00000010);
}
uint8_t pru_rc_lib_IsCmdSupported(unsigned char* cmd, uint8_t numBytes)
{
    return (numBytes > 1) && (cmd[0] == PRU_RC_LIB_CMD_ID)
            && (cmd[1] == PRU_RC_LIB_CMD_START)
            || (cmd[1] == PRU_RC_LIB_CMD_STOP)
            || (cmd[1] == PRU_RC_LIB_CMD_GET_DATA);
}
uint8_t pru_rc_lib_Init()
{
    if (!pru_rc_lib_initialized)
    {
        // Disabilito ed azzero interrupts
        CT_ECAP.ECEINT = 0x00;
        CT_ECAP.ECCTL2 &= PRU_RC_LIB_EC_STOP_MSK; // Stop ecap
        CT_ECAP.ECCLR &= PRU_RC_LIB_ECCLR_MSK;

        // Abilito interrupts
        CT_ECAP.ECEINT = PRU_RC_LIB_ECEINT_CFG;

        // Configure & start ecap
        CT_ECAP.ECCTL1 = PRU_RC_LIB_ECCTL1_CFG; // all rising edge, reset counter at any capture
        CT_ECAP.ECCTL2 = PRU_RC_LIB_ECCTL2_CFG & PRU_RC_LIB_EC_STOP_MSK; // continuous, capture mode, wrap after capture 4, rearm, free running,synci/o disabled
        pru_rc_lib_initialized = 1;
        return pru_rc_lib_Start();

    }
    return 1;
}
uint8_t pru_rc_lib_Start()
{
    if (!pru_rc_lib_IsRunning())
    {
        CT_ECAP.TSCTR = 0x00000000;
        CT_ECAP.ECCTL2 = PRU_RC_LIB_ECCTL2_CFG; // start ecap
    }
    return 1;
}
uint8_t pru_rc_lib_Stop()
{
    if (pru_rc_lib_IsRunning())
    {
        CT_ECAP.ECCTL2 &= 0xFFEF; // stop ecap
    }
    return 1;
}

uint8_t pru_rc_lib_ExecCmd(unsigned char* cmd, uint8_t numBytes)
{
    if (cmd[0] == PRU_RC_LIB_CMD_ID)
    {
        switch (cmd[1])
        {
        case (PRU_RC_LIB_CMD_START):
        {
            pru_rc_lib_cmd_rsp[0] = PRU_RC_LIB_CMD_ID;
            pru_rc_lib_cmd_rsp[1] = PRU_RC_LIB_CMD_START_RSP;
            return pru_rc_lib_Start()
                    && (pru_rc_lib_config->onStart)(pru_rc_lib_cmd_rsp, 2);
        }
        case (PRU_RC_LIB_CMD_STOP):
        {
            pru_rc_lib_cmd_rsp[0] = PRU_RC_LIB_CMD_ID;
            pru_rc_lib_cmd_rsp[1] = PRU_RC_LIB_CMD_STOP_RSP;
            return pru_rc_lib_Stop()
                    && (pru_rc_lib_config->onStop)(pru_rc_lib_cmd_rsp, 2);
        }
        case (PRU_RC_LIB_CMD_GET_DATA):
        {
            pru_rc_lib_publishedBuffer[0] = PRU_RC_LIB_CMD_ID;
            pru_rc_lib_publishedBuffer[1] = PRU_RC_LIB_CMD_GET_DATA_RSP;
            return (pru_rc_lib_config->onGetData)(pru_rc_lib_publishedBuffer,
                                                  PRU_RC_LIB_CMD_DATA_BUFF_SIZE);
        }
        }
    }
    return 0;
}

uint8_t pru_rc_lib_ExtractData()
{

    if (pru_rc_lib_IsRunning())
    {
        if (CT_ECAP.ECFLG & 0x0002)
        {
            pru_rc_lib_cap[0] = CT_ECAP.CAP1_bit.CAP1;
            pru_rc_lib_cap[1] = CT_ECAP.CAP2_bit.CAP2;
            pru_rc_lib_cap[2] = CT_ECAP.CAP3_bit.CAP3;
            pru_rc_lib_cap[3] = CT_ECAP.CAP4_bit.CAP4;
            CT_ECAP.ECCLR |= PRU_RC_LIB_ECCLR_MSK; // remove EVT4 interrupt and INT
            uint8_t i = 0;
            for (i = 0; i < 4; i++)
            {
                if ((PRU_RC_LIB_MIN_RISE < pru_rc_lib_cap[i])
                        && (pru_rc_lib_cap[i] < PRU_RC_LIB_MAX_RISE)
                        && (pru_rc_lib_currentChannel < PRU_RC_LIB_NUM_CHANNELS))
                {
                    pru_rc_lib_extractedData[pru_rc_lib_currentChannel++] =
                            pru_rc_lib_cap[i];
                }
                else if (PRU_RC_LIB_MAX_RISE <= pru_rc_lib_cap[i])
                {
                    if (pru_rc_lib_currentChannel == PRU_RC_LIB_NUM_CHANNELS)
                    {
                        uint8_t j = 0;
                        for (j = 0; j < PRU_RC_LIB_NUM_CHANNELS; j++)
                        {
                            pru_rc_lib_publishedData[j] =
                                    pru_rc_lib_extractedData[j];
                        }
                    }
                    pru_rc_lib_currentChannel = 0;
                }
            }
        }
    }
    return 1;
}

uint8_t pru_rc_lib_Pulse()
{
    if (pru_rc_lib_IsConfigured())
    {
        if (!pru_rc_lib_IsInitialized())
        {
            if (!pru_rc_lib_Init())
            {
                return 0;
            }
        }
        else
        {
            return pru_rc_lib_ExtractData();
        }
    }
    else
    {
        return 0;
    }
    return 1;
}
uint32_t* pru_rc_lib_GetData()
{
    return pru_rc_lib_publishedData;
}

uint8_t pru_rc_lib_Conf(PruRCLibConfig* config)
{
    pru_rc_lib_config = config;
    return 1;
}
