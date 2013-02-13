/**
 * @file libnxtusb.c
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
 * Implementation of Lego NXT direct-command protocol.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libnxtusb.h"

// for libusb
const int NXT_USB_ID_VENDOR_LEGO = 0x0694;
const int NXT_USB_ID_PRODUCT_NXT = 0x0002;
const int NXT_USB_ENDPOINT_OUT = 0x01; //1
const int NXT_USB_ENDPOINT_IN = 0x82; //130
const int NXT_USB_TIMEOUT = 1000;
const int NXT_USB_READSIZE = 64;
const int NXT_USB_INTERFACE = 0;


// packet type

enum {
  NXT_DIRECT_COMMAND_DOREPLY = 0x00,
  NXT_SYSTEM_COMMAND_DOREPLY = 0x01,
  NXT_COMMAND_REPLY = 0x02,
  NXT_DIRECT_COMMAND_NOREPLY = 0x80,
  NXT_SYSTEM_COMMAND_NOREPLY = 0x81,
};

//packet opcodes

enum {
  NXT_OPCODE_STARTPROGRAM = 0x00,
  NXT_OPCODE_STOPPROGRAM = 0x01,
  NXT_OPCODE_PLAYSOUND = 0x02,
  NXT_OPCODE_PLAYTONE = 0x03,
  NXT_OPCODE_SET_OUTPUTSTATE = 0x04,
  NXT_OPCODE_SET_INPUTMODE = 0x05,
  NXT_OPCODE_GET_OUTPUTSTATE = 0x06,
  NXT_OPCODE_GET_INPUTVALUES = 0x07,
  NXT_OPCODE_RESET_INPUT_SCALEDVALUES = 0x08,
  NXT_OPCODE_MESSAGE_WRITE = 0x09,
  NXT_OPCODE_MESSAGE_READ = 0x13,
  NXT_OPCODE_RESET_MOTOR_POSITION = 0x0A,
  NXT_OPCODE_BATTERYLEVEL = 0x0B,
  NXT_OPCODE_STOP_SOUND = 0x0C,
  NXT_OPCODE_KEEPALIVE = 0x0D,
  NXT_OPCODE_LS_GET_STATUS = 0x0E,
  NXT_OPCODE_LS_WRITE = 0x0F,
  NXT_OPCODE_LS_READ = 0x10,
  NXT_OPCODE_GET_CURRENTPROGRAM_NAME = 0x11,
  /** \todo system commands */
  NXT_OPCODE_SYS_OPENREAD = 0x80,
  NXT_OPCODE_SYS_OPENWRITE = 0x81,
  NXT_OPCODE_SYS_READ = 0x82,
  NXT_OPCODE_SYS_WRITE = 0x83,
  NXT_OPCODE_SYS_CLOSE = 0x84,
  NXT_OPCODE_SYS_DELETE = 0x85,
  NXT_OPCODE_SYS_FINDFIRST = 0x86,
  NXT_OPCODE_SYS_FINDNEXT = 0x87,
  NXT_OPCODE_SYS_GET_FIRMVAREVERSION = 0x88,
  NXT_OPCODE_SYS_OPENLINEARWRITE = 0x89,
  NXT_OPCODE_SYS_OPENLINEARREAD = 0x8A,
  NXT_OPCODE_SYS_OPENWRITEDATA = 0x8B,
  NXT_OPCODE_SYS_OPENAPPENDDATA = 0x8C,
  NXT_OPCODE_SYS_BOOT = 0x97,
  NXT_OPCODE_SYS_SETBRICKNAME = 0x98,
  NXT_OPCODE_SYS_GET_DEVICEINFO = 0x9B,
  NXT_OPCODE_SYS_DELETE_USERFLASH = 0xA0,
  NXT_OPCODE_SYS_POLLCOMMAND_LENGTH = 0xA1,
  NXT_OPCODE_SYS_POLLCOMMAND = 0xA2,
  NXT_OPCODE_SYS_RESET_BLUETOOTH = 0xA4
};

