<?php
if(count($argv) != 4) {
    die("Usage:\n\t{$argv[0]} [e|d] otp1 otp2");
}
$str = $argv[2];
$otp = $argv[3];
$out = '';
$a = ord('A');
$z = ord('Z');
$len = strlen($str);
$operation = null;
switch($argv[1]) {
    case 'd':
        $operation = function($a, $b) { return ord($a) - (ord($b) - ord('A')); };
        break;
    case 'e':
        $operation = function($a, $b) { return ord($a) + ord($b) - ord('A'); };
        break;
    default:
        die('Invalid argument for operation, must be one of: d, e.');
}
for($i = 0; $i < $len; $i++) {
    $char = $operation($str[$i], $otp[$i]);
    if($char < $a)
        $char += 26;
    if($char > $z)
        $char -= 26;
    $out .= chr($char);
}
print $out;


