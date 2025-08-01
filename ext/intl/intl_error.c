/*
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.php.net/license/3_01.txt                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Vadim Savchuk <vsavchuk@productengine.com>                  |
   |          Dmitry Lakhtyuk <dlakhtyuk@productengine.com>               |
   |          Stanislav Malyshev <stas@zend.com>                          |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <php.h>
#include <zend_exceptions.h>

#include "php_intl.h"
#include "intl_error.h"
#include "intl_convert.h"

ZEND_EXTERN_MODULE_GLOBALS( intl )

zend_class_entry *IntlException_ce_ptr;

/* {{{ Return global error structure. */
static intl_error* intl_g_error_get( void )
{
	return &INTL_G( g_error );
}
/* }}} */

/* {{{ Free mem. */
static void intl_free_custom_error_msg( intl_error* err )
{
	if( !err && !( err = intl_g_error_get(  ) ) )
		return;

	if (err->custom_error_message) {
		zend_string_release_ex(err->custom_error_message, false);
		err->custom_error_message = NULL;
	}
}
/* }}} */

/* {{{ Create and initialize  internals of 'intl_error'. */
intl_error* intl_error_create( void )
{
	intl_error* err = ecalloc( 1, sizeof( intl_error ) );

	intl_error_init( err );

	return err;
}
/* }}} */

/* {{{ Initialize internals of 'intl_error'. */
void intl_error_init( intl_error* err )
{
	if( !err && !( err = intl_g_error_get(  ) ) )
		return;

	err->code                      = U_ZERO_ERROR;
	err->custom_error_message      = NULL;
}
/* }}} */

/* {{{ Set last error code to 0 and unset last error message */
void intl_error_reset( intl_error* err )
{
	if( !err && !( err = intl_g_error_get(  ) ) )
		return;

	err->code = U_ZERO_ERROR;

	intl_free_custom_error_msg( err );
}
/* }}} */

/* {{{ Set last error message to msg copying it if needed. */
void intl_error_set_custom_msg( intl_error* err, const char* msg)
{
	/* See ext/intl/tests/bug70451.phpt and uchar.c:zif_IntlChar_charFromName */
	if (UNEXPECTED(msg == NULL)) {
		return;
	}

	zend_string *method_or_func = get_active_function_or_method_name();
	zend_string *prefixed_message = zend_string_concat3(
		ZSTR_VAL(method_or_func), ZSTR_LEN(method_or_func),
		ZEND_STRL("(): "),
		msg, strlen(msg)
	);
	zend_string_release_ex(method_or_func, false);

	if( !err ) {
		if (INTL_G(error_level)) {
			/* Docref will prefix the function/method for us, so use original message */
			php_error_docref( NULL, INTL_G( error_level ), "%s", msg );
		}
		if (INTL_G(use_exceptions)) {
			/* Use this variant as we have a zend_string already */
			zend_throw_error_exception(IntlException_ce_ptr, prefixed_message, 0, 0);
		}
	}
	if (!err && !(err = intl_g_error_get() )) {
		zend_string_release_ex(prefixed_message, false);
		return;
	}

	/* Free previous message if any */
	intl_free_custom_error_msg( err );

	/* Set user's error text message */
	err->custom_error_message = prefixed_message;
}
/* }}} */

/* {{{ Create output message in format "<intl_error_text>: <extra_user_error_text>". */
zend_string * intl_error_get_message( intl_error* err )
{
	const char *uErrorName = NULL;
	zend_string *errMessage = NULL;

	if( !err && !( err = intl_g_error_get(  ) ) )
		return ZSTR_EMPTY_ALLOC();

	uErrorName = u_errorName( err->code );
	size_t uErrorLen = strlen(uErrorName);

	/* Format output string */
	if (err->custom_error_message) {
		errMessage = zend_string_concat3(
			ZSTR_VAL(err->custom_error_message), ZSTR_LEN(err->custom_error_message),
			ZEND_STRL(": "),
			uErrorName, uErrorLen);
	} else {
		errMessage = zend_string_init(uErrorName, strlen(uErrorName), false);
	}

	return errMessage;
}
/* }}} */

