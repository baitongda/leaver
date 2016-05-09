/*
 * Leaver - A fast logger for PHP
 *
 * Copyright (c) 2016 You Ming
 * The MIT License (MIT)
 * http://leaver.funcuter.org
 */

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <php.h>

#include "../../php_leaver.h"
#include "../appender.h"
#include "file.h"
#include "daily_file.h"

// Usable tokens:
//  %% for '%'; %Y year; %YYY year; %M month; %D day; %H hour; %I minute; %A am or pm.
char *leaver_appender_daliyfile_format_path(char *log_file, size_t log_file_len)
{
    char *p, *buf, *b;
    size_t buf_len, add_len;
    char token;

    time_t timestamp;
    struct tm * timeinfo;

    timestamp = time(NULL);
    timeinfo = localtime(&timestamp);

    p = log_file;
    buf = emalloc(log_file_len + 1);
    b = buf;
    buf_len = 0;

    while ('\0' != *p) {
        if (!token && '%' == *p) {
            token = 1;
            p++;
            continue;
        }

        if (token) {
            token = 0;

            switch (*p) {
                case '%':
                    *b = '%';
                    add_len = 1;
                    break;

                case 'Y':
                    if (p - log_file + 2 < log_file_len && 'Y' == *(p + 1) && 'Y' == *(p + 2)) {
                        p += 2;
                        add_len = snprintf(b, 5, "%d", timeinfo->tm_year + 1900);
                    } else {
                        add_len = snprintf(b, 3, "%d", (timeinfo->tm_year + 1900) % 100);
                    }
                    break;

                case 'M':
                    add_len = snprintf(b, 3, "%02d", timeinfo->tm_mon + 1);
                    break;

                case 'D':
                    add_len = snprintf(b, 3, "%02d", timeinfo->tm_mday);
                    break;

                case 'H':
                    add_len = snprintf(b, 3, "%02d", timeinfo->tm_hour);
                    break;

                case 'I':
                    add_len = snprintf(b, 3, "%02d", timeinfo->tm_min);
                    break;

                case 'A':
                    add_len = snprintf(b, 3, "%s", timeinfo->tm_hour < 12 ? "am" : "pm");
                    break;

                default:
                    break;
            }

            buf_len += add_len;
            b += add_len;
            p++;
            continue;
        }

        *b = *p;
        buf_len++;
        b++;
        p++;
    }

    buf[buf_len] = '\0';

    return buf;
}

zend_class_entry *leaver_appender_dailyfile_ce;

// public \Leaver\Appender\DailyFileAppender::setFilePath(string $filePath) : void
ZEND_BEGIN_ARG_INFO_EX(leaver_appender_dailyfile_setFilePath_arginfo, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, filePath, IS_STRING, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(leaver_appender_dailyfile, setFilePath)
{
    zval *this = getThis();
    zend_string *filePath;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &filePath)) {
        return;
    }

    zend_update_property_str(leaver_appender_dailyfile_ce, this, ZEND_STRL("filePath"), filePath);
}

// public \Leaver\Appender\DailyFileAppender::onAppend(int level, string $message, Throwable $throwable = NULL) : void
ZEND_BEGIN_ARG_INFO_EX(leaver_appender_dailyfile_onAppend_arginfo, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, message, IS_STRING, 0)
    ZEND_ARG_OBJ_INFO(0, throwable, Throwable, 1)
ZEND_END_ARG_INFO()

PHP_METHOD(leaver_appender_dailyfile, onAppend)
{
    zval *this = getThis();
    zend_long level;
    zend_string *message;
    zval *z_exception = NULL;
    zval *z_format, *z_file_path, *z_log_exception;
    zend_string *format, *file_path;
    zend_bool log_exception;
    char *log, *log_file;
    size_t log_len;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "lS|z", &level, &message, &z_exception)) {
        return;
    }

    z_format = zend_read_property(Z_OBJCE_P(this), this, ZEND_STRL("format"), 1, NULL);
    z_log_exception = zend_read_property(Z_OBJCE_P(this), this, ZEND_STRL("logException"), 1, NULL);
    z_file_path = zend_read_property(Z_OBJCE_P(this), this, ZEND_STRL("filePath"), 1, NULL);

    format = zval_get_string(z_format);
    log_exception = Z_TYPE_P(z_log_exception) == IS_TRUE ? (char) 1 : (char) 0;
    file_path = zval_get_string(z_file_path);

    log_len = leaver_appender_format_log(&log, format, level, message);

    if (log_exception && z_exception && Z_TYPE_P(z_exception) == IS_OBJECT) {
        // Add exception log if exist.
        zval *z_exception_format;
        zend_string *exception_format;
        char *exception_log, *log_tmp;
        size_t exception_log_len;

        z_exception_format = zend_read_property(Z_OBJCE_P(this), this, ZEND_STRL("exceptionFormat"), 1, NULL);
        exception_format = zval_get_string(z_exception_format);

        exception_log_len = leaver_appender_format_exception_log(&exception_log, exception_format, z_exception);

        log_tmp = emalloc(log_len + exception_log_len + 2);

        strncpy(log_tmp, log, log_len);
        log_tmp[log_len] = '\n';
        strncpy(log_tmp + log_len + 1, exception_log, exception_log_len);
        log_tmp[log_len + exception_log_len + 1] = '\0';

        zend_string_release(exception_format);
        efree(exception_log);
        efree(log);

        log_len = log_len + exception_log_len + 1;
        log = log_tmp;
    }

    log_file = leaver_appender_daliyfile_format_path(file_path->val, file_path->len);

    leaver_appender_file_write_log(log_file, log, log_len);

    zend_string_release(format);
    zend_string_release(file_path);
    efree(log);
    efree(log_file);
}

zend_function_entry leaver_appender_dailyfile_methods[] = {
    PHP_ME(leaver_appender_dailyfile, setFilePath, leaver_appender_dailyfile_setFilePath_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_appender_dailyfile, onAppend, leaver_appender_dailyfile_onAppend_arginfo, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

LEAVER_CREATE_FUNCTION(appender_dailyfile)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Leaver\\Appender\\DailyFileAppender", leaver_appender_dailyfile_methods);
    leaver_appender_dailyfile_ce = zend_register_internal_class_ex(&ce, leaver_appender_ce);

    zend_declare_property_null(leaver_appender_dailyfile_ce, ZEND_STRL("filePath"), ZEND_ACC_PROTECTED);

    return SUCCESS;
}
