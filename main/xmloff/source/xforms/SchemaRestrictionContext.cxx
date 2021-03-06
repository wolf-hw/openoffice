/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#include "SchemaRestrictionContext.hxx"
#include "xformsapi.hxx"

#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlimp.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/xforms/XDataTypeRepository.hpp>
#include <com/sun/star/xsd/DataTypeClass.hpp>
#include <com/sun/star/xsd/WhiteSpaceTreatment.hpp>

#include <tools/debug.hxx>


using rtl::OUString;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Exception;
using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::util::Date;
using com::sun::star::util::DateTime;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::XPropertySetInfo;
using com::sun::star::xforms::XDataTypeRepository;
using namespace xmloff::token;




static SvXMLTokenMapEntry aAttributes[] =
{
    TOKEN_MAP_ENTRY( NONE, BASE ),
    XML_TOKEN_MAP_END
};

static SvXMLTokenMapEntry aChildren[] =
{
    TOKEN_MAP_ENTRY( XSD, LENGTH         ),
    TOKEN_MAP_ENTRY( XSD, MINLENGTH      ),
    TOKEN_MAP_ENTRY( XSD, MAXLENGTH      ),
    TOKEN_MAP_ENTRY( XSD, MININCLUSIVE   ),
    TOKEN_MAP_ENTRY( XSD, MINEXCLUSIVE   ),
    TOKEN_MAP_ENTRY( XSD, MAXINCLUSIVE   ),
    TOKEN_MAP_ENTRY( XSD, MAXEXCLUSIVE   ),
    TOKEN_MAP_ENTRY( XSD, PATTERN        ),
    // ??? XML_ENUMERATION
    TOKEN_MAP_ENTRY( XSD, WHITESPACE     ),
    TOKEN_MAP_ENTRY( XSD, TOTALDIGITS    ),
    TOKEN_MAP_ENTRY( XSD, FRACTIONDIGITS ),
    XML_TOKEN_MAP_END
};


SchemaRestrictionContext::SchemaRestrictionContext(
    SvXMLImport& rImport, 
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    Reference<com::sun::star::xforms::XDataTypeRepository>& rRepository,
    const OUString& sTypeName ) :
        TokenContext( rImport, nPrefix, rLocalName, aAttributes, aChildren ),
        mxRepository( rRepository ),
        msTypeName( sTypeName ),
        msBaseName()
{
    DBG_ASSERT( mxRepository.is(), "need repository" );
}

SchemaRestrictionContext::~SchemaRestrictionContext()
{
}

void SchemaRestrictionContext::CreateDataType()
{
    // only do something if we don't have a data type already
    if( mxDataType.is() )
        return;

    DBG_ASSERT( msBaseName.getLength() > 0, "no base name?" );
    DBG_ASSERT( mxRepository.is(), "no repository?" );

    try
    {
        mxDataType = 
            Reference<XPropertySet>( 
                mxRepository->cloneDataType( 
                    lcl_getBasicTypeName( mxRepository,
                                          GetImport().GetNamespaceMap(),
                                          msBaseName ),
                    msTypeName ),
                UNO_QUERY );
    }
    catch( const Exception& )
    {
        DBG_ERROR( "exception during type creation" );
    }
    DBG_ASSERT( mxDataType.is(), "can't create type" );
}

void SchemaRestrictionContext::HandleAttribute( 
    sal_uInt16 nToken, 
    const OUString& rValue )
{
    if( nToken == XML_BASE )
    {
        msBaseName = rValue;
    }
}

typedef Any (*convert_t)( const OUString& );

Any lcl_string( const OUString& rValue )
{
    return makeAny( rValue );
}

Any lcl_int32( const OUString& rValue )
{
    sal_Int32 nValue;
    bool bSuccess = SvXMLUnitConverter::convertNumber( nValue, rValue );
    return bSuccess ? makeAny( nValue ) : Any();
}

