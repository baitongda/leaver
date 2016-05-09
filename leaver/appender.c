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
#include <zend_exceptions.h>

#include "../php_leaver.h"
#include "appender.h"

#define LEAVER_APPENDER_FORMAT_BUFFER_INCREASE      256
#define LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, now_len, need_len)           \
    if (UNEXPECTED(now_len + need_len > alloc_len)) {                               \
        leaver_appender_ensure_length(&buf, &b, now_len, &alloc_len, need_len);     \
    }

char *leaver_appender_get_level_string(zend_long level)
{
    switch (level) {
        case LEAVER_LEVEL_EMERGENCY_INT:
        default:
            return LEAVER_LEVEL_EMERGENCY;
        case LEAVER_LEVEL_ALERT_INT:
            return LEAVER_LEVEL_ALERT;
        case LEAVER_LEVEL_ERROR_INT:
            return LEAVER_LEVEL_ERROR;
        case LEAVER_LEVEL_WARNING_INT:
            return LEAVER_LEVEL_WARNING;
        case LEAVER_LEVEL_NOTICE_INT:
            return LEAVER_LEVEL_NOTICE;
        case LEAVER_LEVEL_INFO_INT:
            return LEAVER_LEVEL_INFO;
        case LEAVER_LEVEL_DEBUG_INT:
            return LEAVER_LEVEL_DEBUG;
    }
}

void leaver_appender_ensure_length(char **buffer, char **b, size_t now_len, size_t *alloc_len, size_t need)
{
    size_t now_alloc_len = *alloc_len;

    now_alloc_len += (need / LEAVER_APPENDER_FORMAT_BUFFER_INCREASE + 1) * LEAVER_APPENDER_FORMAT_BUFFER_INCREASE;

    size_t offset = b - buffer;

    char *new = emalloc(now_alloc_len);
    strncpy(new, *buffer, now_len);

    // Free stale.
    efree(*buffer);

    *buffer = new;
    *b = *buffer + offset;
    *alloc_len = now_alloc_len;
}

// Usable tokens:
//  %% for '%'; %n new line; %Y year; %M month; %D day; %H hour; %I minute; %S second; %s millisecond;
//  %u microsecond; %t timestamp; %r millisecond from request start; %p process id; %l log level;
//  %m log message;
size_t leaver_appender_format_log(char **output, zend_string *format, zend_long level, zend_string *message)
{
    char token;
    char *p, *buf, *b;
    size_t buf_len, add_len, alloc_len;
    // Time
    time_t timestamp;
    struct tm * timeinfo;
    struct timeval timev;

    timestamp = time(NULL);
    timeinfo = localtime(&timestamp);
    gettimeofday(&timev, NULL);

    alloc_len = LEAVER_APPENDER_FORMAT_BUFFER_INCREASE;
    buf = emalloc(alloc_len);
    buf_len = 0;

    b = buf;
    token = 0;
    p = format->val;
    while ('\0' != *p) {
        if (!token && '%' == *p) {
            token = 1;
            p++;
            continue;
        }

        if (token) {
            token = 0;

            switch(*p) {
                case '%':
                    LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 2);
                    *b = '%';
                    add_len = 1;
                    break;

                case 'n':
                    LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 2);
                    *b = '\n';
                    add_len = 1;
                    break;

                case 'Y':
                    LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 5);
                    add_len = snprintf(b, 5, "%4d", timeinfo->tm_year + 1900);
                    break;

                case 'M':
                    LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 3);
                    add_len = snprintf(b, 3, "%02d", timeinfo->tm_mon + 1);
                    break;

                case 'D':
                    LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 3);
                    add_len = snprintf(b, 3, "%02d", timeinfo->tm_mday);
                    break;

                case 'H':
                    LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 3);
                    add_len = snprintf(b, 3, "%02d", timeinfo->tm_hour);
                    break;

                case 'I':
                    LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 3);
                    add_len = snprintf(b, 3, "%02d", timeinfo->tm_min);
                    break;

                case 'S':
                    LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 3);
                    add_len = snprintf(b, 3, "%02d", timeinfo->tm_sec);
                    break;

                case 's':
                    LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 4);
                    add_len = snprintf(b, 4, "%03d", timev.tv_usec / 1000);
                    break;

                case 'u':
                    LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 7);
                    add_len = snprintf(b, 7, "%06d", timev.tv_usec);
                    break;

                case 't':
                    LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 11);
                    add_len = snprintf(b, 11, "%d", timev.tv_sec);
                    break;

                case 'r':
                    LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 11);
                    int time = (int) ((timev.tv_sec - LEAVER_G(request_time).tv_sec) * 1000
                                + (timev.tv_usec - LEAVER_G(request_time).tv_usec) / 1000);
                    add_len = snprintf(b, 11, "%d", time);
                    break;

                case 'p':
                    LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 11);
                    add_len = snprintf(b, 11, "%d", getpid());
                    break;

                case 'l':
                    LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 10); // Length of EMERGENCY.
                    add_len = snprintf(b, 10, "%s", leaver_appender_get_level_string(level));
                    break;

                case 'm':
                    LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, message->len + 1);
                    add_len = snprintf(b, message->len + 1, "%s", message->val);
                    break;

                default:
                    add_len = 1;
            }

            buf_len += add_len;
            b += add_len;
            p++;
            continue;
        }

        LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 1);

        *b = *p;
        buf_len++;
        b++;
        p++;
    }

    buf[buf_len] = '\0';

    *output = buf;
    return buf_len;
}

