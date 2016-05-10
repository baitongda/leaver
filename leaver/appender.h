/*
 * Leaver - A fast logger for PHP
 *
 * Copyright (c) 2016 You Ming
 * The MIT License (MIT)
 * http://leaver.funcuter.org
 */

#ifndef LEAVER_APPENDER_H
#define LEAVER_APPENDER_H

extern char *leaver_appender_get_level_string(zend_long level);
extern char *leaver_appender_get_datetime_string(struct timeval timev);
extern size_t leaver_appender_format_log(char **output, zend_string *format, zend_long level, zend_string *message);
extern size_t leaver_appender_format_exception_log(char **output, zend_string *format, zval *z_exception);

extern zend_class_entry *leaver_appender_ce;

LEAVER_CREATE_FUNCTION(appender);

#endif