//error messages
static const char const *err_str[] = {
  "Pending communication transaction in progress",
  "Specified mailbox queue is empty",
  "Request failed (i.e. specified file not found)",
  "Unknown command opcode",
  "Insane packet",
  "Data contains out-of-range values",
  "Communication bus error",
  "No free memory in communication buffer",
  "Specified channel/connection is not valid",
  "Specified channel/connection not configured or busy",
  "No active program",
  "Illegal size specified",
  "Illegal mailbox queue ID specified",
  "Attempted to access invalid field of a structure",
  "Bad input or output specified",
  "Insufficient memory available",
  "Bad arguments"
};


// packet return types
#pragma pack(push,1)

typedef struct {
  uint8_t type;
  uint8_t opcode;
  uint8_t status;
} ret_status_t;

typedef struct {
  uint8_t type;
  uint8_t opcode;
  uint8_t status;
  uint16_t mv;
} ret_battery_t;

typedef struct {
  uint8_t type;
  uint8_t opcode;
  uint8_t status;
  uint32_t msec;
} ret_keepalive_t;

typedef struct {
  uint8_t type;
  uint8_t opcode;
  uint8_t status;
  char filename[20];
} ret_currentprogram_t;

typedef struct {
  uint8_t type;
  uint8_t opcode;
  uint8_t status;
  uint8_t bytes_ready;
} ret_lsstatus_t;

typedef struct {
  uint8_t type;
  uint8_t opcode;
  uint8_t status;
  uint8_t bytes_read;
  char data[16];
} ret_lsread_t;

typedef struct {
  uint8_t type;
  uint8_t opcode;
  uint8_t status;
  uint8_t local_inbox;
  uint8_t msg_size;
  char data[58];
} ret_msgread_t;

// command packet types

typedef struct {
  uint8_t type;
  uint8_t opcode;
} cmd_simple_t;

typedef struct {
  uint8_t type;
  uint8_t opcode;
  uint8_t port;
} cmd_port_t;

typedef struct {
  uint8_t type;
  uint8_t opcode;
  uint8_t port;
  uint8_t relative;
} cmd_resetport_t;

typedef struct {
  uint8_t type;
  uint8_t opcode;
  char filename[20];
} cmd_startprogram_t;

typedef struct {
  uint8_t type;
  uint8_t opcode;
  uint8_t loop;
  char filename[20];
} cmd_playsound_t;

typedef struct {
  uint8_t type;
  uint8_t opcode;
  uint16_t freq;
  uint16_t duration;
} cmd_playtone_t;

typedef struct {
  uint8_t type;
  uint8_t opcode;
  uint8_t port;
  int8_t power;
  uint8_t mode;
  uint8_t regulation;
  int8_t turn_ratio;
  uint8_t run_state;
  uint32_t tacho_limit;
} cmd_setoutput_t;

typedef struct {
  uint8_t type;
  uint8_t opcode;
  uint8_t port;
  uint8_t stype;
  uint8_t smode;
} cmd_setinput_t;

typedef struct {
  uint8_t type;
  uint8_t opcode;
  uint8_t port;
  uint8_t tx_size;
  uint8_t rx_size;
  char data[20];
} cmd_lswrite_t;

typedef struct {
  uint8_t type;
  uint8_t opcode;
  uint8_t remote_inbox;
  uint8_t local_inbox;
  uint8_t remove;
} cmd_msgread_t;

typedef struct {
  uint8_t type;
  uint8_t opcode;
  uint8_t inbox;
  uint8_t message_size;
  char message[59];
} cmd_msgwrite_t;


#pragma pack(pop)

