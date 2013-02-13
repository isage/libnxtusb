/**
 * @file libnxtusb.h
 * @author Epifanov Ivan <isage.dna@gmail.com>
 * @version 1.0
 *
 * @section LICENSE
 *
 *            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *                        Version 2, December 2004
 *    
 *     Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>
 *    
 *     Everyone is permitted to copy and distribute verbatim or modified
 *     copies of this license document, and changing it is allowed as long
 *     as the name is changed.
 *    
 *                 DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *        TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 *    
 *       0. You just DO WHAT THE FUCK YOU WANT TO
 *
 * @section DESCRIPTION
 *
 * Implementation of Lego NXT direct-command protocol. Public header
 */

/**
 * \mainpage libnxtusb-1.0 api reference
 * \section sintro Introduction
 * libnxtusb provides direct-commant usb interface to Lego NXT brick
 *
 * \section sdevice Device handling
 * Refer to \ref device
 *
 * \section sdc Direct commands
 * Refer to \ref dc
 *
 * \section serror Error handling
 * Refer to \ref error
 */


#ifndef LIBNXTUSB_H
#define LIBNXTUSB_H
#include <stdint.h>
#include <libusb-1.0/libusb.h>

static uint8_t libnxtusb_error;

/** \ingroup dc
 * Output ports
 */
typedef enum {
  /** Port A */
  NXT_OUT_A = 0x00,
  /** Port B */
  NXT_OUT_B = 0x01,
  /** Port C */
  NXT_OUT_C = 0x02,
  /** All ports*/
  NXT_OUT_ALL = 0xFF
} libnxtusb_out_t;

/** \ingroup dc
 * Input ports
 */
typedef enum {
  /** Port 1 */
  NXT_IN_1 = 0x00,
  /** Port 2 */
  NXT_IN_2 = 0x01,
  /** Port 3*/
  NXT_IN_3 = 0x02,
  /** Port 4*/
  NXT_IN_4 = 0x03
} libnxtusb_in_t;

/** \ingroup dc
 * Motor modes
 */
typedef enum {
  /** Turn motor on */
  NXT_MOTOR_MODE_ON = 0x01,
  /** Turn brake on */
  NXT_MOTOR_MODE_BRAKE = 0x02,
  /** Turn regulation on */
  NXT_MOTOR_MODE_REGULATED = 0x04
} libnxtusb_motor_mode_t;

/** \ingroup dc
 * Motor regulations
 */
typedef enum {
  /** Idle */
  NXT_MOTOR_REGULATION_IDLE = 0x01,
  /** Speed */
  NXT_MOTOR_REGULATION_SPEED,
  /** Sync */
  NXT_MOTOR_REGULATION_SYNC
} libnxtusb_motor_regulation_t;

/** \ingroup dc
 * Motor runstates
 */
typedef enum {
  /** Idle */
  NXT_MOTOR_RUNSTATE_IDLE = 0x00,
  /** Ramping up */
  NXT_MOTOR_RUNSTATE_RAMPUP = 0x10,
  /** Running */
  NXT_MOTOR_RUNSTATE_RUNNING = 0x20,
  /** Ramping down */
  NXT_MOTOR_RUNSTATE_RAMPDOWN = 0x40
} libnxtusb_motor_runstate_t;

/** \ingroup dc
 * Sensors types
 */
typedef enum {
  /** No sensor */
  NXT_SENSOR_NONE = 0x00,
  /** Button */
  NXT_SENSOR_SWITCH,
  /** Temperature */
  NXT_SENSOR_TEMPERATURE,
  /** Light/reflection */
  NXT_SENSOR_REFLECTION,
  /** Angle */
  NXT_SENSOR_ANGLE,
  /** Active light */
  NXT_SENSOR_LIGHT_ACTIVE,
  /** Inactive light */
  NXT_SENSOR_LIGHT_INACTIVE,
  /** Sound (measure in DB) */
  NXT_SENSOR_SOUND_DB,
  /** Sound (measure in DBA) */
  NXT_SENSOR_SOUND_DBA,
  /** Custom sensor */
  NXT_SENSOR_CUSTOM,
  /** Lowspeed (aka ultrasonic) */
  NXT_SENSOR_LOWSPEED,
  /** Lowspeed (9V) */
  NXT_SENSOR_LOWSPEED_9V,

  NXT_SENSOR_NOST
} libnxtusb_sensor_type_t;

/** \ingroup dc
 * Sensor modes
 */
typedef enum {
  /** Raw values */
  NXT_SENSOR_MODE_RAW = 0x00,
  /** Boolean */
  NXT_SENSOR_MODE_BOOLEAN = 0x20,
  /** ??? */
  NXT_SENSOR_MODE_TRANSITION_CNT = 0x40,
  /** ??? */
  NXT_SENSOR_MODE_PERIOD_CNT = 0x60,
  /** ??? */
  NXT_SENSOR_MODE_PCT_FULLSCALE = 0x80,
  /** Temp. in celsius */
  NXT_SENSOR_MODE_CELSIUS = 0xA0,
  /** Temp. in fahrenheit */
  NXT_SENSOR_MODE_FAHRENHEIT = 0xC0,
  /** Angles */
  NXT_SENSOR_MODE_ANGLE_STEP = 0xE0,
  /** Slope  */
  NXT_SENSOR_MASK_SLOPE = 0x1F,

  NXT_SENSOR_MASK_MODE = 0xE0
} libnxtusb_sensor_mode_t;

