#pragma once
/*! \file ..\include\smartio_api.h
 * \brief This header file defines the Smart.IO Host Interface Layer to Smart.IO
 * Copyright 2017-2018 ImageCraft Creations Inc., All rights reserved.
 * https://imagecraft.com/smartio/
 */

/*! \mainpage Smart.IO API
 * https://imagecraft.com/smartio/ Copyright 2017-2018 ImageCraft Creations Inc. All Rights Reserved.
 *
 * Smart.IO is the patent pending technology that lets you create an App UI for an embedded design with no app or wireless coding.
 *
 * MCU firmware uses the Smart.IO API to access Smart.IO features. For more information, please refer to the
 * "Smart.IO User Guide", available at http://imagecraft.com/documentation/smart-io-documentation
 */
#include <stdint.h>

/** \brief Configurable defines. These defines affect how much host MCU SRAM space will be used so they cannot be too
 * large. However, if they are too small, they may not support your UI properly.
 * Please see "Smart.IO User Guide" for detail.
 *
 * HOST_SRAM_POOL_SIZE is the scratch pad SRAM in the host MCU for used by Smart.IO. 128-256 bytes is a good value to use.
 * CALLBACK_TABLE_SIZE basically specifies how many callback functions, which are proportional to the number of input
 * UI elements the UI will use.
 */
#if defined(_AVR)
#define HOST_SRAM_POOL_SIZE         128
#define CALLBACK_TABLE_SIZE         50
#else
#define HOST_SRAM_POOL_SIZE         256
#define CALLBACK_TABLE_SIZE         100
#endif

#define FETCH_WORD(s, i)            (((s)[i+1] << 8) | (s)[i])  /**< Fetch a 16-bit word from 2 Little Endian bytes */
#define SMARTIO_MAX_EEPROM_SIZE     (2048-32)                   /**< Usable Smart.IO EEPROM size (32 bytes reserved by Smart.IO) */
extern unsigned char *host_sram_block;                          /**< SRAM scratch pad in host MCU */

typedef uint16_t tHandle;       /**< Generic 16-bit Handle */
typedef int16_t tStatus;        /**< Generic status type */

/** \brief UI icon enum
 */
enum SMARTIO_ICON {
    SMARTIO_ICON_NONE,          /**< No icon */
    SMARTIO_ICON_ALERT,         /**< Alert icon */
    SMARTIO_ICON_ERROR,         /**< Error icon */
    SMARTIO_ICON_H_ONOFF,       /**< On/off horizontal icon */
    SMARTIO_ICON_INFO,          /**< Info icon */
    SMARTIO_ICON_NEXT,          /**< "next" arrow icon */
    SMARTIO_ICON_POWER,         /**< Power icon */
    SMARTIO_ICON_PREVIOUS,      /**< "previous" arrow icon */
    SMARTIO_ICON_QUERY,         /**< Query (?) icon */
    SMARTIO_ICON_SETTINGS,      /**< (Gear) settings icon */
    SMARTIO_ICON_V_ONOFF,       /**< On/off vertical icon */
};

/** \brief System command enum. FOR INTERNAL USE ONLY.
 * Use API functions to access these features.
 */
enum SMARTIO_SYS_CMD {
    __SAMRTIO_SYS_START = 0xFF00 - 1,
    SMARTIO_SYS_EEPROM_WRITE,   /**< EEPROM write command */
    SMARTIO_SYS_LED_SET,        /**< Set LED */
    SMARTIO_SYS_LED_CLEAR,      /**< Clear LED */
    SMARTIO_SYS_LED_TOGGLE,     /**< Toggle LED */
    SMARTIO_SYS_RAND,           /**< Generate random number */
    SMARTIO_SYS_DEBUG_UART,     /**< Enable/Disable debug UART */
    SMARTIO_SYS_PHONE_BEEP,     /**< Emit phone beep */
    SMARTIO_SYS_UART_WRITE,     /**< Write to UART */

    // These return multiple bytes
    __SMARTIO_SYS_MULTIBYTE_RETURN = 0xFF80 - 1,
    SMARTIO_SYS_EEPROM_READ,    /**< EEPROM read */
    SMARTIO_SYS_UNIQUE_ID,      /**< Return a 96-bit unique ID */
    SMARTIO_SYS_PHONE_TIME,     /**< Return phone time */
    SMARTIO_SYS_PHONE_GPS,      /**< Return phone GPS coordinates */
};

/** \brief Host MCU connection status
 */
enum SMARTIO_HOST_STATUS {
    __SMARTIO_HOST_START = 0x100 - 1,
    SMARTIO_HOST_DISCONNECT,    /**< Disconnected from Phone App host */
    SMARTIO_HOST_CONNECT,       /**< Connected to Phone App host */
};

/** \defgroup control-cmd  Smart.IO Control Commands
 *  @{
 */
/** \brief Get Smart.IO firmware version.
 *
 * \return revision string
 */
char *  SmartIO_GetVersion(void);

/** \brief Initialize the Smart.IO environment. Must be the first Smart.IO call to be made.
 *
 * \param connect_callback      user function to called when connected to the phone app.
 * \param disconnect_callback   user function to called when disconnected from the phone app.
 */
void    SmartIO_Init(void (*connect_callback)(void), void (*disconnect_callback)(void));

/** \brief Reset the Samrt.IO module
 */
void    SmartIO_HardReset(void);

/** \brief "Soft" reset the Smart.IO module.
 *
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_Reset(void);

/** \brief Pauses the App to stop any app user interaction, until another Smart.IO command is sent from the host MCU
 *         including another SmartIO_StopResume(). When stopped, the app displays a spinning circle.
 *         NOTE: "System commands" that are serviced by the Smart.IO and not the app, e.g.
 *         Read/Write EEPROM, will NOT un-freeze the app
 *
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_StopResume(void);

/** \todo
 */
tStatus SmartIO_Resumeable(uint16_t);
/** @} */

/** \defgroup cache-cmd  Cache Commands
 *  @{
 */
/** \brief If a cache exists with matching IDs, then run the commands from cache until the SmartIO_SaveCache()
 *         command. Otherwise, create a new cache with the IDs.
 *
 * \param product_id            32-bit value, 13-bit company ID | 19 user definable bits.
 *                              the 13-bit company ID must be obtained from ImageCraft.
 * \param build_id              16-bit firmware build version.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_LoadCache(uint32_t product_id, uint16_t build_id);

/** \brief Save the cache if a new cache was created, or stopping running from cache otherwise.
 *
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_SaveCache(void);
/** @} */