const char *libnxtusb_errstr() {
  switch (libnxtusb_error) {
    case 0x00:
      return "OK";
      break;
    case 0x20:
      return err_str[0];
      break;
    case 0x40:
      return err_str[1];
      break;
    case 0xBD:
      return err_str[2];
      break;
    case 0xBE:
      return err_str[3];
      break;
    case 0xBF:
      return err_str[4];
      break;
    case 0xC0:
      return err_str[5];
      break;
    case 0xDD:
      return err_str[6];
      break;
    case 0xDE:
      return err_str[7];
      break;
    case 0xDF:
      return err_str[8];
      break;
    case 0xE0:
      return err_str[9];
      break;
    case 0xEC:
      return err_str[10];
      break;
    case 0xED:
      return err_str[11];
      break;
    case 0xEE:
      return err_str[12];
      break;
    case 0xEF:
      return err_str[13];
      break;
    case 0xF0:
      return err_str[14];
      break;
    case 0xFB:
      return err_str[15];
      break;
    case 0xFF:
      return err_str[16];
      break;
    default:
      return "Unknown error";
      break;
  }
}

libnxtusb_device_handle *libnxtusb_getnxt() {
  libnxtusb_device_handle *nxtdev = NULL;
  ssize_t dev_count;
  int ret;
  struct libusb_device **list;
  struct libusb_device_descriptor desc;

  nxtdev = malloc(sizeof (libnxtusb_device_handle));
  libusb_init(&nxtdev->ctx);
  libusb_set_debug(nxtdev->ctx, 3);

  dev_count = libusb_get_device_list(nxtdev->ctx, &list);
  if (dev_count < 0) {
    printf("Get device list error\n");
    return NULL;
  }
  printf("Total usb devices: %ld\n", dev_count);

  int i;
  for (i = 0; i < dev_count; i++) {
    ret = libusb_get_device_descriptor(list[i], &desc);
    if (ret < 0) {
      printf("Failed to get device descriptor\n");
      free(nxtdev);
      return NULL;
    }
    if (desc.idVendor == NXT_USB_ID_VENDOR_LEGO && desc.idProduct == NXT_USB_ID_PRODUCT_NXT) {
      struct libusb_device_handle *handle;
      ret = libusb_open(list[i], &nxtdev->handle);
      if (ret < 0) {
        printf("Failed to open device\n");
        free(nxtdev);
        return NULL;
      }
      ret = libusb_claim_interface(nxtdev->handle, NXT_USB_INTERFACE);
      if (ret < 0) {
        printf("Cannot claim interface\n");
        libusb_close(nxtdev->handle);
        libusb_free_device_list(list, 1);
        free(nxtdev);
        return NULL;
      }
      libusb_free_device_list(list, 1);
      return nxtdev;
    }
  }
  libusb_free_device_list(list, 1);
  free(nxtdev);
  return NULL;
}

int libnxtusb_closenxt(libnxtusb_device_handle *nxtdev) {
  libusb_release_interface(nxtdev->handle, NXT_USB_INTERFACE);
  libusb_close(nxtdev->handle);
  libusb_exit(nxtdev->ctx);
  free(nxtdev);
  return 0;
}


//internal. send packet

int nxt_send(
             const libnxtusb_device_handle *handle, const unsigned char *request,
             const unsigned int length
             ) {
  int res;
  int transferred;
  res = libusb_bulk_transfer(
    handle->handle,
    (NXT_USB_ENDPOINT_OUT | LIBUSB_ENDPOINT_OUT),
    (unsigned char*) request, length, &transferred, NXT_USB_TIMEOUT
    );
  if (res < 0) {
    return -1;
  }
  return transferred;
}

//internal. receive packet

int nxt_recv(
             const libnxtusb_device_handle *handle,
             unsigned char *result
             ) {
  int res;
  int transferred;
  res = libusb_bulk_transfer(
    handle->handle,
    (NXT_USB_ENDPOINT_IN | LIBUSB_ENDPOINT_IN),
    result, NXT_USB_READSIZE, &transferred, NXT_USB_TIMEOUT
    );
  if (res < 0) {
    return -1;
  }
  return transferred;
}

/*
 *  PUBLIC COMMANDS
 */

