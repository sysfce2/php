--TEST--
Test getimagesize() function : variation - For shockwave-flash format
--EXTENSIONS--
zlib
--SKIPIF--
<?php
if (!defined("IMAGETYPE_SWC")) {
    die("skip IMAGETYPE_SWC is not available");
}
?>
--FILE--
<?php
echo "*** Testing getimagesize() : variation ***\n";

var_dump( getimagesize(__DIR__."/test13pix.swf", $info) );
var_dump( $info );
?>
--EXPECT--
*** Testing getimagesize() : variation ***
array(7) {
  [0]=>
  int(550)
  [1]=>
  int(400)
  [2]=>
  int(13)
  [3]=>
  string(24) "width="550" height="400""
  ["mime"]=>
  string(29) "application/x-shockwave-flash"
  ["width_unit"]=>
  string(2) "px"
  ["height_unit"]=>
  string(2) "px"
}
array(0) {
}
