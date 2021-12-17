/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

// -=- CleanDesktop.cxx

#include <rfb_win32/CleanDesktop.h>
#include <rfb_win32/CurrentUser.h>
#include <rfb_win32/Registry.h>
#include <rfb/LogWriter.h>
#include <rdr/Exception.h>
#include <os/os.h>
#include <set>

using namespace rfb;
using namespace rfb::win32;

static LogWriter vlog("CleanDesktop");

#ifndef HAVE_ACTIVE_DESKTOP_L
DEFINE_GUID(CLSID_ActiveDesktop, 0x75048700L, 0xEF1F, 0x11D0, 0x98, 0x88, 0x00, 0x60, 0x97, 0xDE, 0xAC, 0xF9);
#endif
#ifndef HAVE_ACTIVE_DESKTOP_H
DEFINE_GUID(IID_IActiveDesktop, 0xF490EB00L, 0x1240, 0x11D1, 0x98, 0x88, 0x00, 0x60, 0x97, 0xDE, 0xAC, 0xF9);
#endif

/* IActiveDesktop. As of 2011-10-12, MinGW does not define
   IActiveDesktop in any way (see tracker 2877129), while MinGW64 is
   broken: has the headers but not the lib symbols. */
#if !(defined(HAVE_ACTIVE_DESKTOP_H) || defined(HAVE_ACTIVE_DESKTOP_L))
extern const GUID CLSID_ActiveDesktop;
extern const GUID IID_IActiveDesktop;

/* IActiveDesktop::AddUrl */
#define ADDURL_SILENT		0x0001

/* IActiveDesktop::AddDesktopItemWithUI */
#define DTI_ADDUI_DEFAULT	0x00000000
#define DTI_ADDUI_DISPSUBWIZARD	0x00000001
#define DTI_ADDUI_POSITIONITEM	0x00000002

/* IActiveDesktop::ModifyDesktopItem */
#define COMP_ELEM_TYPE		0x00000001
#define COMP_ELEM_CHECKED	0x00000002
#define COMP_ELEM_DIRTY		0x00000004
#define COMP_ELEM_NOSCROLL	0x00000008
#define COMP_ELEM_POS_LEFT	0x00000010
#define COMP_ELEM_POS_TOP	0x00000020
#define COMP_ELEM_SIZE_WIDTH	0x00000040
#define COMP_ELEM_SIZE_HEIGHT	0x00000080
#define COMP_ELEM_POS_ZINDEX	0x00000100
#define COMP_ELEM_SOURCE	0x00000200
#define COMP_ELEM_FRIENDLYNAME	0x00000400
#define COMP_ELEM_SUBSCRIBEDURL	0x00000800
#define COMP_ELEM_ORIGINAL_CSI	0x00001000
#define COMP_ELEM_RESTORED_CSI	0x00002000
#define COMP_ELEM_CURITEMSTATE	0x00004000
#define COMP_ELEM_ALL		0x00007FFF /* OR-ed all COMP_ELEM_ */

/* IActiveDesktop::GetWallpaper */
#define AD_GETWP_BMP		0x00000000
#define AD_GETWP_IMAGE		0x00000001
#define AD_GETWP_LAST_APPLIED	0x00000002

/* IActiveDesktop::ApplyChanges */
#define AD_APPLY_SAVE		0x00000001
#define AD_APPLY_HTMLGEN	0x00000002
#define AD_APPLY_REFRESH	0x00000004
#define AD_APPLY_ALL		0x00000007 /* OR-ed three AD_APPLY_ above */
#define AD_APPLY_FORCE		0x00000008
#define AD_APPLY_BUFFERED_REFRESH 0x00000010
#define AD_APPLY_DYNAMICREFRESH	0x00000020

/* Structures for IActiveDesktop */
typedef struct {
    DWORD dwSize;
    int iLeft;
    int iTop;
    DWORD dwWidth;
    DWORD dwHeight;
    DWORD dwItemState;
} COMPSTATEINFO, * LPCOMPSTATEINFO;
typedef const COMPSTATEINFO* LPCCOMPSTATEINFO;

typedef struct {
    DWORD dwSize;
    int iLeft;
    int iTop;
    DWORD dwWidth;
    DWORD dwHeight;
    int izIndex;
    BOOL fCanResize;
    BOOL fCanResizeX;
    BOOL fCanResizeY;
    int iPreferredLeftPercent;
    int iPreferredTopPercent;
} COMPPOS, * LPCOMPPOS;
typedef const COMPPOS* LPCCOMPPOS;