/** \defgroup app-appearance  Modifying App Appearance
 *  @{
 */
/** \brief Set the App title.
 *
 * \param title                 title string. To be sure that it fits in most phones, it should be no more than 16-20 characters
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_AppTitle(char *title);

/** \brief Add an app menu item.
 *
 * \param label                 menu item string.
 * \param callback              user function to call when the menu item is tapped.
 * \return 0 if success, -1 otherwise
 */
tHandle SmartIO_AddMenu(char *label, void (*callback)(uint16_t));
/** @} */

/** \defgroup page-cmd  Page Related Commands
 *  @{
 */
/** \brief Create a new page. Before any UI controls can be created, at least one default page must be created.
 *
 * \return the handle for the page
 */
tHandle SmartIO_MakePage(void);

/** \brief Set the current page to argument. The phone app will change focus to the new page. Any UI creations
 *         will be done on the current page.
 *
 * \param page                  handle for the page
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_SetCurrentPage(tHandle page);

/** \brief Phone app changes focus to the page and locks it so that the app user cannot swipe to a different page.
 *
 * \param page                  handle for the page
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_LockPage(tHandle page);

/** \brief Re-enable page navigation.
 *
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UnlockPage(void);

/** \brief Set the page title.
 *
 * \param page                  handle for the page
 * \param title                 title string. To be sure that it fits in most phones, it should be no more than 16-20 characters
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_PageTitle(tHandle page, char *title);
/** @} */

/** \defgroup ui-control  UI Element Commands
 *  @{
 */
/** \brief Group sets of UI objects. UI handles that are adjacent will be grouped with a group box placed around them.
 *         Multiple disjoint sets can be specified with a single call.
 *
 * \param option                reserved. Must be zero.
 * \param handle                handle of the first object.
 * \param ...                   up to 7 additional handles. End with a 0 null handle (not included as part of the 8).
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_GroupObjects(uint16_t option, tHandle handle, ...);

/** \brief Make a set of UI elements dependent on a control UI: if the control UI is not selected/enabled, then the
 *         dependent controls are also not enabled.
 *
 * \param control               handle of the controlling object.
 * \param dependent1            handle of the first dependent object.
 * \param ...                   up to 7 additional handles. End with a 0 null handle (not included as part of the 8).
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_EnableIf(tHandle control, tHandle dependent1, ...);

// Page and object management
// These apply to any object, including page or other UI elements
//
// Enable implies show
// Show does not imply enable
//
// Disable does not imply hide
// Hide implies disable
/** \brief Enable an object, which can be a UI element, GUI slice, page etc. Disabled elements cannot be used but is still visible.
 *
 * \param handle                handle of the object.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_EnableObject(tHandle handle);

/** \brief Disable an object, which can be a UI element, GUI slice, page etc. Disabled elements cannot be used but is still visible.
 *
 * \param handle                handle of the object.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_DisableObject(tHandle handle);

/** \brief Show an object, which can be a UI element, GUI slice, page etc. Does not enable the object per se.
 *
 * \param handle                handle of the object.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_ShowObject(tHandle handle);

/** \brief Hide an object, which can be a UI element, GUI slice, page etc.
 *
 * \param handle                handle of the object.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_HideObject(tHandle handle);

/** \brief Delete an object, which can be a UI element, GUI slice, page etc. Will NOT re-auto-balance the GUI slices automatically.
 *
 * \param handle                handle of the object.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_DeleteObject(tHandle handle);
/** @} */

/** \defgroup input-UI  Input UI Elements
 *  @{
 */
