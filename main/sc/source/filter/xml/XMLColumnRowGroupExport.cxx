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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------
#include "XMLColumnRowGroupExport.hxx"
#include "xmlexprt.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>

#include <algorithm>

using namespace xmloff::token;

ScMyColumnRowGroup::ScMyColumnRowGroup()
{
}

sal_Bool ScMyColumnRowGroup::operator<(const ScMyColumnRowGroup& rGroup) const
{
	if (rGroup.nField > nField)
		return sal_True;
	else
		if (rGroup.nField == nField && rGroup.nLevel > nLevel)
			return sal_True;
		else
			return sal_False;
}

ScMyOpenCloseColumnRowGroup::ScMyOpenCloseColumnRowGroup(ScXMLExport& rTempExport, sal_uInt32 nToken)
	: rExport(rTempExport),
	rName(rExport.GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, GetXMLToken(XMLTokenEnum(nToken)))),
	aTableStart(),
	aTableEnd()
{
}

ScMyOpenCloseColumnRowGroup::~ScMyOpenCloseColumnRowGroup()
{
}

void ScMyOpenCloseColumnRowGroup::NewTable()
{
	aTableStart.clear();
	aTableEnd.clear();
}

void ScMyOpenCloseColumnRowGroup::AddGroup(const ScMyColumnRowGroup& aGroup, const sal_Int32 nEndField)
{
	aTableStart.push_back(aGroup);
	aTableEnd.push_back(nEndField);
}

sal_Bool ScMyOpenCloseColumnRowGroup::IsGroupStart(const sal_Int32 nField)
{
	sal_Bool bGroupStart(sal_False);
	if (!aTableStart.empty())
	{
		ScMyColumnRowGroupVec::iterator aItr(aTableStart.begin());
		sal_Int32 nItrField = aItr->nField;
		if ( nItrField < nField )
		{
			//	#103327# when used to find repeated rows at the beginning of a group,
			//	aTableStart may contain entries before nField. They must be skipped here
			//	(they will be used for OpenGroups later in the right order).

			ScMyColumnRowGroupVec::iterator aEnd(aTableStart.end());
			while ( aItr != aEnd && nItrField < nField )
			{
				++aItr;
				if ( aItr != aEnd )
					nItrField = aItr->nField;
			}
		}

		if (nItrField == nField)
			bGroupStart = sal_True;
	}
	return bGroupStart;
}

void ScMyOpenCloseColumnRowGroup::OpenGroup(const ScMyColumnRowGroup& rGroup)
{
	if (!rGroup.bDisplay)
		rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY, XML_FALSE);
	rExport.StartElement( rName, sal_True);
}

void ScMyOpenCloseColumnRowGroup::OpenGroups(const sal_Int32 nField)
{
	ScMyColumnRowGroupVec::iterator aItr(aTableStart.begin());
	ScMyColumnRowGroupVec::iterator aEndItr(aTableStart.end());
	sal_Bool bReady(sal_False);
	while(!bReady && aItr != aEndItr)
	{
		if (aItr->nField == nField)
		{
			OpenGroup(*aItr);
			aItr = aTableStart.erase(aItr);
		}
		else
			bReady = sal_True;
	}
}

sal_Bool ScMyOpenCloseColumnRowGroup::IsGroupEnd(const sal_Int32 nField)
{
	sal_Bool bGroupEnd(sal_False);
	if (!aTableEnd.empty())
	{
		if (*(aTableEnd.begin()) == nField)
			bGroupEnd = sal_True;
	}
	return bGroupEnd;
}

void ScMyOpenCloseColumnRowGroup::CloseGroup()
{
	rExport.EndElement( rName, sal_True );
}

void ScMyOpenCloseColumnRowGroup::CloseGroups(const sal_Int32 nField)
{
	ScMyFieldGroupVec::iterator aItr(aTableEnd.begin());
	ScMyFieldGroupVec::iterator aEndItr(aTableEnd.end());
	sal_Bool bReady(sal_False);
	while(!bReady && aItr != aEndItr)
	{
		if (*aItr == nField)
		{
			CloseGroup();
			aItr = aTableEnd.erase(aItr);
		}
		else
			bReady = sal_True;
	}
}

sal_Int32 ScMyOpenCloseColumnRowGroup::GetLast()
{
	sal_Int32 maximum(-1);
    ScMyFieldGroupVec::iterator i(aTableEnd.begin());
    ScMyFieldGroupVec::iterator endi(aTableEnd.end());
	while (i != endi)
    {
		if (*i > maximum)
			maximum = *i;
        ++i;
    }
	return maximum;
}

void ScMyOpenCloseColumnRowGroup::Sort()
{
	aTableStart.sort();
	aTableEnd.sort();
}

