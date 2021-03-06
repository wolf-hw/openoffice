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

#include "precompiled_sfx2.hxx"

#include "ContextMatcher.hxx"
#include "Context.hxx"

using ::rtl::OUString;

namespace sfx2 { namespace sidebar {

namespace {
    static const sal_Char* gsAny = "any";
}



ContextMatcher::ContextMatcher (void)
    : maEntries()
{
}




ContextMatcher::~ContextMatcher (void)
{
}




sal_Int32 ContextMatcher::EvaluateMatch (
    const Context& rContext) const
{
    sal_Int32 nBestMatch (Context::NoMatch);
    
    for (::std::vector<Entry>::const_iterator
             iEntry(maEntries.begin()),
             iEnd(maEntries.end());
         iEntry!=iEnd;
         ++iEntry)
    {
        const sal_Int32 nMatch (EvaluateMatch(rContext, *iEntry));
        if (nMatch < nBestMatch)
            nBestMatch = nMatch;
            if (nBestMatch == Context::OptimalMatch)
            break;
    }

    return nBestMatch;
}




sal_Int32 ContextMatcher::EvaluateMatch (
    const Context& rContext,
    const Entry& rEntry) const
{
    sal_Int32 nApplicationMatch (Context::NoMatch);
    if (rContext.msApplication.equals(rEntry.msApplicationName))
        nApplicationMatch = 0;
    else if (rEntry.msApplicationName.equalsAscii(gsAny))
        nApplicationMatch = Context::ApplicationWildcardMatch;
    else
        return Context::NoMatch;

    sal_Int32 nBestContextMatch (Context::NoMatch);
    for (::std::vector<OUString>::const_iterator
             iContext(rEntry.maContextNames.begin()),
             iEnd(rEntry.maContextNames.end());
         iContext!=iEnd;
         ++iContext)
    {
        sal_Int32 nContextMatch (Context::NoMatch);
        if (rContext.msContext.equals(*iContext))
            nContextMatch = 0;
        else if (iContext->equalsAscii(gsAny))
            nContextMatch = Context::ContextWildcardMatch;
        else
            continue;
        if (nContextMatch < nBestContextMatch)
            nBestContextMatch = nContextMatch;
    }

    if (rEntry.mbIsContextListNegated)
        nBestContextMatch = Context::NoMatch - nBestContextMatch;
        
    return nApplicationMatch + nBestContextMatch;
}




void ContextMatcher::AddMatcher (
    const ::rtl::OUString& rsApplicationName,
    const ::std::vector<rtl::OUString>& rContextNames,
    const bool bIsContextListNegated)
{
    maEntries.push_back(Entry());
    maEntries.back().msApplicationName = rsApplicationName;
    maEntries.back().maContextNames = rContextNames;
    maEntries.back().mbIsContextListNegated = bIsContextListNegated;
}




void ContextMatcher::AddMatcher (
    const ::rtl::OUString& rsApplicationName,
    const ::rtl::OUString& rsContextName)
{
    maEntries.push_back(Entry());
    maEntries.back().msApplicationName = rsApplicationName;
    maEntries.back().maContextNames.push_back(rsContextName);
    maEntries.back().mbIsContextListNegated = false;
}


} } // end of namespace sfx2::sidebar
