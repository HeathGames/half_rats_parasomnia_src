// ====================================
// Paranoia subtitle system interface
//		written by BUzer
// ====================================

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_types.h"
#include "cdll_int.h"
#include "vgui_TeamFortressViewport.h"
#include "paranoia_vgui_subtitles.h"
#include "VGUI_TextImage.h"

cvar_t *gcvSubtitlesEnabled;
cvar_t *gcvSubtitlesFadeSpeed;
cvar_t *gcvSubtitlesScrollSpeed;
cvar_t *gcvSubtitlesTimeMin;
cvar_t *gcvSubtitlesTimeMax;

Font* FontFromMessage( const char* &cText )
{
	char cFontName[64] = "Default Text";
	if ( cText != NULL && cText[0] != 0 )
	{
		if ( cText[0] == '@' )
		{
			cText++;
			cText = gEngfuncs.COM_ParseFile( (char *)cText, cFontName );
			cText += 2;
		}
	}
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hTextScheme = pSchemes->getSchemeHandle( cFontName );
	return pSchemes->getFont( hTextScheme );
}

void SubtitlesMessageAdd( client_textmessage_t *pTempMessage )
{
	if ( !gcvSubtitlesEnabled->value )
		return;

	if ( gViewPort && gViewPort->m_pSubtitles )
		gViewPort->m_pSubtitles->AddMessage( pTempMessage );
	else
		gEngfuncs.Con_Printf( "[HR:P] SUBTITLES ERROR: CSubtitles or ViewPort is NULL!\n" );
}

void SubtitlesInit( void )
{
	gcvSubtitlesEnabled = gEngfuncs.pfnRegisterVariable( "cl_subtitles", "1", 0 );
	gcvSubtitlesFadeSpeed = gEngfuncs.pfnRegisterVariable( "cl_subtitles_fade_speed", "0.2", 0 );
	gcvSubtitlesScrollSpeed = gEngfuncs.pfnRegisterVariable( "cl_subtitles_scroll_speed", "250", 0 );
	gcvSubtitlesTimeMin = gEngfuncs.pfnRegisterVariable( "cl_subtitles_min_time", "0", 0 );
	gcvSubtitlesTimeMax = gEngfuncs.pfnRegisterVariable( "cl_subtitles_max_time", "0", 0 );
}

void CSubtitleTextPanel::paintBackground( void )
{
	float flFade = gcvSubtitlesFadeSpeed->value;
	float flTime = gEngfuncs.GetClientTime();

	int iRed, iGreen, iBlue, iAlpha;
	getFgColor( iRed, iGreen, iBlue ,iAlpha );
	if ( m_flBirthTime && (m_flBirthTime + flFade > flTime) )
	{
		float flAlpha = (flTime - m_flBirthTime) / flFade;
		setFgColor( iRed, iGreen, iBlue, 255 - (flAlpha * 255) );
		return;
	}

	if ( gViewPort && gViewPort->m_pSubtitles )
	{
		if ( gViewPort->m_pSubtitles->m_pCur == this )
		{
			float flDIeTime = gViewPort->m_pSubtitles->m_flCurStartTime + m_flHoldTime;
			if ( flDIeTime - flFade < flTime )
			{
				float flAlpha = (flDIeTime - flTime) / flFade;
				setFgColor( iRed, iGreen, iBlue, 255 - (flAlpha * 255) );
				return;
			}
		}
	}

	setFgColor( iRed, iGreen, iBlue, 0 );
}

void CSubtitleTextPanel::paint( void )
{
	int iRed, iGreen, iBlue, iAlpha;
	int iX, iY;
	getFgColor( iRed, iGreen, iBlue, iAlpha );
	getTextImage()->getPos( iX, iY );
	getTextImage()->setPos( iX + 1, iY + 1 );
	getTextImage()->setColor( Color( 0, 0, 0, iAlpha ) );
	getTextImage()->doPaint( this );
	getTextImage()->setPos( iX, iY );
	getTextImage()->setColor( Color( iRed, iGreen, iBlue, iAlpha ) );
	getTextImage()->doPaint( this );
}

CSubtitles::CSubtitles( void ) : Panel( XRES( 10 ), YRES( 10 ), XRES( 330 ), YRES( 300 ) )
{	
	m_pLayer = new Panel;
	m_pLayer->setParent( this );
	m_pLayer->setPaintBackgroundEnabled( false );
	setVisible( false );
	m_pCur = NULL;
	m_pWait = NULL;
	m_flLastTime = 0.0f;
	m_flLayerPos = 0.0f;
}

void CSubtitles::Initialize( void )
{
	m_pLayer->removeAllChildren();
	setVisible( false );
	m_pCur = NULL;
	m_pWait = NULL;
	m_flLastTime = 0.0f;
	m_flLayerPos = 0.0f;
}

