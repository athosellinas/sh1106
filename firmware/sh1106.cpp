//
//  sh1106.cpp
//
//
//  Created by Daniel Milligan on 8/3/14.
//  Adapted for Spark by Dustin Sellinger, April 2015.
//
//#include <stdio.h>

// Pull in the wire library to communicate with the LCD over I2C
//#include <Wire.h>

#include "application.h"

#include "sh1106.h"

typedef enum
{
    HORIZONTAL = 0,
    VERTICAL,
    PAGE,
    INVALID,
    END_MEMORY_ADDRESSING_MODES
} MEMORY_ADDRESSING_MODES;

typedef enum
{
    Success = 0,
    DataTooLong,
    NackAddress,
    NackData,
    OtherError,
    END_I2C_STATUS
} I2CStatus;

// Each character will be an 8x6 character with one space blank to the left and bottom
#define CHARACTER_WIDTH 6

//ASCII Table
byte ASCII[][CHARACTER_WIDTH]=
{
{0x00,0x00,0x00,0x00,0x00,0x00},   //   0x20 32
{0x00,0x00,0x00,0x6f,0x00,0x00},   // ! 0x21 33
{0x00,0x00,0x07,0x00,0x07,0x00},   // " 0x22 34
{0x00,0x14,0x7f,0x14,0x7f,0x14},   // # 0x23 35
{0x00,0x24,0x2A,0x7F,0x2A,0x12},   // $ 0x24 36
{0x00,0x23,0x13,0x08,0x64,0x62},   // % 0x25 37
{0x00,0x36,0x49,0x56,0x20,0x50},   // & 0x26 38
{0x00,0x00,0x00,0x07,0x00,0x00},   // ' 0x27 39
{0x00,0x00,0x1c,0x22,0x41,0x00},   // ( 0x28 40
{0x00,0x00,0x41,0x22,0x1c,0x00},   // ) 0x29 41
{0x00,0x14,0x08,0x3e,0x08,0x14},   // * 0x2a 42
{0x00,0x08,0x08,0x3e,0x08,0x08},   // + 0x2b 43
{0x00,0x00,0x50,0x30,0x00,0x00},   // , 0x2c 44
{0x00,0x08,0x08,0x08,0x08,0x08},   // - 0x2d 45
{0x00,0x00,0x60,0x60,0x00,0x00},   // . 0x2e 46
{0x00,0x20,0x10,0x08,0x04,0x02},   // / 0x2f 47
{0x00,0x3e,0x51,0x49,0x45,0x3e},   // 0 0x30 48
{0x00,0x00,0x42,0x7f,0x40,0x00},   // 1 0x31 49
{0x00,0x42,0x61,0x51,0x49,0x46},   // 2 0x32 50
{0x00,0x21,0x41,0x45,0x4b,0x31},   // 3 0x33 51
{0x00,0x18,0x14,0x12,0x7f,0x10},   // 4 0x34 52
{0x00,0x27,0x45,0x45,0x45,0x39},   // 5 0x35 53
{0x00,0x3c,0x4a,0x49,0x49,0x30},   // 6 0x36 54
{0x00,0x01,0x71,0x09,0x05,0x03},   // 7 0x37 55
{0x00,0x36,0x49,0x49,0x49,0x36},   // 8 0x38 56
{0x00,0x06,0x49,0x49,0x29,0x1e},   // 9 0x39 57
{0x00,0x00,0x36,0x36,0x00,0x00},   // : 0x3a 58
{0x00,0x00,0x56,0x36,0x00,0x00},   // ; 0x3b 59
{0x00,0x08,0x14,0x22,0x41,0x00},   // < 0x3c 60
{0x00,0x14,0x14,0x14,0x14,0x14},   // = 0x3d 61
{0x00,0x00,0x41,0x22,0x14,0x08},   // > 0x3e 62
{0x00,0x02,0x01,0x51,0x09,0x06},   // ? 0x3f 63
{0x00,0x3e,0x41,0x5d,0x49,0x4e},   // @ 0x40 64
{0x00,0x7e,0x09,0x09,0x09,0x7e},   // A 0x41 65
{0x00,0x7f,0x49,0x49,0x49,0x36},   // B 0x42 66
{0x00,0x3e,0x41,0x41,0x41,0x22},   // C 0x43 67
{0x00,0x7f,0x41,0x41,0x41,0x3e},   // D 0x44 68
{0x00,0x7f,0x49,0x49,0x49,0x41},   // E 0x45 69
{0x00,0x7f,0x09,0x09,0x09,0x01},   // F 0x46 70
{0x00,0x3e,0x41,0x49,0x49,0x7a},   // G 0x47 71
{0x00,0x7f,0x08,0x08,0x08,0x7f},   // H 0x48 72
{0x00,0x00,0x41,0x7f,0x41,0x00},   // I 0x49 73
{0x00,0x20,0x40,0x41,0x3f,0x01},   // J 0x4a 74
{0x00,0x7f,0x08,0x14,0x22,0x41},   // K 0x4b 75
{0x00,0x7f,0x40,0x40,0x40,0x40},   // L 0x4c 76
{0x00,0x7f,0x02,0x0c,0x02,0x7f},   // M 0x4d 77
{0x00,0x7f,0x04,0x08,0x10,0x7f},   // N 0x4e 78
{0x00,0x3e,0x41,0x41,0x41,0x3e},   // O 0x4f 79
{0x00,0x7f,0x09,0x09,0x09,0x06},   // P 0x50 80
{0x00,0x3e,0x41,0x51,0x21,0x5e},   // Q 0x51 81
{0x00,0x7f,0x09,0x19,0x29,0x46},   // R 0x52 82
{0x00,0x46,0x49,0x49,0x49,0x31},   // S 0x53 83
{0x00,0x01,0x01,0x7f,0x01,0x01},   // T 0x54 84
{0x00,0x3f,0x40,0x40,0x40,0x3f},   // U 0x55 85
{0x00,0x0f,0x30,0x40,0x30,0x0f},   // V 0x56 86
{0x00,0x3f,0x40,0x30,0x40,0x3f},   // W 0x57 87
{0x00,0x63,0x14,0x08,0x14,0x63},   // X 0x58 88
{0x00,0x07,0x08,0x70,0x08,0x07},   // Y 0x59 89
{0x00,0x61,0x51,0x49,0x45,0x43},   // Z 0x5a 90
{0x00,0x3c,0x4a,0x49,0x29,0x1e},   // [ 0x5b 91
{0x00,0x02,0x04,0x08,0x10,0x20},   // \ 0x5c 92
{0x00,0x00,0x41,0x7f,0x00,0x00},   // ] 0x5d 93
{0x00,0x04,0x02,0x01,0x02,0x04},   // ^ 0x5e 94
{0x00,0x40,0x40,0x40,0x40,0x40},   // _ 0x5f 95
{0x00,0x00,0x00,0x03,0x04,0x00},   // ` 0x60 96
{0x00,0x20,0x54,0x54,0x54,0x78},   // a 0x61 97
{0x00,0x7f,0x48,0x44,0x44,0x38},   // b 0x62 98
{0x00,0x38,0x44,0x44,0x44,0x20},   // c 0x63 99
{0x00,0x38,0x44,0x44,0x48,0x7f},   // d 0x64 100
{0x00,0x38,0x54,0x54,0x54,0x18},   // e 0x65 101
{0x00,0x08,0x7e,0x09,0x01,0x02},   // f 0x66 102
{0x00,0x0c,0x52,0x52,0x52,0x3e},   // g 0x67 103
{0x00,0x7f,0x08,0x04,0x04,0x78},   // h 0x68 104
{0x00,0x00,0x44,0x7d,0x40,0x00},   // i 0x69 105
{0x00,0x20,0x40,0x44,0x3d,0x00},   // j 0x6a 106
{0x00,0x00,0x7f,0x10,0x28,0x44},   // k 0x6b 107
{0x00,0x00,0x41,0x7f,0x40,0x00},   // l 0x6c 108
{0x00,0x7c,0x04,0x18,0x04,0x78},   // m 0x6d 109
{0x00,0x7c,0x08,0x04,0x04,0x78},   // n 0x6e 110
{0x00,0x38,0x44,0x44,0x44,0x38},   // o 0x6f 111
{0x00,0x7c,0x14,0x14,0x14,0x08},   // p 0x70 112
{0x00,0x08,0x14,0x14,0x18,0x7c},   // q 0x71 113
{0x00,0x7c,0x08,0x04,0x04,0x08},   // r 0x72 114
{0x00,0x48,0x54,0x54,0x54,0x20},   // s 0x73 115
{0x00,0x04,0x3f,0x44,0x40,0x20},   // t 0x74 116
{0x00,0x3c,0x40,0x40,0x20,0x7c},   // u 0x75 117
{0x00,0x1c,0x20,0x40,0x20,0x1c},   // v 0x76 118
{0x00,0x3c,0x40,0x30,0x40,0x3c},   // w 0x77 119
{0x00,0x44,0x28,0x10,0x28,0x44},   // x 0x78 120
{0x00,0x0c,0x50,0x50,0x50,0x3c},   // y 0x79 121
{0x00,0x44,0x64,0x54,0x4c,0x44},   // z 0x7a 122
{0x00,0x00,0x08,0x36,0x41,0x41},   // { 0x7b 123
{0x00,0x00,0x00,0x7f,0x00,0x00},   // | 0x7c 124
{0x00,0x41,0x41,0x36,0x08,0x00},   // } 0x7d 125
{0x00,0x04,0x02,0x04,0x08,0x04},   // ~ 0x7e 126
};


