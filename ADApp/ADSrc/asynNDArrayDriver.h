#ifndef asynNDArrayDriver_H
#define asynNDArrayDriver_H

#include <epicsMutex.h>
#include <epicsEvent.h>

#include "asynPortDriver.h"
#include "NDArray.h"
#include "ADCoreVersion.h"

/** Maximum length of a filename or any of its components */
#define MAX_FILENAME_LEN 256

/** Enumeration of file saving modes */
typedef enum {
    NDFileModeSingle,       /**< Write 1 array per file */
    NDFileModeCapture,      /**< Capture NDNumCapture arrays into memory, write them out when capture is complete.
                              *  Write all captured arrays to a single file if the file format supports this */
    NDFileModeStream        /**< Stream arrays continuously to a single file if the file format supports this */
} NDFileMode_t;

typedef enum {
    NDFileWriteOK,
    NDFileWriteError
} NDFileWriteStatus_t;

typedef enum {
    NDAttributesOK,
    NDAttributesFileNotFound,
    NDAttributesXMLSyntaxError,
    NDAttributesMacroError
} NDAttributesStatus_t;

/** Strings defining parameters that affect the behaviour of the detector.
  * These are the values passed to drvUserCreate.
  * The driver will place in pasynUser->reason an integer to be used when the
  * standard asyn interface methods are called. */
 /*                               String                 asyn interface  access   Description  */
#define NDPortNameSelfString    "PORT_NAME_SELF"    /**< (asynOctet,    r/o) Asyn port name of this driver instance */

/** ADCore version string */
#define NDADCoreVersionString   "ADCORE_VERSION"    /**< (asynOctet,    r/o) Version of ADCore */
#define NDDriverVersionString   "DRIVER_VERSION"    /**< (asynOctet,    r/o) Version of this driver or plugin */

/* These parameters were previously in ADDriver.h.
 * We moved them here so they can be used by other types of drivers
 * For consistency the #define and parameter names should begin with ND rather than AD but that would break
 * backwards compatibility. */
#define ADManufacturerString        "MANUFACTURER"          /**< (asynOctet,    r/o) Detector manufacturer name */
#define ADModelString               "MODEL"                 /**< (asynOctet,    r/o) Detector model name */
#define ADSerialNumberString        "SERIAL_NUMBER"         /**< (asynOctet,    r/o) Detector serial number */
#define ADSDKVersionString          "SDK_VERSION"           /**< (asynOctet,    r/o) Vendor SDK version */
#define ADFirmwareVersionString     "FIRMWARE_VERSION"      /**< (asynOctet,    r/o) Detector firmware version */
#define ADAcquireString             "ACQUIRE"               /**< (asynInt32,    r/w) Start(1) or Stop(0) acquisition */
#define ADAcquireBusyString         "ACQUIRE_BUSY"          /**< (asynInt32,    r/w) 0 when acquire done including plugins */
#define ADWaitForPluginsString      "WAIT_FOR_PLUGINS"      /**< (asynInt32,    r/w) Wait for plugins before AcquireBusy goes to 0 */

/* Parameters defining characteristics of the array data from the detector.
 * NDArraySizeX and NDArraySizeY are the actual dimensions of the array data,
 * including effects of the region definition and binning */
#define NDArraySizeXString      "ARRAY_SIZE_X"      /**< (asynInt32,    r/o) Size of the array data in the X direction */
#define NDArraySizeYString      "ARRAY_SIZE_Y"      /**< (asynInt32,    r/o) Size of the array data in the Y direction */
#define NDArraySizeZString      "ARRAY_SIZE_Z"      /**< (asynInt32,    r/o) Size of the array data in the Z direction */
#define NDArraySizeString       "ARRAY_SIZE"        /**< (asynInt32,    r/o) Total size of array data in bytes */
#define NDNDimensionsString     "ARRAY_NDIMENSIONS" /**< (asynInt32,    r/o) Number of dimensions in array */
#define NDDimensionsString      "ARRAY_DIMENSIONS"  /**< (asynInt32Array, r/o) Array dimensions */
#define NDDataTypeString        "DATA_TYPE"         /**< (asynInt32,    r/w) Data type (NDDataType_t) */
#define NDBitsPerPixelString    "BITS_PER_PIXEL"    /**< (asynInt32,    r/w) Number of bits per pixel */
#define NDColorModeString       "COLOR_MODE"        /**< (asynInt32,    r/w) Color mode (NDColorMode_t) */
#define NDUniqueIdString        "UNIQUE_ID"         /**< (asynInt32,    r/o) Unique ID number of array */
#define NDTimeStampString       "TIME_STAMP"        /**< (asynFloat64,  r/o) Time stamp of array */
#define NDEpicsTSSecString      "EPICS_TS_SEC"      /**< (asynInt32,    r/o) EPOCS time stamp secPastEpoch of array */
#define NDEpicsTSNsecString     "EPICS_TS_NSEC"     /**< (asynInt32,    r/o) EPOCS time stamp nsec of array */
#define NDBayerPatternString    "BAYER_PATTERN"     /**< (asynInt32,    r/o) Bayer pattern of array  (from bayerPattern array attribute if present) */
#define NDCodecString           "CODEC"             /**< (asynOctet,    r/o) Codec name */
#define NDCompressedSizeString  "COMPRESSED_SIZE"   /**< (asynInt32,    r/o) Compressed size in bytes */

