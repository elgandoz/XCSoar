/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Form/SymbolButton.hpp"
#include "Screen/Animation.hpp"
#include "Screen/Layout.hpp"
#include "Interface.hpp"
#include "resource.h"
#include "Dialogs/Internal.hpp"
#include "MainWindow.hpp"

void
WndSymbolButton::on_paint(Canvas &canvas)
{
  /* background and selector */
  RECT rc = get_client_rect();
  canvas.fill_rectangle(rc, background_brush);

  if (has_focus())
    WindowControl::PaintSelector(canvas, get_client_rect());

  // Get button RECT and shrink it to make room for the selector/focus
  InflateRect(&rc, -2, -2); // todo border width

  // Draw button to the background
  canvas.draw_button(rc, mDown);

  // If button has text on it
  if (mCaption == NULL || mCaption[0] == '\0')
    return;

  // If button is pressed, offset the text for 3D effect
  if (mDown)
    OffsetRect(&rc, Layout::FastScale(1), Layout::FastScale(1));

  const Pen p(0, text_color);
  canvas.select(p);
  const Brush b(text_color);
  canvas.select(b);

  // Draw arrow symbols instead of < and >
  if (!_tcscmp(mCaption, _T("<")) || !_tcscmp(mCaption, _T(">"))) {
    int size = min(rc.right - rc.left, rc.bottom - rc.top) / 5;

    static POINT Arrow[4];
    Arrow[0].x = (rc.left + rc.right) / 2 +
                 (!_tcscmp(mCaption, _T("<")) ? size : -size);
    Arrow[0].y = (rc.top + rc.bottom) / 2 + size;
    Arrow[1].x = (rc.left + rc.right) / 2 +
                 (!_tcscmp(mCaption, _T("<")) ? -size : size);
    Arrow[1].y = (rc.top + rc.bottom) / 2;
    Arrow[2].x = (rc.left + rc.right) / 2 +
                 (!_tcscmp(mCaption, _T("<")) ? size : -size);
    Arrow[2].y = (rc.top + rc.bottom) / 2 - size;
    Arrow[3].x = Arrow[0].x;
    Arrow[3].y = Arrow[0].y;

    canvas.polygon(Arrow, 4);
  }

  // Draw symbols instead of + and -
  if (!_tcscmp(mCaption, _T("+")) || !_tcscmp(mCaption, _T("-"))) {
    int size = min(rc.right - rc.left, rc.bottom - rc.top) / 5;

    canvas.rectangle((rc.left + rc.right) / 2 - size,
                     (rc.top + rc.bottom) / 2 - size / 3,
                     (rc.left + rc.right) / 2 + size,
                     (rc.top + rc.bottom) / 2 + size / 3);

    if (!_tcscmp(mCaption, _T("+")))
      canvas.rectangle((rc.left + rc.right) / 2 - size / 3,
                       (rc.top + rc.bottom) / 2 - size,
                       (rc.left + rc.right) / 2 + size / 3,
                       (rc.top + rc.bottom) / 2 + size);
  }

  // Draw Fly bitmap
  if (!_tcscmp(mCaption, _T("Fly"))) {

    Bitmap launcher1_bitmap(IDB_LAUNCHER1);
    BitmapCanvas bitmap_canvas(canvas, launcher1_bitmap);
    canvas.stretch(bitmap_canvas, 0, 0, 112, 30);
  }

  // Draw Simulator bitmap
  if (!_tcscmp(mCaption, _T("Simulator"))) {

    Bitmap launcher2_bitmap(IDB_LAUNCHER2);
    BitmapCanvas bitmap_canvas(canvas, launcher2_bitmap);
    canvas.stretch(bitmap_canvas, 0, 0, 112, 30);
  }

}
