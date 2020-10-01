/^[^S]/d
/^SCSI device sdi:/d
s/^SCSI device \(sd.*\): \([0-9][0-9]*\) 512-byte hdwr sectors .*$/.\/diskpattern -s \2 -n NIBBLE \1 > DISK.\1 \&/
