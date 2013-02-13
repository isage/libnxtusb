#include "libnxtusb.h"
#include <stdio.h>

int main(void) {
  libnxtusb_device_handle *handle = NULL;
  handle = libnxtusb_getnxt();
  if (handle == NULL) {
    printf("No NXT devices found\n");
  } else {
    printf("Found NXT device\n");
    int te;
    nxt_get_battery_level_mv(handle, &te);
    printf("%u\n", te);
    te = nxt_play_soundfile(handle, "! Attention.rso", 0);
    if (te != NXT_STATUS_OK)
      printf("%d %s\n", te, libnxtusb_errstr());
    te = nxt_set_input_mode(handle, NXT_IN_2, NXT_SENSOR_SWITCH, NXT_SENSOR_MODE_BOOLEAN);
    if (te != NXT_STATUS_OK)
      printf("%d %s\n", te, libnxtusb_errstr());
    libnxtusb_inputstate_t st;
    while (st.scaled_value!=1) {
      te = nxt_get_input_values(handle, NXT_IN_2, &st);
      if (te != NXT_STATUS_OK)
        printf("%d %s\n", te, libnxtusb_errstr());
      printf("%u\n", st.scaled_value);
    }

    libnxtusb_closenxt(handle);
  }
  return 0;
}