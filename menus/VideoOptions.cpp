/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "Framework.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "SpinControl.h"
#include "Slider.h"
#include "CheckBox.h"

#define ART_BANNER	  	"gfx/shell/head_vidoptions"
#define ART_GAMMA		"gfx/shell/gamma"

class CMenuRenderResolutionModel : public CMenuBaseArrayModel
{
public:
	void Update() override
	{
		m_resolutions.AddToTail( resolution { 0.5, "0.5x" } );
		m_resolutions.AddToTail( resolution { 1.0, "1.0x" } );
		m_resolutions.AddToTail( resolution { 1.5, "1.5x" } );
		m_resolutions.AddToTail( resolution { 2.0, "2.0x" } );
	}

	int GetRows() const override { return m_resolutions.Count(); }

	const char *GetShortName( int i ) { return m_resolutions[i].name; }
	const char *GetText( int i ) override { return m_resolutions[i].name; }

private:
	struct resolution {
		float k;
		const char *name;
	};

	CUtlVector<resolution> m_resolutions;
};


/*
 * =============================================================================
 */

class CMenuVidOptions : public CMenuFramework
{
private:
	void _Init() override;
	void _VidInit() override;

public:
	CMenuVidOptions() : CMenuFramework( "CMenuVidOptions" ) { }
	void SaveAndPopMenu() override;
	void UpdateConfig();
	void GetConfig();

	int		outlineWidth;

	class CMenuVidPreview : public CMenuBitmap
	{
		void Draw() override;
	} testImage;

	CMenuPicButton	done;

	CMenuSlider	screenSize;

	CMenuRenderResolutionModel renderResolutionsModel;
	CMenuSpinControl renderResolution;

	CMenuSlider	gammaIntensity;

	CMenuSlider	glareReduction;

	CMenuCheckBox   vbo;

	CMenuCheckBox   bump;

	CMenuSlider	  rtxBounces;

	HIMAGE		hTestImage;
};

/*
=================
CMenuVidOptions::UpdateConfig
=================
*/
void CMenuVidOptions::UpdateConfig( void )
{
	float val1 = RemapVal( gammaIntensity.GetCurrentValue(), 0.0, 1.0, 1.8, 3.0 );
	float val2 = RemapVal( glareReduction.GetCurrentValue(), 0.0, 1.0, 0.0, 3.0 );
	EngFuncs::CvarSetValue( "gamma", val1 );
	EngFuncs::CvarSetValue( "brightness", val2 );
	EngFuncs::ProcessImage( hTestImage, val1, val2 );
}

void CMenuVidOptions::GetConfig( void )
{
	float val1 = EngFuncs::GetCvarFloat( "gamma" );
	float val2 = EngFuncs::GetCvarFloat( "brightness" );

	gammaIntensity.SetCurrentValue( RemapVal( val1, 1.8f, 3.0f, 0.0f, 1.0f ) );
	glareReduction.SetCurrentValue( RemapVal( val2, 0.0f, 3.0f, 0.0f, 1.0f ) );
	EngFuncs::ProcessImage( hTestImage, val1, val2 );

	gammaIntensity.SetOriginalValue( val1 );
	glareReduction.SetOriginalValue( val2 );
}

void CMenuVidOptions::SaveAndPopMenu( void )
{
	screenSize.WriteCvar();
	vbo.WriteCvar();
	bump.WriteCvar();
	// gamma and brightness is already written

	CMenuFramework::SaveAndPopMenu();
}

/*
=================
CMenuVidOptions::Ownerdraw
=================
*/
void CMenuVidOptions::CMenuVidPreview::Draw( )
{
	int		color = 0xFFFF0000; // 255, 0, 0, 255
	int		viewport[4];
	int		viewsize, size, sb_lines;

	viewsize = EngFuncs::GetCvarFloat( "viewsize" );

	if( viewsize >= 120 )
		sb_lines = 0;	// no status bar at all
	else if( viewsize >= 110 )
		sb_lines = 24;	// no inventory
	else sb_lines = 48;

	size = Q_min( viewsize, 100 );

	viewport[2] = m_scSize.w * size / 100;
	viewport[3] = m_scSize.h * size / 100;

	if( viewport[3] > m_scSize.h - sb_lines )
		viewport[3] = m_scSize.h - sb_lines;
	if( viewport[3] > m_scSize.h )
		viewport[3] = m_scSize.h;

	viewport[2] &= ~7;
	viewport[3] &= ~1;

	viewport[0] = (m_scSize.w - viewport[2]) / 2;
	viewport[1] = (m_scSize.h - sb_lines - viewport[3]) / 2;

	UI_DrawPic( m_scPos.x + viewport[0], m_scPos.y + viewport[1], viewport[2], viewport[3], uiColorWhite, szPic );
	UI_DrawRectangleExt( m_scPos, m_scSize, color, ((CMenuVidOptions*)Parent())->outlineWidth );
}

