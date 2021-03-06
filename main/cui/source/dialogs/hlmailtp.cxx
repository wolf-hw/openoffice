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
#include "precompiled_cui.hxx"

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <sfx2/request.hxx>

#include <comphelper/processfactory.hxx>
#include <sfx2/viewfrm.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/moduleoptions.hxx>

#include "hlmailtp.hxx"
#include "hyperdlg.hrc"

using namespace ::rtl;
using namespace ::com::sun::star;

/*************************************************************************
|*
|* Constructor / Destructor
|*
|************************************************************************/

SvxHyperlinkMailTp::SvxHyperlinkMailTp ( Window *pParent, const SfxItemSet& rItemSet)
:   SvxHyperlinkTabPageBase ( pParent, CUI_RES( RID_SVXPAGE_HYPERLINK_MAIL ),
                              rItemSet ),
    maGrpMailNews   ( this, CUI_RES (GRP_MAILNEWS) ),
	maRbtMail		( this, CUI_RES (RB_LINKTYP_MAIL) ),
	maRbtNews		( this, CUI_RES (RB_LINKTYP_NEWS) ),
	maFtReceiver	( this, CUI_RES (FT_RECEIVER) ),
	maCbbReceiver	( this, INET_PROT_MAILTO ),
    maBtAdrBook     ( this, CUI_RES (BTN_ADRESSBOOK) ),
    maFtSubject     ( this, CUI_RES (FT_SUBJECT) ),
    maEdSubject     ( this, CUI_RES (ED_SUBJECT) )
{
	// Set HC bitmaps and disable display of bitmap names.
	maBtAdrBook.SetModeImage( Image( CUI_RES( IMG_ADRESSBOOK_HC ) ), BMP_COLOR_HIGHCONTRAST );
    maBtAdrBook.EnableTextDisplay (sal_False);

	InitStdControls();
	FreeResource();

	// Init URL-Box (pos&size, Open-Handler)
	maCbbReceiver.SetPosSizePixel ( LogicToPixel( Point( COL_2, 25 ), MAP_APPFONT ),
		                            LogicToPixel( Size ( 176 - COL_DIFF, 60), MAP_APPFONT ) );

	maCbbReceiver.Show();
	maCbbReceiver.SetHelpId( HID_HYPERDLG_MAIL_PATH );

	SetExchangeSupport ();

	// set defaults
	maRbtMail.Check ();

	// overload handlers
	maRbtMail.SetClickHdl		 ( LINK ( this, SvxHyperlinkMailTp, Click_SmartProtocol_Impl ) );
	maRbtNews.SetClickHdl		 ( LINK ( this, SvxHyperlinkMailTp, Click_SmartProtocol_Impl ) );
	maBtAdrBook.SetClickHdl		 ( LINK ( this, SvxHyperlinkMailTp, ClickAdrBookHdl_Impl ) );
	maCbbReceiver.SetModifyHdl	 ( LINK ( this, SvxHyperlinkMailTp, ModifiedReceiverHdl_Impl) );

    if ( !SvtModuleOptions().IsModuleInstalled( SvtModuleOptions::E_SDATABASE ) )
        maBtAdrBook.Hide();

	maBtAdrBook.SetAccessibleRelationMemberOf( &maGrpMailNews );
	maBtAdrBook.SetAccessibleRelationLabeledBy( &maFtReceiver );
}

SvxHyperlinkMailTp::~SvxHyperlinkMailTp ()
{
}

/*************************************************************************
|*
|* Fill the all dialog-controls except controls in groupbox "more..."
|*
|************************************************************************/

