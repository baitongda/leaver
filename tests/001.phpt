--TEST--
Check leaver installation
--SKILIF--
<?php if (!extension_loaded("leaver") print "skip!"; ?>
--FILE--
<?php
echo "leaver is available";

?>
--EXPECT--
leaver is available