/* {{{ Set last error code. */
void intl_error_set_code( intl_error* err, UErrorCode err_code )
{
	if( !err && !( err = intl_g_error_get(  ) ) )
		return;

	err->code = err_code;
}
/* }}} */

/* {{{ Return last error code. */
UErrorCode intl_error_get_code( intl_error* err )
{
	if( !err && !( err = intl_g_error_get(  ) ) )
		return U_ZERO_ERROR;

	return err->code;
}
/* }}} */

/* {{{ Set error code and message. */
void intl_error_set( intl_error* err, UErrorCode code, const char* msg)
{
	intl_error_set_code( err, code );
	intl_error_set_custom_msg( err, msg);
}
/* }}} */

/* {{{ Set error code and message. */
void intl_errors_set( intl_error* err, UErrorCode code, const char* msg)
{
	intl_errors_set_code( err, code );
	intl_errors_set_custom_msg( err, msg);
}
/* }}} */

/* {{{ */
void intl_errors_reset( intl_error* err )
{
	if(err) {
		intl_error_reset( err );
	}
	intl_error_reset( NULL );
}
/* }}} */

/* {{{ */
void intl_errors_set_custom_msg(intl_error* err, const char* msg)
{
	if(err) {
		intl_error_set_custom_msg( err, msg);
	}
	intl_error_set_custom_msg( NULL, msg);
}
/* }}} */

/* {{{ */
void intl_errors_set_code( intl_error* err, UErrorCode err_code )
{
	if(err) {
		intl_error_set_code( err, err_code );
	}
	intl_error_set_code( NULL, err_code );
}
/* }}} */

smart_str intl_parse_error_to_string( UParseError* pe )
{
	smart_str    ret = {0};
	zend_string *u8str;
	UErrorCode   status;
	int          any = 0;

	assert( pe != NULL );

	smart_str_appends( &ret, "parse error " );
	if( pe->line > 0 )
	{
		smart_str_appends( &ret, "on line " );
		smart_str_append_long( &ret, (zend_long ) pe->line );
		any = 1;
	}
	if( pe->offset >= 0 ) {
		if( any )
			smart_str_appends( &ret, ", " );
		else
			smart_str_appends( &ret, "at " );

		smart_str_appends( &ret, "offset " );
		smart_str_append_long( &ret, (zend_long ) pe->offset );
		any = 1;
	}

	if (pe->preContext[0] != 0 ) {
		if( any )
			smart_str_appends( &ret, ", " );

		smart_str_appends( &ret, "after \"" );
		u8str = intl_convert_utf16_to_utf8(pe->preContext, -1, &status );
		if( !u8str )
		{
			smart_str_appends( &ret, "(could not convert parser error pre-context to UTF-8)" );
		}
		else {
			smart_str_append( &ret, u8str );
			zend_string_release_ex( u8str, 0 );
		}
		smart_str_appends( &ret, "\"" );
		any = 1;
	}

	if( pe->postContext[0] != 0 )
	{
		if( any )
			smart_str_appends( &ret, ", " );

		smart_str_appends( &ret, "before or at \"" );
		u8str = intl_convert_utf16_to_utf8(pe->postContext, -1, &status );
		if( !u8str )
		{
			smart_str_appends( &ret, "(could not convert parser error post-context to UTF-8)" );
		}
		else
		{
			smart_str_append( &ret, u8str );
			zend_string_release_ex( u8str, 0 );
		}
		smart_str_appends( &ret, "\"" );
		any = 1;
	}

	if( !any )
	{
		smart_str_free( &ret );
		smart_str_appends( &ret, "no parse error" );
	}

	smart_str_0( &ret );
	return ret;
}
