#pragma once

// Shared dark palette for every custom-drawn/owner-drawn panel in this
// tool (MainFrm's action bar, FileView's tab strip, ClassView's tree +
// search bar, PropertiesWnd). One place to keep the look consistent --
// this MFC toolset has no built-in dark CMFCVisualManager to lean on
// (only up through Office2007), so panels that need to look modern own
// their own painting instead.
namespace Theme
{
	// Backgrounds, darkest to lightest.
	const COLORREF Bg0 = RGB(24, 24, 27);		// window/frame background
	const COLORREF Bg1 = RGB(30, 30, 33);		// panel background (tree, grid host)
	const COLORREF Bg2 = RGB(38, 38, 42);		// raised surface (action bar, tab strip, search bar)
	const COLORREF Bg3 = RGB(54, 54, 60);		// idle button/chip sitting on a Bg2 surface
	const COLORREF Bg4 = RGB(68, 68, 75);		// hovered button/chip

	const COLORREF Border = RGB(58, 58, 64);

	const COLORREF Text = RGB(225, 225, 228);
	const COLORREF TextMuted = RGB(150, 150, 158);
	const COLORREF TextDisabled = RGB(100, 100, 108);

	// Windows 11's own accent blue -- already used for tree row selection,
	// reused everywhere else so the accent reads as one deliberate choice.
	const COLORREF Accent = RGB(0, 120, 215);
	const COLORREF AccentHover = RGB(24, 138, 226);
	const COLORREF AccentText = RGB(255, 255, 255);
}