/** \brief Create a GUI slice with an on/off button.
 *
 * \param alignment             0: right, 1: left, 2: middle. Alignment refers to the location of the UI element.
 * \param variation             0 - 4, see "Smart.IO User Guide" for detail.
 * \param initial_value         0: off, 1: on, the initial value of the control.
 * \param callback              the user function to call when the app user changes the input.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeOnOffButton(uint16_t alignment, uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));

/** \brief Create a GUI slice with a 3-pos switch.
 *
 * \param alignment             0: right, 1: left, 2: middle. Alignment refers to the location of the UI element.
 * \param variation             0 - 8, see "Smart.IO User Guide" for detail.
 * \param initial_value         0 (left), 1 (mid), and 2 (right), the initial value of the control.
 * \param callback              the user function to call when the app user changes the input.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_Make3PosButton(uint16_t alignment, uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));

/** \brief Create a GUI slice with an incrementer.
 *
 * \param alignment             0: right, 1: left, 2: middle. Alignment refers to the location of the UI element.
 * \param variation             0 - 5, see "Smart.IO User Guide" for detail.
 * \param initial_value         0 to 0xFFFF, the initial value of the control.
 * \param callback              the user function to call when the app user changes the input.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeIncrementer(uint16_t alignment, uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));

/** \brief Create a GUI slice with a slider.
 *
 * \param alignment             0: right, 1: left, 2: middle. Alignment refers to the location of the UI element.
 * \param variation             0 - 3, see "Smart.IO User Guide" for detail.
 * \param initial_value         0 to 100 the initial value of the control.
 * \param callback              the user function to call when the app user changes the input.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeSlider(uint16_t alignment, uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));

/** \brief Create a GUI slice with an expandable list. Use SmartIO_AddListItem(handle+1, "text") to add items.
 *
 * \param alignment             0: right, 1: left. Alignment refers to the location of the UI element.
 * \param nentries              the number of entries.
 * \param callback              the user function to call when the app user changes the input.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeExpandableList(uint16_t alignment, uint16_t nentries, void (*callback)(uint16_t));

/** \brief Create a GUI slice with a picker. Use SmartIO_AddListItem(handle+1, "text") to add items.
 *
 * \param alignment             0: right, 1: left. Alignment refers to the location of the UI element.
 * \param nentries              the number of entries.
 * \param callback              the user function to call when the app user changes the input.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakePicker(uint16_t alignment, uint16_t nentries, void (*callback)(uint16_t));

/** \brief Create a GUI slice with a multi-selector. Use SmartIO_AddListItem(handle+1, "text") to add items.
 *
 * \param alignment             0: right, 1: left. Alignment refers to the location of the UI element.
 * \param nentries              the number of entries.
 * \param isSingleSelectOnly    0: allow multi-selection, 1: single selection only.
 * \param callback              the user function to call when the app user changes the input.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeMultiSelector(uint16_t alignment, uint16_t nentries, uint16_t isSingleSelectOnly, void (*callback)(uint16_t));

/** \brief Create a GUI slice with a number selector.
 *
 * \param alignment             0: right, 1: left. Alignment refers to the location of the UI element.
 * \param default_val           the initial value to display.
 * \param low                   the lower bound of selectable number.
 * \param high                  the upper bound of selectable number.
 * \param callback              the user function to call when the app user changes the input.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeNumberSelector(uint16_t alignment, uint16_t default_val, uint16_t low, uint16_t high, void (*callback)(uint32_t));

/** \brief Create a GUI slice with a digital time selector.
 *
 * \param alignment             0: right, 1: left. Alignment refers to the location of the UI element.
 * \param display_24h           0: 12 hour display with AM/PM, 1: 24 hour display.
 * \param initial_value         initial time in "HH:MM" 24 hour format.
 * \param callback              the user function to call when the app user changes the input.
 *                              The function is called with HH and MM (numeric values, e.g. 12 05 for "12:05") 24 hour format.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeTimeSelector(uint16_t alignment, uint16_t display_24h, char *initial_value, void (*callback)(uint16_t hh, uint16_t mm));

/** \brief Create a GUI slice with an analog time selector.
 *
 * \param alignment             0: right, 1: left. Alignment refers to the location of the UI element.
 * \param display_24h           0: 12 hour display with AM/PM, 1: 24 hour display.
 * \param initial_value         initial time in "HH:MM" 24 hour format.
 * \param callback              the user function to call when the app user changes the input.
 *                              The function is called with HH and MM (numeric values, e.g. 12 05 for "12:05") 24 hour format.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeAnalogTimeSelector(uint16_t alignment, uint16_t display_24h, char *initial_value, void (*callback)(uint16_t, uint16_t));

/** \brief Create a GUI slice with a calendar selector.
 *
 * \param alignment             0: right, 1: left. Alignment refers to the location of the UI element.
 * \param initial_value         initial date in "MTH DD YYYY" format. MTH can be 1-12, or 3 letter code JAN/FEB/.../DEC. DD is 1-31.
 *                              Note that the phone OS may not support dates that are far in the past or in the future.
 * \param callback              the user function to call when the app user changes the input. The argument format is
 *                              "weekday MTH DD YYYY" where all 4 fields are integers. You can use
 *                              SmartIO_ConvertCalendarDay() to extract the data.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeCalendarSelector(uint16_t alignment, char *initial_value, void (*callback)(char *));

/** \brief Create a GUI slice with a weekday selector.
 *
 * \param alignment             0: right, 1: left. Alignment refers to the location of the UI element.
 * \param initial_value         bitmask of weekdays. bit 0 is Monday, bir 1 is Tuesday etc.
 * \param callback              the user function to call when the app user changes the input. A bitmask of selected days is returned.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeWeekdaySelector(uint16_t alignment, uint16_t initial_value, void (*callback)(uint16_t));

/** \brief Create a GUI slice with an [OK] button.
 *
 * \param alignment             0: right, 1: left. Alignment refers to the location of the UI element.
 * \param text                  label of the button. If null, then "OK" is used.
 * \param callback              the user function to call when the app user changes the input. An argument of 1 is always used.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeOK(uint16_t alignment, char *text, void (*callback)(uint16_t));

/** \brief Create a GUI slice with a [Cancel|OK] button.
 *
 * \param alignment             0: right, 1: left. Alignment refers to the location of the UI element.
 * \param text                  label of the button. If null, then "Cancel | OK" is used.
 * \param callback              the user function to call when the app user changes the input. 0: Cancel is tapped, 1: OK is tapped.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeCancelOK(uint16_t alignment, char *text, void (*callback)(uint16_t));

/** \brief Create a GUI slice with an [OK] button with automatic linking to another (popup) object.
 *
 * \param alignment             0: right, 1: left. Alignment refers to the location of the UI element.
 * \param popup_handle          handle of a SmartIO_PopupAlert() object. When the button is tapped, the App displays this object automatically.
 * \param text                  label of the button. If null, then "OK" is used.
 * \param callback              the user function to call when the app user changes the input. An argument of 1 is always used.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeOKLinkTo(uint16_t alignment, tHandle popup_handle, char *text, void (*callback)(uint16_t));

/** \brief Create a GUI slice with a set of checkboxes. Use SmartIO_AddListItem(handle+1, "text") to add items.
 *
 * \param alignment             0: right, 1: left. Alignment refers to the location of the UI element.
 * \param nentries              the number of entries.
 * \param callback              the user function to call when the app user changes the input. A bitmask of selected items is passed.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeCheckboxes(uint16_t alignment, uint16_t nentries, void (*callback)(uint16_t));

/** \brief Create a GUI slice with a set of radio buttons. Use SmartIO_AddListItem(handle+1, "text") to add items.
 *
 * \param alignment             0: right, 1: left. Alignment refers to the location of the UI element.
 * \param nentries              the number of entries.
 * \param callback              the user function to call when the app user changes the input. 1..nentries, indicating the selected button, is passed.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeRadioButtons(uint16_t alignment, uint16_t nentries, void (*callback)(uint16_t));

/** \brief Create a GUI slice with a text entry box.
 *
 * \param alignment             0: right, 1: left. Alignment refers to the location of the UI element.
 * \param isRoundCorners        0: entry box has sharp corners, 1: entry box has round corners.
 * \param nlines                number of lines in the text entry box. Default is 1.
 * \param callback              the user function to call when the app user changes the input. The user clicks 'Done' on the virtual keyboard to finish text entry.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeTextEntry(uint16_t alignment, uint16_t isRoundCorners, uint16_t nlines, void (*callback)(char *));

/** \brief Create a GUI slice with a number entry box. The vritual keyboard only displays numeric keys.
 *
 * \param alignment             0: right, 1: left. Alignment refers to the location of the UI element.
 * \param isRoundCorners        0: entry box has sharp corners, 1: entry box has round corners.
 * \param callback              the user function to call when the app user changes the input. The user clicks 'Done' on the virtual keyboard to finish text entry.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeNumberEntry(uint16_t alignment, uint16_t isRoundCorners, void (*callback)(uint32_t));

/** \brief Create a GUI slice with a password entry box. The entered character is replaced with a '*' as it is entered.
 *         Use SmartIO_AddText(handle, "password hint") to add hint.
 *
 * \param alignment             0: right, 1: left. Alignment refers to the location of the UI element.
 * \param callback              the user function to call when the app user changes the input. The user clicks 'Done' on the virtual keyboard to finish text entry.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakePasswordEntry(uint16_t alignment, void (*callback)(char *));
/** @} */

/** \defgroup output-UI  Output UI Elements
 *  @{
 */