size_t leaver_appender_format_exception_log(char **output, zend_string *format, zval *z_exception)
{
    char token;
    char *p, *buf, *b;
    size_t buf_len, add_len, alloc_len;

    if (!z_exception || Z_TYPE_P(z_exception) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(z_exception), zend_ce_throwable)) {
        *output = emalloc(1);
        **output = '\0';
        return 0;
    }

    alloc_len = LEAVER_APPENDER_FORMAT_BUFFER_INCREASE;
    buf = emalloc(alloc_len);
    buf_len = 0;

    b = buf;
    token = 0;
    p = format->val;
    while ('\0' != *p) {
        if (!token && '%' == *p) {
            token = 1;
            p++;
            continue;
        }

        if (token) {
            token = 0;

            switch(*p) {
                case '%':
                    LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 2);
                    *b = '%';
                    add_len = 1;
                    break;

                case 'n':
                    LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 2);
                    *b = '\n';
                    add_len = 1;
                    break;

                case 'e': {
                    zend_class_entry *exception_ce = Z_OBJCE_P(z_exception);
                    LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, exception_ce->name->len + 1);
                    add_len = snprintf(b, exception_ce->name->len + 1, "%s", exception_ce->name->val);
                    break;
                }

                case 'c': {
                    zval z_code;
                    leaver_call_method_without_params(z_exception, "getcode", &z_code, NULL);
                    if (Z_TYPE(z_code) == IS_LONG) {
                        LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 21);
                        add_len = snprintf(b, 21, "%d", Z_LVAL(z_code));
                    }
                    break;
                }

                case 'm': {
                    zval z_message;
                    leaver_call_method_without_params(z_exception, "getmessage", &z_message, NULL);
                    if (Z_TYPE(z_message) == IS_STRING) {
                        LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, Z_STRLEN(z_message) + 1);
                        add_len = snprintf(b, Z_STRLEN(z_message) + 1, "%s", Z_STRVAL(z_message));
                    }
                    if (!Z_ISUNDEF(z_message)) {
                        ZVAL_PTR_DTOR(&z_message);
                    }
                    break;
                }

                case 'f': {
                    zval z_file;
                    leaver_call_method_without_params(z_exception, "getfile", &z_file, NULL);
                    if (Z_TYPE(z_file) == IS_STRING) {
                        LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, Z_STRLEN(z_file) + 1);
                        add_len = snprintf(b, Z_STRLEN(z_file) + 1, "%s", Z_STRVAL(z_file));
                    }
                    if (!Z_ISUNDEF(z_file)) {
                        ZVAL_PTR_DTOR(&z_file);
                    }
                    break;
                }

                case 'l': {
                    zval z_line;
                    leaver_call_method_without_params(z_exception, "getline", &z_line, NULL);
                    if (Z_TYPE(z_line) == IS_LONG) {
                        LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 21);
                        add_len = snprintf(b, 21, "%d", Z_LVAL(z_line));
                    }
                    break;
                }

                case 't': {
                    zval z_trace;
                    leaver_call_method_without_params(z_exception, "gettraceasstring", &z_trace, NULL);
                    if (Z_TYPE(z_trace) == IS_STRING) {
                        LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, Z_STRLEN(z_trace) + 1);
                        add_len = snprintf(b, Z_STRLEN(z_trace) + 1, "%s", Z_STRVAL(z_trace));
                    }
                    if (!Z_ISUNDEF(z_trace)) {
                        ZVAL_PTR_DTOR(&z_trace);
                    }
                    break;
                }

                default:
                    add_len = 1;
            }

            buf_len += add_len;
            b += add_len;
            p++;
            continue;
        }

        LEAVER_APPENDER_ENSURE_SIZE(buf, b, alloc_len, buf_len, 1);

        *b = *p;
        buf_len++;
        b++;
        p++;
    }

    buf[buf_len] = '\0';

    *output = buf;
    return buf_len;
}