int nxt_start_program(const libnxtusb_device_handle *handle, const char *filename) {

  cmd_startprogram_t cmd = {NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_STARTPROGRAM, ""};
  strncat(cmd.filename, filename, 19);

  int sent = nxt_send(handle, (const unsigned char*) &cmd, sizeof (cmd));

  if (sent != sizeof (cmd)) {
    return -1;
  }

  ret_status_t st;
  int ret = nxt_recv(handle, (unsigned char*) &st);
  if (ret != sizeof (ret_status_t)) {
    return -1;
  }
  if (st.type != NXT_COMMAND_REPLY || st.opcode != NXT_OPCODE_STARTPROGRAM) {
    return -1;
  }
  if (st.status != NXT_STATUS_OK) {
    libnxtusb_error = st.status;
    return -1;
  }
  return 0;
}

int nxt_stop_program(const libnxtusb_device_handle *handle) {

  cmd_simple_t cmd = {NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_STOPPROGRAM};

  int sent = nxt_send(handle, (unsigned char*) &cmd, sizeof (cmd));

  if (sent != 2) {
    return -1;
  }

  ret_status_t st;
  int ret = nxt_recv(handle, (unsigned char*) &st);
  if (ret != sizeof (ret_status_t)) {
    return -1;
  }
  if (st.type != NXT_COMMAND_REPLY || st.opcode != NXT_OPCODE_STOPPROGRAM) {
    return -1;
  }
  if (st.status != NXT_STATUS_OK) {
    libnxtusb_error = st.status;
    return -1;
  }
  return 0;
}

int nxt_get_current_program_name(const libnxtusb_device_handle *handle, char* filename) {

  cmd_simple_t cmd = {NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_GET_CURRENTPROGRAM_NAME};

  int sent = nxt_send(handle, (unsigned char*) &cmd, sizeof (cmd));

  if (sent != sizeof (cmd)) {
    return -1;
  }

  ret_currentprogram_t st;
  int ret = nxt_recv(handle, (unsigned char*) &st);
  if (ret != sizeof (ret_currentprogram_t)) {
    return -1;
  }
  if (st.type != NXT_COMMAND_REPLY || st.opcode != NXT_OPCODE_GET_CURRENTPROGRAM_NAME) {
    return -1;
  }
  if (st.status != NXT_STATUS_OK) {
    libnxtusb_error = st.status;
    return -1;
  }
  strncpy(filename, st.filename, 20);
  return 0;
}

int nxt_play_soundfile(
                       const libnxtusb_device_handle *handle,
                       const char *filename, const unsigned short loop
                       ) {

  cmd_playsound_t cmd = {NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_PLAYSOUND, loop, ""};
  strncat(cmd.filename, filename, 19);

  int sent = nxt_send(handle, (const unsigned char*) &cmd, sizeof (cmd));

  if (sent != sizeof (cmd)) {
    return -1;
  }

  ret_status_t st;
  int ret = nxt_recv(handle, (unsigned char*) &st);
  if (ret != sizeof (ret_status_t)) {
    return -1;
  }
  if (st.type != NXT_COMMAND_REPLY || st.opcode != NXT_OPCODE_PLAYSOUND) {
    return -1;
  }
  if (st.status != NXT_STATUS_OK) {
    libnxtusb_error = st.status;
    return -1;
  }
  return loop;
}

int nxt_play_tone(
                  const libnxtusb_device_handle *handle,
                  const unsigned int freq, const unsigned int duration
                  ) {

  cmd_playtone_t cmd = {NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_PLAYTONE, freq, duration};

  int sent = nxt_send(handle, (const unsigned char*) &cmd, sizeof (cmd));

  ret_status_t st;
  int ret = nxt_recv(handle, (unsigned char*) &st);
  if (ret != sizeof (ret_status_t)) {
    return -1;
  }
  if (st.type != NXT_COMMAND_REPLY || st.opcode != NXT_OPCODE_PLAYTONE) {
    return -1;
  }
  if (st.status != NXT_STATUS_OK) {
    libnxtusb_error = st.status;
    return -1;
  }
  return 0;
}

