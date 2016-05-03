PHP_ARG_ENABLE(leaver, whether to enable leaver support,
    [  --enable-leaver        Enable leaver support])

AC_ARG_ENABLE(leaver-debug,
    [  --enable-leaver-debug  Enable leaver debug mode],
    [PHP_LEAVER_DEBUG=$enableval],
    [PHP_LEAVER_DEBUG="no"])

if test "$PHP_LEAVER" != "no"; then

    if test "$PHP_LEAVER_DEBUG" = "yes"; then
        AC_DEFINE(PHP_LEAVER_DEBUG, 1, [define to 1 if you want to run with debug mode])
    else
        AC_DEFINE(PHP_LEAVER_DEBUG, 0, [define to 1 if you want to run with debug mode])
    fi

    PHP_NEW_EXTENSION(leaver,
        leaver.c                            \
        leaver/logger.c                     \
        leaver/appender.c                   \
        leaver/appender/null.c              \
        leaver/appender/file.c,
        $ext_shared)

fi