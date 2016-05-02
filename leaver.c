/*
 * Leaver - A fast logger for PHP
 *
 * Copyright (c) 2016 You Ming
 * The MIT License (MIT)
 * http://leaver.funcuter.org
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include <ext/standard/info.h>

#include "php_leaver.h"
#include "leaver/logger.h"
#include "leaver/appender.h"
#include "leaver/appender/null.h"
#include "leaver/appender/file.h"

ZEND_DECLARE_MODULE_GLOBALS(leaver);

PHP_INI_BEGIN()

PHP_INI_END()

PHP_GINIT_FUNCTION(leaver)
{
}

PHP_MINIT_FUNCTION(leaver)
{
    LEAVER_CREATE(logger);
    LEAVER_CREATE(appender);
    LEAVER_CREATE(appender_null);
    LEAVER_CREATE(appender_file);

    LEAVER_LOG("Leaver is registered to PHP.");

    return SUCCESS;
}

PHP_RINIT_FUNCTION(leaver)
{
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(leaver)
{

    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(leaver)
{

    return SUCCESS;
}

PHP_GSHUTDOWN_FUNCTION(leaver)
{
}

PHP_MINFO_FUNCTION(leaver)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "Leaver Support", "enabled");
    php_info_print_table_row(2, "Version", LEAVER_VERSION);
    php_info_print_table_row(2, "Author", LEAVER_AUTHOR);
    php_info_print_table_row(2, "Homepage", LEAVER_HOMEPAGE);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}

const zend_function_entry leaver_functions[] = {
    PHP_FE_END
};

zend_module_entry leaver_module_entry = {
    STANDARD_MODULE_HEADER,
    "leaver",
    leaver_functions,
    PHP_MINIT(leaver),
    PHP_MSHUTDOWN(leaver),
    PHP_RINIT(leaver),
    PHP_RSHUTDOWN(leaver),
    PHP_MINFO(leaver),
    LEAVER_VERSION,
    PHP_MODULE_GLOBALS(leaver),
    PHP_GINIT(leaver),
    PHP_GSHUTDOWN(leaver),
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_LEAVER
ZEND_GET_MODULE(leaver)
#endif

int leaver_call_method(zend_class_entry *obj_ce, zval *obj, const char *func_name, size_t func_name_len,
                       uint32_t params_count, zval* params, zval *retval, zend_function **func)
{
    int result;
    zend_fcall_info fci;
    zend_array *func_table;

    if (obj && Z_TYPE_P(obj) != IS_OBJECT) {
        obj = NULL;
    }

    fci.size = sizeof(fci);
    // fci.function_table
    ZVAL_STRINGL(&fci.function_name, func_name, func_name_len);
    fci.symbol_table = NULL;
    fci.retval = retval;
    fci.params = params;
    fci.object = obj ? Z_OBJ_P(obj) : NULL;
    fci.no_separation = 1;
    fci.param_count = params_count;

    if (!func && !obj_ce) {
        fci.function_table = !obj ? EG(function_table) : NULL;
        result = zend_call_function(&fci, NULL);
    } else {
        zend_fcall_info_cache fcic;

        fcic.initialized = 1;
        if (!obj_ce) {
            obj_ce = obj ? Z_OBJCE_P(obj) : NULL;
        }

        if (obj_ce) {
            func_table = &obj_ce->function_table;
        } else {
            func_table = EG(function_table);
        }

        if (!func || !*func) {
            if (NULL == (fcic.function_handler = zend_hash_find_ptr(func_table, Z_STR(fci.function_name)))) {
                // Core error.
                zend_error_noreturn(E_CORE_ERROR, "Couldn't find implementation for method %s%s%s",
                                    obj_ce ? ZSTR_VAL(obj_ce->name) : "", obj_ce ? "::" : "", func_name);
            }

            if (func) {
                *func = fcic.function_handler;
            }
        } else {
            // Provided function handler.
            fcic.function_handler = *func;
        }

        fcic.calling_scope = obj_ce;
        if (obj) {
            fcic.called_scope = Z_OBJCE_P(obj);
        } else {
            zend_class_entry *called_scope = zend_get_called_scope(EG(current_execute_data));

            if (obj_ce && (!called_scope || !instanceof_function(called_scope, obj_ce))) {
                fcic.called_scope = obj_ce;
            } else {
                fcic.called_scope = called_scope;
            }
        }

        fcic.object = obj ? Z_OBJ_P(obj) : NULL;
        result = zend_call_function(&fci, &fcic);
    }

    zval_ptr_dtor(&fci.function_name);

    if (FAILURE == result) {
        if (!obj_ce) {
            obj_ce = obj ? Z_OBJCE_P(obj) : NULL;
        }

        if (!EG(exception)) {
            // If no exception thrown, case core error.
            zend_error_noreturn(E_CORE_ERROR, "Couldn't execute method %s%s%s",
                                obj_ce ? ZSTR_VAL(obj_ce->name) : "", obj_ce ? "::" : "", func_name);
        }
    }

    return result;
}