sh1106_lcd *sh1106_lcd::m_pInstance = NULL;

sh1106_lcd *sh1106_lcd::getInstance()
{
    if (m_pInstance == NULL)
    {
        m_pInstance = (sh1106_lcd *)malloc(sizeof(sh1106_lcd));
        m_pInstance->Initialize();
    }
    return m_pInstance;
}

void sh1106_lcd::Initialize()
{
    Wire.begin(); // begin without address as a master device

    SendCommand(0xAE, StartSend); //DISPLAY_OFF

    SendCommand(0x02, MidSend); //LOW_COLUMN_ADDRESS
    SendCommand(0x10, MidSend); //HIGH_COLUMN_ADDRESS

    SendCommand(0x40, MidSend); //START_LINE_ADDRESS

    SendCommand(0xB0, MidSend); //SET_PAGE_ADDRESS start at page address 0 up to 7

    SendCommand(0x81, MidSend); //SET_CONTRAST_CTRL_REG
    SendCommand(0x80, MidSend); //CONTRAST VALUE *128

    SendCommand(0xA1, MidSend); //SET_SEGMENT_REMAP

    SendCommand(0xA6, MidSend); //SET_NORMAL_DISPLAY (0xA7 to invert)
    SendCommand(0xA8, MidSend); //SET_MULTIPLEX_RATIO
    SendCommand(0x3F, MidSend); //duty cycle = 1/32

    SendCommand(0xAD, MidSend); //set charge pump enable
	     /****  From docs  ****
	     Set DC-DC OFF/ON: (Double Bytes Command)
         This command is to control the DC-DC voltage converter. The converter will be turned on by issuing this command then display ON command. The panel display must be off while issuing this command.
         * DC-DC Control Mode Set: (ADH) */
    SendCommand(0x8B, MidSend); //external VCC on / 0x8A Disabled

    SendCommand(0x30, MidSend); // 0X30---0X33  set VPP  9V liangdu!!!!  (wtf does that mean?)

    SendCommand(0xC8, MidSend); //SET_COM_OUTPUT_SCAN_DIRECTION

    SendCommand(0xD3, MidSend); //SET_DISPLAY_OFFSET
    SendCommand(0x00, MidSend); // no offset or 0x20)

    SendCommand(0xD5, MidSend); //SET_DISPLAY_CLOCK_DIVIDE / osc division
    SendCommand(0x80, MidSend); // Osc Freq 0x80, 0xF0

    SendCommand(0xD9, MidSend); //SET_PRE_CHARGE_PERIOD
    SendCommand(0x1F, MidSend); // 0x1F, or 0x22

    SendCommand(0xDA, MidSend); //SET_COM_PINS_HARDWARE_CONFIG
    SendCommand(0x12, MidSend);

    SendCommand(0xDB, MidSend); //SET_VCOMH
    SendCommand(0x40, MidSend); // 0.77xVcc = 0x20  (or 0x40)  VCOM  = � X V REF = (0.430 + A[7:0] X 0.006415) X VREF

    SendCommand(0xAF, FinishSend); //DISPLAY_ON

/*****  Possibly unneeded controls ***
    SendCommand(0x20, MidSend); //SET_MEMORY_ADDRESSING_MODE
    SendCommand(0x02, MidSend);  //PAGE (was undefined)

    SendCommand(0x7F, MidSend);  //Display Startline

    SendCommand(0xA4, MidSend); //OUTPUT_FOLLOWS_RAM

    SendCommand(0x8D, MidSend); //SET_DC_DC_ENABLE  (Docs say this is entire display on/off)
    SendCommand(0x14, MidSend); //enable, 0x10=disable
*********************************/

    memset(m_screen, 0, SCREEN_WIDTH * MAX_PAGE_COUNT);
    m_currentLine = 0;
    m_cursor = 0;
}