/* Statistics on number of arrays collected */
#define NDArrayCounterString    "ARRAY_COUNTER"     /**< (asynInt32,    r/w) Number of arrays since last reset */

/* File name related parameters for saving data.
 * Drivers are not required to implement file saving, but if they do these parameters
 * should be used.
 * The driver will normally combine NDFilePath, NDFileName, and NDFileNumber into
 * a file name that order using the format specification in NDFileTemplate.
 * For example NDFileTemplate might be "%s%s_%d.tif" */
#define NDFilePathString        "FILE_PATH"         /**< (asynOctet,    r/w) The file path */
#define NDFilePathExistsString  "FILE_PATH_EXISTS"  /**< (asynInt32,    r/w) File path exists? */
#define NDFileNameString        "FILE_NAME"         /**< (asynOctet,    r/w) The file name */
#define NDFileNumberString      "FILE_NUMBER"       /**< (asynInt32,    r/w) The next file number */
#define NDFileTemplateString    "FILE_TEMPLATE"     /**< (asynOctet,    r/w) The file format template; C-style format string */
#define NDAutoIncrementString   "AUTO_INCREMENT"    /**< (asynInt32,    r/w) Autoincrement file number; 0=No, 1=Yes */
#define NDFullFileNameString    "FULL_FILE_NAME"    /**< (asynOctet,    r/o) The actual complete file name for the last file saved */
#define NDFileFormatString      "FILE_FORMAT"       /**< (asynInt32,    r/w) The data format to use for saving the file.  */
#define NDAutoSaveString        "AUTO_SAVE"         /**< (asynInt32,    r/w) Automatically save files */
#define NDWriteFileString       "WRITE_FILE"        /**< (asynInt32,    r/w) Manually save the most recent array to a file when value=1 */
#define NDReadFileString        "READ_FILE"         /**< (asynInt32,    r/w) Manually read file when value=1 */
#define NDFileWriteModeString   "WRITE_MODE"        /**< (asynInt32,    r/w) File saving mode (NDFileMode_t) */
#define NDFileWriteStatusString "WRITE_STATUS"      /**< (asynInt32,    r/w) File write status */
#define NDFileWriteMessageString "WRITE_MESSAGE"    /**< (asynOctet,    r/w) File write message */
#define NDFileNumCaptureString  "NUM_CAPTURE"       /**< (asynInt32,    r/w) Number of arrays to capture */
#define NDFileNumCapturedString "NUM_CAPTURED"      /**< (asynInt32,    r/o) Number of arrays already captured */
#define NDFileCaptureString     "CAPTURE"           /**< (asynInt32,    r/w) Start or stop capturing arrays */
#define NDFileDeleteDriverFileString  "DELETE_DRIVER_FILE"  /**< (asynInt32,    r/w) Delete driver file */
#define NDFileLazyOpenString    "FILE_LAZY_OPEN"    /**< (asynInt32,    r/w) Don't open file until first frame arrives in Stream mode */
#define NDFileCreateDirString   "CREATE_DIR"        /**< (asynInt32,    r/w) Create the target directory up to this depth */
#define NDFileTempSuffixString  "FILE_TEMP_SUFFIX"  /**< (asynOctet,    r/w) Temporary filename suffix while writing data to file. The file will be renamed (suffix removed) upon closing the file. */

#define NDAttributesFileString    "ND_ATTRIBUTES_FILE"   /**< (asynOctet,    r/w) Attributes file name */
#define NDAttributesStatusString  "ND_ATTRIBUTES_STATUS" /**< (asynInt32,    r/o) Attributes status */
#define NDAttributesMacrosString  "ND_ATTRIBUTES_MACROS" /**< (asynOctet,    r/w) Attributes macros string */

/* The detector array data */
#define NDArrayDataString       "ARRAY_DATA"        /**< (asynGenericPointer,   r/w) NDArray data */
#define NDArrayCallbacksString  "ARRAY_CALLBACKS"   /**< (asynInt32,    r/w) Do callbacks with array data (0=No, 1=Yes) */

/* NDArray Pool status and control */
#define NDPoolMaxBuffersString      "POOL_MAX_BUFFERS"
#define NDPoolAllocBuffersString    "POOL_ALLOC_BUFFERS"
#define NDPoolFreeBuffersString     "POOL_FREE_BUFFERS"
#define NDPoolMaxMemoryString       "POOL_MAX_MEMORY"
#define NDPoolUsedMemoryString      "POOL_USED_MEMORY"
#define NDPoolEmptyFreeListString   "POOL_EMPTY_FREELIST"

