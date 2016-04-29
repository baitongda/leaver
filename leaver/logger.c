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

#include "../php_leaver.h"
#include "logger.h"

extern zend_array *leaver_logger_get_appenders(zval *logger);
extern zend_string *leaver_logger_format_message(zend_string *message, zend_array *context);
extern char *leaver_logger_message_repleace(char *msg, size_t msg_len, zend_string *search, zend_string *replace);
extern void leaver_logger_common_method(INTERNAL_FUNCTION_PARAMETERS, zend_long level);
extern void leaver_logger_log(zend_array *appenders, zend_long level, zend_string *message, zend_array *context);

zend_class_entry *leaver_logger_ce;

// public \Leaver\Logger::addAppender(\Leaver\Appender $appender) : void
ZEND_BEGIN_ARG_INFO_EX(leaver_logger_addAppender_arginfo, 0, 0, 1)
    ZEND_ARG_OBJ_INFO(0, appender, Leaver\\Appender, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(leaver_logger, addAppender)
{
    zval *this = getThis();
    zval *z_appender;
    zval z_appender_name;
    zend_array *appenders;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z", &z_appender)) {
        return;
    }

    leaver_call_method_without_params(z_appender, "getname", &z_appender_name, NULL);

    appenders = leaver_logger_get_appenders(this);

    if (Z_TYPE(z_appender_name) == IS_STRING) {
        // Use string key.
        zend_hash_update(appenders, Z_STR(z_appender_name), z_appender);
    } else {
        // Use index.
        zend_hash_next_index_insert(appenders, z_appender);
    }
}

ZEND_BEGIN_ARG_INFO_EX(leaver_logger_common_logs_arginfo, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, message, IS_STRING, 0)
    ZEND_ARG_ARRAY_INFO(0, context, 1)
ZEND_END_ARG_INFO()

// public \Leaver\Logger::debug(string $message, array $context = NULL) : void
PHP_METHOD(leaver_logger, debug)
{
    leaver_logger_common_method(INTERNAL_FUNCTION_PARAM_PASSTHRU, LEAVER_LEVEL_DEBUG_INT);
}

// public \Leaver\Logger::info(string $message, array $context = NULL) : void
PHP_METHOD(leaver_logger, info)
{
    leaver_logger_common_method(INTERNAL_FUNCTION_PARAM_PASSTHRU, LEAVER_LEVEL_INFO_INT);
}

// public \Leaver\Logger::notice(string $message, array $context = NULL) : void
PHP_METHOD(leaver_logger, notice)
{
    leaver_logger_common_method(INTERNAL_FUNCTION_PARAM_PASSTHRU, LEAVER_LEVEL_DEBUG_INT);
}

// public \Leaver\Logger::warning(string $message, array $context = NULL) : void
PHP_METHOD(leaver_logger, warning)
{
    leaver_logger_common_method(INTERNAL_FUNCTION_PARAM_PASSTHRU, LEAVER_LEVEL_DEBUG_INT);
}

// public \Leaver\Logger::error(string $message, array $context = NULL) : void
PHP_METHOD(leaver_logger, error)
{
    leaver_logger_common_method(INTERNAL_FUNCTION_PARAM_PASSTHRU, LEAVER_LEVEL_DEBUG_INT);
}

// public \Leaver\Logger::critical(string $message, array $context = NULL) : void
PHP_METHOD(leaver_logger, critical)
{
    leaver_logger_common_method(INTERNAL_FUNCTION_PARAM_PASSTHRU, LEAVER_LEVEL_CRITICAL_INT);
}

// public \Leaver\Logger::alert(string $message, array $context = NULL) : void
PHP_METHOD(leaver_logger, alert)
{
    leaver_logger_common_method(INTERNAL_FUNCTION_PARAM_PASSTHRU, LEAVER_LEVEL_ALERT_INT);
}

