/*
 * Leaver - A fast logger for PHP
 *
 * Copyright (c) 2016 You Ming
 * The MIT License (MIT)
 * http://leaver.funcuter.org
 */

#ifndef LEAVER_APPENDER_H
#define LEAVER_APPENDER_H

extern zend_class_entry *leaver_appender_ce;

LEAVER_CREATE_FUNCTION(appender);

#define LEAVER_APPENDER_FORMAT_BUFFER_INCREASE      256

#define LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, now_len, need_len)           \
    if (UNEXPECTED(now_len + need_len > alloc_len)) {                               \
        leaver_appender_ensure_length(&buf, &b, now_len, &alloc_len, need_len);     \
    }

extern size_t leaver_appender_common_format_log(char **output, zend_string *format, zend_long level,
                                                zend_string *message);

#endif