int nxt_stop_sound(const libnxtusb_device_handle *handle) {

  cmd_simple_t cmd = {NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_STOP_SOUND};

  int sent = nxt_send(handle, (const unsigned char*) &cmd, sizeof (cmd));

  ret_status_t st;
  int ret = nxt_recv(handle, (unsigned char*) &st);
  if (ret != sizeof (ret_status_t)) {
    return -1;
  }
  if (st.type != NXT_COMMAND_REPLY || st.opcode != NXT_OPCODE_STOP_SOUND) {
    return -1;
  }
  if (st.status != NXT_STATUS_OK) {
    libnxtusb_error = st.status;
    return -1;
  }
  return 0;
}

int nxt_set_output_state(
                         const libnxtusb_device_handle *handle, const libnxtusb_out_t port,
                         const int8_t power, const libnxtusb_motor_mode_t mode, const libnxtusb_motor_regulation_t regulation,
                         const int8_t turn_ratio, const libnxtusb_motor_runstate_t run_state, const uint32_t tacho_limit
                         ) {

  cmd_setoutput_t cmd = {
    NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_SET_OUTPUTSTATE, port, power,
    mode, regulation, turn_ratio, run_state, tacho_limit
  };

  int sent = nxt_send(handle, (const unsigned char*) &cmd, sizeof (cmd));

  ret_status_t st;
  int ret = nxt_recv(handle, (unsigned char*) &st);
  if (ret != sizeof (ret_status_t)) {
    return -1;
  }
  if (st.type != NXT_COMMAND_REPLY || st.opcode != NXT_OPCODE_SET_OUTPUTSTATE) {
    return -1;
  }
  if (st.status != NXT_STATUS_OK) {
    libnxtusb_error = st.status;
    return -1;
  }
  return 0;
}

int nxt_set_input_mode(
                       const libnxtusb_device_handle *handle, const libnxtusb_in_t port,
                       const libnxtusb_sensor_type_t stype, const libnxtusb_sensor_mode_t smode
                       ) {

  cmd_setinput_t cmd = {
    NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_SET_INPUTMODE, port, stype, smode
  };

  int sent = nxt_send(handle, (const unsigned char*) &cmd, sizeof (cmd));

  ret_status_t st;
  int ret = nxt_recv(handle, (unsigned char*) &st);

  if (ret != sizeof (ret_status_t)) {
    return -1;
  }
  if (st.type != NXT_COMMAND_REPLY || st.opcode != NXT_OPCODE_SET_INPUTMODE) {
    return -1;
  }
  if (st.status != NXT_STATUS_OK) {
    libnxtusb_error = st.status;
    return -1;
  }
  return 0;
}

int nxt_get_output_state(
                         const libnxtusb_device_handle *handle, const libnxtusb_out_t port, libnxtusb_outputstate_t *out
                         ) {

  cmd_port_t cmd = {
    NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_GET_OUTPUTSTATE, port
  };

  int sent = nxt_send(handle, (const unsigned char*) &cmd, sizeof (cmd));

  int ret = nxt_recv(handle, (unsigned char*) out);
  if (ret != sizeof (libnxtusb_outputstate_t)) {
    return -1;
  }
  if (out->type != NXT_COMMAND_REPLY || out->opcode != NXT_OPCODE_GET_OUTPUTSTATE) {
    return -1;
  }
  if (out->status != NXT_STATUS_OK) {
    libnxtusb_error = out->status;
    return -1;
  }
  return 0;
}

int nxt_get_input_values(
                         const libnxtusb_device_handle *handle, const libnxtusb_in_t port, libnxtusb_inputstate_t *out
                         ) {

  cmd_port_t cmd = {
    NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_GET_INPUTVALUES, port
  };

  int sent = nxt_send(handle, (const unsigned char*) &cmd, sizeof (cmd));

  int ret = nxt_recv(handle, (unsigned char*) out);

  if (ret != sizeof (libnxtusb_inputstate_t)) {
    return -1;
  }
  if (out->type != NXT_COMMAND_REPLY || out->opcode != NXT_OPCODE_GET_INPUTVALUES) {
    return -1;
  }
  if (out->status != NXT_STATUS_OK) {
    libnxtusb_error = out->status;

    return -1;
  }
  return 0;
}