/** \ingroup error
 * Command statuses
 *
 * @sa libnxtusb_errtostr
 */
typedef enum {
  NXT_STATUS_OK = 0x00,
  NXT_STATUS_PENDING = 0x20,
  NXT_STATUS_QUEUE_EMPTY = 0x40,
  NXT_STATUS_SYS_NO_MORE_HANDLES = 0x81,
  NXT_STATUS_SYS_NO_SPACE = 0x82,
  NXT_STATUS_SYS_NO_MORE_FILES = 0x83,
  NXT_STATUS_SYS_EOF_EXPECTED = 0x84,
  NXT_STATUS_SYS_EOF = 0x85,
  NXT_STATUS_SYS_NOT_A_LINEAR_FILE = 0x86,
  NXT_STATUS_SYS_FILE_NOT_FOUND = 0x87,
  NXT_STATUS_SYS_HANDLE_ALREADY_CLOSED = 0x88,
  NXT_STATUS_SYS_NO_LINEAR_SPACE = 0x89,
  NXT_STATUS_SYS_UNDEFINED_ERROR = 0x8A,
  NXT_STATUS_SYS_FILE_BUSY = 0x8B,
  NXT_STATUS_SYS_NO_WRITE_BUFFERS = 0x8C,
  NXT_STATUS_SYS_APPEND_IMPOSSIBLE = 0x8D,
  NXT_STATUS_SYS_FILE_IS_FULL = 0x8E,
  NXT_STATUS_SYS_FILE_EXISTS = 0x8F,
  NXT_STATUS_SYS_MODULE_NOT_FOUND = 0x90,
  NXT_STATUS_SYS_OUT_OF_BOUNDARY = 0x91,
  NXT_STATUS_SYS_ILLEGAL_FILENAME = 0x92,
  NXT_STATUS_SYS_ILLEGAL_HANDLE = 0x93,
  NXT_STATUS_REQUEST_FAILED = 0xBD,
  NXT_STATUS_UNKNOWN_OPCODE = 0xBE,
  NXT_STATUS_INSANE_PACKET = 0xBF,
  NXT_STATUS_DATA_OUT_OF_RANGE = 0xC0,
  NXT_STATUS_COMMUNICATION_ERROR = 0xDD,
  NXT_STATUS_NO_BUFFER = 0xDE,
  NXT_STATUS_CHANNEL_INVALID = 0xDF,
  NXT_STATUS_CHANNEL_BUSY = 0xE0,
  NXT_STATUS_NO_ACTIVE_PROGRAM = 0xEC,
  NXT_STATUS_ILLEGAL_SIZE = 0xED,
  NXT_STATUS_ILLEGAL_MAILBOX = 0xEE,
  NXT_STATUS_ILLEGAL_FIELD = 0xEF,
  NXT_STATUS_BAD_IO = 0xF0,
  NXT_STATUS_NO_MEMORY = 0xFB,
  NXT_STATUS_BAD_ARGS = 0xFF
} libnxtusb_status_t;

#pragma pack(push,1)

/**
 *  Output port state
 */
typedef struct {
  uint8_t type;
  uint8_t opcode;
  uint8_t status;
  /** Output port */
  uint8_t port;
  /** Current power */
  int8_t power;
  /** Current mode */
  uint8_t mode;
  /** Current regulation */
  uint8_t regulation;
  /** Current turn ratio */
  int8_t turn_ratio;
  /** Current run state*/
  uint8_t run_state;
  /** Current tacho limit */
  uint32_t tacho_limit;
  /** Current tacho count relative to last reset */
  int32_t tacho_count;
  /** Current position relative to last position */
  int32_t block_tacho_count;
  /** Current position relative to last reset */
  int32_t rotation_count;
} libnxtusb_outputstate_t;

/**
 *  Input port state
 */
typedef struct {
  uint8_t type;
  uint8_t opcode;
  uint8_t status;
  /** Input port */
  uint8_t port;
  /** 1 = valid data, 0 = invalid */
  uint8_t valid;
  /** 1 if calibration file found and used for calibrated_value */
  uint8_t calibrated;
  /** Sensor type*/
  uint8_t sensor_type;
  /** Sensor mode*/
  uint8_t sensor_mode;
  /** Raw A/D value */
  uint16_t raw_value;
  /** Normalized value, type-depended, 0-1023 */
  uint16_t normalized_value;
  /** Scaled value, mode-depended */
  int16_t scaled_value;
  /** Calibrated value. Scaled, according to calibration. CURRENTLY UNUSED*/
  int16_t calibrated_value;
} libnxtusb_inputstate_t;
#pragma pack(pop)

