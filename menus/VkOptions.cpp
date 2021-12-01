#include "Framework.h"
#include "MenuStrings.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "CheckBox.h"
#include "Action.h"
#include "SpinControl.h"
#include "utlvector.h"
#include "Slider.h"

#define ART_BANNER		"gfx/shell/head_vidmodes"

class CMenuVsyncModeModel : public CMenuBaseArrayModel
{
public:
	void Update() override
	{
		m_vsync_modes.AddToTail( vsync_mode { 0, "Immediate" } );
		m_vsync_modes.AddToTail( vsync_mode { 1, "MailBox" } );
		m_vsync_modes.AddToTail( vsync_mode { 2, "Fifo" } );
		m_vsync_modes.AddToTail( vsync_mode { 3, "Fifo relaxed" } );
	}

	int GetRows() const override { return m_vsync_modes.Count(); }

	const char *GetShortName( int i ) { return m_vsync_modes[i].text; }
	const char *GetText( int i ) override { return m_vsync_modes[i].text; }

private:
	struct vsync_mode {
		int i;
		const char *text;
	};

	CUtlVector<vsync_mode> m_vsync_modes;
};

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

class CMenuVkOptions : public CMenuFramework
{
private:
	void _Init();
public:
	CMenuVkOptions() : CMenuFramework( "CMenuVkOptions" ) { }

	void SaveAndPopMenu() override;

	CMenuPicButton	done;

	CMenuRenderResolutionModel renderResolutionsModel;
	CMenuSpinControl renderResolution;

	CMenuVsyncModeModel vsyncModeModel;
	CMenuSpinControl vsyncMode;

	CMenuCheckBox hdr;

	CMenuCheckBox debugLightmap;

	CMenuCheckBox rtx;
	CMenuSlider	  rtxBounces;
};


/*
 * =============================================================================
 */

void CMenuVkOptions::SaveAndPopMenu( void )
{
	CMenuFramework::SaveAndPopMenu();
}

void CMenuVkOptions::_Init( void )
{
	banner.SetPicture(ART_BANNER);

	done.SetNameAndStatus( L( "GameUI_OK" ), L( "Go back to the Video Menu" ) );
	done.SetCoord( 72, 280 );
	done.SetPicture( PC_DONE );
	done.onReleased = VoidCb( &CMenuVkOptions::SaveAndPopMenu );

	renderResolutionsModel.Update();
	renderResolution.szName = L( "Render size" );
	renderResolution.Setup( &renderResolutionsModel );
	renderResolution.SetRect( 244, 280, 300, 32 );
	renderResolution.SetCharSize( QM_SMALLFONT );
	//renderResolution.onCvarGet = VoidCb( &CMenuVidModes::GetXXXConfig );
	//renderResolution.onCvarWrite = VoidCb( &CMenuVidModes::WriteXXXConfig );
	renderResolution.bUpdateImmediately = true;
	renderResolution.SetCurrentValue( 0.0 );

	vsyncModeModel.Update();
	vsyncMode.szName = L( "VSync mode" );
	vsyncMode.Setup( &vsyncModeModel );
	vsyncMode.SetRect( 244, 360, 300, 32 );
	vsyncMode.SetCharSize( QM_SMALLFONT );
	//vsyncMode.onCvarGet = VoidCb( &CMenuVidModes::GetXXXConfig );
	//vsyncMode.onCvarWrite = VoidCb( &CMenuVidModes::WriteXXXConfig );
	vsyncMode.bUpdateImmediately = true;
	vsyncMode.SetCurrentValue( 0.0 );

	hdr.SetNameAndStatus( L( "Enable HDR" ), L( "Enable HDR" ) );
	hdr.SetCoord( 244, 420 );
	//rtx.LinkCvar( "vk_hdr" );

	debugLightmap.SetNameAndStatus( L( "Debug lightmap" ), L( "Only for debug" ) );
	debugLightmap.SetCoord( 244, 480 );
	//rtx.LinkCvar( "r_lightmap" );

	rtx.SetNameAndStatus( L( "RTX mode" ), L( "Enable realtime ray tracing" ) );
	rtx.SetCoord( 650, 280 );
	//rtx.LinkCvar( "vk_rtx" );

	rtxBounces.SetNameAndStatus( L( "Ray bounces" ), L( "Set the count of ray max bounds" ) );
	rtxBounces.SetCoord( 650, 380 );
	rtxBounces.Setup( 1, 50, 1 );
	rtxBounces.onChanged = CMenuEditable::WriteCvarCb;

	AddItem( background );
	AddItem( banner );
	AddItem( done );
	AddItem( renderResolution );
	AddItem( vsyncMode );
	AddItem( hdr );
	AddItem( debugLightmap );
	AddItem( rtx );
	AddItem( rtxBounces );
}

ADD_MENU( menu_vkoptions, CMenuVkOptions, UI_VkOptions_Menu );