typedef struct {
    DWORD dwSize;
    DWORD dwID;
    int iComponentType;
    BOOL fChecked;
    BOOL fDirty;
    BOOL fNoScroll;
    COMPPOS cpPos;
    WCHAR wszFriendlyName[MAX_PATH];
    WCHAR wszSource[INTERNET_MAX_URL_LENGTH];
    WCHAR wszSubscribedURL[INTERNET_MAX_URL_LENGTH];
    DWORD dwCurItemState;
    COMPSTATEINFO csiOriginal;
    COMPSTATEINFO csiRestored;
} COMPONENT, * LPCOMPONENT;
typedef const COMPONENT* LPCCOMPONENT;

typedef struct {
    DWORD dwSize;
    BOOL fEnableComponents;
    BOOL fActiveDesktop;
} COMPONENTSOPT, * LPCOMPONENTSOPT;
typedef const COMPONENTSOPT* LPCCOMPONENTSOPT;

typedef struct {
    DWORD dwSize;
    DWORD dwStyle;
} WALLPAPEROPT, * LPWALLPAPEROPT;
typedef const WALLPAPEROPT* LPCWALLPAPEROPT;

/* WALLPAPEROPT styles */
#define WPSTYLE_CENTER		0x0
#define WPSTYLE_TILE		0x1
#define WPSTYLE_STRETCH		0x2
#define WPSTYLE_MAX		0x3

/* Those two are defined in Windows 7 and newer, we don't need them now */
#if 0
#define WPSTYLE_KEEPASPECT	0x3
#define WPSTYLE_CROPTOFIT	0x4
#endif

#define INTERFACE IActiveDesktop
DECLARE_INTERFACE_(IActiveDesktop, IUnknown)
{
    STDMETHOD(QueryInterface)(THIS_ REFIID, PVOID*) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
    STDMETHOD(AddDesktopItem)(THIS_ LPCOMPONENT, DWORD) PURE;
    STDMETHOD(AddDesktopItemWithUI)(THIS_ HWND, LPCOMPONENT, DWORD) PURE;
    STDMETHOD(AddUrl)(THIS_ HWND, LPCWSTR, LPCOMPONENT, DWORD) PURE;
    STDMETHOD(ApplyChanges)(THIS_ DWORD) PURE;
    STDMETHOD(GenerateDesktopItemHtml)(THIS_ LPCWSTR, LPCOMPONENT, DWORD) PURE;
    STDMETHOD(GetDesktopItem)(THIS_ int, LPCOMPONENT, DWORD) PURE;
    STDMETHOD(GetDesktopItemByID)(THIS_ DWORD, LPCOMPONENT, DWORD) PURE;
    STDMETHOD(GetDesktopItemBySource)(THIS_ LPCWSTR, LPCOMPONENT, DWORD) PURE;
    STDMETHOD(GetDesktopItemCount)(THIS_ LPINT, DWORD) PURE;
    STDMETHOD(GetDesktopItemOptions)(THIS_ LPCOMPONENTSOPT, DWORD) PURE;
    STDMETHOD(GetPattern)(THIS_ LPWSTR, UINT, DWORD) PURE;
    STDMETHOD(GetWallpaper)(THIS_ LPWSTR, UINT, DWORD) PURE;
    STDMETHOD(GetWallpaperOptions)(THIS_ LPWALLPAPEROPT, DWORD) PURE;
    STDMETHOD(ModifyDesktopItem)(THIS_ LPCCOMPONENT, DWORD) PURE;
    STDMETHOD(RemoveDesktopItem)(THIS_ LPCCOMPONENT, DWORD) PURE;
    STDMETHOD(SetDesktopItemOptions)(THIS_ LPCCOMPONENTSOPT, DWORD) PURE;
    STDMETHOD(SetPattern)(THIS_ LPCWSTR, DWORD) PURE;
    STDMETHOD(SetWallpaper)(THIS_ LPCWSTR, DWORD) PURE;
    STDMETHOD(SetWallpaperOptions)(THIS_ LPCWALLPAPEROPT, DWORD) PURE;
};