zend_class_entry *leaver_appender_ce;

// public \Leaver\Appender::getName() : string|null
PHP_METHOD(leaver_appender, getName)
{
    zval *this = getThis();
    zval *z_name;

    z_name = zend_read_property(leaver_appender_ce, this, ZEND_STRL("name"), 1, NULL);

    RETURN_ZVAL(z_name, 1, 0);
}

// public \Leaver\Appender::setName(string $name) : void
ZEND_BEGIN_ARG_INFO_EX(leaver_appender_setName_arginfo, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(leaver_appender, setName)
{
    zval *this = getThis();
    zend_string *name;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &name)) {
        return;
    }

    zend_update_property_str(leaver_appender_ce, this, ZEND_STRL("name"), name);
}

// public \Leaver\Appender::setLevel(int $level, bool $special = false) : void
ZEND_BEGIN_ARG_INFO_EX(leaver_appender_setLevel_arginfo, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, special, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

PHP_METHOD(leaver_appender, setLevel)
{
    zval *this = getThis();
    zend_long level, accept_levels;
    zend_bool special = 0;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "l|b", &level, &special)) {
        return;
    }

    if (special) {
        accept_levels = level;
    } else {
        accept_levels = 0;
        switch (level) {
            case LEAVER_LEVEL_ALL_INT: // all
            case LEAVER_LEVEL_DEBUG_INT: // debug
                accept_levels |= LEAVER_LEVEL_DEBUG_INT;
            case LEAVER_LEVEL_INFO_INT: // info
                accept_levels |= LEAVER_LEVEL_INFO_INT;
            case LEAVER_LEVEL_NOTICE_INT: // notice
                accept_levels |= LEAVER_LEVEL_NOTICE_INT;
            case LEAVER_LEVEL_WARNING_INT: // warning
                accept_levels |= LEAVER_LEVEL_WARNING_INT;
            case LEAVER_LEVEL_ERROR_INT: // error
                accept_levels |= LEAVER_LEVEL_ERROR_INT;
            case LEAVER_LEVEL_CRITICAL_INT: // critical
                accept_levels |= LEAVER_LEVEL_CRITICAL_INT;
            case LEAVER_LEVEL_ALERT_INT: // alert
                accept_levels |= LEAVER_LEVEL_ALERT_INT;
            case LEAVER_LEVEL_EMERGENCY_INT: // emergency
                accept_levels |= LEAVER_LEVEL_EMERGENCY_INT;
            default:
                break;
        }
    }

    zend_update_property_long(leaver_appender_ce, this, ZEND_STRL("acceptLevels"), accept_levels);
}

// public \Leaver\Appender::setFormat(string $format) : void
ZEND_BEGIN_ARG_INFO_EX(leaver_appender_setFormat_arginfo, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, format, IS_STRING, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(leaver_appender, setFormat)
{
    zval *this = getThis();
    zend_string *format;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &format)) {
        return;
    }

    zend_update_property_str(leaver_appender_ce, this, ZEND_STRL("format"), format);
}

// public \Leaver\Appender::setExceptionFormat(string $exceptionFormat) : void
ZEND_BEGIN_ARG_INFO_EX(leaver_appender_setExceptionFormat_arginfo, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, exceptionFormat, IS_STRING, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(leaver_appender, setExceptionFormat)
{
    zval *this = getThis();
    zend_string *exception_format;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &exception_format)) {
        return;
    }

    zend_update_property_str(leaver_appender_ce, this, ZEND_STRL("exceptionFormat"), exception_format);
}

