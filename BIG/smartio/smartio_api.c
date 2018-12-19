/* Copyright 2017 ImageCraft Creations Inc., All rights reserved.
 * Smart.IO Host Interface Layer
 * https://imagecraft.com/smartio/
 */
#define VERSION     "1.02"

#include <stdarg.h>
#include "smartio_api.h"
#include "smartio_interface.h"
#include "smartio_hardware_interface.h"

uint32_t host_sram_aligned_block[HOST_SRAM_POOL_SIZE/sizeof(uint32_t)];
unsigned char *host_sram_block = (unsigned char *)&host_sram_aligned_block[0];


#define CALLBACK(h, cb, argtype)    if (cb) SmartIO__RegisterCallback(h, (void (*)())cb, argtype);

// 0b01 uint16_t
// 0b10 char *
// 0b11 uint32_t
#define _1ARG       0b0000000000000001
#define _2ARGS      0b0000000000000101
#define _3ARGS      0b0000000000010101
#define _4ARGS      0b0000000001010101
#define _5ARGS      0b0000000101010101
#define _6ARGS      0b0000010101010101
#define _7ARGS      0b0001010101010101
#define _8ARGS      0b0101010101010101

#define STRARG(i)   (0b10 << (i-1)*2)

#define SmartIO__SendCommand(cmdbuf, cmdlen)    SmartIO__SendBytes((unsigned char *)cmdbuf, cmdlen * sizeof (uint16_t))

static uint16_t *Command = (uint16_t *)&host_sram_aligned_block[0];

static uint32_t APICall(unsigned argmask, ...);

char *SmartIO_GetVersion(void)
    {
    return VERSION;
    }

tStatus SmartIO_LoadCache(uint32_t product_id, uint16_t build_id)
    {
    return APICall(_4ARGS, 71, product_id & 0xFFFF, product_id >> 16, build_id);
    }

tStatus SmartIO_SaveCache(void)
    {
    return APICall(_1ARG, 70);
    }

tStatus SmartIO_AppTitle(char *title)
    {
    return APICall(_1ARG|STRARG(2), 1, title);
    }

tHandle SmartIO_AddMenu(char *item, void (*callback)(uint16_t))
    {
    tHandle h = APICall(_1ARG|STRARG(2), 2, item);
    CALLBACK(h, callback, SMARTIO_ARG_ONE);
    return h;
    }

tStatus SmartIO_Reset(void)
    {
    return APICall(_1ARG, 5);
    }

tHandle SmartIO_MakePage(void)
    {
    return APICall(_1ARG, 50);
    }

tStatus SmartIO_SetCurrentPage(tHandle page_id)
    {
    return APICall(_2ARGS, 51, page_id);
    }

tStatus SmartIO_LockPage(tHandle handle)
    {
    return APICall(_2ARGS, 53, handle);
    }

tStatus SmartIO_UnlockPage(void)
    {
    return APICall(_1ARG, 54);
    }

tStatus SmartIO_PageTitle(tHandle page_id, char *title)
    {
    return APICall(_2ARGS|STRARG(3), 52, page_id, title);
    }

tStatus SmartIO_PopupAlert(uint16_t variation)
    {
    return APICall(_2ARGS, 24, variation);
    }

tStatus SmartIO_GroupObjects(uint16_t option, tHandle h1, ...)
    {
    int index;
    tStatus status;
    tHandle h;
    va_list args;
    va_start(args, h1);

    Command[0] = 32;
    Command[1] = option;
    index = 2;
    h = h1;
    do
        {
        Command[index++] = h;
        h = va_arg(args, unsigned);
        } while (h != 0);

    return SmartIO__SendCommand(Command, index);
    }