/** \brief Create a GUI slice with a label. You may change the text with SmartIO_AddText().
 *
 * \param alignment             0: left, 1: right. Alignment refers to the location of the UI element.
 *                              NOTE: This is the opposite of other UI commands where 0 is right and 1 is left. However, this allows
 *                              the same alignment value to be used in consecutive calls to align the label with the slice text for the
 *                              UI element that follows it. Use SmartIO_MakeLabel2() if you want the same alignment value as other calls.
 * \param height                0: small  gap between this GUI slice and the following slice.
 *                              1: normal gap between this GUI slice and the following slice.
 *                              If you want the label to appear to be the label for the following element(s), use a height of 0.
 * \param label                 text for the label.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeLabel(uint16_t alignment, uint16_t height, char *label);

/** \brief Create a GUI slice with a label. You may change the text with SmartIO_AddText().
 * \todo
 *
 * \param alignment             0: right, 1: left. Alignment refers to the location of the UI element.
 * \param height                0: small  gap between this GUI slice and the following slice.
 *                              1: normal gap between this GUI slice and the following slice.
 *                              If you want the label to appear to be the label for the following element(s), use a height of 0.
 * \param label                 text for the label.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeLabel2(uint16_t alignment, uint16_t height, char *label);

/** \brief Create a GUI slice with a text box. You may change the text with SmartIO_AddText(). A textbox can have a slice icon.
 *         The height of the box is derived from "nlines" and the initial text. If you change the text later with SmartIO_AddText(),
 *         a vertical scroll bar may be added if the text gets to be too long to fit.
 *
 * \param alignment             0: right, 1: left. Alignment refers to the location of the UI element.
 * \param width                 1-200, width in virtual pixels of the text box. Obviously the low value is not 1, but some value that
 *                              would accommodate some text. 320 is the width of the full screen in virtual pixels.
 * \param nlines                approximate number of lines.
 * \param text                  text to be displayed.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeTextBox(uint16_t alignment, uint16_t width, uint16_t nlines, char *text);

/** \brief Create a GUI slice with a text box that takes the full width of the screen. You may change the text with SmartIO_AddText().
 *         The height of the box is derived from "nlines" and the initial text. If you change the text later with SmartIO_AddText(),
 *         a vertical scroll bar may be added if the text gets to be too long to fit.
 *
 * \param nlines                approximate number of lines.
 * \param text                  text to be displayed.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeMultilineBox(uint16_t nlines, char *text);

/** \brief Create a GUI slice and display a number within a white-background box. ndigits specifies the size.
 *
 * \param alignment             0: right, 1: left, 2: middle. Alignment refers to the location of the UI element.
 * \param ndigits               number of digits. If zero, then the width of the bounding box changes to accommodate the number.
 * \param initial_value         initial value of the counter.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeCounter(uint16_t alignment, uint16_t ndigits, uint32_t initial_value);

/** \brief Create a GUI slice with a progress bar.
 *
 * \param alignment             0: right, 1: left, 2: middle. Alignment refers to the location of the UI element.
 * \param variation             To Be Documented, see "Smart.IO User Guide" for detail.
 * \param percentage            0-100. Initial percentage.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeProgressBar(uint16_t alignment, uint16_t variation, uint16_t initial_percentage);

/** \brief Create a GUI slice with a progress circle.
 *
 * \param alignment             0: right, 1: left, 2: middle. Alignment refers to the location of the UI element.
 * \param variation             To Be Documented, see "Smart.IO User Guide" for detail.
 * \param percentage            0-100. Initial percentage.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeProgressCircle(uint16_t alignment, uint16_t variation, uint16_t initial_percentage);

/** \brief Create a GUI slice with a horizontal gauge.
 *
 * \param alignment             0: right, 1: left, 2: middle. Alignment refers to the location of the UI element.
 * \param variation             To Be Documented, see "Smart.IO User Guide" for detail.
 * \param percentage            0-100. Initial percentage.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeHGauge(uint16_t alignment, uint16_t variation, uint16_t initial_percentage);

/** \brief Create a GUI slice with a vertical gauge.
 *
 * \param alignment             0: right, 1: left, 2: middle. Alignment refers to the location of the UI element.
 * \param variation             To Be Documented, see "Smart.IO User Guide" for detail.
 * \param percentage            0-100. Initial percentage.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeVGauge(uint16_t alignment, uint16_t variation, uint16_t initial_percentage);

/** \brief Create a GUI slice with a semicircular gauge.
 *
 * \param alignment             0: right, 1: left, 2: middle. Alignment refers to the location of the UI element.
 * \param max_mark              value of the max mark, e.g. 10 for a 0 to 10 display. Division marks will be based on this value.
 * \param percentage            0-100. Initial percentage.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeSemiCircularGauge(uint16_t alignment, uint16_t max_mark, uint16_t initial_percentage);

/** \brief Create a GUI slice with a circular gauge.
 *
 * \param alignment             0: right, 1: left, 2: middle. Alignment refers to the location of the UI element.
 * \param max_mark              value of the max mark, e.g. 10 for a 0 to 10 display. Division marks will be based on this value.
 * \param percentage            0-100. Initial percentage.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeCircularGauge(uint16_t alignment, uint16_t max_mark, uint16_t initial_percentage);

/** \brief Create a GUI slice with a battery level display.
 *
 * \param alignment             0: right, 1: left, 2: middle. Alignment refers to the location of the UI element.
 * \param variation             0: blue fill level, 1: green fill level.
 * \param percentage            0-100. Initial percentage. 5 levels are displayed and percentage is round to nearest 20% segment.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeBatteryLevel(uint16_t alignment, uint16_t variation, uint16_t initial_percentage);

/** \brief Create a GUI slice with a RGB LED.
 *
 * \param alignment             0: right, 1: left, 2: middle. Alignment refers to the location of the UI element.
 * \param color                 0: Red, 1: Green, 2: Blue (RGB).
 * \param initial_state         0: off, 1: on.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeRGBLed(uint16_t alignment, uint16_t color, uint16_t initial_state);

/** \brief Create a GUI slice with a horizontal gauge with custom end colors.
 *
 * \param alignment             0: right, 1: left, 2: middle. Alignment refers to the location of the UI element.
 * \param start_color           32-bit web color value for the left end of gauge.
 * \param end_color             32-bit web color value for the right end of gauge.
 * \param percentage            0-100. Initial percentage.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeCustomHGauge(uint16_t alignment, uint32_t start_color, uint32_t end_color, uint16_t initial_percentage);

/** \brief Create a GUI slice with a vertical gauge with custom end colors.
 *
 * \param alignment             0: right, 1: left, 2: middle. Alignment refers to the location of the UI element.
 * \param start_color           32-bit web color value for the bottom end of gauge.
 * \param end_color             32-bit web color value for the top end of gauge.
 * \param percentage            0-100. Initial percentage.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_MakeCustomVGauge(uint16_t alignment, uint32_t start_color, uint32_t end_color, uint16_t initial_percentage);
/** @} */

