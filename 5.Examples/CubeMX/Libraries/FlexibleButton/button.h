#ifndef __FLEXIBLE_BUTTON_H__
#define __FLEXIBLE_BUTTON_H__

#include <stdint.h>

#define BTN_SCAN_FREQ_HZ             50  // How often button_scan () is called
#define MS_TO_SCAN_CNT(ms)           (ms / (1000 / BTN_SCAN_FREQ_HZ))

/* Multiple clicks interval, default 300ms */
#define MAX_MULTIPLE_CLICKS_INTERVAL (MS_TO_SCAN_CNT(300))

typedef void (*button_response_callback)(void*);

typedef enum {
    BTN_EVENT_DOWN = 0,
    BTN_EVENT_CLICK,
    BTN_EVENT_DOUBLE_CLICK,
    BTN_EVENT_REPEAT_CLICK,
    BTN_EVENT_SHORT_START,
    BTN_EVENT_SHORT_UP,
    BTN_EVENT_LONG_START,
    BTN_EVENT_LONG_UP,
    BTN_EVENT_LONG_HOLD,
    BTN_EVENT_LONG_HOLD_UP,
    BTN_EVENT_MAX,
    BTN_EVENT_NONE,
} button_event_t;

/**
 * button_t
 * 
 * @brief Button data structure
 *        Below are members that need to user init before scan.
 * 
 * @member next
 *         Internal use.
 *         One-way linked list, pointing to the next button.
 * 
 * @member usr_button_read
 *         User function is used to read button vaule.
 * 
 * @member cb
 *         Button event callback function.
 * 
 * @member scan_cnt
 *         Internal use, user read-only.
 *         Number of scans, counted when the button is pressed, plus one per scan cycle.
 * 
 * @member click_cnt
 *         Internal use, user read-only.
 *         Number of button clicks
 * 
 * @member max_multiple_clicks_interval
 *         Multiple click interval. Default 'MAX_MULTIPLE_CLICKS_INTERVAL'.
 *         Need to use MS_TO_SCAN_CNT to convert milliseconds into scan cnts.
 * 
 * @member debounce_tick
 *         Debounce. Not used yet.
 *         Need to use MS_TO_SCAN_CNT to convert milliseconds into scan cnts.
 * 
 * @member short_press_start_tick
 *         Short press start time. Requires user configuration.
 *         Need to use MS_TO_SCAN_CNT to convert milliseconds into scan cnts.
 * 
 * @member long_press_start_tick
 *         Long press start time. Requires user configuration.
 *         Need to use MS_TO_SCAN_CNT to convert milliseconds into scan cnts.
 * 
 * @member long_hold_start_tick
 *         Long hold press start time. Requires user configuration.
 * 
 * @member id
 *         Button id. Requires user configuration.
 *         When multiple buttons use the same button callback function, 
 *         they are used to distinguish the buttons. 
 *         Each button id must be unique.
 * 
 * @member pressed_logic_level
 *         Requires user configuration.
 *         The logic level of the button pressed, each bit represents a button.
 * 
 * @member event
 *         Internal use, users can call 'button_event_read' to get current button event.
 *         Used to record the current button event.
 * 
 * @member status
 *         Internal use, user unavailable.
 *         Used to record the current state of buttons.
 * 
*/
typedef struct button {
    struct button* next;

    uint8_t (*usr_button_read)(void*);
    button_response_callback cb;

    uint16_t scan_cnt;
    uint16_t click_cnt;
    uint16_t max_multiple_clicks_interval;

    uint16_t debounce_tick;
    uint16_t short_press_start_tick;
    uint16_t long_press_start_tick;
    uint16_t long_hold_start_tick;

    uint8_t id;
    uint8_t pressed_logic_level : 1;
    uint8_t event               : 4;
    uint8_t status              : 3;
} button_t;

#ifdef __cplusplus
extern "C" {
#endif

int32_t        button_register(button_t* button);
button_event_t button_event_read(button_t* button);
uint8_t        button_scan(void);

#ifdef __cplusplus
}
#endif
#endif /* __FLEXIBLE_BUTTON_H__ */