// up to 8 dependent handles. End with a 0 null handle (not included as part of the 8)
tStatus SmartIO_EnableIf(tHandle control, tHandle h1, ...)
    {
    int index;
    tHandle h;
    tStatus status;
    va_list args;
    va_start(args, h1);

    Command[0] = 44;
    Command[1] = control;
    index = 2;
    h = h1;
    do
        {
        Command[index++] = h;
        h = va_arg(args, unsigned);
        } while (h != 0);

    return SmartIO__SendCommand(Command, index);
    }

tStatus SmartIO_StopResume(void)
    {
    return APICall(_1ARG, 45);
    }

tHandle SmartIO_MakeOnOffButton(uint16_t alignment, uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t))
    {
    tHandle h = APICall(_5ARGS, 10, alignment, 200, variation, initial_value);
    CALLBACK(h+1, callback, SMARTIO_ARG_ONE);
    return h;
    }

tHandle SmartIO_Make3PosButton(uint16_t alignment, uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t))
    {
    tHandle h = APICall(_5ARGS, 10, alignment, 201, variation, initial_value);
    CALLBACK(h+1, callback, SMARTIO_ARG_ONE);
    return h;
    }

tHandle SmartIO_MakeIncrementer(uint16_t alignment, uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t))
    {
    tHandle h = APICall(_5ARGS, 10, alignment, 202, variation, initial_value);
    CALLBACK(h+1, callback, SMARTIO_ARG_ONE);
    return h;
    }

tHandle SmartIO_MakeSlider(uint16_t alignment, uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t))
    {
    tHandle h = APICall(_5ARGS, 10, alignment, 203, variation, initial_value);
    CALLBACK(h+1, callback, SMARTIO_ARG_ONE);
    return h;
    }

tHandle SmartIO_MakeExpandableList(uint16_t alignment, uint16_t nentries, void (*callback)(uint16_t))
    {
    tHandle h = APICall(_4ARGS, 10, alignment, 204, nentries);
    CALLBACK(h+1, callback, SMARTIO_ARG_ONE);
    return h;
    }

tHandle SmartIO_MakePicker(uint16_t alignment, uint16_t nentries, void (*callback)(uint16_t))
    {
    tHandle h = APICall(_4ARGS, 10, alignment, 205, nentries);
    CALLBACK(h+1, callback, SMARTIO_ARG_ONE);
    return h;
    }

tHandle SmartIO_MakeMultiSelector(uint16_t alignment, uint16_t nentries, uint16_t isSingleSelectOnly, void (*callback)(uint16_t))
    {
    tHandle h = APICall(_5ARGS, 10, alignment, 220, nentries, isSingleSelectOnly);
    CALLBACK(h+1, callback, SMARTIO_ARG_ONE);
    return h;
    }

tHandle SmartIO_MakeNumberSelector(uint16_t alignment, uint16_t default_val, uint16_t low, uint16_t high, void (*callback)(uint32_t))
    {
    tHandle h = APICall(_6ARGS, 10, alignment, 206, default_val, low, high);
    CALLBACK(h+1, callback, SMARTIO_ARG_ONE32);
    return h;
    }

tHandle SmartIO_MakeTimeSelector(uint16_t alignment, uint16_t display_24h, char *initial_value, void (*callback)(uint16_t, uint16_t))
    {
    tHandle h = APICall(_4ARGS|STRARG(5), 10, alignment, 207, display_24h, initial_value);
    CALLBACK(h+1, callback, SMARTIO_ARG_TWO);
    return h;
    }

tHandle SmartIO_MakeAnalogTimeSelector(uint16_t alignment, uint16_t display_24h, char *initial_value, void (*callback)(uint16_t, uint16_t))
    {
    tHandle h = APICall(_4ARGS|STRARG(5), 10, alignment, 210, display_24h, initial_value);
    CALLBACK(h+1, callback, SMARTIO_ARG_TWO);
    return h;
    }

tHandle SmartIO_MakeCalendarSelector(uint16_t alignment, char *initial_value, void (*callback)(char *))
    {
    tHandle h = APICall(_3ARGS|STRARG(4), 10, alignment, 208, initial_value);
    CALLBACK(h+1, callback, SMARTIO_ARG_STRING);
    return h;
    }

