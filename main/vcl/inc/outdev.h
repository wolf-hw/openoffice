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



#ifndef _SV_OUTDEV_H
#define _SV_OUTDEV_H

#include <tools/solar.h>
#include <outfont.hxx>

#include <vector>
#include <list>
#include <set>

class Size;
class Font;
class VirtualDevice;
class ImplServerGraphics;
class ImplGetDevFontList;
class GetDevSizeList;

// -----------------------
// - ImplDevFontListData -
// -----------------------

// flags for mnMatchType member
#define IMPL_DEVFONT_SCALABLE       ((sal_uIntPtr)0x00000001)
#define IMPL_DEVFONT_SYMBOL         ((sal_uIntPtr)0x00000002)
#define IMPL_DEVFONT_NONESYMBOL     ((sal_uIntPtr)0x00000004)
#define IMPL_DEVFONT_LIGHT          ((sal_uIntPtr)0x00000010)
#define IMPL_DEVFONT_BOLD           ((sal_uIntPtr)0x00000020)
#define IMPL_DEVFONT_NORMAL         ((sal_uIntPtr)0x00000040)
#define IMPL_DEVFONT_NONEITALIC     ((sal_uIntPtr)0x00000100)
#define IMPL_DEVFONT_ITALIC         ((sal_uIntPtr)0x00000200)

// TODO: rename ImplDevFontListData to PhysicalFontFamily
class ImplDevFontListData
{
public:
                        ImplDevFontListData( const String& rSearchName );
                        ~ImplDevFontListData();

    const String&       GetFamilyName() const    { return maName; }
    const String&       GetSearchName() const    { return maSearchName; }
    const String&       GetAliasNames() const    { return maMapNames; }
    bool                IsScalable() const       { return mpFirst->IsScalable(); }
    int                 GetMinQuality() const    { return mnMinQuality; }

    bool                AddFontFace( ImplFontData* );
    void                InitMatchData( const utl::FontSubstConfiguration&,
                            const String& rSearchName );
    ImplFontData*       FindBestFontFace( const ImplFontSelectData& rFSD ) const;

    void                GetFontHeights( std::set<int>& rHeights ) const;
    void                UpdateDevFontList( ImplGetDevFontList& ) const;
    void                UpdateCloneFontList( ImplDevFontList&,
                            bool bScalable, bool bEmbeddable ) const;

private:
friend class ImplDevFontList; // TODO: remove soon
    ImplFontData*       mpFirst;            // linked list of physical font faces
    String              maName;             // Fontname (original font family name)
    String              maSearchName;       // normalized font family name
    String              maMapNames;         // fontname aliases
    sal_uIntPtr               mnTypeFaces;        // Typeface Flags
    sal_uIntPtr               mnMatchType;        // MATCH - Type
    String              maMatchFamilyName;  // MATCH - FamilyName
    FontWeight          meMatchWeight;      // MATCH - Weight
    FontWidth           meMatchWidth;       // MATCH - Width
    FontFamily          meFamily;
    FontPitch           mePitch;
    int                 mnMinQuality;       // quality of the worst font face
};


// ----------------------
// - ImplGetDevFontList -
// ----------------------

// an ImplGetDevFontList is created by an ImplDevFontList
// it becomes invalid when original ImplDevFontList is modified
class ImplGetDevFontList
{
private:
    std::vector<ImplFontData*> maDevFontVector;

public:
                        ImplGetDevFontList()        { maDevFontVector.reserve(1024); }
    void                Add( ImplFontData* pFace )  { maDevFontVector.push_back( pFace ); }
    ImplFontData*       Get( int nIndex ) const     { return maDevFontVector[ nIndex ]; }
    int                 Count() const               { return maDevFontVector.size(); }
};

// ----------------------
// - ImplGetDevSizeList -
// ----------------------

class ImplGetDevSizeList
{
private:
    String              maFontName;
    std::vector<int>    maSizeList;

public:
                        ImplGetDevSizeList( const String& rFontName )
                        : maFontName( rFontName ) { maSizeList.reserve( 32 ); }
    void                Add( int nHeight )      { maSizeList.push_back( nHeight ); }
    int                 Count() const           { return maSizeList.size(); }
    int                 Get( int nIndex ) const { return maSizeList[ nIndex ]; }
    const String&       GetFontName() const     { return maFontName; }
};

// ------------------------
// - ImplFontSubstitution -
// ------------------------
// nowadays these substitutions are needed for backward compatibility and tight platform integration:
// - substitutions from configuration entries (Tools->Options->FontReplacement and/or fontconfig)
// - device specific substitutions (e.g. for PS printer builtin fonts)
// - substitutions for missing fonts defined by configuration entries (generic and/or platform dependent fallbacks) 
// - substitutions for missing fonts defined by multi-token fontnames (e.g. fontname="SpecialFont;FallbackA;FallbackB")
// - substitutions for incomplete fonts (implicit, generic, EUDC and/or platform dependent fallbacks)
// - substitutions for missing symbol fonts by translating code points into other symbol fonts 

