// ======================================
// Paranoia subtitle system header file
//		written by BUzer
// ======================================

#ifndef PARANOIA_VGUI_SUBTITLES_H
#define PARANOIA_VGUI_SUBTITLES_H
using namespace vgui;

class CSubtitleTextPanel : public TextPanel
{
public:
	CSubtitleTextPanel( const char* cText, int iX, int iY, int iWide, int iTall ) : TextPanel( cText, iX, iY, iWide, iTall )
	{
		m_flBirthTime = 0.0f;
		m_flHoldTime = 0.0f;
		pMsgAfterDeath = NULL;
	}

	virtual void paintBackground( void );
	virtual void paint( void );

	float m_flBirthTime;
	float m_flHoldTime;
	client_textmessage_t *pMsgAfterDeath;
};

class CSubtitles : public Panel
{
public:
    CSubtitles( void );
	void AddMessage( client_textmessage_t *pTempMessage );
	void Initialize( void );

protected:
	virtual void paintBackground( void );

public:
	Panel *m_pLayer;
	CSubtitleTextPanel *m_pCur;
	CSubtitleTextPanel *m_pWait;

	float m_flLayerPos;
	float m_flLastTime;
	float m_flCurStartTime;
};

void SubtitlesMessageAdd( client_textmessage_t *pTempMessage );
void SubtitlesInit( void );

#endif // PARANOIA_VGUI_SUBTITLES_H