// public \Leaver\Appender::setLogException(bool $logException) : void
ZEND_BEGIN_ARG_INFO_EX(leaver_appender_setLogException_arginfo, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, logException, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(leaver_appender, setLogException)
{
    zval *this = getThis();
    zend_bool log_exception;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "b", &log_exception)) {
        return;
    }

    zend_update_property_bool(leaver_appender_ce, this, ZEND_STRL("logException"), log_exception);
}

// public \Leaver\Appender::append(int level, string $message, Throwable $throwable = NULL) : bool
ZEND_BEGIN_ARG_INFO_EX(leaver_appender_append_arginfo, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, message, IS_STRING, 0)
    ZEND_ARG_OBJ_INFO(0, throwable, Throwable, 1)
ZEND_END_ARG_INFO()

PHP_METHOD(leaver_appender, append)
{
    zval *this = getThis();
    zend_long level, accept_levels;
    zend_string *message;
    zval *z_exception, *z_accept_levels;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "lS|z", &level, &message, &z_exception)) {
        RETURN_FALSE;
    }

    level = level % (LEAVER_LEVEL_ALL_INT + 1);

    z_accept_levels = zend_read_property(leaver_appender_ce, this, ZEND_STRL("acceptLevels"), 1, NULL);
    accept_levels = zval_get_long(z_accept_levels);

    if (level & accept_levels) {
        zval append_params[3], retval;
        ZVAL_LONG(&append_params[0], level);
        ZVAL_STR_COPY(&append_params[1], message);
        if (Z_TYPE_P(z_exception) == IS_OBJECT) {
            ZVAL_COPY(&append_params[2], z_exception);
        } else {
            ZVAL_NULL(&append_params[2]);
        }

        leaver_call_method_with_params(this, "onappend", 3, append_params, &retval, NULL);

        zval_ptr_dtor(&append_params[1]);
        zval_ptr_dtor(&append_params[2]);

        if (Z_TYPE(retval) != IS_UNDEF) {
            zval_ptr_dtor(&retval);
        }

        RETURN_TRUE;
    }

    RETURN_FALSE;
}

// abstract public \Leaver\Appender::onAppend(int level, string $message, Throwable $throwable = NULL) : void
ZEND_BEGIN_ARG_INFO_EX(leaver_appender_onAppend_arginfo, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, message, IS_STRING, 0)
    ZEND_ARG_OBJ_INFO(0, throwable, Throwable, 1)
ZEND_END_ARG_INFO()

zend_function_entry leaver_appender_methods[] = {
    PHP_ME(leaver_appender, getName, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_appender, setName, leaver_appender_setName_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_appender, setLevel, leaver_appender_setLevel_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_appender, setFormat, leaver_appender_setFormat_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_appender, setExceptionFormat, leaver_appender_setExceptionFormat_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_appender, setLogException, leaver_appender_setLogException_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_appender, append, leaver_appender_append_arginfo, ZEND_ACC_PUBLIC)
    PHP_ABSTRACT_ME(leaver_appender, onAppend, leaver_appender_onAppend_arginfo)
    PHP_FE_END
};

LEAVER_CREATE_FUNCTION(appender)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Leaver\\Appender", leaver_appender_methods);
    leaver_appender_ce = zend_register_internal_class_ex(&ce, NULL);

    zend_declare_property_null(leaver_appender_ce, ZEND_STRL("name"), ZEND_ACC_PROTECTED);
    zend_declare_property_long(leaver_appender_ce, ZEND_STRL("acceptLevels"), LEAVER_LEVEL_ALL_INT, ZEND_ACC_PROTECTED);
    zend_declare_property_stringl(leaver_appender_ce, ZEND_STRL("format"), ZEND_STRL(LEAVER_DEFAULT_APPENDER_FORMAT), ZEND_ACC_PROTECTED);
    zend_declare_property_stringl(leaver_appender_ce, ZEND_STRL("exceptionFormat"), ZEND_STRL(LEAVER_DEFAULT_APPENDER_EXCEPTION_FORMAT), ZEND_ACC_PROTECTED);
    zend_declare_property_bool(leaver_appender_ce, ZEND_STRL("logException"), 1, ZEND_ACC_PROTECTED);

    return SUCCESS;
}