void sh1106_lcd::Show()
{
    SendState state = StartSend;

    for (int index = 0; index < MAX_PAGE_COUNT; index++)
    {
        SendCommand(SET_PAGE_ADDRESS + index, StartSend);
        SendCommand(0x02, MidSend); // low column start address
        SendCommand(0x10, FinishSend); // high column start address
        for (int pixel = 0; pixel < SCREEN_WIDTH; pixel++)
        {
            SendData(m_screen[index][pixel]);

            if (state == StartSend)
            {
                state = MidSend;
            }
            else if (pixel == (SCREEN_WIDTH - 2))
            {
                state = FinishSend;
            }
        }
        state = StartSend;
    }
}

void sh1106_lcd::FillScreen(byte fillData)
{
    for (int index = 0; index < MAX_PAGE_COUNT; index++)
    {
        for (int pixel = 0; pixel < SCREEN_WIDTH; pixel++)
        {
            m_screen[index][pixel] = fillData;
        }
    }

    m_currentLine = 0;
    m_cursor = 0;

    Show();
}

void sh1106_lcd::ClearScreen()
{
   FillScreen(0x00);
}

/*
 * x can be from 0 to 131
 * y can be from 0 to 63
 *
 * Show must be called after all pixel's are drawn
 */
void sh1106_lcd::DrawPixel(byte x, byte y, bool on)
{
    byte pageId = y / MAX_PAGE_COUNT;       // convert from byte to page
    byte bitOffset = y % MAX_PAGE_COUNT;    // establish the bit offset

    if (pageId < MAX_PAGE_COUNT)
    {
        if (x < SCREEN_WIDTH)
        {
            if (on == true)
            {
                m_screen[pageId][x] |= 1 << bitOffset; // turn this bit on
            }
            else
            {
                m_screen[pageId][x] &= ~(1 << bitOffset); // turn this bit off
            }
        }
    }
}