class ImplFontSubstitution
{
    // TODO: there is more commonality between the different substitutions
protected:
    virtual ~ImplFontSubstitution() {}
};

// ImplDirectFontSubstitution is for Tools->Options->FontReplacement and PsPrinter substitutions
// The clss is just a simple port of the unmaintainable manual-linked-list based mechanism
// TODO: get rid of this class when the Tools->Options->FontReplacement tabpage is gone for good

struct ImplFontSubstEntry
{
    String                  maName;
    String                  maReplaceName;
    String                  maSearchName;
    String                  maSearchReplaceName;
    sal_uInt16                  mnFlags;

    ImplFontSubstEntry(  const String& rFontName, const String& rSubstFontName, sal_uInt16 nSubstFlags );
};

class ImplDirectFontSubstitution
:   public ImplFontSubstitution
{
private:
    typedef std::list<ImplFontSubstEntry> FontSubstList;
    FontSubstList maFontSubstList;
public:
    void    AddFontSubstitute( const String& rFontName, const String& rSubstName, sal_uInt16 nFlags );
    void    RemoveFontSubstitute( int nIndex );
    bool    GetFontSubstitute( int nIndex, String& rFontName, String& rSubstName, sal_uInt16& rFlags ) const;
    int     GetFontSubstituteCount() const { return maFontSubstList.size(); };
    bool    Empty() const { return maFontSubstList.empty(); }
    void    Clear() { maFontSubstList.clear(); }

    bool    FindFontSubstitute( String& rSubstName, const String& rFontName, sal_uInt16 nFlags ) const;
};

// PreMatchFontSubstitution
// abstracts the concept of a configured font substitution
// before the availability of the originally selected font has been checked
class ImplPreMatchFontSubstitution
:   public ImplFontSubstitution
{
public:
    virtual bool FindFontSubstitute( ImplFontSelectData& ) const = 0;
};

// ImplGlyphFallbackFontSubstitution
// abstracts the concept of finding the best font to support an incomplete font
class ImplGlyphFallbackFontSubstitution
:   public ImplFontSubstitution
{
public:
    virtual bool FindFontSubstitute( ImplFontSelectData&, rtl::OUString& rMissingCodes ) const = 0;
};

// -----------------
// - ImplFontCache -
// -----------------
// TODO: closely couple with ImplDevFontList

class ImplFontCache
{
private:
    ImplFontEntry*      mpFirstEntry;
    int                 mnRef0Count;    // number of unreferenced ImplFontEntries
    bool                mbPrinter;

    // cache of recently used font instances
    struct IFSD_Equal { bool operator()( const ImplFontSelectData&, const ImplFontSelectData& ) const; };
    struct IFSD_Hash { size_t operator()( const ImplFontSelectData& ) const; };
    typedef ::std::hash_map<ImplFontSelectData,ImplFontEntry*,IFSD_Hash,IFSD_Equal > FontInstanceList;
    FontInstanceList    maFontInstanceList;

    // cache of recently requested font names vs. selected font names
    typedef ::std::hash_map<String,String,FontNameHash> FontNameList;
    FontNameList        maFontNameList;

public:
                        ImplFontCache( bool bPrinter );
                        ~ImplFontCache();

    ImplFontEntry*      GetFontEntry( ImplDevFontList*,
                             const Font&, const Size& rPixelSize, float fExactHeight,
				ImplDirectFontSubstitution* pDevSpecific );
    ImplFontEntry*      GetFontEntry( ImplDevFontList*,
		            ImplFontSelectData&, ImplDirectFontSubstitution* pDevSpecific );
    ImplFontEntry*      GetGlyphFallbackFont( ImplDevFontList*, ImplFontSelectData&,
                            int nFallbackLevel, rtl::OUString& rMissingCodes );
    void                Release( ImplFontEntry* );
    void                Invalidate();
};

// ------------------
// - ImplOutDevData -
// ------------------

namespace vcl { struct ControlLayoutData; }
// #i75163#
namespace basegfx { class B2DHomMatrix; }

struct ImplOutDevData
{
    VirtualDevice*				mpRotateDev;
    vcl::ControlLayoutData*		mpRecordLayout;
	Rectangle					maRecordRect;
    ImplDirectFontSubstitution		maDevFontSubst;

	// #i75163#
	basegfx::B2DHomMatrix*		mpViewTransform;
	basegfx::B2DHomMatrix*		mpInverseViewTransform;
};

void ImplFreeOutDevFontData();

#endif // _SV_OUTDEV_H
