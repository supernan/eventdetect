FILE(GLOB SOURCE_1 "${PROJECT_ROOT_PATH}/src/*.cpp")
FILE(GLOB SOURCE_2 "${PROJECT_ROOT_PATH}/thirdparty/src/*.cpp")
ADD_LIBRARY(eventdetect STATIC ${SOURCE_1} ${SOURCE_2} )
TARGET_LINK_LIBRARIES(eventdetect glog nemodel pthread ictlap_core_ne litecrf)