Any lcl_int16( const OUString& rValue )
{
    sal_Int32 nValue;
    bool bSuccess = SvXMLUnitConverter::convertNumber( nValue, rValue );
    return bSuccess ? makeAny( static_cast<sal_Int16>( nValue ) ) : Any();
}

Any lcl_whitespace( const OUString& rValue )
{
    Any aValue;
    if( IsXMLToken( rValue, XML_PRESERVE ) )
        aValue <<= com::sun::star::xsd::WhiteSpaceTreatment::Preserve;
    else if( IsXMLToken( rValue, XML_REPLACE ) )
        aValue <<= com::sun::star::xsd::WhiteSpaceTreatment::Replace;
    else if( IsXMLToken( rValue, XML_COLLAPSE ) )
        aValue <<= com::sun::star::xsd::WhiteSpaceTreatment::Collapse;
    return aValue;
}

Any lcl_double( const OUString& rValue )
{
    double fValue;
    bool bSuccess = SvXMLUnitConverter::convertDouble( fValue, rValue );
    return bSuccess ? makeAny( fValue ) : Any();
}

Any lcl_date( const OUString& rValue )
{
    Any aAny;

    // parse ISO date
    sal_Int32 nPos1 = rValue.indexOf( sal_Unicode('-') );
    sal_Int32 nPos2 = rValue.indexOf( sal_Unicode('-'), nPos1 + 1 );
    if( nPos1 > 0  &&  nPos2 > 0 )
    {
        Date aDate;
        aDate.Year = static_cast<sal_uInt16>( 
                     rValue.copy( 0, nPos1 ).toInt32() );
        aDate.Month = static_cast<sal_uInt16>( 
                      rValue.copy( nPos1 + 1, nPos2 - nPos1 - 1 ).toInt32() );
        aDate.Day   = static_cast<sal_uInt16>( 
                      rValue.copy( nPos2 + 1 ).toInt32() );
        aAny <<= aDate;
    }
    return aAny;
}

Any lcl_dateTime( const OUString& rValue )
{
    DateTime aDateTime;
    bool bSuccess = SvXMLUnitConverter::convertDateTime( aDateTime, rValue );
    return bSuccess ? makeAny( aDateTime ) : Any();
}

Any lcl_time( const OUString& rValue )
{
    Any aAny;
    DateTime aDateTime;
    if( SvXMLUnitConverter::convertTime( aDateTime, rValue ) )
    {
        com::sun::star::util::Time aTime;
        aTime.Hours = aDateTime.Hours;
        aTime.Minutes = aDateTime.Minutes;
        aTime.Seconds = aDateTime.Seconds;
        aTime.HundredthSeconds = aDateTime.HundredthSeconds;
        aAny <<= aTime;
    }
    return aAny;
}