tHandle SmartIO_MakeWeekdaySelector (uint16_t alignment, uint16_t initial_value, void (*callback)(uint16_t))
    {
    tHandle h = APICall(_4ARGS, 10, alignment, 209, initial_value);
    CALLBACK(h+1, callback, SMARTIO_ARG_ONE);
    return h;
    }

tHandle SmartIO_MakeOK(uint16_t alignment, char *text, void (*callback)(uint16_t))
    {
    tHandle h = APICall(_3ARGS|STRARG(4), 10, alignment, 215, text);
    CALLBACK(h+1, callback, SMARTIO_ARG_ONE);
    return h;
    }

tHandle SmartIO_MakeCancelOK(uint16_t alignment, char *text, void (*callback)(uint16_t))
    {
    tHandle h = APICall(_3ARGS|STRARG(4), 10, alignment, 217, text);
    CALLBACK(h+1, callback, SMARTIO_ARG_ONE);
    return h;
    }

tHandle SmartIO_MakeOKLinkTo(uint16_t alignment, tHandle popup_handle, char *text, void (*callback)(uint16_t))
    {
    tHandle h = APICall(_4ARGS|STRARG(5), 10, alignment, 216, popup_handle, text);
    CALLBACK(h+1, callback, SMARTIO_ARG_ONE);
    return h;
    }

tHandle SmartIO_MakeCheckboxes(uint16_t alignment, uint16_t nentries, void (*callback)(uint16_t))
    {
    tHandle h = APICall(_4ARGS, 10, alignment, 221, nentries);
    CALLBACK(h+1, callback, SMARTIO_ARG_ONE);
    return h;
    }

tHandle SmartIO_MakeRadioButtons(uint16_t alignment, uint16_t nentries, void (*callback)(uint16_t))
    {
    tHandle h = APICall(_4ARGS, 10, alignment, 222, nentries);
    CALLBACK(h+1, callback, SMARTIO_ARG_ONE);
    return h;
    }

tHandle SmartIO_MakeTextEntry(uint16_t alignment, uint16_t isRoundCorners, uint16_t nlines, void (*callback)(char *))
    {
    tHandle h = APICall(_4ARGS, 10, alignment, 223, isRoundCorners, nlines == 0 ? 1 : nlines);
    CALLBACK(h+1, callback, SMARTIO_ARG_STRING);
    return h;
    }

tHandle SmartIO_MakeNumberEntry(uint16_t alignment, uint16_t isRoundCorners, void (*callback)(uint32_t))
    {
    tHandle h = APICall(_4ARGS, 10, alignment, 224, isRoundCorners);
    CALLBACK(h+1, callback, SMARTIO_ARG_ONE32);
    return h;
    }

tHandle SmartIO_MakePasswordEntry(uint16_t alignment, void (*callback)(char *))
    {
    tHandle h = APICall(_3ARGS, 10, alignment, 225);
    CALLBACK(h+1, callback, SMARTIO_ARG_STRING);
    return h;
    }

tHandle SmartIO_MakeTextBox(uint16_t alignment, uint16_t width, uint16_t nlines, char *text)
    {
    return APICall(_5ARGS|STRARG(6), 10, alignment, 300, width, nlines, text);
    }

tHandle SmartIO_MakeMultilineBox(uint16_t nlines, char *text)
    {
    return APICall(_4ARGS|STRARG(5), 10, 0, 401, nlines, text);
    }

tHandle SmartIO_MakeCounter(uint16_t alignment, uint16_t ndigits, uint32_t initial_value)
    {
    return APICall(_6ARGS, 10, alignment, 320, ndigits, initial_value & 0xFFFFFFFF, initial_value >> 16);
    }

tHandle SmartIO_MakeProgressBar(uint16_t alignment, uint16_t variation, uint16_t initial_percentage)
    {
    return APICall(_5ARGS, 10, alignment, 301, variation, initial_percentage);
    }