void sh1106_lcd::DrawRectangle(byte x1, byte y1, byte x2, byte y2)
{
    byte pageId1 = y1 / MAX_PAGE_COUNT;
    byte pageId2 = y2 / MAX_PAGE_COUNT;
    byte bit1 = 1 << (y1 % MAX_PAGE_COUNT);
    byte bit2 = 1 << (y2 % MAX_PAGE_COUNT);

    if ((pageId1 < MAX_PAGE_COUNT) && (pageId2 < MAX_PAGE_COUNT))
    {
        if (x1 >= SCREEN_WIDTH)
        {
            x1 = SCREEN_WIDTH - 1;
        }

        if (x2 >= SCREEN_WIDTH)
        {
            x2 = SCREEN_WIDTH - 1;
        }

        if (x1 == x2)
        {
            x2++; // give at least one
        }
        // Sets top and bottom line
        for (byte xCord = x1; xCord < x2; xCord++)
        {
            m_screen[pageId1][xCord] |= bit1;
            m_screen[pageId2][xCord] |= bit2;
        }

        if (y2 < y1)
        {
            byte temp = y1;
            y1 = y2;
            y2 = temp;
        }

        // Sets left and right line
        for (/* set above */; y1 < y2; y1++)
        {
            m_screen[pageId1][x1] |= bit1;
            m_screen[pageId1][x2] |= bit1;
            bit1 <<= 1;
            if (bit1 == 0)
            {
                bit1++;

                pageId1++; // move to next page we just rolled

                if (pageId1 >= MAX_PAGE_COUNT)
                {
                    break;
                }
            }
        }
    }
}

