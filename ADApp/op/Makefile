TOP = ../..
include $(TOP)/configure/CONFIG

ADL_DIR = adl
UI_DIR  = ui/autoconvert
EDL_DIR = edl/autoconvert
OPI_DIR = opi/autoconvert

include $(CONFIG)/RULES_DIRS

ADL2EDL = adl2edl
EDMFONTFILE = edl/fonts.adl2edl
SUPPORT = $(EPICS_MODULES)/synapps/latest

-include $(SUPPORT)/configure/RULES_OPI
