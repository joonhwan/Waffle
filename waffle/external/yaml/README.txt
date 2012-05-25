* yaml modified in waffle *

yaml 홈페이지에서 소스코드 다운로드

source/*.c
source/*.h
win32/config.h

상기 파일을 복사.

yaml_private.h

내의 다음 #include 문을 수정

// #if HAVE_CONFIG_H
// #include <config.h>
// #endif
// #include <yaml.h>
#include "config.h"
#include "yaml.h"
