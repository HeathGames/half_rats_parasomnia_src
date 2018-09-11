/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

//=========================================================
// CVGUIDebugOverlay - VGUI debugging overlay
// @author Shepard
//=========================================================

#ifndef CVGUIDEBUGOVERLAY_H
#define CVGUIDEBUGOVERLAY_H

// Window's position and size
#define DEBUG_OVERLAY_WINDOW_X			XRES( 0 )
#define DEBUG_OVERLAY_WINDOW_Y			YRES( 16 )
#define DEBUG_OVERLAY_WINDOW_SIZE_X		XRES( 320 )
#define DEBUG_OVERLAY_WINDOW_SIZE_Y		YRES( 240 )

// Color of keys
#define DEBUG_OVERLAY_KEY_COLOR_R		64
#define DEBUG_OVERLAY_KEY_COLOR_G		192
#define DEBUG_OVERLAY_KEY_COLOR_B		255

// Color of title
#define DEBUG_OVERLAY_TITLE_COLOR_R		255
#define DEBUG_OVERLAY_TITLE_COLOR_G		192
#define DEBUG_OVERLAY_TITLE_COLOR_B		64

// Color of values
#define DEBUG_OVERLAY_VALUE_COLOR_R		255
#define DEBUG_OVERLAY_VALUE_COLOR_G		64
#define DEBUG_OVERLAY_VALUE_COLOR_B		64

// Class declaration
class CVGUIDebugOverlay : public CTransparentPanel
{
public:
	CVGUIDebugOverlay();

	void Update();

private:
	// Dynamic values labels
	Label *m_pLabelMapValue;
	Label *m_pLabelOriginValue;
	Label *m_pLabelAnglesValue;
};

#endif // CVGUIDEBUGOVERLAY_H
