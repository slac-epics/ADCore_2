/** NDAttribute.h
 *
 * Mark Rivers
 * University of Chicago
 * October 18, 2013
 *
 */

#ifndef NDAttribute_H
#define NDAttribute_H

#include <string>
#include <stdio.h>

#include <ellLib.h>
#include <epicsTypes.h>

/* asynDriver.h is needed only to define epicsInt64 on 3.14 */
#include <asynDriver.h>
#include <ADCoreAPI.h>

/** Success return code  */
#define ND_SUCCESS 0
/** Failure return code  */
#define ND_ERROR -1


/** Enumeration of NDArray data types */
typedef enum
{
    NDInt8,     /**< Signed 8-bit integer */
    NDUInt8,    /**< Unsigned 8-bit integer */
    NDInt16,    /**< Signed 16-bit integer */
    NDUInt16,   /**< Unsigned 16-bit integer */
    NDInt32,    /**< Signed 32-bit integer */
    NDUInt32,   /**< Unsigned 32-bit integer */
    NDInt64,    /**< Signed 64-bit integer */
    NDUInt64,   /**< Unsigned 64-bit integer */
    NDFloat32,  /**< 32-bit float */
    NDFloat64   /**< 64-bit float */
} NDDataType_t;

extern int GetNDDataTypeBits( NDDataType_t );

/** Enumeration of NDAttribute attribute data types */
typedef enum
{
    NDAttrInt8    = NDInt8,     /**< Signed 8-bit integer */
    NDAttrUInt8   = NDUInt8,    /**< Unsigned 8-bit integer */
    NDAttrInt16   = NDInt16,    /**< Signed 16-bit integer */
    NDAttrUInt16  = NDUInt16,   /**< Unsigned 16-bit integer */
    NDAttrInt32   = NDInt32,    /**< Signed 32-bit integer */
    NDAttrUInt32  = NDUInt32,   /**< Unsigned 32-bit integer */
    NDAttrInt64   = NDInt64,    /**< Signed 64-bit integer */
    NDAttrUInt64  = NDUInt64,   /**< Unsigned 64-bit integer */
    NDAttrFloat32 = NDFloat32,  /**< 32-bit float */
    NDAttrFloat64 = NDFloat64,  /**< 64-bit float */
    NDAttrString,               /**< Dynamic length string */
    NDAttrUndefined             /**< Undefined data type */
} NDAttrDataType_t;

/** Enumeration of NDAttibute source types */
typedef enum
{
    NDAttrSourceDriver,    /**< Attribute is obtained directly from driver */
    NDAttrSourceParam,     /**< Attribute is obtained from parameter library */
    NDAttrSourceEPICSPV,   /**< Attribute is obtained from an EPICS PV */
    NDAttrSourceFunct,     /**< Attribute is obtained from a user-specified function */
    NDAttrSourceUndefined  /**< Attribute source is undefined */
} NDAttrSource_t;

/** Union defining the values in an NDAttribute object */
typedef union {
    epicsInt8    i8;    /**< Signed 8-bit integer */
    epicsUInt8   ui8;   /**< Unsigned 8-bit integer */
    epicsInt16   i16;   /**< Signed 16-bit integer */
    epicsUInt16  ui16;  /**< Unsigned 16-bit integer */
    epicsInt32   i32;   /**< Signed 32-bit integer */
    epicsUInt32  ui32;  /**< Unsigned 32-bit integer */
    epicsInt64   i64;   /**< Signed 64-bit integer */
    epicsUInt64  ui64;  /**< Unsigned 64-bit integer */
    epicsFloat32 f32;   /**< 32-bit float */
    epicsFloat64 f64;   /**< 64-bit float */
} NDAttrValue;

/** Structure used by the EPICS ellLib library for linked lists of C++ objects.
  * This is needed for ellLists of C++ objects, for which making the first data element the ELLNODE
  * does not work if the class has virtual functions or derived classes. */
typedef struct NDAttributeListNode {
    ELLNODE node;
    class NDAttribute *pNDAttribute;
} NDAttributeListNode;

/** NDAttribute class; an attribute has a name, description, source type, source string,
  * data type, and value.
  */
class ADCORE_API NDAttribute {
public:
    /* Methods */
    NDAttribute(const char *pName, const char *pDescription,
                NDAttrSource_t sourceType, const char *pSource, NDAttrDataType_t dataType, void *pValue);
    NDAttribute(NDAttribute& attribute);
    static const char *attrSourceString(NDAttrSource_t type);
    virtual ~NDAttribute();
    virtual NDAttribute* copy(NDAttribute *pAttribute);
    virtual const char *getName();
    virtual const char *getDescription();
    virtual const char *getSource();
    virtual const char *getSourceInfo(NDAttrSource_t *pSourceType);
    virtual NDAttrDataType_t getDataType();
    virtual int getValueInfo(NDAttrDataType_t *pDataType, size_t *pDataSize);
    virtual int getValue(NDAttrDataType_t dataType, void *pValue, size_t dataSize=0);
    virtual int getValue(std::string& value);
    virtual int setDataType(NDAttrDataType_t dataType);
    virtual int setValue(const void *pValue);
    virtual int setValue(const std::string&);
    virtual int updateValue();
    virtual int report(FILE *fp, int details);
    friend class NDArray;
    friend class NDAttributeList;


private:
    template <typename epicsType> int getValueT(void *pValue, size_t dataSize);
    std::string name_;              /**< Name string */
    std::string description_;       /**< Description string */
    NDAttrDataType_t dataType_;     /**< Data type of attribute */
    NDAttrValue value_;             /**< Value of attribute except for strings */
    std::string string_;            /**< Value of attribute for strings */
    std::string source_;            /**< Source string - EPICS PV name or DRV_INFO string */
    NDAttrSource_t sourceType_;     /**< Source type */
    std::string sourceTypeString_;  /**< Source type string */
    NDAttributeListNode listNode_;  /**< Used for NDAttributeList */
};

#endif
