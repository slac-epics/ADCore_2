TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

DIRS += tiffBin
DIRS += jpegSrc

ifeq (win,$(findstring win,$(EPICS_HOST_ARCH)))
DIRS += tiffSrcWin
endif

ifeq (linux,$(findstring linux,$(EPICS_HOST_ARCH)))
DIRS += tiffSrcLinux
endif

#=============================

include $(TOP)/configure/RULES_DIRS
#----------------------------------------
#  ADD RULES AFTER THIS LINE

