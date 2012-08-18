#             __________               __   ___.
#   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
#   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
#   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
#   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
#                     \/            \/     \/    \/            \/
# $Id$
#

TOTPSRCDIR := $(APPSDIR)/plugins/totp
TOTPBUILDDIR := $(BUILDDIR)/apps/plugins/totp

ROCKS += $(TOTPBUILDDIR)/totp.rock

TOTP_SRC := $(call preprocess, $(TOTPSRCDIR)/SOURCES)
TOTP_OBJ := $(call c2obj, $(TOTP_SRC))

# add source files to OTHER_SRC to get automatic dependencies
OTHER_SRC += $(TOTP_SRC)

$(TOTPBUILDDIR)/totp.rock: $(TOTP_OBJ)