void sh1106_lcd::DrawRectangle(byte x1, byte y1, byte x2, byte y2, byte thickness)
{
    // Cheat and call it a couple of times
    // otherwise my head hurts tring to get the bits right...
    for (byte index = 0; index < thickness; index++)
    {
        DrawRectangle(x1, y1, x2, y2);
        x1++;
        x2--;
        y1++;
        y2--;

        if (x1 >= SCREEN_WIDTH || x2 >= SCREEN_WIDTH || y1 >= SCREEN_HEIGHT || y2 >= SCREEN_HEIGHT)
        {
            break;
        }
    }
}

void sh1106_lcd::FillRectangle(byte x1, byte y1, byte x2, byte y2)
{
    byte pageId1 = y1 / MAX_PAGE_COUNT;
    byte pageId2 = y2 / MAX_PAGE_COUNT;
    byte bit1 = 1 << (y1 % MAX_PAGE_COUNT);
    byte bit2 = 1 << (y2 % MAX_PAGE_COUNT);

    if ((pageId1 < MAX_PAGE_COUNT) && (pageId2 < MAX_PAGE_COUNT))
    {
        if (x1 >= SCREEN_WIDTH)
        {
            x1 = SCREEN_WIDTH - 1;
        }

        if (x2 >= SCREEN_WIDTH - 1)
        {
            x2 = SCREEN_WIDTH;
        }

        if (x1 == x2)
        {
            x2++;
        }

        if (y2 < y1)
        {
            byte temp = y1;
            y1 = y2;
            y2 = temp;
        }

        for (/* set above */; y1 < y2; y1++)
        {
            for (byte xCord = x1; xCord < x2; xCord++)
            {
                m_screen[pageId1][xCord] |= bit1;
            }

            bit1 <<= 1;
            if (bit1 == 0)
            {
                bit1++;

                pageId1++; // move to next page we just rolled

                if (pageId1 >= MAX_PAGE_COUNT)
                {
                    break;
                }
            }
        }
    }
}

void sh1106_lcd::DrawLine(byte x1, byte y1, byte x2, byte y2)
{
    byte pageId1 = y1 / MAX_PAGE_COUNT;
    byte pageId2 = y2 / MAX_PAGE_COUNT;
    byte bit1 = 1 << (y1 % MAX_PAGE_COUNT);
    byte bit2 = 1 << (y2 % MAX_PAGE_COUNT);
    byte dy = y2 - y1;
    byte dx = x2 - x1;

    if (x1 > x2)
    {
        dx = x1 - x2;

        byte xtemp = x1;
        x1 = x2;
        x2 = xtemp;
    }

    if ((pageId1 < MAX_PAGE_COUNT) && (pageId2 < MAX_PAGE_COUNT))
    {
        if (x1 >= SCREEN_WIDTH)
        {
            x1 = SCREEN_WIDTH - 1;
        }

        if (x2 >= SCREEN_WIDTH)
        {
            x2 = SCREEN_WIDTH - 1;
        }

        if (x1 == x2)
        {
            x2++; // give us one pixel width
        }

        /* Utilizing the Bresenham algorithm */
        int eps = 0;

        if (y1 < y2)
        {
            for (/* set above */; y1 < y2; y1++)
            {
                for (byte xCord = x1; xCord < x2; xCord++)
                {
                    m_screen[pageId1][xCord] |= bit1;

                    eps += dy;

                    if ((eps << 1) >= dx)
                    {
                        y1++;
                        bit1 <<= 1;
                        if (bit1 == 0)
                        {
                            pageId1++; // move to next page we just rolled

                            bit1++; // back to bit 1 equal to 1

                            if (pageId1 >= MAX_PAGE_COUNT)
                            {
                                break; // done
                            }
                        }
                        eps -= dx;
                    }
                }
            }
        }
        else if (y1 == y2)
        {
            for (byte xCord = x1; xCord < x2; xCord++)
            {
                m_screen[pageId1][xCord] |= bit1;
            }
        }
        else
        {
            dy = y1 - y2;
            for (/* set above */; y1 > y2; y1--)
            {
                for (byte xCord = x1; xCord < x2; xCord++)
                {
                    m_screen[pageId2][xCord] |= bit2;

                    eps += dy;

                    if ((eps << 1) >= dx)
                    {
                        y1--;
                        bit2 >>= 1;

                        if (bit2 == 0)
                        {
                            pageId2--; // move to next page we just rolled

                            bit2 = 0x80; // high order bit set and then will shift down to 0

                            // if it rolled around
                            if (pageId2 > MAX_PAGE_COUNT)
                            {
                                break; // done
                            }
                        }
                        eps -= dx;
                    }
                }
            }
        }
    }
}