tHandle SmartIO_MakeProgressCircle(uint16_t alignment, uint16_t variation, uint16_t initial_percentage)
    {
    return APICall(_5ARGS, 10, alignment, 302, variation, initial_percentage);
    }

tHandle SmartIO_MakeHGauge(uint16_t alignment, uint16_t variation, uint16_t initial_percentage)
    {
    return APICall(_5ARGS, 10, alignment, 303, variation, initial_percentage);
    }

tHandle SmartIO_MakeVGauge(uint16_t alignment, uint16_t variation, uint16_t initial_percentage)
    {
    return APICall(_5ARGS, 10, alignment, 304, variation, initial_percentage);
    }

tHandle SmartIO_MakeSemiCircularGauge(uint16_t alignment, uint16_t max_mark, uint16_t initial_percentage)
    {
    return APICall(_5ARGS, 10, alignment, 305, max_mark, initial_percentage);
    }

tHandle SmartIO_MakeCircularGauge(uint16_t alignment, uint16_t max_mark, uint16_t initial_percentage)
    {
    return APICall(_5ARGS, 10, alignment, 306, max_mark, initial_percentage);
    }

tHandle SmartIO_MakeBatteryLevel(uint16_t alignment, uint16_t variation, uint16_t initial_percentage)
    {
    return APICall(_5ARGS, 10, alignment, 308, variation, initial_percentage);
    }

tHandle SmartIO_MakeRGBLed(uint16_t alignment, uint16_t color, uint16_t initial_state)
    {
    return APICall(_5ARGS, 10, alignment, 307, color, initial_state);
    }

tHandle SmartIO_MakeCustomHGauge(uint16_t alignment, uint32_t start_color, uint32_t end_color, uint16_t initial_percentage)
    {
    return APICall(0b01111101010101, 10, alignment, 310, start_color, end_color, initial_percentage);
    }

tHandle SmartIO_MakeCustomVGauge(uint16_t alignment, uint32_t start_color, uint32_t end_color, uint16_t initial_percentage)
    {
    return APICall(0b01111101010101, 10, alignment, 311, start_color, end_color, initial_percentage);
    }

tStatus SmartIO_SetSliceIcon(tHandle handle, uint16_t icon_enum)
    {
    return APICall(_3ARGS, 12, handle, icon_enum);
    }

tHandle SmartIO_MakeFreeformSlice(uint16_t y_size)
    {
    return APICall(_2ARGS, 14, y_size);
    }

tHandle SmartIO_FFS_OnOffButton(tHandle ffs_handle, uint16_t loc_x, uint16_t loc_y, uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t))
    {
    tHandle h = APICall(_7ARGS, 15, ffs_handle, loc_x, loc_y, 200, variation, initial_value);
    CALLBACK(h, callback, SMARTIO_ARG_ONE);
    return h;
    }

tHandle SmartIO_FFS_3PosButton (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));
tHandle SmartIO_FFS_Incrementer (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));
tHandle SmartIO_FFS_Slider (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));

tHandle SmartIO_FFS_Picker (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t nentries, void (*callback)(uint16_t));
tHandle SmartIO_FFS_MultiSelector (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t nentries, uint16_t isSingleSelectOnly, void (*callback)(uint16_t));
tHandle SmartIO_FFS_NumberSelector (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t default_val, uint16_t low, uint16_t high, void (*callback)(uint16_t));
tHandle SmartIO_FFS_TimeSelector (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t display_24h, char *initial_value, void (*callback)(uint16_t, uint16_t));
tHandle SmartIO_FFS_AnalogTimeSelector
                                    (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t display_24h, char *initial_value, void (*callback)(uint16_t, uint16_t));
tHandle SmartIO_FFS_CalendarSelector(tHandle, uint16_t loc_x, uint16_t loc_y, char *initial_value, void (*callback)(char *));
tHandle SmartIO_FFS_WeekdaySelector (tHandle, uint16_t loc_x, uint16_t loc_y, char *initial_value, void (*callback)(char *));

