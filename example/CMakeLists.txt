add_executable(test test.c)
target_link_libraries(test nxtusb)

add_executable(test_motor test_motor.c)
target_link_libraries(test_motor nxtusb)

add_executable(test_button test_button.c)
target_link_libraries(test_button nxtusb)

add_executable(test_sound test_sound.c)
target_link_libraries(test_sound nxtusb)

add_executable(test_light test_light.c)
target_link_libraries(test_light nxtusb)