/** \defgroup ffs-input  Freeform Input Elements
 *  @{
 */
/** \brief Create a Freeform Slice. All SmartIO_FFS_XXX functions take the returned handle as the first argument. A Freeform slice
 *         takes the full width of the screen (320 virtual pixels).
 *
 * \param y_size                height in virtual pxiels of the freeform slice.
 * \return a 16-bit handle, the ID for the freeform slice.
 */
tHandle SmartIO_MakeFreeformSlice(uint16_t y_size);

/** \brief Create an on/off button in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param variation             0 - 4, see "Smart.IO User Guide" for detail.
 * \param initial_value         0: off, 1: on, the initial value of the control.
 * \param callback              the user function to call when the app user changes the input.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_OnOffButton(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));

/** \brief Create a 3-pos button in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.

 * \param variation             0 - 8, see "Smart.IO User Guide" for detail.
 * \param initial_value         0 (left), 1 (mid), and 2 (right), the initial value of the control.
 * \param callback              the user function to call when the app user changes the input.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_3PosButton(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));

/** \brief Create a in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param variation             0 - 5, see "Smart.IO User Guide" for detail.
 * \param initial_value         0 to 0xFFFF, the initial value of the control.
 * \param callback              the user function to call when the app user changes the input.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_Incrementer(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));

/** \brief Create a slide in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param variation             0 - 3, see "Smart.IO User Guide" for detail.
 * \param initial_value         0 to 100 the initial value of the control.
 * \param callback              the user function to call when the app user changes the input.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_Slider(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));

/** \brief Create a picker in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param nentries              the number of entries.
 * \param callback              the user function to call when the app user changes the input.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_Picker(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t nentries, void (*callback)(uint16_t));

/** \brief Create a multi-selector in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param nentries              the number of entries.
 * \param isSingleSelectOnly    0: allow multi-selection, 1: single selection only.
 * \param callback              the user function to call when the app user changes the input.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_MultiSelector(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t nentries, uint16_t isSingleSelectOnly, void (*callback)(uint16_t));

/** \brief Create a number selector in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param default_val           the initial value to display.
 * \param low                   the lower bound of selectable number.
 * \param high                  the upper bound of selectable number.
 * \param callback              the user function to call when the app user changes the input.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_NumberSelector(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t default_val, uint16_t low, uint16_t high, void (*callback)(uint16_t));

/** \brief Create a digital time selector in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param display_24h           0: 12 hour display with AM/PM, 1: 24 hour display.
 * \param initial_value         initial time in "HH:MM" 24 hour format.
 * \param callback              the user function to call when the app user changes the input.
 *                              The function is called with HH and MM (numeric values, e.g. 12 05 for "12:05") 24 hour format.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_TimeSelector(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t display_24h, char *initial_value, void (*callback)(uint16_t, uint16_t));

/** \brief Create a analog time selector in an Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param display_24h           0: 12 hour display with AM/PM, 1: 24 hour display.
 * \param initial_value         initial time in "HH:MM" 24 hour format.
 * \param callback              the user function to call when the app user changes the input.
 *                              The function is called with HH and MM (numeric values, e.g. 12 05 for "12:05") 24 hour format.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_AnalogTimeSelector(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t display_24h, char *initial_value, void (*callback)(uint16_t, uint16_t));

/** \brief Create a calendar selector in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param initial_value         initial date in "MTH DD YYYY" format. MTH can be 1-12, or 3 letter code JAN/FEB/.../DEC. DD is 1-31.
 *                              Note that the phone OS may not support dates that are far in the past or in the future.
 * \param callback              the user function to call when the app user changes the input. The argument format is
 *                              "weekday MTH DD YYYY" where all 4 fields are integers. You can use
 *                              SmartIO_ConvertCalendarDay() to extract the data.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_CalendarSelector(tHandle handle, uint16_t loc_x, uint16_t loc_y, char *initial_value, void (*callback)(char *));

/** \brief Create a weekday selector in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param text                  label of the button. If null, then "OK" is used.
 * \param callback              the user function to call when the app user changes the input. An argument of 1 is always used.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_WeekdaySelector(tHandle handle, uint16_t loc_x, uint16_t loc_y, char *initial_value, void (*callback)(char *));

/** \brief Create a set of checkboxes in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param nentries              the number of entries.
 * \param callback              the user function to call when the app user changes the input. A bitmask of selected items is passed.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_Checkboxes(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t nentries, void (*callback)(uint16_t));

/** \brief Create a set of radio buttons in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param nentries              the number of entries.
 * \param callback              the user function to call when the app user changes the input. 1..nentries, indicating the selected button, is passed.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_RadioButtons(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t nentries, void (*callback)(uint16_t));

/** \brief Create a text entry in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param isRoundCorners        0: entry box has sharp corners, 1: entry box has round corners.
 * \param callback              the user function to call when the app user changes the input. The user clicks 'Done' on the virtual keyboard to finish text entry.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_TextEntry(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t isRoundCorners, void (*callback)(char *));

/** \brief Create a number entry in a Freeform slice. The vritual keyboard only displays numeric keys.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param isRoundCorners        0: entry box has sharp corners, 1: entry box has round corners.
 * \param callback              the user function to call when the app user changes the input. The user clicks 'Done' on the virtual keyboard to finish text entry.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_NumberEntry(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t isRoundCorners, void (*callback)(char *));

/** \brief Create a password entry in a Freeform slice. The entered character is replaced with a '*' as it is entered.
 *         Use SmartIO_AddText(handle, "password hint") to add hint.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param callback              the user function to call when the app user changes the input. The user clicks 'Done' on the virtual keyboard to finish text entry.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_PasswordEntry(tHandle handle, uint16_t loc_x, uint16_t loc_y, void (*callback)(char *));
/** @} */

/** \defgroup ffs-output  Freeform Output Elements
 *  @{
 */