tHandle SmartIO_FFS_Checkboxes (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t nentries, void (*callback)(uint16_t));
tHandle SmartIO_FFS_RadioButtons (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t nentries, void (*callback)(uint16_t));

tHandle SmartIO_FFS_TextEntry (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t isRoundCorners, void (*callback)(char *));
tHandle SmartIO_FFS_NumberEntry (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t isRoundCorners, void (*callback)(char *));

// Use SmartIO_AddText(tHandle, uint16_t loc_x, uint16_t loc_y, handle, "password hint") to add hint
tHandle SmartIO_FFS_PasswordEntry (tHandle, uint16_t loc_x, uint16_t loc_y, void (*callback)(char *));

// Text Box is meant to be narrower and can have a slice icon
// whereas multiline box takes the full width
tHandle SmartIO_FFS_TextBox (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t width, uint16_t nlines, char *text);
tHandle SmartIO_FFS_MultilineBox (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t nlines, char *text);
tHandle SmartIO_FFS_Counter (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t ndigits, uint32_t initial_value);

// width is in virtual pixels with a max of 320 virtual pixels
tHandle SmartIO_FFS_ProgressBar (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t variation, uint16_t initial_percentage);
tHandle SmartIO_FFS_ProgressCircle (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t variation, uint16_t initial_percentage);
tHandle SmartIO_FFS_HGauge (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t variation, uint16_t initial_percentage);
tHandle SmartIO_FFS_VGauge (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t variation, uint16_t initial_percentage);
tHandle SmartIO_FFS_SemiCircularGauge
                                    (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t max_mark, uint16_t initial_percentage);
tHandle SmartIO_FFS_CircularGauge (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t max_mark, uint16_t initial_percentage);
tHandle SmartIO_FFS_BatteryLevel (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t variation, uint16_t initial_percentage);
tHandle SmartIO_FFS_RGBLed (tHandle, uint16_t loc_x, uint16_t loc_y, uint16_t color, uint16_t initial_state);

tHandle SmartIO_FFS_CustomHGauge (tHandle, uint16_t loc_x, uint16_t loc_y, uint32_t start_color, uint32_t end_color, uint16_t initial_percentage);
tHandle SmartIO_FFS_CustomVGauge (tHandle, uint16_t loc_x, uint16_t loc_y, uint32_t start_color, uint32_t end_color, uint16_t initial_percentage);

tHandle SmartIO_FFS_Label (tHandle, uint16_t loc_x, uint16_t loc_y, char *label);

tHandle SmartIO_PopupOnOffButton (uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));
tHandle SmartIO_Popup3PosButton (uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));
tHandle SmartIO_PopupIncrementer (uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));
tHandle SmartIO_PopupSlider (uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));

tHandle SmartIO_PopupPicker (uint16_t nentries, void (*callback)(uint16_t));
tHandle SmartIO_PopupMultiSelector (uint16_t nentries, uint16_t isSingleSelectOnly, void (*callback)(uint16_t));
tHandle SmartIO_PopupNumberSelector (uint16_t default_val, uint16_t low, uint16_t high, void (*callback)(uint16_t));
tHandle SmartIO_PopupTimeSelector (uint16_t display_24h, char *initial_value, void (*callback)(uint16_t, uint16_t));
tHandle SmartIO_PopupAnalogTimeSelector
                                    (uint16_t display_24h, char *initial_value, void (*callback)(uint16_t, uint16_t));
tHandle SmartIO_PopupCalendarSelector
                                    (tHandle, uint16_t loc_x, uint16_t loc_y, char *initial_value, void (*callback)(char *));
tHandle SmartIO_PopupWeekdaySelector(char *initial_value, void (*callback)(char *));

