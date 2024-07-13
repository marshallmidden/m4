#!/usr/bin/python3 -B

import re

def check_ar(txt, ar):
#--     my_regex = re.escape(ar) + r'$'
    my_regex = ar + r'$'

    if re.match(my_regex, txt):
        print(f'Found ar={ar} in txt="{txt}"')
    else:
        print(f'NOT FOUND ar={ar} in txt="{txt}"')
    # fi
#   End of check_ar

er = 'ERROR - .*'
num = 'NUMBER'

check_ar('This is a test', er)
check_ar('This is a test', num)

check_ar('ERROR - This is a test', er)
check_ar('ERROR - This is a test', num)

check_ar('NUMBER', er)
check_ar('NUMBER', num)

quit()