void sh1106_lcd::Print(char *data)
{
    PrintData(data, false);
    Show();
}

void sh1106_lcd::PrintLine(char *data)
{
    PrintData(data, true);
    Show();
}

void sh1106_lcd::PrintData(char *data, bool incrementLine)
{
    bool eol = false;
    int index = 0;
    int lineIndex = 0;

    // time to scroll
    if (m_currentLine == MAX_PAGE_COUNT)
    {
        while (lineIndex < (MAX_PAGE_COUNT - 1))
        {
            // Copy the lines up scrolling off the top
            memcpy(m_screen[lineIndex], m_screen[lineIndex + 1], SCREEN_WIDTH);
            lineIndex++;
        }
        // Line index is now MAX_PAGE_COUNT - 1
        memset(m_screen[lineIndex], 0, SCREEN_WIDTH); // clear this line
    }
    else
    {
        lineIndex = m_currentLine;
    }

    while (eol == false)
    {
        if (data[index] != NULL)
        {
            byte *pPtr = NULL;
            byte dataByte = data[index++];

            if (dataByte < 0x20 || dataByte > 0x7E) pPtr = (byte*)&ASCII[0x3F-0x20];  //Give a "?" if out of range

            dataByte -= 0x20;
            pPtr = (byte*)&ASCII[dataByte];

            if (pPtr != NULL)
            {
                for (int numberIndex = 0; numberIndex < CHARACTER_WIDTH; numberIndex++)
                {
                    if (m_cursor < SCREEN_WIDTH)
                    {
                        m_screen[lineIndex][m_cursor++] = pPtr[numberIndex];
                    }
                    else
                    {
                        eol = true;
                        break; // get out
                    }
                }
            }
        }
        else
        {
            eol = true; // done
        }
    }

    if (incrementLine == true)
    {
        if (m_currentLine < MAX_PAGE_COUNT)
        {
            m_currentLine++;
        }
        m_cursor = 0;
    }
}

byte sh1106_lcd::SendCommand(byte command, SendState state)
{
    if (state == StartSend || state == Complete)
    {
        Wire.beginTransmission(SH1106_ADDR1);
        Wire.write(SH1106_COMMAND);
    }

    return SendByte(command, state);
}

byte sh1106_lcd::SendData(byte data, SendState state)
{
    if (state == StartSend || state == Complete)
    {
        Wire.beginTransmission(SH1106_ADDR1);
        Wire.write(SH1106_DATA);
    }

    return SendByte(data, state);
}

byte sh1106_lcd::SendByte(byte data, SendState state)
{
    Wire.write(data);

    byte transmissionStatus = 0;

    if (state == FinishSend || state == Complete)
    {
        transmissionStatus = Wire.endTransmission();
    }

    return transmissionStatus;
}
