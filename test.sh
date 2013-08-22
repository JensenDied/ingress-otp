#!/bin/bash
echo "----"
echo "---- Starting Format Tests"
echo "----"
echo "---- Format 1"
echo "-- Minotaur 1"
time ./otp.bin SLQAYZCAOPRIOHRWYZHRDHLRBLQRGIVTGOEIBVSSIYCTPDOTB e31492102271d3d2dfb46e12bb3643f0 2 1
echo "-- Minotaur 2"
time ./otp.bin SUZGNEKXBNORVSNVEACAVDHFYXZGDWPISLOVOXLAGESFLAMAN 6b7ca401083b34628db954c1e8b1c5de 3 1
echo "-- Minotaur 3"
time ./otp.bin LNGFOVHMJXMLGXRIJLOZMHRRONBHHQLZJUQXVUWQRUJASQOKR f0f8fd5d99d227caaa3d02bad43179cd 4 1
echo "-- Minotaur 4"
time ./otp.bin PHPUHTXFJZCQPKVACWBDSZTVAJKLEWGKVPYQXFGKMWWJYDJZF 029d2072bd788bac6ebfcf431c590001 5 1
echo "-- Milan 2"
time ./otp.bin MZPGMGYBVTWXUYIRCURRIWYCDAQEPGVRSHNKNFHIDCHREZFBEFEQFMPGDCV 3ba87d382ee137fc9d6737437f57b550 8 1
echo "-- Tokyo 1"
time ./otp.bin LSEFLMCXCDNLEBGOLTAAGUDKJJIEQYLREGGHVHSXDNZAQGJGVSZAWZPVUBX 430a199d8124796826030643ed15e8f3 6 1
echo "-- Dusseldorf 1"
time ./otp.bin DZMWLIPRIVLOBSUPJMEDOIXWXQASQKYIXUXOYYSSFINOICBLPTEELJDWCSJ d811aa7b690d26a4052eef7fb0f292b7 2 1

echo "---- format 3"
echo "-- Sydney 2"
time ./otp.bin VJUCHIYAIVIYYCPXYGTJOBZDCDNMLZJYJRDAUYZICNQRQTNUWKHZDUDHOTB 544444f7085ec9c2bd6152c25e808e2d 2 3

echo "---- format 12"
echo "-- Paris 1"
time ./otp.bin VHAZKCXSDWUETLPDVYIQPTQTZRLLRDQTAPCMMAMNCQROTRTDTQWHMXRTUSV 913689159c64388286f36d451bba48fc 7 12
echo "----"
echo "---- Starting Batch Test"
echo "----"
