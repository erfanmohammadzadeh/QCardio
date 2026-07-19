#ifndef DEFINE_H
#define DEFINE_H
#define SOFTWARE_VERSION_MAJOR 0
#define SOFTWARE_VERSION_MINOR 1
#define SOFTWARE_VERSION_PATCH 0

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define DISPLAY_DATE_FORMAT "yyyy/MM/dd"
#define DISPLAY_DATETIME_FORMAT "yyyy/MM/dd hh:mm:ss"
#define DISPLAY_TIME_FORMAT "hh:mm:ss ap"
#define DISPLAY_TIME_DURATION_FORMAT "hh:mm:ss"
#define CLOCK_DATETIME_FORMAT "ddd dd MMM yyyy hh:mm ap"

#ifdef QT_DEBUG
#define SHOW_TYPE show();
#define WINDOW_FLAG_SOTH_AND_SHEET ;
#define WINDOW_STAYS_ON_TOPHINT ;
// #define LIBRARY_DEBUGGING_MODE true
#else
#define SHOW_TYPE showFullScreen();
#define WINDOW_FLAG_SOTH_AND_SHEET setWindowFlags(Qt::WindowStaysOnTopHint|Qt::Sheet);
#define WINDOW_STAYS_ON_TOPHINT setWindowFlags(Qt::WindowStaysOnTopHint);
#endif
#define SHOWING_COMPANY_LOGO false
#define SHOWING_COMPANY_LOGO_IN_REPORT false
#define REMOVE_LOWER_TEMPLATE false
#define SOFTWARE_VERSION_STR TOSTRING(SOFTWARE_VERSION_MAJOR) "." TOSTRING(SOFTWARE_VERSION_MINOR) "." TOSTRING(SOFTWARE_VERSION_PATCH)
#define MAX_WIDGET_SIZE 16777215
#endif // DEFINE_H