/** \brief Create a textbox in a Freeform slice.  You may change the text with SmartIO_AddText(). A textbox can have a slice icon.
 *         The height of the box is derived from "nlines" and the initial text. If you change the text later with SmartIO_AddText(),
 *         a vertical scroll bar may be added if the text gets to be too long to fit.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param width                 1-200, width in virtual pixels of the text box. Obviously the low value is not 1, but some value that
 *                              would accommodate some text. 320 is the width of the full screen in virtual pixels.
 * \param nlines                approximate number of lines.
 * \param text                  text to be displayed.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_TextBox(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t width, uint16_t nlines, char *text);

/** \brief Create a counter in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param ndigits               number of digits. If zero, then the width of the bounding box changes to accommodate the number.
 * \param initial_value         initial value of the counter.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_Counter(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t ndigits, uint32_t initial_value);

/** \brief Create a progress bar in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param variation             To Be Documented, see "Smart.IO User Guide" for detail.
 * \param percentage            0-100. Initial percentage.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_ProgressBar(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t variation, uint16_t initial_percentage);

/** \brief Create a progress circle in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param variation             To Be Documented, see "Smart.IO User Guide" for detail.
 * \param percentage            0-100. Initial percentage.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_ProgressCircle(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t variation, uint16_t initial_percentage);

/** \brief Create a horizontal gauge in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param variation             To Be Documented, see "Smart.IO User Guide" for detail.
 * \param percentage            0-100. Initial percentage.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_HGauge(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t variation, uint16_t initial_percentage);

/** \brief Create a vertical gauge in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param variation             To Be Documented, see "Smart.IO User Guide" for detail.
 * \param percentage            0-100. Initial percentage.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_VGauge(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t variation, uint16_t initial_percentage);

/** \brief Create a semicircular gauge in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param max_mark              value of the max mark, e.g. 10 for a 0 to 10 display. Division marks will be based on this value.
 * \param percentage            0-100. Initial percentage.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_SemiCircularGauge(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t max_mark, uint16_t initial_percentage);

/** \brief Create a circular gauge in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param max_mark              value of the max mark, e.g. 10 for a 0 to 10 display. Division marks will be based on this value.
 * \param percentage            0-100. Initial percentage.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_CircularGauge(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t max_mark, uint16_t initial_percentage);

/** \brief Create a battery level display in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param variation             0: blue fill level, 1: green fill level.
 * \param percentage            0-100. Initial percentage. 5 levels are displayed and percentage is round to nearest 20% segment.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_BatteryLevel(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t variation, uint16_t initial_percentage);

/** \brief Create a RGB LED in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param color                 0: Red, 1: Green, 2: Blue (RGB).
 * \param initial_state         0: off, 1: on.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_RGBLed(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint16_t color, uint16_t initial_state);

/** \brief Create a custom horizontal gauge in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param start_color           32-bit web color value for the left end of gauge.
 * \param end_color             32-bit web color value for the right end of gauge.
 * \param percentage            0-100. Initial percentage.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_CustomHGauge(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint32_t start_color, uint32_t end_color, uint16_t initial_percentage);

/** \brief Create a custom vertical gauge in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param start_color           32-bit web color value for the bottom end of gauge.
 * \param end_color             32-bit web color value for the top end of gauge.
 * \param percentage            0-100. Initial percentage.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_CustomVGauge(tHandle handle, uint16_t loc_x, uint16_t loc_y, uint32_t start_color, uint32_t end_color, uint16_t initial_percentage);

/** \brief Create a label in a Freeform slice.
 *
 * \param handle                handle of the freeform slice.
 * \param loc_x                 X location of the upper left corner of the object, relative to the freeform slice.
 * \param loc_y                 Y location of the upper left corner of the object, relative to the freeform slice.
 * \param label                 text for the label.
 * \return a 16-bit handle, the ID for the GUI slice, handle+1 is the ID of the UI element.
 */
tHandle SmartIO_FFS_Label(tHandle handle, uint16_t loc_x, uint16_t loc_y, char *label);
/** @} */

/** \defgroup popup-input  Pop Up Input Elemnts. TO BE DOCUMENTED.
 *  @{
 */
tHandle SmartIO_PopupOnOffButton(uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));
tHandle SmartIO_Popup3PosButton(uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));
tHandle SmartIO_PopupIncrementer(uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));
tHandle SmartIO_PopupSlider(uint16_t variation, uint16_t initial_value, void (*callback)(uint16_t));

tHandle SmartIO_PopupPicker(uint16_t nentries, void (*callback)(uint16_t));
tHandle SmartIO_PopupMultiSelector(uint16_t nentries, uint16_t isSingleSelectOnly, void (*callback)(uint16_t));
tHandle SmartIO_PopupNumberSelector(uint16_t default_val, uint16_t low, uint16_t high, void (*callback)(uint16_t));
tHandle SmartIO_PopupTimeSelector(uint16_t display_24h, char *initial_value, void (*callback)(uint16_t, uint16_t));
tHandle SmartIO_PopupAnalogTimeSelector(uint16_t display_24h, char *initial_value, void (*callback)(uint16_t, uint16_t));
tHandle SmartIO_PopupCalendarSelector
                                (tHandle handle, uint16_t loc_x, uint16_t loc_y, char *initial_value, void (*callback)(char *));
tHandle SmartIO_PopupWeekdaySelector(char *initial_value, void (*callback)(char *));

tHandle SmartIO_PopupCheckboxes(uint16_t nentries, void (*callback)(uint16_t));
tHandle SmartIO_PopupRadioButtons(uint16_t nentries, void (*callback)(uint16_t));

tHandle SmartIO_PopupTextEntry(uint16_t isRoundCorners, void (*callback)(char *));
tHandle SmartIO_PopupNumberEntry(uint16_t isRoundCorners, void (*callback)(char *));

// Use SmartIO_AddText(tHandle handle, (uint16_t loc_x, uint16_t loc_y, handle, "password hint") to add hint
tHandle SmartIO_PopupPasswordEntry(void (*callback)(char *));
/** @} */

/** \defgroup popup-output  Pop Up Output Elemnts. TO BE DOCUMENTED.
 *  @{
 */
// Text Box is meant to be narrower and can have a slice icon
// whereas multiline box takes the full width
tHandle SmartIO_PopupTextBox(uint16_t width, uint16_t nlines, char *text);
tHandle SmartIO_PopupMultilineBox(uint16_t nlines, char *text);
tHandle SmartIO_PopupCounter(uint16_t ndigits, uint32_t initial_value);