int nxt_reset_input_scaled_value(
                                 const libnxtusb_device_handle *handle, const libnxtusb_in_t port
                                 ) {

  cmd_port_t cmd = {
    NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_RESET_INPUT_SCALEDVALUES, port
  };

  int sent = nxt_send(handle, (const unsigned char*) &cmd, sizeof (cmd));

  ret_status_t st;
  int ret = nxt_recv(handle, (unsigned char*) &st);
  if (ret != sizeof (ret_status_t)) {
    return -1;
  }
  if (st.type != NXT_COMMAND_REPLY || st.opcode != NXT_OPCODE_RESET_INPUT_SCALEDVALUES) {
    return -1;
  }
  if (st.status != NXT_STATUS_OK) {
    libnxtusb_error = st.status;

    return -1;
  }
  return 0;
}

int nxt_reset_motor_position(
                             const libnxtusb_device_handle *handle, const libnxtusb_in_t port,
                             const unsigned short relative
                             ) {

  cmd_resetport_t cmd = {
    NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_RESET_MOTOR_POSITION, port, (relative > 0) ? 1 : 0
  };

  int sent = nxt_send(handle, (const unsigned char*) &cmd, sizeof (cmd));

  ret_status_t st;
  int ret = nxt_recv(handle, (unsigned char*) &st);
  if (ret != sizeof (ret_status_t)) {
    return -1;
  }
  if (st.type != NXT_COMMAND_REPLY || st.opcode != NXT_OPCODE_RESET_MOTOR_POSITION) {
    return -1;
  }
  if (st.status != NXT_STATUS_OK) {
    libnxtusb_error = st.status;

    return -1;
  }
  return 0;
}

int nxt_ls_get_status(
                      const libnxtusb_device_handle *handle, const libnxtusb_in_t port,
                      int* bytes_ready
                      ) {

  cmd_port_t cmd = {
    NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_LS_GET_STATUS, port
  };

  int sent = nxt_send(handle, (const unsigned char*) &cmd, sizeof (cmd));

  ret_lsstatus_t st;
  int ret = nxt_recv(handle, (unsigned char*) &st);
  if (ret != sizeof (ret_lsstatus_t)) {
    return -1;
  }
  if (st.type != NXT_COMMAND_REPLY || st.opcode != NXT_OPCODE_LS_GET_STATUS) {
    return -1;
  }
  if (st.status != NXT_STATUS_OK) {
    libnxtusb_error = st.status;
    return -1;
  }
  *bytes_ready = st.bytes_ready;

  return 0;
}

int nxt_ls_write(
                 const libnxtusb_device_handle *handle, const libnxtusb_in_t port,
                 const char* data, const uint8_t data_size, const uint8_t expected_data_size
                 ) {

  cmd_lswrite_t cmd = {
    NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_LS_WRITE, port, data_size, expected_data_size
  };
  strncat(cmd.data, data, 20);

  int sent = nxt_send(handle, (const unsigned char*) &cmd, sizeof (cmd));

  ret_status_t st;
  int ret = nxt_recv(handle, (unsigned char*) &st);
  if (ret != sizeof (ret_status_t)) {
    return -1;
  }
  if (st.type != NXT_COMMAND_REPLY || st.opcode != NXT_OPCODE_LS_WRITE) {
    return -1;
  }
  if (st.status != NXT_STATUS_OK) {
    libnxtusb_error = st.status;

    return -1;
  }
  return 0;
}

int nxt_ls_read(
                const libnxtusb_device_handle *handle, const libnxtusb_in_t port,
                char* data
                ) {

  cmd_port_t cmd = {
    NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_LS_READ, port
  };

  int sent = nxt_send(handle, (const unsigned char*) &cmd, sizeof (cmd));

  ret_lsread_t st;
  int ret = nxt_recv(handle, (unsigned char*) &st);
  if (ret != sizeof (ret_lsread_t)) {
    return -1;
  }
  if (st.type != NXT_COMMAND_REPLY || st.opcode != NXT_OPCODE_LS_READ) {
    return -1;
  }
  if (st.status != NXT_STATUS_OK) {
    libnxtusb_error = st.status;
    return -1;
  }
  strncpy(data, st.data, 16);

  return 0;
}

