--TEST--
Test basic logging for the Opcache
--DESCRIPTION--
This test runs a simple PHP script and ensures the Opcache
outputs the correct logging at the highest log_verbosity_level
--INI--
opcache.enable=1
opcache.enable_cli=1
opcache.file_cache=
opcache.file_cache_only=0
opcache.error_log=
opcache.log_verbosity_level=4
opcache.huge_code_pages=0
opcache.preload=
opcache.interned_strings_buffer=8
opcache.blacklist_filename=
--EXTENSIONS--
opcache
--SKIPIF--
<?php
// Prints "Debug Restarting!" message on next request.
if (getenv('SKIP_REPEAT')) die("skip Not repeatable");
if (PHP_ZTS) die("skip ZTS prints extra messages");
?>
--FILE--
<?php
echo "Foo Bar\n";
opcache_reset();
echo "Opcache reset";
?>
--EXPECTF--
%s Message Cached script '%sbasic_logging%s'
Foo Bar
%s Debug Restart Scheduled! Reason: user
Opcache reset
