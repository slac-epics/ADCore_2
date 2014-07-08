TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#----------------------------------------------------
#  Optimization of db files using dbst (DEFAULT: NO)
#DB_OPT = YES

#----------------------------------------------------
# Create and install (or just install)
# databases, templates, substitutions like this

# General
DB += ADBase.template
DB += ADPrefixes.template

# Drivers
DB += simDetector.template

# Plugins
DB += NDColorConvert.template
DB += NDFile.template
DB += NDFileHDF5.template
DB += NDFileJPEG.template
DB += NDFileMagick.template
DB += NDFileNetCDF.template
DB += NDFileNexus.template
DB += NDFileTIFF.template
DB += NDOverlay.template
DB += NDOverlayN.template
DB += NDPluginBase.template
DB += NDProcess.template
DB += NDROI.template
DB += NDStats.template
DB += NDStdArrays.template
DB += NDTransform.template
DB += NDAttribute.template

# Install archive files
ARCHIVE += ADBase.tpl-arch
ARCHIVE += ADPrefixes.tpl-arch
ARCHIVE += NDColorConvert.tpl-arch
ARCHIVE += NDFileHDF5.tpl-arch
ARCHIVE += NDFileJPEG.tpl-arch
ARCHIVE += NDFileMagick.tpl-arch
ARCHIVE += NDFileNetCDF.tpl-arch
ARCHIVE += NDFileNexus.tpl-arch
ARCHIVE += NDFileTIFF.tpl-arch
ARCHIVE += NDFile.tpl-arch
ARCHIVE += NDOverlayN.tpl-arch
ARCHIVE += NDOverlay.tpl-arch
ARCHIVE += NDPluginBase.tpl-arch
ARCHIVE += NDProcess.tpl-arch
ARCHIVE += NDROI_sync.tpl-arch
ARCHIVE += NDROI.tpl-arch
ARCHIVE += NDStats.tpl-arch
ARCHIVE += NDStdArrays.tpl-arch
ARCHIVE += NDTransform.tpl-arch

# Install autosave files
REQ += ADBase_settings.req
REQ += commonPlugin_settings.req
REQ += NDColorConvert_settings.req
REQ += NDFileHDF5_settings.req
REQ += NDFileJPEG_settings.req
REQ += NDFileMagick_settings.req
REQ += NDFileNetCDF_settings.req
REQ += NDFileNexus_settings.req
REQ += NDFileTIFF_settings.req
REQ += NDFile_settings.req
REQ += NDOverlayN_settings.req
REQ += NDOverlay_settings.req
REQ += NDPluginBase_settings.req
REQ += NDProcess_settings.req
REQ += NDROI_settings.req
REQ += NDStats_settings.req
REQ += NDStdArrays_settings.req
REQ += NDTransform_settings.req

#----------------------------------------------------
# If <anyname>.db template is not named <anyname>*.template add
# <anyname>_template = <templatename>

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

