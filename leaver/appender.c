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
#include <zend_smart_str.h>

#include "../php_leaver.h"
#include "appender.h"

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

char *leaver_appender_get_datetime_string(struct timeval timev)
{
    char *buf;
    struct tm * timeinfo;

    buf = emalloc(17);
    timeinfo = localtime(&timev.tv_sec);

    strftime(buf, 17, "%Y%m%d%H%M%S%p", timeinfo);

    return buf;
}

// Usable tokens:
//  %% for '%'; %n new line; %Y year; %M month; %D day; %H hour; %I minute; %S second; %s millisecond;
//  %A AM or PM; %u microsecond; %t timestamp; %r millisecond from request start; %p process id; %l log level;
//  %m log message;
size_t leaver_appender_format_log(char **output, zend_string *format, zend_long level, zend_string *message)
{
    char token;
    char *p, *datetime = NULL;
    size_t output_len;
    smart_str buf = {0};
    struct timeval timev;

    gettimeofday(&timev, NULL);

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
                    smart_str_appendc(&buf, '%');
                    break;

                case 'n':
                    smart_str_appendc(&buf, '\n');
                    break;

                case 'Y':
                    if (!datetime) {
                        datetime = leaver_appender_get_datetime_string(timev);
                    }
                    smart_str_appendl(&buf, datetime, 4);
                    break;

                case 'M':
                    if (!datetime) {
                        datetime = leaver_appender_get_datetime_string(timev);
                    }
                    smart_str_appendl(&buf, datetime + 4, 2);
                    break;

                case 'D':
                    if (!datetime) {
                        datetime = leaver_appender_get_datetime_string(timev);
                    }
                    smart_str_appendl(&buf, datetime + 6, 2);
                    break;

                case 'H':
                    if (!datetime) {
                        datetime = leaver_appender_get_datetime_string(timev);
                    }
                    smart_str_appendl(&buf, datetime + 8, 2);
                    break;

                case 'I':
                    if (!datetime) {
                        datetime = leaver_appender_get_datetime_string(timev);
                    }
                    smart_str_appendl(&buf, datetime + 10, 2);
                    break;

                case 'S':
                    if (!datetime) {
                        datetime = leaver_appender_get_datetime_string(timev);
                    }
                    smart_str_appendl(&buf, datetime + 12, 2);
                    break;

                case 's': {
                    int msec = timev.tv_usec / 1000;
                    if (msec < 10) {
                        smart_str_appendl(&buf, "00", 2);
                    } else if (msec < 100) {
                        smart_str_appendl(&buf, "0", 1);
                    }
                    smart_str_append_long(&buf, timev.tv_usec);
                    break;
                }

                case 'A':
                    if (!datetime) {
                        datetime = leaver_appender_get_datetime_string(timev);
                    }
                    smart_str_appendl(&buf, datetime + 14, 2);
                    break;

                case 'u':
                    smart_str_append_long(&buf, timev.tv_usec);
                    break;

                case 't':
                    smart_str_append_long(&buf, timev.tv_sec);
                    break;

                case 'r': {
                    int time = (int) ((timev.tv_sec - LEAVER_G(request_time).tv_sec) * 1000
                                      + (timev.tv_usec - LEAVER_G(request_time).tv_usec) / 1000);
                    smart_str_append_long(&buf, time);
                    break;
                }

                case 'p':
                    smart_str_append_long(&buf, getpid());
                    break;

                case 'l': {
                    char *clevel = leaver_appender_get_level_string(level);
                    smart_str_appendl(&buf, clevel, strlen(clevel));
                    break;
                }

                case 'm':
                    smart_str_append(&buf, message);
                    break;

                default:
                    break;
            }
        } else {
            smart_str_appendc(&buf, *p);
        }

        p++;
    }

    smart_str_0(&buf);

    *output = estrndup(ZSTR_VAL(buf.s), ZSTR_LEN(buf.s));
    output_len = ZSTR_LEN(buf.s);

    smart_str_free(&buf);
    if (datetime) {
        efree(datetime);
    }

    return output_len;
}

size_t leaver_appender_format_exception_log(char **output, zend_string *format, zval *z_exception)
{
    char token;
    char *p;
    size_t output_len;
    smart_str buf = {0};

    if (format->len == 0 || !z_exception || Z_TYPE_P(z_exception) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(z_exception), zend_ce_throwable)) {
        *output = "";
        return 0;
    }

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
                    smart_str_appendc(&buf, '%');
                    break;

                case 'n':
                    smart_str_appendc(&buf, '\n');
                    break;

                case 'e': {
                    zend_class_entry *exception_ce = Z_OBJCE_P(z_exception);
                    smart_str_append(&buf, exception_ce->name);
                    break;
                }

                case 'c': {
                    zval z_code;
                    leaver_call_method_without_params(z_exception, "getcode", &z_code, NULL);
                    if (Z_TYPE(z_code) == IS_LONG) {
                        smart_str_append_long(&buf, Z_LVAL(z_code));
                    }
                    break;
                }

                case 'm': {
                    zval z_message;
                    leaver_call_method_without_params(z_exception, "getmessage", &z_message, NULL);
                    if (Z_TYPE(z_message) == IS_STRING) {
                        smart_str_append(&buf, Z_STR(z_message));
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
                        smart_str_append(&buf, Z_STR(z_file));
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
                        smart_str_append_long(&buf, Z_LVAL(z_line));
                    }
                    break;
                }

                case 't': {
                    zval z_trace;
                    leaver_call_method_without_params(z_exception, "gettraceasstring", &z_trace, NULL);
                    if (Z_TYPE(z_trace) == IS_STRING) {
                        smart_str_append(&buf, Z_STR(z_trace));
                    }
                    if (!Z_ISUNDEF(z_trace)) {
                        ZVAL_PTR_DTOR(&z_trace);
                    }
                    break;
                }

                default:
                    break;
            }
        } else {
            smart_str_appendc(&buf, *p);
        }

        p++;
    }

    smart_str_0(&buf);

    *output = estrndup(ZSTR_VAL(buf.s), ZSTR_LEN(buf.s));
    output_len = ZSTR_LEN(buf.s);

    smart_str_free(&buf);

    return output_len;
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