/*
=================
CMenuVidOptions::Init
=================
*/
void CMenuVidOptions::_Init( void )
{
	const char *r_refdll_value = EngFuncs::GetCvarString("r_refdll");
	qboolean is_vk = strcmp(r_refdll_value, "vk") == 0;
	qboolean is_vk_rtx = EngFuncs::GetCvarFloat( "vk_rtx" );

	// =========================================================================

	banner.SetPicture(ART_BANNER);

	done.SetNameAndStatus( L( "GameUI_OK" ), L( "Go back to the Video Menu" ) );
	done.SetCoord( 72, 230 );
	done.SetPicture( PC_DONE );
	done.onReleased = VoidCb( &CMenuVidOptions::SaveAndPopMenu );

	// =========================================================================

#ifdef PIC_KEEP_RGBDATA
	hTestImage = EngFuncs::PIC_Load( ART_GAMMA, PIC_KEEP_RGBDATA );
#else
	hTestImage = EngFuncs::PIC_Load( ART_GAMMA, PIC_KEEP_SOURCE );
#endif

	testImage.iFlags = QMF_INACTIVE;
	testImage.SetRect( 390, 225, 480, 450 );
	testImage.SetPicture( ART_GAMMA );

	// =========================================================================

	renderResolution.SetVisibility(is_vk);
	screenSize.SetVisibility(!is_vk);

	if (is_vk && is_vk_rtx) {
		renderResolutionsModel.Update();
		renderResolution.szName = L( "Render size" );
		renderResolution.Setup( &renderResolutionsModel );
		renderResolution.SetRect( 80, 320, 200, 32 );
		renderResolution.SetCharSize( QM_SMALLFONT );
		//renderResolution.onCvarGet = VoidCb( &CMenuVidModes::GetXXXConfig );
		//renderResolution.onCvarWrite = VoidCb( &CMenuVidModes::WriteXXXConfig );
		renderResolution.bUpdateImmediately = true;
		renderResolution.SetCurrentValue( 0.0 );
	} else {
		screenSize.SetNameAndStatus( L( "Screen size" ), L( "Set the screen size" ) );
		screenSize.SetCoord( 80, 350 );
		screenSize.Setup( 30, 120, 10 );
		screenSize.onChanged = CMenuEditable::WriteCvarCb;
	}

	// =========================================================================

	gammaIntensity.SetNameAndStatus( L( "GameUI_Gamma" ), L( "Set gamma value" ) );
	gammaIntensity.SetCoord( 80, 420 );
	gammaIntensity.Setup( 0.0, 1.0, 0.025 );
	gammaIntensity.onChanged = VoidCb( &CMenuVidOptions::UpdateConfig );
	gammaIntensity.onCvarGet = VoidCb( &CMenuVidOptions::GetConfig );

	// =========================================================================

	glareReduction.SetCoord( 80, 490 );
	glareReduction.SetNameAndStatus( L( "GameUI_Brightness" ), L( "Set brightness level" ) );
	glareReduction.Setup( 0, 1.0, 0.025 );
	glareReduction.onChanged = VoidCb( &CMenuVidOptions::UpdateConfig );
	glareReduction.onCvarGet = VoidCb( &CMenuVidOptions::GetConfig );

	// =========================================================================

	if (is_vk && is_vk_rtx) {
		rtxBounces.SetNameAndStatus( L( "Ray bounces" ), L( "Set the count of ray max bounds" ) );
		rtxBounces.SetCoord( 80, 560 );
		rtxBounces.Setup( 1, 50, 1 );
		rtxBounces.onChanged = CMenuEditable::WriteCvarCb;
	}

	// =========================================================================

	if (!is_vk) {
		bump.SetNameAndStatus( L( "Bump-mapping" ), L( "Enable bump mapping" ) );
		bump.SetCoord( 80, 560 );
		if( !EngFuncs::GetCvarFloat( "r_vbo" ) )
			bump.SetGrayed( true );

		vbo.SetNameAndStatus( L( "Use VBO" ), L( "Use new world renderer. Faster, but rarely glitchy" ) );
		vbo.SetCoord( 80, 610 );
		vbo.onChanged = CMenuCheckBox::BitMaskCb;
		vbo.onChanged.pExtra = &bump.iFlags;
		vbo.bInvertMask = true;
		vbo.iMask = QMF_GRAYED;
	}

	// =========================================================================

	AddItem( background );
	AddItem( banner );
	AddItem( done );
	AddItem( gammaIntensity );
	AddItem( glareReduction );
	AddItem( testImage );

	if (is_vk && is_vk_rtx) {
		AddItem( renderResolution );
		AddItem( rtxBounces );
	} else {
		AddItem( screenSize );
		AddItem( bump );
		AddItem( vbo );
	}

	gammaIntensity.LinkCvar( "gamma" );
	glareReduction.LinkCvar( "brightness" );

	//renderResolution.LinkCvar( "vk_rtx_render_resolution" );
	rtxBounces.LinkCvar( "vk_rtx_bounces" );

	screenSize.LinkCvar( "viewsize" );
	bump.LinkCvar( "r_bump" );
	vbo.LinkCvar( "r_vbo" );
}

void CMenuVidOptions::_VidInit()
{
	outlineWidth = 2;
	UI_ScaleCoords( NULL, NULL, &outlineWidth, NULL );
}

ADD_MENU( menu_vidoptions, CMenuVidOptions, UI_VidOptions_Menu );