tHandle SmartIO_PopupProgressBar(uint16_t variation, uint16_t initial_percentage);
tHandle SmartIO_PopupProgressCircle(uint16_t variation, uint16_t initial_percentage);
tHandle SmartIO_PopupHGauge(uint16_t variation, uint16_t initial_percentage);
tHandle SmartIO_PopupVGauge(uint16_t variation, uint16_t initial_percentage);
tHandle SmartIO_PopupSemiCircularGauge(uint16_t max_mark, uint16_t initial_percentage);
tHandle SmartIO_PopupCircularGauge(uint16_t max_mark, uint16_t initial_percentage);
tHandle SmartIO_PopupBatteryLevel(uint16_t variation, uint16_t initial_percentage);
tHandle SmartIO_PopupRGBLed(uint16_t color, uint16_t initial_state);

tHandle SmartIO_PopupCustomHGauge(uint32_t start_color, uint32_t end_color, uint16_t initial_percentage);
tHandle SmartIO_PopupCustomVGauge(uint32_t start_color, uint32_t end_color, uint16_t initial_percentage);

tHandle SmartIO_PopupLabel(char *label);

tStatus SmartIO_AppendPopup(tHandle source, tHandle next);
/** @} */

/** \defgroup misc-UI   Miscellaneous UI Commands
 *  @{
 */
/** \brief Popup an alert box
 *
 * \param variation             variation of the alert. See "Smart.IO User Guide"
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_PopupAlert(uint16_t variation);

/** \brief Set the GUI slice icon. Slice icon is valid for any UI slice that allows alignment.
 *
 * \param handle                handle of the GUI slice (NOT the handle+1 for the UI element).
 * \param icon_enum             one of the "enum SMARTIO_ICON" values.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_SetSliceIcon(tHandle handle, uint16_t icon_enum);

/** \brief Set the text attribute. Can be text label for a GUI slice, which is valid for GUI slice with
 *         on/off, 2-pos, incrementer, picker, number_selector, time_selector, OK_LinkTo, Progress Circle, Gauges, LED, Battery_level.
 *         Can also set text for UI elements such as text box, multi-line text, etc.
 *
 * \param handle                handle of the object. For GUI slice, be sure to use the ID of the GUI slice, and not handle+1 of the UI element.
 * \param text                  text.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_AddText(tHandle handle, char *text);

/** \brief Clear the text field. Works with any text field, but mostly useful for TEXT_BOX and MULTILINE_TEXT
 *
 * \param handle                handle of the object. For GUI slice, be sure to use the ID of the GUI slice, and not handle+1 of the UI element.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_ClearText(tHandle handle);

/** \brief Add a list item to a suitable UI element. NOTE: list items are added in order, and there is
 *         no mechanism to delete or arrange list items.
 *
 * \param handle                handle of the UI element.
 * \param text                  text for the element.
 * \return a 16-bit handle for the list item.
 */
tHandle SmartIO_AddListItem(tHandle handle, char *text);

/** \brief Set the background color of some objects.
 *
 * \param handle                handle of the object.
 * \param color                 32-bit web color value.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_FillColor(tHandle handle, uint32_t color);

/** \brief Set the font used for an object.
 * \todo NOT YET IMPLEMENTED.
 *
 * \param handle                handle of the object.
 * \param isSansSerifFont       0: serif font, 1: sans serif font.
 * \param fontsize              0: small, 1: normal, 2: large. See "Smart.IO User Guide" for detail.
 * \param font_color            32-bit web color value.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_SetFont(tHandle handle, uint16_t isSansSerifFont, uint16_t fontsize, uint32_t font_color);

/** \brief Create (invisible) spacer slices. Space slices works with SmartIO_AutoBalance() to create good looking UI.
 *         See "Smart.IO User Guide" for detail.
 *
 * \param nslices               number of spacer slices to create at this location,
 * \return a 16-bit handle for the first slice. handle+1 is the next slice etc. up to handle+nslices.
 */
tHandle SmartIO_MakeSpacerSlice     (int nslices);

/** \brief Auto-balance the UI elements on the page. See "Smart.IO User Guide" for detail.
 *
 * \param handle                handle of the page.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_AutoBalance         (tHandle handle);
/** @} */

/** \defgroup update-cmd   UI Update Commands. These are useful for restoring UI controls to previous settings.
 *  @{
 */

/** \brief A generic function to update the value of a UI element.
 *
 * \param handle                handle of the UI element.
 * \param value                 new value.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateIntValue      (tHandle handle, uint16_t value);

/** \brief A generic function to update a UI element with two updateable values.
 *
 * \param handle                handle of the UI element.
 * \param value                 new value.
 * \param value2                new value.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateIntValue2     (tHandle handle, uint16_t value, uint16_t value2);

/** \brief A generic function to update a UI element with three updateable values.
 *
 * \param handle                handle of the UI element.
 * \param value                 new value.
 * \param value2                new value.
 * \param value3                new value.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateIntValue3     (tHandle handle, uint16_t value, uint16_t value2, uint16_t value3);

/** \brief A generic function to update the string value of a UI element.
 *
 * \param handle                handle of the UI element.
 * \param text                  new (char *) value.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateStringValue   (tHandle handle, char *text);

/** \brief Update the value of an On/Off button.
 *
 * \param handle                handle of the UI element.
 * \param value                 0: off, 1: on.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateOnOffButton	(tHandle handle, uint16_t value);

/** \brief Update the value of a 3-pos button.
 *
 * \param handle                handle of the UI element.
 * \param value                 0: left, 1: middle, 2: right.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_Update3PosButton	(tHandle handle, uint16_t value);

/** \brief Update the value of an incrementer.
 *
 * \param handle                handle of the UI element.
 * \param value                 new value to display.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateIncrementer	(tHandle handle, uint16_t value);

/** \brief Update the value of a slider.
 *
 * \param handle                handle of the UI element.
 * \param value                 new percentage.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateSlider	    (tHandle handle, uint16_t value);

/** \brief Update the value of an expandable list.
 *
 * \param handle                handle of the UI element.
 * \param value                 new selected item index.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateExpandableList(tHandle handle, uint16_t value);

/** \brief Update the value of a Picker.
 *
 * \param handle                handle of the UI element.
 * \param value                 new selected item index.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdatePicker	    (tHandle handle, uint16_t value);


/** \brief Update the value of a multi-selector.
 *
 * \param handle                handle of the UI element.
 * \param value                 new bitmask of selected items.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateMultiSelector	(tHandle handle, uint16_t value);


/** \brief Update the value of a number selector.
 *
 * \param handle                handle of the UI element.
 * \param value                 new number.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateNumberSelector(tHandle handle, uint16_t value);

/** \brief Update the value of a set of checkboxes.
 *
 * \param handle                handle of the UI element.
 * \param value                 bitmask of selected items.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateCheckboxes	(tHandle handle, uint16_t value);

/** \brief Update the value of a set of radio buttons.
 *
 * \param handle                handle of the UI element.
 * \param value                 new selected item index.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateRadioButtons	(tHandle handle, uint16_t value);

/** \brief Update the value of a counter.
 *
 * \param handle                handle of the UI element.
 * \param value                 new counter value.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateCounter   	(tHandle handle, uint16_t value);

/** \brief Update the value of a progress bar.
 *
 * \param handle                handle of the UI element.
 * \param value                 new percentage.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateProgressBar	(tHandle handle, uint16_t value);

/** \brief Update the value of a proress circle.
 *
 * \param handle                handle of the UI element.
 * \param value                 new percentage.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateProgressCircle(tHandle handle, uint16_t value);

/** \brief Update the value of a horizontal gauge.
 *
 * \param handle                handle of the UI element.
 * \param value                 new percentage.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateHGauge	    (tHandle handle, uint16_t value);

/** \brief Update the value of a vertical gauge.
 *
 * \param handle                handle of the UI element.
 * \param value                 new percentage.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateVGauge	    (tHandle handle, uint16_t value);

/** \brief Update the value of a semicircular gauge.
 *
 * \param handle                handle of the UI element.
 * \param value                 new percentage.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateSemiCircularGauge(tHandle handle, uint16_t value);

/** \brief Update the value of a circular gauge.
 *
 * \param handle                handle of the UI element.
 * \param value                 new percentage.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateCircularGauge	(tHandle handle, uint16_t value);

/** \brief Update the value of a custom horizontal gauge.
 *
 * \param handle                handle of the UI element.
 * \param value                 new percentage.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateCustomHGauge	(tHandle handle, uint16_t value);

/** \brief Update the value of a custom vertical gauge.
 *
 * \param handle                handle of the UI element.
 * \param value                 new percentage.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateCustomVGauge	(tHandle handle, uint16_t value);

/** \brief Update the value of a battery level.
 *
 * \param handle                handle of the UI element.
 * \param value                 new value. 5 levels are displayed and percentage is round to nearest 20% segment.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateBatteryLevel	(tHandle handle, uint16_t value);

/** \brief Update the value of a RGB Led.
 *
 * \param handle                handle of the UI element.
 * \param color                 0: Red, 1: Green, 2: Blue (RGB).
 * \param on_off                0: off, 1: on.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateRGBLed        (tHandle handle, uint16_t color, uint16_t on_off);

/** \brief Update the value of a text box or mutli-line text box.
 *
 * \param handle                handle of the UI element.
 * \param value                 new text to display.
 * \return 0 if success, -1 otherwise
 */
