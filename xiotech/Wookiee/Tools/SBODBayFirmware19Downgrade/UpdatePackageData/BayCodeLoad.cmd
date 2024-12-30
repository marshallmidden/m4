

# Config file for DriveCodeLoad.pl


# choices for the device are 'drive' or 'bay'
[Type]
Device         bay


# Paths to the files can be relative (to the launch point of the script)
# or absolute as needed.
# FW rev is what would come from pdiskinfo in CCBCL.pl, ditto for the model
# Remove any spaces in the model name
#    i.e. "Fibre Bay 1000" should be written FibreBay1000

[Bay]
# model          fw rev     main file                                   secondary (servo) file
SBODFCBay        1919       ./Fbdv19_00.ima                               none 