/**
 * Nxt brick handle
 */
typedef struct {
  libusb_device_handle *handle;
  libusb_context *ctx;
} libnxtusb_device_handle;

/**
 * \defgroup device Device (de-)initialisation.
 */

/** \ingroup device
 * Find and open nxt device
 * @return libnxtusb_device_handle handle to nxt brick
 */
libnxtusb_device_handle *libnxtusb_getnxt();

/** \ingroup device
 * Close nxt device
 * @param nxtdev libnxtusb_device_handle handle to nxt brick
 */
int libnxtusb_closenxt(libnxtusb_device_handle *nxtdev);


/**
 * \defgroup error Error handling.
 */

/** \ingroup error
 *  Convert error code to string
 * @return char* error string
 */
const char *libnxtusb_errstr();

/**
 * \defgroup dc Direct commands.
 */

/** \ingroup dc
 *  Start program stored on nxt brick
 * @param handle nxt brick handle
 * @param filename file name of program in 15.3 format
 * @return 0 on success, -1 on failure
 */
int nxt_start_program(const libnxtusb_device_handle *handle, const char *filename);

/** \ingroup dc
 *  Stop currently running program
 * @param handle nxt brick handle
 * @return 0 on success, -1 on failure
 */
int nxt_stop_program(const libnxtusb_device_handle *handle);

/** \ingroup dc
 *  Play sound file stored on NXT brick
 * @param handle nxt brick handle
 * @param filename filename of sound file in 15.3 format
 * @param loop 1 = loop, 0 = play once
 * @return loop or -1 on error
 */
int nxt_play_soundfile(const libnxtusb_device_handle *handle, const char *filename, const unsigned short loop);

/** \ingroup dc
 *  Play tone 
 * @param handle nxt brick handle
 * @param freq Frequency in Hz (from 200 to 14000)
 * @param duration Duration in ms
 * @return 
 */
int nxt_play_tone(
        const libnxtusb_device_handle *handle,
        const unsigned int freq, const unsigned int duration
        );

/** \ingroup dc
 *  Set output port state
 * @param handle nxt brick handle 
 * @param port libnxtusb_out_t port
 * @param power Power (-100 to 100)
 * @param mode libnxtusb_motor_mode_t Mode 
 * @param regulation libnxtusb_motor_regulation_t Regulation
 * @param turn_ratio Turn ratio (-100 to 100)
 * @param run_state libnxtusb_motor_runstate_t Run state
 * @param tacho_limit Tacho limit (0 = run forever)
 * @return 0 on success, -1 on failure
 */
int nxt_set_output_state(
        const libnxtusb_device_handle *handle, const libnxtusb_out_t port,
        const int8_t power, const libnxtusb_motor_mode_t mode, const libnxtusb_motor_regulation_t regulation,
        const int8_t turn_ratio, const libnxtusb_motor_runstate_t run_state, const uint32_t tacho_limit
        );

/** \ingroup dc
 *  Set input port state
 * @param handle nxt brick handle
 * @param port libnxtusb_in_t Input port
 * @param stype libnxtusb_sensor_type_t Sensor type
 * @param smode libnxtusb_sensor_mode_t Sensor mode
 * @return 0 on success, -1 on failure
 */
int nxt_set_input_mode(
        const libnxtusb_device_handle *handle, const libnxtusb_in_t port,
        const libnxtusb_sensor_type_t stype, const libnxtusb_sensor_mode_t smode
        );

/** \ingroup dc
 *  Get output port state
 * @param handle nxt brick handle
 * @param port libnxtusb_out_t Port
 * @param out libnxtusb_outputstate_t* Output state (preallocated)
 * @return 0 on success, -1 on failure
 */
int nxt_get_output_state(
        const libnxtusb_device_handle *handle, const libnxtusb_out_t port, libnxtusb_outputstate_t *out
        );

/** \ingroup dc
 *  Get input values
 * @param handle nxt brick handle
 * @param port libnxtusb_in_t Port
 * @param out libnxtusb_inputstate_t* Input values (preallocated)
 * @return 0 on success, -1 on failure
 */
int nxt_get_input_values(
        const libnxtusb_device_handle *handle, const libnxtusb_in_t port, libnxtusb_inputstate_t *out
        );

/** \ingroup dc
 *  Reset scaled values
 * @param handle nxt brick handle
 * @param port libnxtusb_in_t port
 * @return 0 on success, -1 on failure
 */
int nxt_reset_input_scaled_value(
        const libnxtusb_device_handle *handle, const libnxtusb_in_t port
        );

/** \ingroup dc
 *  Get battery level in millivolts
 * @param handle nxt brick handle
 * @param mv voltage in mv
 * @return 0 on success, -1 on failure
 */
int nxt_get_battery_level_mv(const libnxtusb_device_handle *handle, unsigned int* mv);

#endif