tStatus SmartIO_UpdateTextBox       (tHandle handle, char *text);
/** @} */

/** \defgroup static-opbj   Static Objects NOT YET IMPLEMENTED
 *  @{
 */
tHandle SmartIO_MakeLine(uint16_t loc_x, uint16_t loc_y, uint16_t end_x, uint16_t end_y, uint16_t color);
tHandle SmartIO_MakeCircle(uint16_t loc_x, uint16_t loc_y, uint16_t diameter_x, uint16_t diameter_y, uint16_t outline_color, uint16_t fill_color);
tHandle SmartIO_MakeRectangle(uint16_t loc_x, uint16_t loc_y, uint16_t end_x, uint16_t end_y, uint16_t outline_color, uint16_t fill_color);
tHandle SmartIO_MakeTriangle(uint16_t loc_x, uint16_t loc_y, uint16_t loc_z, uint16_t outline_color, uint16_t fill_color);
tHandle SmartIO_MakeIcon(uint16_t);
tHandle SmartIO_MakeGraphics(uint16_t);
/** @} */

/** \defgroup smartio-sys   SmartIO Module Commands
 *  @{
 */
/** \brief Read from the Smart.IO internal EEPROM. Useful for storing UI state. See "Smart.IO User Guide" for detail.
 *
 * \param address               EEPROM address, must not exceed SMARTIO_MAX_EEPROM_SIZE-1.
 * \param length                number of bytes to read. address+length must not exceed SMARTIO_MAX_EEPROM_SIZE.
 *                              length also cannot exceed HOST_SRAM_POOL_SIZE.
 * \return content of the EEPROM or null if error.
 */
unsigned char *SmartIO_ReadEEPROM(uint16_t address, uint16_t length);

/** \brief Write to the Smart.IO internal EEPROM. Useful for storing UI state. See "Smart.IO User Guide" for detail.
 *
 * \param address               EEPROM address, must not exceed SMARTIO_MAX_EEPROM_SIZE-1.
 * \param length                number of bytes to write. address+length must not exceed SMARTIO_MAX_EEPROM_SIZE.
 *                              length also cannot exceed HOST_SRAM_POOL_SIZE.
 * \param buffer                buffer containing the data to be written.
 * \return 0 if success, -1 otherwise.
 */
tStatus SmartIO_WriteEEPROM(uint16_t address, uint16_t length, unsigned char *buffer);

/** \brief Set the onboard Smart.IO LED.
 *
 * \param led                   reserved. Zero should be used.
 * \return 0 if success, -1 otherwise.
 */
tStatus SmartIO_SetLED(uint16_t led);

/** \brief Clear the onboard Smart.IO LED.
 *
 * \param led                  reserved. Zero should be used.
 * \return 0 if success, -1 otherwise.
 */
tStatus SmartIO_ClearLED(uint16_t led);

/** \brief Toggle the onboard Smart.IO LED.
 *
 * \param led                   reserved. Zero should be used.
 * \return 0 if success, -1 otherwise.
 */
tStatus SmartIO_ToggleLED(uint16_t led);

/** \brief Genearte a random number using the BlueNRG1 Smart.IO MCU. The quality of randomness is very high as it
 *         is hardware based.
 *
 * \return 32-bit random number.
 */
uint32_t SmartIO_GenRandomNumber(void);

/** \brief Enable or Disable the DebugUART. NOT YET IMPLEMENTED.
 * \todo
 * \return 0 if success, -1 otherwise.
 */
tStatus SmartIO_DebugUART(uint16_t on_off);

/** \brief Return 64-bit unique ID. Always return the same ID from the same Smart.IO module.
 *
 * \return two 32-bit values
 */
uint32_t *SmartIO_GetUniqueID(void);
/** @} */

/** \defgroup phone-sys   Phone Commands NOT YET IMPLEMENTED
 *  @{
 */
// Phone Commands
char *SmartIO_GetPhoneTime(void);
char *SmartIO_GetPhoneGPS(void);
tStatus SmartIO_Beep(void);
/** @} */
