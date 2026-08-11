#ifndef _APP_VERSION_H_
#define _APP_VERSION_H_

/* The template values come from cmake/version.cmake
 * BUILD_VERSION related template values will be 'git describe',
 * alternatively user defined BUILD_VERSION.
 */

/* #undef ZEPHYR_VERSION_CODE */
/* #undef ZEPHYR_VERSION */

#define APPVERSION                   0x10e0346
#define APP_VERSION_NUMBER           0x10e03
#define APP_VERSION_MAJOR            01
#define APP_VERSION_MINOR            14
#define APP_PATCHLEVEL               03
#define APP_TWEAK                    70
#define APP_VERSION_STRING           "01.14.03-master"
#define APP_VERSION_EXTENDED_STRING  "01.14.03-master+70"
#define APP_VERSION_TWEAK_STRING     "01.14.03+70"

#define APP_BUILD_VERSION efc33479b12a


#endif /* _APP_VERSION_H_ */
