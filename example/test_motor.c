#include "libnxtusb.h"
#include <stdio.h>

int main(void)
{
  libnxtusb_device_handle *handle = NULL;
  handle = libnxtusb_getnxt();
  if (handle == NULL)
  {
    printf("No NXT devices found\n");
  }
  else
  {
    printf("Found NXT device\n");
    int te;
    nxt_get_battery_level_mv(handle,&te);
    printf("%u\n",te);
    te=nxt_play_soundfile(handle, "! Attention.rso", 0);
    if (te!=NXT_STATUS_OK)
      printf("%d %s\n",te,libnxtusb_errstr());
    nxt_set_output_state(handle,NXT_OUT_B,100,NXT_MOTOR_MODE_ON,NXT_MOTOR_REGULATION_SPEED,50,NXT_MOTOR_RUNSTATE_RUNNING,0);
    sleep(5);
    nxt_set_output_state(handle,NXT_OUT_B,0,NXT_MOTOR_MODE_BRAKE,NXT_MOTOR_REGULATION_IDLE,50,NXT_MOTOR_RUNSTATE_IDLE,0);
    libnxtusb_closenxt(handle);
  }
  return 0;
}