tHandle SmartIO_PopupCheckboxes (uint16_t nentries, void (*callback)(uint16_t));
tHandle SmartIO_PopupRadioButtons (uint16_t nentries, void (*callback)(uint16_t));

tHandle SmartIO_PopupTextEntry (uint16_t isSquareCorners, void (*callback)(char *));
tHandle SmartIO_PopupNumberEntry (uint16_t isSquareCorners, void (*callback)(char *));

// Use SmartIO_AddText(tHandle, (uint16_t loc_x, uint16_t loc_y, handle, "password hint") to add hint
tHandle SmartIO_PopupPasswordEntry (void (*callback)(char *));

// Text Box is meant to be narrower and can have a slice icon
// whereas multiline box takes the full width
tHandle SmartIO_PopupTextBox (uint16_t width, uint16_t nlines, char *text);
tHandle SmartIO_PopupMultilineBox (uint16_t nlines, char *text);
tHandle SmartIO_PopupCounter (uint16_t ndigits, uint32_t initial_value);

// width is in virtual pixels with a max of 320 virtual pixels
tHandle SmartIO_PopupProgressBar (uint16_t variation, uint16_t initial_percentage);
tHandle SmartIO_PopupProgressCircle (uint16_t variation, uint16_t initial_percentage);
tHandle SmartIO_PopupHGauge (uint16_t variation, uint16_t initial_percentage);
tHandle SmartIO_PopupVGauge (uint16_t variation, uint16_t initial_percentage);
tHandle SmartIO_PopupSemiCircularGauge
                                    (uint16_t max_mark, uint16_t initial_percentage);
tHandle SmartIO_PopupCircularGauge (uint16_t max_mark, uint16_t initial_percentage);
tHandle SmartIO_PopupBatteryLevel (uint16_t variation, uint16_t initial_percentage);
tHandle SmartIO_PopupRGBLed (uint16_t color, uint16_t initial_state);

tHandle SmartIO_PopupCustomHGauge (uint32_t start_color, uint32_t end_color, uint16_t initial_percentage);
tHandle SmartIO_PopupCustomVGauge (uint32_t start_color, uint32_t end_color, uint16_t initial_percentage);

tHandle SmartIO_PopupLabel (char *label);

tStatus SmartIO_AppendPopup (tHandle source, tHandle next);

tStatus SmartIO_AddText(tHandle handle, char *text)
    {
    return APICall(_2ARGS|STRARG(3), 11, handle, text);
    }

tStatus SmartIO_ClearText(tHandle handle)
    {
    return APICall(_2ARGS, 25, handle);
    }

tHandle SmartIO_AddListItem(tHandle handle, char *label)
    {
    return APICall(_2ARGS|STRARG(3), 23, handle, label);
    }

tStatus SmartIO_FillColor(tHandle h, uint32_t color)
    {
    return APICall(_4ARGS, 31, h, color & 0xFFFF, (color >> 16) & 0xFFFF);
    }

tStatus SmartIO_SetFont(tHandle h, uint16_t isSansSerifFont, uint16_t fontsize, uint32_t color)
    {
    return APICall(_6ARGS, 30, h, isSansSerifFont, fontsize, color & 0xFFFF, (color >> 16) & 0xFFFF);
    }

tStatus SmartIO_UpdateIntValue(tHandle handle, uint16_t value)
    {
    return APICall(_3ARGS, 20, handle, value);
    }

tStatus SmartIO_UpdateIntValue2(tHandle handle, uint16_t value1, uint16_t value2)
    {
    return APICall(_4ARGS, 20, handle, value1, value2);
    }
tStatus SmartIO_UpdateIntValue3(tHandle handle, uint16_t value1, uint16_t value2, uint16_t value3)
    {
    return APICall(_5ARGS, 20, handle, value1, value2, value3);
    }

tStatus SmartIO_UpdateStringValue(tHandle handle, char *str)
    {
    return APICall(_2ARGS|STRARG(3), 20, handle, str);
    }