#endif /* HAVE_ACTIVE_DESKTOP_H */
struct ActiveDesktop {
  ActiveDesktop() : handle(0) {
    // - Contact Active Desktop
    HRESULT result = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER,
                                      IID_IActiveDesktop, (PVOID*)&handle);
    if (result != S_OK)
      throw rdr::SystemException("failed to contact Active Desktop", result);
  }
  ~ActiveDesktop() {
    if (handle)
      handle->Release();
  }

  // enableItem
  //   enables or disables the Nth Active Desktop item
  bool enableItem(int i, bool enable_) {
    COMPONENT item;
    memset(&item, 0, sizeof(item));
    item.dwSize = sizeof(item);

    HRESULT hr = handle->GetDesktopItem(i, &item, 0);
    if (hr != S_OK) {
      vlog.error("unable to GetDesktopItem %d: %ld", i, hr);
      return false;
    }
    item.fChecked = enable_;
    vlog.debug("%sbling %d: \"%s\"", enable_ ? "ena" : "disa", i, (const char*)CStr(item.wszFriendlyName));

    hr = handle->ModifyDesktopItem(&item, COMP_ELEM_CHECKED);
    return hr == S_OK;
  }
  
  // enable
  //   Attempts to enable/disable Active Desktop, returns true if the setting changed,
  //   false otherwise.
  //   If Active Desktop *can* be enabled/disabled then that is done.
  //   If Active Desktop is always on (XP/2K3) then instead the individual items are
  //   disabled, and true is returned to indicate that they need to be restored later.
  bool enable(bool enable_) {
    bool modifyComponents = false;

    vlog.debug("ActiveDesktop::enable");

    // - Firstly, try to disable Active Desktop entirely
    HRESULT hr;
    COMPONENTSOPT adOptions;
    memset(&adOptions, 0, sizeof(adOptions));
    adOptions.dwSize = sizeof(adOptions);

    // Attempt to actually disable/enable AD
    hr = handle->GetDesktopItemOptions(&adOptions, 0);
    if (hr == S_OK) {
      // If Active Desktop is already in the desired state then return false (no change)
      // NB: If AD is enabled AND restoreItems is set then we regard it as disabled...
      if (((adOptions.fActiveDesktop==0) && restoreItems.empty()) == (enable_==false))
        return false;
      adOptions.fActiveDesktop = enable_;
      hr = handle->SetDesktopItemOptions(&adOptions, 0);
    }
    // Apply the change, then test whether it actually took effect
    if (hr == S_OK)
      hr = handle->ApplyChanges(AD_APPLY_REFRESH);
    if (hr == S_OK)
      hr = handle->GetDesktopItemOptions(&adOptions, 0);
    if (hr == S_OK)
      modifyComponents = (adOptions.fActiveDesktop==0) != (enable_==false);
    if (hr != S_OK) {
      vlog.error("failed to get/set Active Desktop options: %ld", hr);
      return false;
    }

    if (enable_) {
      // - We are re-enabling Active Desktop.  If there are components in restoreItems
      //   then restore them!
      std::set<int>::const_iterator i;
      for (i=restoreItems.begin(); i!=restoreItems.end(); i++) {
        enableItem(*i, true);
      }
      restoreItems.clear();
    } else if (modifyComponents) {
      // - Disable all currently enabled items, and add the disabled ones to restoreItems
      int itemCount = 0;
      hr = handle->GetDesktopItemCount(&itemCount, 0);
      if (hr != S_OK) {
        vlog.error("failed to get desktop item count: %ld", hr);
        return false;
      }
      for (int i=0; i<itemCount; i++) {
        if (enableItem(i, false))
          restoreItems.insert(i);
      }
    }

    // - Apply whatever changes we have made, but DON'T save them!
    hr = handle->ApplyChanges(AD_APPLY_REFRESH);
    return hr == S_OK;
  }
  IActiveDesktop* handle;
  std::set<int> restoreItems;
};


DWORD SysParamsInfo(UINT action, UINT param, PVOID ptr, UINT ini) {
  DWORD r = ERROR_SUCCESS;
  if (!SystemParametersInfo(action, param, ptr, ini)) {
    r = GetLastError();
    vlog.info("SPI error: %lu", r);
  }
  return r;
}


CleanDesktop::CleanDesktop() : restoreActiveDesktop(false),
                               restoreWallpaper(false),
                               restoreEffects(false) {
  CoInitialize(0);
}

CleanDesktop::~CleanDesktop() {
  enableEffects();
  enableWallpaper();
  CoUninitialize();
}

void CleanDesktop::disableWallpaper() {
  try {
    ImpersonateCurrentUser icu;

    vlog.debug("disable desktop wallpaper/Active Desktop");

    // -=- First attempt to remove the wallpaper using Active Desktop
    try {
      ActiveDesktop ad;
      if (ad.enable(false))
        restoreActiveDesktop = true;
    } catch (rdr::Exception& e) {
      vlog.error("%s", e.str());
    }

    // -=- Switch of normal wallpaper and notify apps
    SysParamsInfo(SPI_SETDESKWALLPAPER, 0, (PVOID) "", SPIF_SENDCHANGE);
    restoreWallpaper = true;

  } catch (rdr::Exception& e) {
    vlog.info("%s", e.str());
  }
}