void SvxHyperlinkMailTp::FillDlgFields ( String& aStrURL )
{
    const sal_Char sMailtoScheme[] = INET_MAILTO_SCHEME;

    INetURLObject aURL( aStrURL );
	String aStrScheme = GetSchemeFromURL( aStrURL );

    // set URL-field and additional controls
	String aStrURLc ( aStrURL );
    // set additional controls for EMail:
	if ( aStrScheme.SearchAscii( sMailtoScheme ) == 0 )
	{
		// Find mail-subject
		String aStrSubject, aStrTmp ( aStrURLc );

		const sal_Char sSubject[] = "subject";
		xub_StrLen nPos = aStrTmp.ToLowerAscii().SearchAscii( sSubject, 0 );
		nPos = aStrTmp.Search( sal_Unicode( '=' ), nPos );

		if ( nPos != STRING_NOTFOUND )
			aStrSubject = aStrURLc.Copy( nPos+1, aStrURLc.Len() );

		nPos = aStrURLc.Search ( sal_Unicode( '?' ), 0);

		aStrURLc = aStrURLc.Copy( 0, ( nPos == STRING_NOTFOUND ?
				                           aStrURLc.Len() : nPos ) );

		maEdSubject.SetText ( aStrSubject );
	}
	else
	{
		maEdSubject.SetText (aEmptyStr);
	}

	maCbbReceiver.SetText ( aStrURLc );

    SetScheme( aStrScheme );
}

/*************************************************************************
|*
|* retrieve and prepare data from dialog-fields
|*
|************************************************************************/

void SvxHyperlinkMailTp::GetCurentItemData ( String& aStrURL, String& aStrName,
											 String& aStrIntName, String& aStrFrame,
											 SvxLinkInsertMode& eMode )
{
    aStrURL = CreateAbsoluteURL();
	GetDataFromCommonFields( aStrName, aStrIntName, aStrFrame, eMode );
}

String SvxHyperlinkMailTp::CreateAbsoluteURL() const
{
    String aStrURL = maCbbReceiver.GetText();
    INetURLObject aURL(aStrURL);

	if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
    {
        aURL.SetSmartProtocol( GetSmartProtocolFromButtons() );
        aURL.SetSmartURL(aStrURL);
    }

	// subject for EMail-url
	if( aURL.GetProtocol() == INET_PROT_MAILTO )
	{
		if ( maEdSubject.GetText() != aEmptyStr )
		{
            String aQuery = UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "subject=" ) );
            aQuery.Append( maEdSubject.GetText() );
            aURL.SetParam(aQuery);
		}
	}

	if ( aURL.GetProtocol() != INET_PROT_NOT_VALID )
        return aURL.GetMainURL( INetURLObject::DECODE_WITH_CHARSET );
    else //#105788# always create a URL even if it is not valid
        return aStrURL;
}

/*************************************************************************
|*
|* static method to create Tabpage
|*
|************************************************************************/

IconChoicePage* SvxHyperlinkMailTp::Create( Window* pWindow, const SfxItemSet& rItemSet )
{
	return( new SvxHyperlinkMailTp( pWindow, rItemSet ) );
}

/*************************************************************************
|*
|* Set initial focus
|*
|************************************************************************/

void SvxHyperlinkMailTp::SetInitFocus()
{
	maCbbReceiver.GrabFocus();
}

/*************************************************************************
|************************************************************************/

void SvxHyperlinkMailTp::SetScheme( const String& aScheme )
{
    //if  aScheme is empty or unknown the default beaviour is like it where MAIL
    const sal_Char sNewsScheme[]   = INET_NEWS_SCHEME;

    sal_Bool bMail = aScheme.SearchAscii( sNewsScheme ) != 0;

    //update protocol button selection:
    maRbtMail.Check(bMail);
	maRbtNews.Check(!bMail);

    //update target:
    RemoveImproperProtocol(aScheme);
    maCbbReceiver.SetSmartProtocol( GetSmartProtocolFromButtons() );

    //show/hide  special fields for MAIL:
    maFtSubject.Enable(bMail);
	maEdSubject.Enable(bMail);
}

/*************************************************************************
|*
|* Remove protocol if it does not fit to the current button selection
|*
|************************************************************************/

void SvxHyperlinkMailTp::RemoveImproperProtocol(const String& aProperScheme)
{
    String aStrURL ( maCbbReceiver.GetText() );
	if ( aStrURL != aEmptyStr )
	{
		String aStrScheme = GetSchemeFromURL( aStrURL );
		if ( aStrScheme != aEmptyStr && aStrScheme != aProperScheme )
		{
			aStrURL.Erase ( 0, aStrScheme.Len() );
            maCbbReceiver.SetText ( aStrURL );
		}
	}
}