SvXMLImportContext* SchemaRestrictionContext::HandleChild( 
    sal_uInt16 nToken,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList>& xAttrList )
{
    // find value
    OUString sValue;
    sal_Int16 nLength = xAttrList->getLength();
    for( sal_Int16 n = 0; n < nLength; n++ )
    {
        if( IsXMLToken( xAttrList->getNameByIndex( n ), XML_VALUE ) )
            sValue = xAttrList->getValueByIndex( n );
    }

    // determine property name + suitable converter
    OUString sPropertyName;
    convert_t pConvert = NULL;
    switch( nToken )
    {
    case XML_LENGTH:
        sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("Length"));
        pConvert = &lcl_int32;
        break;
    case XML_MINLENGTH:
        sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("MinLength"));
        pConvert = &lcl_int32;
        break;
    case XML_MAXLENGTH:
        sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("MaxLength"));
        pConvert = &lcl_int32;
        break;
    case XML_TOTALDIGITS:
        sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("TotalDigits"));
        pConvert = &lcl_int32;
        break;
    case XML_FRACTIONDIGITS:
        sPropertyName =OUString(RTL_CONSTASCII_USTRINGPARAM("FractionDigits"));
        pConvert = &lcl_int32;
        break;
    case XML_PATTERN:
        sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("Pattern"));
        pConvert = &lcl_string;
        break;
    case XML_WHITESPACE:
        sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("WhiteSpace"));
        pConvert = &lcl_whitespace;
        break;
    case XML_MININCLUSIVE:
    case XML_MINEXCLUSIVE:
    case XML_MAXINCLUSIVE:
    case XML_MAXEXCLUSIVE:
        {
            // these attributes are mapped to different properties.
            // To determine the property name, we use an attribute
            // dependent prefix and a type dependent suffix. The
            // converter is only type dependent.

            // first, attribute-dependent prefix
            switch( nToken )
            {
            case XML_MININCLUSIVE:
                sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("MinInclusive"));
                break;
            case XML_MINEXCLUSIVE:
                sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("MinExclusive"));
                break;
            case XML_MAXINCLUSIVE:
                sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("MaxInclusive"));
                break;
            case XML_MAXEXCLUSIVE:
                sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("MaxExclusive"));
                break;
            }

            // second, type-dependent suffix + converter
            switch( lcl_getTypeClass( mxRepository,
                                      GetImport().GetNamespaceMap(),
                                      msBaseName ) )
            {
            case com::sun::star::xsd::DataTypeClass::DECIMAL:
            case com::sun::star::xsd::DataTypeClass::DOUBLE:
            case com::sun::star::xsd::DataTypeClass::FLOAT:
                sPropertyName += OUString(RTL_CONSTASCII_USTRINGPARAM("Double"));
                pConvert = &lcl_double;
                break;
            case com::sun::star::xsd::DataTypeClass::DATETIME:
                sPropertyName += OUString(RTL_CONSTASCII_USTRINGPARAM("DateTime"));
                pConvert = &lcl_dateTime;
                break;
            case com::sun::star::xsd::DataTypeClass::DATE:
                sPropertyName += OUString(RTL_CONSTASCII_USTRINGPARAM("Date"));
                pConvert = &lcl_date;
                break;
            case com::sun::star::xsd::DataTypeClass::TIME:
                sPropertyName += OUString(RTL_CONSTASCII_USTRINGPARAM("Time"));
                pConvert = &lcl_time;
                break;
            case com::sun::star::xsd::DataTypeClass::gYear:
            case com::sun::star::xsd::DataTypeClass::gDay:
            case com::sun::star::xsd::DataTypeClass::gMonth:
                sPropertyName += OUString(RTL_CONSTASCII_USTRINGPARAM("Int"));
                pConvert = &lcl_int16;
                break;

            case com::sun::star::xsd::DataTypeClass::STRING:
            case com::sun::star::xsd::DataTypeClass::anyURI:
            case com::sun::star::xsd::DataTypeClass::BOOLEAN:
                // invalid: These shouldn't have min/max-inclusive
                break;
 
                /* data types not yet supported:
                   case com::sun::star::xsd::DataTypeClass::DURATION:
                   case com::sun::star::xsd::DataTypeClass::gYearMonth:
                   case com::sun::star::xsd::DataTypeClass::gMonthDay:
                   case com::sun::star::xsd::DataTypeClass::hexBinary:
                   case com::sun::star::xsd::DataTypeClass::base64Binary:
                   case com::sun::star::xsd::DataTypeClass::QName:
                   case com::sun::star::xsd::DataTypeClass::NOTATION:
                */
            }
        }
        break;

    default:
        DBG_ERROR( "unknown facet" );
    }

    // finally, set the property
    CreateDataType();
    if( mxDataType.is()
        && sPropertyName.getLength() > 0
        && pConvert != NULL
        && mxDataType->getPropertySetInfo()->hasPropertyByName(sPropertyName) )
    {
        try
        {
            mxDataType->setPropertyValue( sPropertyName, pConvert( sValue ) );
        }
        catch( const Exception& )
        {
            ; // can't set property? Then ignore.
        }
    }

    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}