void CleanDesktop::enableWallpaper() {
  try {
    ImpersonateCurrentUser icu;

    if (restoreActiveDesktop) {
      vlog.debug("restore Active Desktop");

      // -=- First attempt to re-enable Active Desktop
      try {
        ActiveDesktop ad;
        ad.enable(true);
        restoreActiveDesktop = false;
      } catch (rdr::Exception& e) {
        vlog.error("%s", e.str());
      }
    }

    if (restoreWallpaper) {
      vlog.debug("restore desktop wallpaper");

      // -=- Then restore the standard wallpaper if required
	    SysParamsInfo(SPI_SETDESKWALLPAPER, 0, NULL, SPIF_SENDCHANGE);
      restoreWallpaper = false;
    }

  } catch (rdr::Exception& e) {
    vlog.info("%s", e.str());
  }
}


void CleanDesktop::disableEffects() {
  try {
    ImpersonateCurrentUser icu;

    vlog.debug("disable desktop effects");

    SysParamsInfo(SPI_SETFONTSMOOTHING, FALSE, 0, SPIF_SENDCHANGE);
    if (SysParamsInfo(SPI_GETUIEFFECTS, 0, &uiEffects, 0) == ERROR_CALL_NOT_IMPLEMENTED) {
      SysParamsInfo(SPI_GETCOMBOBOXANIMATION, 0, &comboBoxAnim, 0);
      SysParamsInfo(SPI_GETGRADIENTCAPTIONS, 0, &gradientCaptions, 0);
      SysParamsInfo(SPI_GETHOTTRACKING, 0, &hotTracking, 0);
      SysParamsInfo(SPI_GETLISTBOXSMOOTHSCROLLING, 0, &listBoxSmoothScroll, 0);
      SysParamsInfo(SPI_GETMENUANIMATION, 0, &menuAnim, 0);
      SysParamsInfo(SPI_SETCOMBOBOXANIMATION, 0, FALSE, SPIF_SENDCHANGE);
      SysParamsInfo(SPI_SETGRADIENTCAPTIONS, 0, FALSE, SPIF_SENDCHANGE);
      SysParamsInfo(SPI_SETHOTTRACKING, 0, FALSE, SPIF_SENDCHANGE);
      SysParamsInfo(SPI_SETLISTBOXSMOOTHSCROLLING, 0, FALSE, SPIF_SENDCHANGE);
      SysParamsInfo(SPI_SETMENUANIMATION, 0, FALSE, SPIF_SENDCHANGE);
    } else {
      SysParamsInfo(SPI_SETUIEFFECTS, 0, FALSE, SPIF_SENDCHANGE);

      // We *always* restore UI effects overall, since there is no Windows GUI to do it
      uiEffects = TRUE;
    }
    restoreEffects = true;

  } catch (rdr::Exception& e) {
    vlog.info("%s", e.str());
  }
}

void CleanDesktop::enableEffects() {
  try {
    if (restoreEffects) {
      ImpersonateCurrentUser icu;

      vlog.debug("restore desktop effects");

      RegKey desktopCfg;
      desktopCfg.openKey(HKEY_CURRENT_USER, _T("Control Panel\\Desktop"));
      SysParamsInfo(SPI_SETFONTSMOOTHING, desktopCfg.getInt(_T("FontSmoothing"), 0) != 0, 0, SPIF_SENDCHANGE);
      if (SysParamsInfo(SPI_SETUIEFFECTS, 0, (void*)(intptr_t)uiEffects, SPIF_SENDCHANGE) == ERROR_CALL_NOT_IMPLEMENTED) {
        SysParamsInfo(SPI_SETCOMBOBOXANIMATION, 0, (void*)(intptr_t)comboBoxAnim, SPIF_SENDCHANGE);
        SysParamsInfo(SPI_SETGRADIENTCAPTIONS, 0, (void*)(intptr_t)gradientCaptions, SPIF_SENDCHANGE);
        SysParamsInfo(SPI_SETHOTTRACKING, 0, (void*)(intptr_t)hotTracking, SPIF_SENDCHANGE);
        SysParamsInfo(SPI_SETLISTBOXSMOOTHSCROLLING, 0, (void*)(intptr_t)listBoxSmoothScroll, SPIF_SENDCHANGE);
        SysParamsInfo(SPI_SETMENUANIMATION, 0, (void*)(intptr_t)menuAnim, SPIF_SENDCHANGE);
      }
      restoreEffects = false;
    }

  } catch (rdr::Exception& e) {
    vlog.info("%s", e.str());
  }
}