// public \Leaver\Logger::emergency(string $message, array $context = NULL) : void
PHP_METHOD(leaver_logger, emergency)
{
    leaver_logger_common_method(INTERNAL_FUNCTION_PARAM_PASSTHRU, LEAVER_LEVEL_EMERGENCY_INT);
}

// public \Leaver\Logger::log(int $level, string $message, array $context = NULL) : void
ZEND_BEGIN_ARG_INFO_EX(leaver_logger_log_arginfo, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, message, IS_STRING, 0)
    ZEND_ARG_ARRAY_INFO(0, context, 1)
ZEND_END_ARG_INFO()

PHP_METHOD(leaver_logger, log)
{
    zval *this = getThis();
    zend_long level;
    zend_string *message;
    zend_array *context = NULL;
    zend_array *appenders;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "lS|h", &level, &message, &context)) {
        return;
    }

    appenders = leaver_logger_get_appenders(this);

    leaver_logger_log(appenders, level, message, context);
}

zend_function_entry leaver_logger_methods[] = {
    PHP_ME(leaver_logger, addAppender, leaver_logger_addAppender_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_logger, debug, leaver_logger_common_logs_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_logger, info, leaver_logger_common_logs_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_logger, notice, leaver_logger_common_logs_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_logger, warning, leaver_logger_common_logs_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_logger, error, leaver_logger_common_logs_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_logger, critical, leaver_logger_common_logs_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_logger, alert, leaver_logger_common_logs_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_logger, emergency, leaver_logger_common_logs_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(leaver_logger, log, leaver_logger_log_arginfo, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

LEAVER_CREATE_FUNCTION(logger)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Leaver\\Logger", leaver_logger_methods);
    leaver_logger_ce = zend_register_internal_class_ex(&ce, NULL);

    zend_declare_class_constant_long(leaver_logger_ce, ZEND_STRL(LEAVER_LEVEL_ALL), LEAVER_LEVEL_ALL_INT);
    zend_declare_class_constant_long(leaver_logger_ce, ZEND_STRL(LEAVER_LEVEL_DEBUG), LEAVER_LEVEL_DEBUG_INT);
    zend_declare_class_constant_long(leaver_logger_ce, ZEND_STRL(LEAVER_LEVEL_INFO), LEAVER_LEVEL_INFO_INT);
    zend_declare_class_constant_long(leaver_logger_ce, ZEND_STRL(LEAVER_LEVEL_NOTICE), LEAVER_LEVEL_NOTICE_INT);
    zend_declare_class_constant_long(leaver_logger_ce, ZEND_STRL(LEAVER_LEVEL_WARNING), LEAVER_LEVEL_WARNING_INT);
    zend_declare_class_constant_long(leaver_logger_ce, ZEND_STRL(LEAVER_LEVEL_ERROR), LEAVER_LEVEL_ERROR_INT);
    zend_declare_class_constant_long(leaver_logger_ce, ZEND_STRL(LEAVER_LEVEL_CRITICAL), LEAVER_LEVEL_CRITICAL_INT);
    zend_declare_class_constant_long(leaver_logger_ce, ZEND_STRL(LEAVER_LEVEL_ALERT), LEAVER_LEVEL_ALERT_INT);
    zend_declare_class_constant_long(leaver_logger_ce, ZEND_STRL(LEAVER_LEVEL_EMERGENCY), LEAVER_LEVEL_EMERGENCY_INT);
    zend_declare_class_constant_long(leaver_logger_ce, ZEND_STRL(LEAVER_LEVEL_OFF), LEAVER_LEVEL_OFF_INT);

    zend_declare_property_null(leaver_logger_ce, ZEND_STRL("appenders"), ZEND_ACC_PRIVATE);

    return SUCCESS;
}

zend_array *leaver_logger_get_appenders(zval *logger)
{
    zend_array *appenders;
    zval *z_appenders;

    z_appenders = zend_read_property(leaver_logger_ce, logger, ZEND_STRL("appenders"), 1, NULL);

    if (Z_TYPE_P(z_appenders) == IS_ARRAY) {
        appenders = Z_ARR_P(z_appenders);
    } else {
        ALLOC_HASHTABLE(appenders);
        zend_hash_init(appenders, 4, NULL, NULL, 0);

        ZVAL_ARR(z_appenders, appenders);

        zend_update_property(leaver_logger_ce, logger, ZEND_STRL("appenders"), z_appenders);
    }

    return appenders;
}

char *leaver_logger_message_repleace(char *msg, size_t msg_len, zend_string *search, zend_string *replace)
{
    char *search_str;
    size_t search_len;
    char *start, *tmp, *result;
    size_t result_len;
    const char *found;

    search_len = search->len + 2;
    search_str = emalloc(search_len + 1);
    search_str[0] = '{';
    strcpy(search_str + 1, search->val);
    search_str[search_len - 1] = '}';
    search_str[search_len] = '\0';

    result = msg;
    result_len = msg_len;
    start = result;

    while ((found = php_memnstr(start, search_str, search_len, result + result_len))) {
        // New length.
        result_len = result_len + replace->len - search_len;
        tmp = emalloc(result_len + 1);

        strncpy(tmp, result, found - result);
        tmp[found - result] = '\0';
        strcat(tmp, replace->val);
        strcat(tmp, found + search_len);

        start = tmp + (found - result) + replace->len;
        result = tmp;
    }

    efree(search_str);

    return result != msg ? result : NULL;
}

// Replaces placeholders with values from the context array.
zend_string *leaver_logger_format_message(zend_string *message, zend_array *context)
{
    zend_string *key;
    zval *z_value;
    zend_string *result;
    char *str, *tmp;

    if (0 == message->len) {
        zend_interned_empty_string_init(&result);
        return result;
    }

    // Duplicates the message string for repeated replacement.
    str = estrdup(message->val);

    ZEND_HASH_FOREACH_STR_KEY_VAL(context, key, z_value) {
        if (key && 0 != strcasecmp(key->val, "exception")) {
            zend_string *value = zval_get_string(z_value);

            tmp = leaver_logger_message_repleace(str, strlen(str), key, value);
            if (tmp) {
                efree(str);
                str = tmp;
            }

            zend_string_release(value);
        }
    } ZEND_HASH_FOREACH_END();

    result = zend_string_init(str, strlen(str), 0);
    efree(str);

    return result;
}

void leaver_logger_common_method(INTERNAL_FUNCTION_PARAMETERS, zend_long level)
{
    zval *this = getThis();
    zend_string *message;
    zend_array *context = NULL;
    zend_array *appenders;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S|h", &message, &context)) {
        return;
    }

    appenders = leaver_logger_get_appenders(this);

    leaver_logger_log(appenders, level, message, context);
}

void leaver_logger_log(zend_array *appenders, zend_long level, zend_string *message, zend_array *context)
{
    zval *z_appender, *z_exception = NULL;
    zval retval;
    zval is_enable_params[1], append_params[3];
    zend_function *func_append = NULL;

    if (context) {
        message = leaver_logger_format_message(message, context);
        z_exception = zend_hash_str_find(context, ZEND_STRL("exception"));
    }

    ZVAL_LONG(&is_enable_params[0], level);
    ZVAL_LONG(&append_params[0], level);
    ZVAL_STR(&append_params[1], message);
    if (z_exception) {
        ZVAL_COPY(&append_params[2], z_exception);
    } else {
        ZVAL_NULL(&append_params[2]);
    }

    ZEND_HASH_FOREACH_VAL(appenders, z_appender) {
        leaver_call_method_with_params(z_appender, "append", 3, append_params, &retval, &func_append);
    } ZEND_HASH_FOREACH_END();

    zval_ptr_dtor(&append_params[1]);
    zval_ptr_dtor(&append_params[2]);
}