void CSubtitles::AddMessage( client_textmessage_t *pMsg )
{
	float flTime = gEngfuncs.GetClientTime();
	float flHoldTime = pMsg->holdtime;	
	if ( gcvSubtitlesTimeMin->value && (flHoldTime < gcvSubtitlesTimeMin->value) )
		flHoldTime = gcvSubtitlesTimeMin->value;

	if ( gcvSubtitlesTimeMax->value && (flHoldTime > gcvSubtitlesTimeMax->value) )
		flHoldTime = gcvSubtitlesTimeMax->value;

	const char *cText = pMsg->pMessage;
	client_textmessage_t *cPostMsgName = NULL;
	if ( cText[0] == '$' )
	{
		// get postMsg
		char postMsgName[64];
		cText = gEngfuncs.COM_ParseFile( (char *)cText, postMsgName );
		cPostMsgName = TextMessageGet( &postMsgName[1] );
		cText += 2;
		if ( !cPostMsgName )
			gEngfuncs.Con_Printf( "[HR:P] SUBTITLES WARNING: post-message %s not found in titles.txt!\n", postMsgName );
	}

	Font *pFont = FontFromMessage( cText );
	int iWide, iTall;
	CSubtitleTextPanel *pText = new CSubtitleTextPanel( cText, 0, 0, getWide(), 64 );
	pText->setParent( m_pLayer );
	pText->setFont( pFont );
	pText->pMsgAfterDeath = cPostMsgName;
	pText->setFgColor( pMsg->r1, pMsg->g1, pMsg->b1, 0 );
	pText->getTextImage()->getTextSizeWrapped( iWide, iTall );
	pText->getTextImage()->setPos( 0, 5 );
	iTall += 10;
	pText->setSize( getWide(), iTall );
	pText->m_flHoldTime = flHoldTime;

	if ( !isVisible() )
	{
		m_flLayerPos = getTall() - iTall;
		m_pLayer->setBounds( 0, m_flLayerPos, getWide(), iTall );
		pText->m_flBirthTime = flTime;
		m_flCurStartTime = flTime;
		m_pCur = pText;
		m_pWait = NULL;
		pText->setVisible( true );
		setVisible( true );
	}
	else
	{
		pText->setVisible( false );
		pText->m_flBirthTime = 0.0f;

		if ( !m_pWait )
		{
			int iLayerTall = m_pLayer->getTall();
			m_pLayer->setSize( getWide(), iLayerTall + iTall );
			pText->setPos( 0, iLayerTall );
			m_pWait = pText;
		}
	}
}

void CSubtitles::paintBackground( void )
{	
	float flCurTime = gEngfuncs.GetClientTime();
	if ( m_flLastTime == 0.0f )
		m_flLastTime = flCurTime;

	float flDeltaTime = flCurTime - m_flLastTime;
	if ( !gcvSubtitlesEnabled->value )
	{
		Initialize();
		return;
	}

	if ( m_pCur )
	{
		if ( m_flCurStartTime + m_pCur->m_flHoldTime <= flCurTime )
		{
			m_flCurStartTime = flCurTime;
			client_textmessage_t *pNewMsg = m_pCur->pMsgAfterDeath;
			m_pLayer->removeChild( m_pCur );
			m_pCur = NULL;

			if ( pNewMsg )
				AddMessage( pNewMsg );

			if ( m_pLayer->getChildCount() == 0 )
			{
				m_pWait = NULL;
				setVisible( false );
				return;
			}

			// find oldest child to start fading int
			float flMinTime = 99999.0f;
			int i;
			for ( i = 0; i < m_pLayer->getChildCount(); i++ )
			{
				CSubtitleTextPanel *pChild = (CSubtitleTextPanel*)m_pLayer->getChild( i );
				if ( pChild->isVisible() && pChild->m_flBirthTime < flMinTime )
				{
					m_pCur = pChild;
					flMinTime = pChild->m_flBirthTime;
				}
			}

			if ( !m_pCur )
			{
				// get cur from waiting queue
				m_pCur = m_pWait;
				if ( m_pCur )
				{
					m_flLayerPos = getTall() - m_pCur->getTall();
					m_pLayer->setBounds( 0, m_flLayerPos, getWide(), m_pCur->getTall() );
					m_pCur->setPos( 0, 0 );
					m_pCur->m_flBirthTime = flCurTime;					
					m_pCur->setVisible( true );

					m_pWait = NULL;
					for ( i = 0; i < m_pLayer->getChildCount(); i++ )
					{
						if ( !m_pLayer->getChild(i)->isVisible() )
						{
							m_pWait = (CSubtitleTextPanel*)m_pLayer->getChild( i );
							int iLayerTall = m_pLayer->getTall();
							m_pLayer->setSize( getWide(), iLayerTall + m_pWait->getTall() );
							m_pWait->setPos( 0, iLayerTall );
							break;
						}
					}					
				}
				else
				{
					gEngfuncs.Con_Printf( "[HR:P] SUBTITLES ERROR: subtitle has childs but no lists!\n" );
					setVisible( false );
					return;
				}
			}
		}
	}

	if ( m_pWait )
	{
		float flRest = m_flLayerPos + m_pLayer->getTall() - getTall();
		float flSpeed = flRest > 40.0f ? 0.5f : (flRest / 40.0f * 0.5f);
		flSpeed += 0.5f;	
		m_flLayerPos -= (flDeltaTime * gcvSubtitlesScrollSpeed->value * flSpeed);
		if ( m_flLayerPos + m_pLayer->getTall() <= getTall() )
		{
			m_flLayerPos = getTall() - m_pLayer->getTall();
			m_pWait->setVisible( true );
			m_pWait->m_flBirthTime = flCurTime;

			m_pWait = NULL;
			for ( int i = 0; i < m_pLayer->getChildCount(); i++ )
			{
				if ( !m_pLayer->getChild( i )->isVisible() )
				{
					m_pWait = (CSubtitleTextPanel*)m_pLayer->getChild( i );
					int iLayerTall = m_pLayer->getTall();
					m_pLayer->setSize( getWide(), iLayerTall + m_pWait->getTall() );
					m_pWait->setPos( 0, iLayerTall );
					break;
				}
			}
		}
		m_pLayer->setPos( 0, m_flLayerPos );
	}

	m_flLastTime = flCurTime;
}