tStatus SmartIO_UpdateOnOffButton(tHandle handle, uint16_t value) { return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_Update3PosButton(tHandle handle, uint16_t value) { return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_UpdateIncrementer(tHandle handle, uint16_t value) { return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_UpdateSlider(tHandle handle, uint16_t value) { return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_UpdateExpandableList(tHandle handle, uint16_t value){ return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_UpdatePicker(tHandle handle, uint16_t value) { return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_UpdateMultiSelector(tHandle handle, uint16_t value) { return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_UpdateNumberSelector(tHandle handle, uint16_t value){ return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_UpdateCheckboxes(tHandle handle, uint16_t value) { return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_UpdateRadioButtons(tHandle handle, uint16_t value) { return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_UpdateCounter(tHandle handle, uint16_t value) { return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_UpdateProgressBar(tHandle handle, uint16_t value) { return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_UpdateProgressCircle(tHandle handle, uint16_t value){ return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_UpdateHGauge(tHandle handle, uint16_t value) { return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_UpdateVGauge(tHandle handle, uint16_t value) { return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_UpdateSemiCircularGauge(tHandle handle, uint16_t value)
                                                                    { return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_UpdateCircularGauge(tHandle handle, uint16_t value) { return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_UpdateCustomHGauge(tHandle handle, uint16_t value) { return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_UpdateCustomVGauge(tHandle handle, uint16_t value) { return SmartIO_UpdateIntValue(handle, value); }
tStatus SmartIO_UpdateBatteryLevel(tHandle handle, uint16_t value) { return SmartIO_UpdateIntValue(handle, value); }

tStatus SmartIO_UpdateRGBLed(tHandle handle, uint16_t color, uint16_t on_off)
                                                                    { return SmartIO_UpdateIntValue2(handle, color, on_off); }

tStatus SmartIO_UpdateTextBox(tHandle handle, char *value)
    {
    return SmartIO_UpdateStringValue(handle, value);
    }

tHandle SmartIO_MakeSpacerSlice(int i)
    {
    tHandle h;

    do {
        h = APICall(_1ARG, 13);
    } while (i-- > 0);
    return h;
    }

tStatus SmartIO_AutoBalance(tHandle page_id)
    {
    return APICall(_2ARGS, 43, page_id);
    }

tStatus SmartIO_EnableObject(tHandle handle)
    {
    return APICall(_3ARGS, 40, handle, 1);
    }

tStatus SmartIO_DisableObject(tHandle handle)
    {
    return APICall(_3ARGS, 40, handle, 0);
    }

tStatus SmartIO_ShowObject(tHandle handle)
    {
    return APICall(_3ARGS, 41, handle, 1);
    }

tStatus SmartIO_HideObject(tHandle handle)
    {
    return APICall(_3ARGS, 41, handle, 0);
    }

tStatus SmartIO_DeleteObject(tHandle handle)
    {
    return APICall(_3ARGS, 42, handle);
    }

// Static Objects
tHandle SmartIO_MakeLabel(uint16_t alignment, uint16_t height, char *label)
    {
    return APICall(_4ARGS|STRARG(5), 10, alignment, 400, height, label);
    }

tHandle SmartIO_MakeLine (uint16_t loc_x, uint16_t loc_y, uint16_t end_x, uint16_t end_y, uint16_t color);
tHandle SmartIO_MakeCircle (uint16_t loc_x, uint16_t loc_y, uint16_t diameter_x, uint16_t diameter_y, uint16_t outline_color, uint16_t fill_color);
tHandle SmartIO_MakeRectangle (uint16_t loc_x, uint16_t loc_y, uint16_t end_x, uint16_t end_y, uint16_t outline_color, uint16_t fill_color);
tHandle SmartIO_MakeTriangle (uint16_t loc_x, uint16_t loc_y, uint16_t loc_z, uint16_t outline_color, uint16_t fill_color);

// To Be Defined
tHandle SmartIO_MakeIcon (uint16_t);
tHandle SmartIO_MakeGraphics (uint16_t);

// System (SMART.IO) Commands
unsigned char *SmartIO_ReadEEPROM(uint16_t address, uint16_t length)
    {
    return (unsigned char *)APICall(_3ARGS, SMARTIO_SYS_EEPROM_READ, address, length);
    }

tStatus SmartIO_WriteEEPROM(uint16_t address, uint16_t length, unsigned char *buffer)
    {
    return APICall(_3ARGS|STRARG(4), SMARTIO_SYS_EEPROM_WRITE, address, length, buffer);
    }

tStatus SmartIO_SetLED(uint16_t led)
    {
    return APICall(_2ARGS, SMARTIO_SYS_LED_SET, led);
    }

tStatus SmartIO_ClearLED(uint16_t led)
    {
    return APICall(_2ARGS, SMARTIO_SYS_LED_CLEAR, led);
    }

tStatus SmartIO_ToggleLED(uint16_t led)
    {
    return APICall(_2ARGS, SMARTIO_SYS_LED_TOGGLE, led);
    }

uint32_t SmartIO_GenRandomNumber(void)
    {
    return APICall(_1ARG, SMARTIO_SYS_RAND);
    }

uint32_t *SmartIO_GetUniqueID(void)
    {
    static uint32_t val[2];
    uint32_t *p = (uint32_t *)APICall(_1ARG, SMARTIO_SYS_UNIQUE_ID);
    val[0] = p[0];
    val[1] = p[1];
    return val;
    }

tStatus SmartIO_DebugUART(uint16_t enable)
    {
    return APICall(_2ARGS, SMARTIO_SYS_DEBUG_UART, enable);
    }

// Phone Commands
char *SmartIO_GetPhoneTime(void)
    {
    return (char *)APICall(_1ARG, SMARTIO_SYS_PHONE_TIME);
    }

char *SmartIO_GetPhoneGPS(void)
    {
    return (char *)APICall(_1ARG, SMARTIO_SYS_PHONE_GPS);
    }

tStatus SmartIO_Beep(void)
    {
    return APICall(_1ARG, SMARTIO_SYS_PHONE_BEEP);
    }

static uint32_t APICall(unsigned argmask, ...)
    {
    int index = 0;
    va_list args;
    int intarg;
    char *strarg;

    va_start(args, argmask);
    for (int i = 0; i < 8; i++)
        {
        unsigned mask = argmask & 0b11;
        if (mask == 0)
            break;
        if (mask == 0b01)
            {
            intarg = va_arg(args, unsigned);
            Command[index] = intarg;
            }
        else if (mask == 0b10 && Command[0] == SMARTIO_SYS_EEPROM_WRITE)
            {
            if (index != 3)
                return (uint32_t)-1;

            strarg = va_arg(args, unsigned char *);
            uint16_t length = Command[2];
            unsigned char *dst = (unsigned char *)&Command[3];
            for (int i = 0; i < length; i++)
                *dst++ = *strarg++;
            return SmartIO__SendBytes((unsigned char *)&Command[0], length + 3*sizeof(uint16_t));
            }
        else if (mask == 0b10)
            {
            strarg = va_arg(args, char *);
            while (*strarg)
                {
                Command[index] = FETCH_WORD(strarg, 0);
                if (strarg[1] == 0 || strarg[2] == 0)
                    break;
                strarg += 2;
                index++;

                if (index >= HOST_SRAM_POOL_SIZE/2)
                    {
                    SmartIO_Error(SMARTIO_ERROR_COMMAND_BUFFER_OVERFLOW);
                    break;
                    }
                }
            }
        else if (mask == 0b11)
            {
            intarg = va_arg(args, unsigned);
            Command[index++] = (uint16_t)intarg;
            Command[index] = (uint16_t)(intarg >> 16);
            }
        index++;
        argmask >>= 2;
        }
    return SmartIO__SendCommand(Command, index);
    }