int nxt_message_write(
                      const libnxtusb_device_handle *handle, const uint8_t inbox,
                      char* message
                      ) {

  cmd_msgwrite_t cmd = {
    NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_MESSAGE_WRITE, strlen(message) + 1
  };
  strncat(cmd.message, message, strlen(message) + 3);

  int sent = nxt_send(handle, (const unsigned char*) &cmd, sizeof (cmd));

  ret_status_t st;
  int ret = nxt_recv(handle, (unsigned char*) &st);
  if (ret != sizeof (ret_status_t)) {
    return -1;
  }
  if (st.type != NXT_COMMAND_REPLY || st.opcode != NXT_OPCODE_MESSAGE_WRITE) {
    return -1;
  }
  if (st.status != NXT_STATUS_OK) {
    libnxtusb_error = st.status;

    return -1;
  }
  return 0;
}

int nxt_message_read(
                     const libnxtusb_device_handle *handle, const uint8_t remote_inbox,
                     uint8_t local_inbox, char* message, uint8_t remove
                     ) {

  cmd_msgread_t cmd = {
    NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_MESSAGE_READ,
    remote_inbox, local_inbox, remove
  };

  int sent = nxt_send(handle, (const unsigned char*) &cmd, sizeof (cmd));

  ret_msgread_t st;
  int ret = nxt_recv(handle, (unsigned char*) &st);
  if (ret != sizeof (ret_msgread_t)) {
    return -1;
  }
  if (st.type != NXT_COMMAND_REPLY || st.opcode != NXT_OPCODE_MESSAGE_READ) {
    return -1;
  }
  if (st.status != NXT_STATUS_OK) {
    libnxtusb_error = st.status;
    return -1;
  }
  strncpy(message, st.data, st.msg_size);

  return 0;
}

int nxt_get_battery_level_mv(const libnxtusb_device_handle *handle, unsigned int* mv) {

  cmd_simple_t cmd = {NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_BATTERYLEVEL};

  int sent = nxt_send(handle, (const unsigned char*) &cmd, sizeof (cmd));
  if (sent != 2) {
    return -1;
  }

  ret_battery_t bt;
  int ret = nxt_recv(handle, (unsigned char*) &bt);
  if (ret != sizeof (ret_battery_t)) {
    return -1;
  }
  if (bt.type != NXT_COMMAND_REPLY || bt.opcode != NXT_OPCODE_BATTERYLEVEL) {
    return -1;
  }
  if (bt.status != NXT_STATUS_OK) {
    libnxtusb_error = bt.status;
    return -1;
  }
  *mv = bt.mv;

  return 0;
}

int nxt_keepalive(const libnxtusb_device_handle *handle, unsigned int* msec) {

  cmd_simple_t cmd = {NXT_DIRECT_COMMAND_DOREPLY, NXT_OPCODE_KEEPALIVE};

  int sent = nxt_send(handle, (const unsigned char*) &cmd, sizeof (cmd));
  if (sent != sizeof (cmd_simple_t)) {
    return -1;
  }

  ret_keepalive_t kt;
  int ret = nxt_recv(handle, (unsigned char*) &kt);
  if (ret != sizeof (ret_keepalive_t)) {
    return -1;
  }
  if (kt.type != NXT_COMMAND_REPLY || kt.opcode != NXT_OPCODE_BATTERYLEVEL) {
    return -1;
  }
  if (kt.status != NXT_STATUS_OK) {
    libnxtusb_error = kt.status;
    return -1;
  }
  *msec = kt.msec;

  return 0;
}

/** \todo Ultrasound helpers */

int nxt_init_ultrasound(const libnxtusb_device_handle * handle) {

}


/** \todo System commands */