String SvxHyperlinkMailTp::GetSchemeFromButtons() const
{
    if( maRbtNews.IsChecked() )
	{
        return String::CreateFromAscii( INET_NEWS_SCHEME );
	}
	return String::CreateFromAscii( INET_MAILTO_SCHEME );
}

INetProtocol SvxHyperlinkMailTp::GetSmartProtocolFromButtons() const
{
	if( maRbtNews.IsChecked() )
	{
		return INET_PROT_NEWS;
	}
	return INET_PROT_MAILTO;
}

/*************************************************************************
|*
|* Click on radiobutton : Type EMail
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkMailTp, Click_SmartProtocol_Impl, void *, EMPTYARG )
{
	String aScheme = GetSchemeFromButtons();
    SetScheme( aScheme );
	return( 0L );
}

/*************************************************************************
|*
|* Contens of editfield "receiver" modified
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkMailTp, ModifiedReceiverHdl_Impl, void *, EMPTYARG )
{
	String aScheme = GetSchemeFromURL( maCbbReceiver.GetText() );
    if(aScheme.Len()!=0)
        SetScheme( aScheme );

	return( 0L );
}

/*************************************************************************
|*
|* Click on imagebutton : addressbook
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkMailTp, ClickAdrBookHdl_Impl, void *, EMPTYARG )
{
	SfxViewFrame* pViewFrame = SfxViewFrame::Current();
	if( pViewFrame )
	{
		SfxItemPool &rPool = pViewFrame->GetPool();
		SfxRequest aReq(SID_VIEW_DATA_SOURCE_BROWSER, 0, rPool);
		pViewFrame->ExecuteSlot( aReq, sal_True );
	}


/*	uno::Reference< frame::XDispatchProvider > xProv( pViewFrame->GetFrame().GetFrameInterface(), uno::UNO_QUERY );
	if ( xProv.is() )
	{
!!! (pb) we need a new config item here
		SfxAppIniManagerProperty aProp;
		GetpApp()->Property( aProp );
		if( !aProp.GetIniManager() )
			return ( 0L );

		String aAddressBook = aProp.GetIniManager()->Get( SFX_KEY_ADDRESSBOOK );
		INetURLObject aObj;
		aObj.SetSmartProtocol( INET_PROT_FILE );
		aObj.SetURL( aAddressBook.GetToken( 0, sal_Unicode( ';' ) ) );

		String aMark( RTL_CONSTASCII_USTRINGPARAM( "db:Table;" ) );
		aMark += aAddressBook.GetToken( 1, sal_Unicode( ';' ) );
		aObj.SetMark( aMark );

		util::URL aURL;
		aURL.Complete = ::rtl::OUString( aObj.GetMainURL( INetURLObject::NO_DECODE ) );

		uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
		if( xFactory.is() )
		{
			uno::Reference< util::XURLTransformer > xTrans( xFactory->createInstance
				( OUString::createFromAscii( "com.sun.star.util.URLTransformer" ) ),
								  uno::UNO_QUERY);
			xTrans->parseStrict( aURL );

			uno::Reference< frame::XDispatch > aDisp = xProv->queryDispatch( aURL,
				                                        OUString::createFromAscii( "_beamer" ),
														frame::FrameSearchFlag::GLOBAL |
														frame::FrameSearchFlag::CREATE );
			if ( aDisp.is() )
			{
				uno::Sequence< beans::PropertyValue > aArgs(1);
				beans::PropertyValue* pArg = aArgs.getArray();
				pArg[0].Name = DEFINE_CONST_UNICODE("Referer");
				pArg[0].Value = uno::makeAny( OUString( DEFINE_CONST_UNICODE("private:user") ) );
				aDisp->dispatch( aURL, aArgs );
			}
		}
	}
*/

	return( 0L );
}


