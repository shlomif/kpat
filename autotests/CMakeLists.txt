include(ECMAddTests)
include_directories(AFTER "${CMAKE_CURRENT_SOURCE_DIR}/..")
ecm_add_test(
    shuffle_test.cpp
    TEST_NAME ShuffleTest
    LINK_LIBRARIES Qt5::Test
    NAME_PREFIX "kpat-"
)
ecm_add_test(
    tutorial1.cpp
    TEST_NAME Tut1
    LINK_LIBRARIES Qt5::Test
    NAME_PREFIX "tut1-"
)