/* Queued arrays */
#define NDNumQueuedArraysString     "NUM_QUEUED_ARRAYS"

/** This is the class from which NDArray drivers are derived; implements the asynGenericPointer functions
  * for NDArray objects.
  * For areaDetector, both plugins and detector drivers are indirectly derived from this class.
  * asynNDArrayDriver inherits from asynPortDriver.
  */
class ADCORE_API asynNDArrayDriver : public asynPortDriver {
public:
    asynNDArrayDriver(const char *portName, int maxAddr, int maxBuffers, size_t maxMemory,
                      int interfaceMask, int interruptMask,
                      int asynFlags, int autoConnect, int priority, int stackSize);
    virtual ~asynNDArrayDriver();
    /* These are the methods that we override from asynPortDriver */
    virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t maxChars,
                          size_t *nActual);
    virtual asynStatus readGenericPointer(asynUser *pasynUser, void *genericPointer);
    virtual asynStatus writeGenericPointer(asynUser *pasynUser, void *genericPointer);
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus setIntegerParam(int index, int value);
    virtual asynStatus setIntegerParam(int list, int index, int value);
    virtual asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);
    virtual asynStatus readFloat64(asynUser *pasynUser, epicsFloat64 *value);
    virtual void report(FILE *fp, int details);

    /* These are the methods that are new to this class */
    virtual asynStatus createFilePath(const char *path, int pathDepth);
    virtual asynStatus checkPath();
    virtual bool checkPath(std::string &filePath);
    virtual asynStatus createFileName(int maxChars, char *fullFileName);
    virtual asynStatus createFileName(int maxChars, char *filePath, char *fileName);
    virtual asynStatus readNDAttributesFile();
    virtual asynStatus getAttributes(NDAttributeList *pAttributeList);

    asynStatus incrementQueuedArrayCount();
    asynStatus decrementQueuedArrayCount();
    int getQueuedArrayCount();
    void updateQueuedArrayCount();

    class NDArrayPool *pNDArrayPool;     /**< An NDArrayPool pointer that is initialized to pNDArrayPoolPvt_ in the constructor.
                                     * Plugins change this pointer to the one passed in NDArray::pNDArrayPool */

protected:
    int NDPortNameSelf;
    #define FIRST_NDARRAY_PARAM NDPortNameSelf
    int NDADCoreVersion;
    int NDDriverVersion;
    int ADManufacturer;
    int ADModel;
    int ADSerialNumber;
    int ADSDKVersion;
    int ADFirmwareVersion;
    int ADAcquire;
    int ADAcquireBusy;
    int ADWaitForPlugins;
    int NDArraySizeX;
    int NDArraySizeY;
    int NDArraySizeZ;
    int NDArraySize;
    int NDNDimensions;
    int NDDimensions;
    int NDDataType;
    int NDBitsPerPixel;
    int NDColorMode;
    int NDUniqueId;
    int NDTimeStamp;
    int NDEpicsTSSec;
    int NDEpicsTSNsec;
    int NDBayerPattern;
    int NDCodec;
    int NDCompressedSize;
    int NDArrayCounter;
    int NDFilePath;
    int NDFilePathExists;
    int NDFileName;
    int NDFileNumber;
    int NDFileTemplate;
    int NDAutoIncrement;
    int NDFullFileName;
    int NDFileFormat;
    int NDAutoSave;
    int NDWriteFile;
    int NDReadFile;
    int NDFileWriteMode;
    int NDFileWriteStatus;
    int NDFileWriteMessage;
    int NDFileNumCapture;
    int NDFileNumCaptured;
    int NDFileCapture;
    int NDFileDeleteDriverFile;
    int NDFileLazyOpen;
    int NDFileCreateDir;
    int NDFileTempSuffix;
    int NDAttributesFile;
    int NDAttributesStatus;
    int NDAttributesMacros;
    int NDArrayData;
    int NDArrayCallbacks;
    int NDPoolMaxBuffers;
    int NDPoolAllocBuffers;
    int NDPoolFreeBuffers;
    int NDPoolMaxMemory;
    int NDPoolUsedMemory;
    int NDPoolEmptyFreeList;
    int NDNumQueuedArrays;

    class NDArray **pArrays;             /**< An array of NDArray pointers used to store data in the driver */
    class NDAttributeList *pAttributeList;  /**< An NDAttributeList object used to obtain the current values of a set of attributes */
    int threadStackSize_;
    int threadPriority_;

private:
    NDArrayPool *pNDArrayPoolPvt_;
    epicsMutex *queuedArrayCountMutex_;
    epicsEventId queuedArrayEvent_;
    int queuedArrayCount_;

    bool queuedArrayUpdateRun_;
    epicsEventId queuedArrayUpdateDone_;

    friend class NDArrayPool;

};

#endif
