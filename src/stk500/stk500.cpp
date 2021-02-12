#include <stdio.h>
#include <iostream>
#include <vector>
#include <SDL.h>
#include "serial/serial.h"
#include "hex/IntelHexFile.h"
#include "stk500/stk500.h"

serial::Serial stk_serial;

std::vector<uint8_t> device_parameters;

#define PAGE_SIZE 126

std::vector<uint8_t> stk500_wait_response(uint8_t bytes)
{
    std::vector<uint8_t> r;
    for (int x = 0; x < bytes; x++)
    {
        r.push_back((uint8_t)stk_serial.read(1)[0]);
    }
    return r;
}

void stk500_write_bytes(std::vector<uint8_t> bytes)
{
    stk_serial.write(bytes);
}
void stk500_get_parameters()
{
    const uint8_t params_list[] = {
        PARAM_STK_HW_VER, //0
        PARAM_STK_SW_MAJOR, //1
        PARAM_STK_SW_MINOR, //2
        PARAM_STK_LEDS, //3
        PARAM_STK_VTARGET, //4
        PARAM_STK_VADJUST, //5
        PARAM_STK_OSC_PSCALE, //6
        PARAM_STK_OSC_CMATCH, //7
        PARAM_STK_RESET_DURATION, //8
        PARAM_STK_SCK_DURATION, //9
        PARAM_STK_BUFSIZEL, //10
        PARAM_STK_BUFSIZEH, //11
        PARAM_STK_DEVICE, //12
        PARAM_STK_PROGMODE, //13
        PARAM_STK_PARAMODE, //14
        PARAM_STK_POLLING, //15
        PARAM_STK_SELFTIMED //16
    };
    std::vector<uint8_t> r;
    for (int x = 0; x < 16; x++)
    {
        stk500_write_bytes({STK_GET_PARAMETER, params_list[x] ,CRC_EOP});
        SDL_Delay(50);
        r = stk500_wait_response(3);
        if (r[0] == STK_INSYNC && r[2] == STK_OK)
        {
            device_parameters.push_back(r[1]);
        }
        else if (r[2] == STK_FAILED)
        {
            printf("Failed getting paramater: %x\n", params_list[x]);
            return;
        }
        else if (r[0] == STK_NOSYNC)
        {
            printf("Failed getting paramater, no sync!\n");
            return;
        }
        r.clear();
    }
    printf("PARAM_STK_HW_VER : %d\n", (int)device_parameters[0]);
    printf("PARAM_STK_SW_MAJOR : %d\n", (int)device_parameters[1]);
    printf("PARAM_STK_BUFSIZEL : %d\n", (int)device_parameters[10]);
    printf("PARAM_STK_BUFSIZEH : %d\n", (int)device_parameters[11]);
    printf("PARAM_STK_DEVICE : %d\n", (int)device_parameters[12]);
}
void stk500_set_parameter(byte p)
{
    stk500_write_bytes({STK_SET_DEVICE, 
    STK_328P_DEVICE_CODE,
    STK_328P_DEVICE_REVISION,
    STK_328P_PROG_TYPE,
    STK_328P_PARALLELMODE,
    STK_328P_POLLING,
    STK_328P_SELF_TIMED,
    STK_328P_LOCK_BYTES,
    STK_328P_FUSE_BYTES,
    STK_328P_TIMEOUT,
    STK_328P_STAB_DELAY,
    STK_328P_CMD_EXEC_DELAY,
    STK_328P_SYNC_LOOPS,
    STK_328P_BYTE_DELAY,
    STK_328P_POLL_INDEX,
    STK_328P_POLL_VALUE,
    CRC_EOP});
}
bool stk500_write_program(const char *intelhex, const char *port)
{
    try
    {
        printf("stk500_write_program() Parsing Hex File: %s\n", intelhex);
        IntelHexFile file(intelhex);
        Program program = file.getProgram();	
        std::vector<ProgramPage> programPages = program.getPages(PAGE_SIZE);
        printf("stk500_write_program() Hex File Size: %lu, Pages: %lu\n", program.data.size(), programPages.size());
        auto timeout = serial::Timeout::simpleTimeout(1000);
        stk_serial.setTimeout(timeout);
        stk_serial.setPort(port);
        stk_serial.setBaudrate(115200);
        stk_serial.open();
        if (stk_serial.isOpen())
        {
            stk_serial.setDTR(true);
            stk_serial.setRTS(true);
            SDL_Delay(250);
            stk_serial.setDTR(false);
            stk_serial.setRTS(false);
            bool have_sync = false;
            for (int x = 0; x < 10; x++)
            {
                stk500_write_bytes({STK_GET_SYNC, CRC_EOP});
                std::vector<uint8_t> r = stk500_wait_response(2);
                if (r[0] == STK_INSYNC && r[1] == STK_OK)
                {
                    printf("Established sync!\n");
                    have_sync = true;
                    break;
                }
                SDL_Delay(250);
            }
            if (have_sync == true)
            {
                stk500_write_bytes({STK_READ_SIGN, CRC_EOP});
                std::vector<uint8_t> r = stk500_wait_response(1);
                if (r[0] == STK_INSYNC)
                {
                    std::vector<uint8_t> r = stk500_wait_response(4);
                    printf("Recieved signature: %x-%x-%x\n", r[0], r[1], r[2]);
                    r.clear();
                    stk500_get_parameters();
                }
                else
                {
                    printf("Did not recieve signature!\n");
                    stk_serial.close();
                    return false;
                }
            }
            else
            {
                printf("Could not establish sync!\n");
                stk_serial.close();
                return false;
            }
        }
        else
        {
            printf("Could not open %s\n", port);
            return false;
        }

        /*for (int x = 0; x < programPages.size(); x++)
        {
            std::cout << "Page: " << programPages[x] << std::endl;
        }*/
        return true;
    }
    catch(...)
    {
        printf("Could not open %s\n", port);
        return false;
    }
}