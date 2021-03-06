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
    te = nxt_set_input_mode(handle, NXT_IN_3, NXT_SENSOR_LIGHT_ACTIVE, NXT_SENSOR_MODE_RAW);
    if (te != NXT_STATUS_OK)
      printf("%d %s\n", te, libnxtusb_errstr());
    nxt_reset_input_scaled_value(handle,NXT_IN_3);
    libnxtusb_inputstate_t st;
    while (st.scaled_value < 500) {
      te = nxt_get_input_values(handle, NXT_IN_3, &st);
      if (te != NXT_STATUS_OK)
        printf("%d %s\n", te, libnxtusb_errstr());
      printf("%u\n", st.scaled_value);
    }
   nxt_set_input_mode(handle, NXT_IN_3, NXT_SENSOR_NONE, NXT_SENSOR_MODE_RAW);
    libnxtusb_closenxt(handle);
  }
  return 0;
}