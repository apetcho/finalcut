/***********************************************************************
* ftermoutput.cpp - Implements the terminal output                     *
*                                                                      *
* This file is part of the FINAL CUT widget toolkit                    *
*                                                                      *
* Copyright 2021 Markus Gans                                           *
*                                                                      *
* FINAL CUT is free software; you can redistribute it and/or modify    *
* it under the terms of the GNU Lesser General Public License as       *
* published by the Free Software Foundation; either version 3 of       *
* the License, or (at your option) any later version.                  *
*                                                                      *
* FINAL CUT is distributed in the hope that it will be useful, but     *
* WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
* GNU Lesser General Public License for more details.                  *
*                                                                      *
* You should have received a copy of the GNU Lesser General Public     *
* License along with this program.  If not, see                        *
* <http://www.gnu.org/licenses/>.                                      *
***********************************************************************/

#if defined(__CYGWIN__)
  #include <unistd.h>  // need for ttyname_r
#endif

#include "final/fapplication.h"
#include "final/fkeyboard.h"
#include "final/fmouse.h"
#include "final/fobject.h"
#include "final/foptiattr.h"
#include "final/foptimove.h"
#include "final/fstartoptions.h"
#include "final/ftermcap.h"
#include "final/ftermdata.h"
#include "final/ftermdetection.h"
#include "final/ftermfreebsd.h"
#include "final/ftermoutput.h"
#include "final/ftermxterminal.h"


namespace finalcut
{

//----------------------------------------------------------------------
// class FTermOutput
//----------------------------------------------------------------------

// constructors and destructor
//----------------------------------------------------------------------
FTermOutput::FTermOutput (const FVTerm& t)  // constructor
  : FOutput{t}
{ }

//----------------------------------------------------------------------
FTermOutput::~FTermOutput() noexcept = default;  // destructor


// public methods of FTermOutput
//----------------------------------------------------------------------
std::size_t FTermOutput::getColumnNumber() const
{
  return FTerm::getColumnNumber();
}

//----------------------------------------------------------------------
std::size_t FTermOutput::getLineNumber() const
{
  return FTerm::getLineNumber();
}

//----------------------------------------------------------------------
int FTermOutput::getTabstop() const
{
  return FTerm::getTabstop();
}

//----------------------------------------------------------------------
int FTermOutput::getMaxColor() const
{
  return FTerm::getMaxColor();
}

//----------------------------------------------------------------------
Encoding FTermOutput::getEncoding() const
{
  return FTerm::getEncoding();
}

//----------------------------------------------------------------------
FString FTermOutput::getKeyName (FKey keynum) const
{
  return FTerm::getKeyName(keynum);
}

//----------------------------------------------------------------------
bool FTermOutput::isMonochron() const
{
  return FTerm::isMonochron();
}

//----------------------------------------------------------------------
bool FTermOutput::isNewFont() const
{
  return FTerm::isNewFont();
}

//----------------------------------------------------------------------
bool FTermOutput::isEncodable (wchar_t wide_char) const
{
  return FTerm::isEncodable(wide_char);
}

//----------------------------------------------------------------------
bool FTermOutput::hasTerminalResized() const
{
  return FTerm::hasChangedTermSize();
}

//----------------------------------------------------------------------
bool FTermOutput::allowsTerminalSizeManipulation() const
{
  return FTerm::isXTerminal();
}

//----------------------------------------------------------------------
bool FTermOutput::canChangeColorPalette() const
{
  return FTerm::canChangeColorPalette();
}

//----------------------------------------------------------------------
bool FTermOutput::hasHalfBlockCharacter() const
{
  return FTerm::hasHalfBlockCharacter();
}

//----------------------------------------------------------------------
bool FTermOutput::hasShadowCharacter() const
{
  return FTerm::hasShadowCharacter();
}

//----------------------------------------------------------------------
bool FTermOutput::areMetaAndArrowKeysSupported() const
{
  return ! FTerm::isLinuxTerm();
}

//----------------------------------------------------------------------
void FTermOutput::setCursor (FPoint p)
{
  // Sets the hardware cursor to the given (x,y) position

  auto x = p.getX();
  auto y = p.getY();

  if ( term_pos->getX() == x && term_pos->getY() == y )
    return;

  const auto term_width = int(getColumnNumber());
  const auto term_height = int(getLineNumber());

  if ( x >= term_width && term_width > 0 )
  {
    y += x / term_width;
    x %= term_width;
  }

  if ( term_pos->getY() >= term_height )
    term_pos->setY(term_height - 1);

  if ( y >= term_height )
    y = term_height - 1;

  const auto term_x = term_pos->getX();
  const auto term_y = term_pos->getY();
  const auto& move_str = FTerm::moveCursorString (term_x, term_y, x, y);

  if ( ! move_str.empty() )
    appendOutputBuffer(FTermControl{move_str});

  term_pos->setPoint(x, y);
}

//----------------------------------------------------------------------
void FTermOutput::setCursor (CursorMode mode)
{
  if ( mode == CursorMode::Insert )
    FTerm::setInsertCursor();
  else if ( mode == CursorMode::Overwrite )
    FTerm::unsetInsertCursor();
  else
    throw std::invalid_argument("Unimplemented cursor mode");
}

//----------------------------------------------------------------------
void FTermOutput::hideCursor (bool enable)
{
  // Hides or shows the input cursor on the terminal

  if ( ! isCursorHideable() )
    return;

  auto visibility_str = FTerm::cursorsVisibilityString (enable);

  if ( visibility_str.empty() )  // Exit the function if the string is empty
    return;

  appendOutputBuffer(FTermControl{visibility_str});
  flush();
}

//----------------------------------------------------------------------
void FTermOutput::setTerminalSize (FSize size)
{
  FTerm::setTermSize(size);
}

//----------------------------------------------------------------------
bool FTermOutput::setVGAFont()
{
  return FTerm::setVGAFont();
}

//----------------------------------------------------------------------
bool FTermOutput::setNewFont()
{
  return FTerm::setNewFont();
}

//----------------------------------------------------------------------
void FTermOutput::setNonBlockingRead (bool enable)
{
#if defined(__CYGWIN__)
  // Fixes problem with mouse input
  char termfilename[256]{};

  if ( ttyname_r(1, termfilename, sizeof(termfilename)) )
    termfilename[0] = '\0';

  if ( std::strncmp(termfilename, "/dev/cons", 9) == 0 )
  {
    FKeyboard::setNonBlockingInputSupport(false);
    return;
  }
#endif

  uInt64 blocking_time = enable ? 5000 : 100000;  // 5 or 100 ms
  FKeyboard::setReadBlockingTime (blocking_time);
}

//----------------------------------------------------------------------
void FTermOutput::initTerminal()
{
  getFTerm().initTerminal();

  // Redefine the color palette
  redefineColorPalette();

  vterm         = getFVTerm().getVirtualTerminal();
  output_buffer = std::make_shared<OutputBuffer>();
  term_pos      = std::make_shared<FPoint>(-1, -1);

  // Hide the input cursor
  cursor_hideable = FTerm::isCursorHideable();
  hideCursor();

  // Initialize character lengths
  init_characterLengths();

  // Check for support for combined characters
  init_combined_character();

  // term_attribute stores the current state of the terminal
  term_attribute.ch           = {{ L'\0' }};
  term_attribute.fg_color     = FColor::Default;
  term_attribute.bg_color     = FColor::Default;
  term_attribute.attr.byte[0] = 0;
  term_attribute.attr.byte[1] = 0;
  term_attribute.attr.byte[2] = 0;
  term_attribute.attr.byte[3] = 0;

  // Initialize the last flush time
  time_last_flush = TimeValue{};
}

//----------------------------------------------------------------------
void FTermOutput::finishTerminal()
{
  // Restore the color palette
  restoreColorPalette();

  // Show the input cursor
  showCursor();

  // Clear the terminal
  if ( FTermData::getInstance().isInAlternateScreen() )
    clearTerm();
}

//----------------------------------------------------------------------
bool FTermOutput::updateTerminal()
{
  // Updates pending changes to the terminal

  // Check if terminal updates were stopped, application is stopping,
  // VTerm has no changes, or the drawing is not completed
  if ( FVTerm::areTerminalUpdatesPaused() || FApplication::isQuit()
    || ! (isFlushTimeout() || FVTerm::isTerminalUpdateForced())
    || ! (FVTerm::hasPendingTerminalUpdates() && FVTerm::isDrawingFinished()) )
  {
    return false;
  }

  std::size_t changedlines = 0;

  for (uInt y{0}; y < uInt(vterm->height); y++)
  {
    if ( updateTerminalLine(y) )
      changedlines++;
  }

  vterm->has_changes = false;

  // sets the new input cursor position
  bool cursor_update = updateTerminalCursor();
  return cursor_update || changedlines > 0;
}

//----------------------------------------------------------------------
void FTermOutput::detectTerminalSize()
{
  FTerm::detectTermSize();
}

//----------------------------------------------------------------------
void FTermOutput::commitTerminalResize()
{
  FTerm::changeTermSizeFinished();
}

//----------------------------------------------------------------------
void FTermOutput::initScreenSettings()
{
  FTerm::initScreenSettings();
}

//----------------------------------------------------------------------
void FTermOutput::scrollAreaForward (FVTerm::FTermArea* area)
{
  auto vdesktop = getFVTerm().getVirtualDesktop();

  if ( ! TCAP(t_scroll_forward) || area != vdesktop )
    return;

  setCursor (FPoint{0, vdesktop->height});
  FTerm::scrollTermForward();
  getFVTerm().putArea (FPoint{1, 1}, vdesktop);
  const int y_max = area->height - 1;

  // avoid update lines from 0 to (y_max - 1)
  for (auto y{0}; y < y_max; y++)
  {
    area->changes[y].xmin = uInt(area->width - 1);
    area->changes[y].xmax = 0;
  }
}

//----------------------------------------------------------------------
void FTermOutput::scrollAreaReverse (FVTerm::FTermArea* area)
{
  auto vdesktop = getFVTerm().getVirtualDesktop();

  if ( ! TCAP(t_scroll_reverse) || area != vdesktop )
    return;

  setCursor (FPoint{0, 0});
  FTerm::scrollTermReverse();
  getFVTerm().putArea (FPoint{1, 1}, vdesktop);
  const int y_max = area->height - 1;

  // avoid update lines from 1 to y_max
  for (auto y{1}; y <= y_max; y++)
  {
    area->changes[y].xmin = uInt(area->width - 1);
    area->changes[y].xmax = 0;
  }
}

//----------------------------------------------------------------------
bool FTermOutput::clearTerm (wchar_t fillchar)
{
  // Clear the real terminal and put cursor at home

  const auto& cl = TCAP(t_clear_screen);
  const auto& cd = TCAP(t_clr_eos);
  const auto& cb = TCAP(t_clr_eol);
  const bool ut = FTermcap::background_color_erase;
  auto& next_attribute = FVTerm::getAttribute();
  const bool normal = FTerm::isNormal (next_attribute);
  appendAttributes (next_attribute);

  if ( ! ( (cl || cd || cb) && (normal || ut) )
    || fillchar != L' '
    || ! FVTerm::isDrawingFinished() )
  {
    return false;
  }

  if ( cl )  // Clear screen
  {
    appendOutputBuffer (FTermControl{cl});
    term_pos->setPoint(0, 0);
  }
  else if ( cd )  // Clear to end of screen
  {
    setCursor (FPoint{0, 0});
    appendOutputBuffer (FTermControl{cd});
    term_pos->setPoint(-1, -1);
  }
  else if ( cb )  // Clear to end of line
  {
    term_pos->setPoint(-1, -1);

    for (auto i{0}; i < int(getLineNumber()); i++)
    {
      setCursor (FPoint{0, i});
      appendOutputBuffer (FTermControl{cb});
    }

    setCursor (FPoint{0, 0});
  }

  flush();
  return true;
}

//----------------------------------------------------------------------
void FTermOutput::flush()
{
  // Flush the output buffer

  flushTimeAdjustment();

  if ( ! output_buffer || output_buffer->empty()
    || ! (isFlushTimeout() || getFVTerm().isTerminalUpdateForced()) )
    return;

  while ( ! output_buffer->empty() )
  {
    const auto& first = output_buffer->front();
    const auto& type = std::get<0>(first);
    const auto& str = std::get<1>(first);

    if ( type == OutputType::String )
    {
      static const FTerm::defaultPutChar& FTermPutchar = FTerm::putchar();

      if ( ! FTermPutchar )
        return;

      for (auto&& ch : str.wstring)
        FTermPutchar(int(ch));
    }
    else if ( type == OutputType::Control )
      FTerm::putstring (str.string);

    output_buffer->pop();
  }

  std::fflush(stdout);
  auto& mouse = FMouseControl::getInstance();
  mouse.drawPointer();
  time_last_flush = FObject::getCurrentTime();
}

//----------------------------------------------------------------------
void FTermOutput::beep() const
{
  return FTerm::beep();
}


// private methods of FTermOutput
//----------------------------------------------------------------------
inline FStartOptions& FTermOutput::getStartOptions()
{
  return FStartOptions::getInstance();
}

//----------------------------------------------------------------------
inline bool FTermOutput::isDefaultPaletteTheme()
{
  FStringList default_themes
  {
    "default8ColorPalette",
    "default16ColorPalette",
    "default16DarkColorPalette"
  };

  auto iter = std::find ( default_themes.begin()
                        , default_themes.end()
                        , FColorPalette::getInstance()->getClassName() );

  if ( iter == default_themes.end() )  // No default theme
    return false;

  return true;
}

//----------------------------------------------------------------------
void FTermOutput::redefineColorPalette()
{
  // Redefine the color palette

  if ( ! (canChangeColorPalette() && getStartOptions().color_change) )
    return;

  FTerm::resetColorMap();
  FTerm::saveColorMap();

  if ( FColorPalette::getInstance().use_count() > 0
    && ! isDefaultPaletteTheme() )
  {
    // A user color palette theme is in use
    FColorPalette::getInstance()->setColorPalette();
    return;
  }

  if ( getStartOptions().dark_theme )
  {
    setColorPaletteTheme<default16DarkColorPalette>();
  }
  else
  {
    if ( getMaxColor() >= 16 )
      setColorPaletteTheme<default16ColorPalette>();
    else  // 8 colors
      setColorPaletteTheme<default8ColorPalette>();
  }
}

//----------------------------------------------------------------------
void FTermOutput::restoreColorPalette()
{
  if ( ! (canChangeColorPalette() && getStartOptions().color_change) )
    return;

  // Reset screen settings
  FColorPalette::getInstance()->resetColorPalette();
  FTermXTerminal::getInstance().resetColorMap();
  FTerm::resetColorMap();
}

//----------------------------------------------------------------------
void FTermOutput::init_characterLengths()
{
  const auto& opti_move = FOptiMove::getInstance();
  cursor_address_length = opti_move.getCursorAddressLength();
  erase_char_length     = opti_move.getEraseCharsLength();
  repeat_char_length    = opti_move.getRepeatCharLength();
  clr_bol_length        = opti_move.getClrBolLength();
  clr_eol_length        = opti_move.getClrEolLength();

  if ( cursor_address_length == 0 )
    cursor_address_length = INT_MAX;

  if ( erase_char_length == 0 )
    erase_char_length = INT_MAX;

  if ( repeat_char_length == 0 )
    repeat_char_length = INT_MAX;

  if ( clr_bol_length == 0 )
    clr_bol_length = INT_MAX;

  if ( clr_eol_length == 0 )
    clr_eol_length = INT_MAX;
}

//----------------------------------------------------------------------
void FTermOutput::init_combined_character()
{
#if defined(__FreeBSD__) || defined(__DragonFly__) || defined(UNIT_TEST)
  if ( FTermFreeBSD::isFreeBSDConsole() )
    return;
#endif

  if ( FTerm::getEncoding() != Encoding::UTF8 )
    return;

  const auto& term_detection = FTermDetection::getInstance();

  if ( term_detection.isCygwinTerminal() )
    return;

  if ( term_detection.isXTerminal()
    || term_detection.isUrxvtTerminal()
    || term_detection.isMinttyTerm()
    || term_detection.isPuttyTerminal() )
  {
    combined_char_support = true;
  }
}

//----------------------------------------------------------------------
bool FTermOutput::canClearToEOL (uInt xmin, uInt y)
{
  // Is the line from xmin to the end of the line blank?
  // => clear to end of line

  auto& vt = vterm;
  const auto& ce = TCAP(t_clr_eol);
  const auto& min_char = vt->data[y * uInt(vt->width) + xmin];

  if ( ce && min_char.ch[0] == L' ' )
  {
    uInt beginning_whitespace = 1;
    const bool normal = FTerm::isNormal(min_char);
    const bool& ut = FTermcap::background_color_erase;

    for (uInt x = xmin + 1; x < uInt(vt->width); x++)
    {
      const auto& ch = vt->data[y * uInt(vt->width) + x];

      if ( min_char == ch )
        beginning_whitespace++;
      else
        break;
    }

    if ( beginning_whitespace == uInt(vt->width) - xmin
      && (ut || normal)
      && clr_eol_length < beginning_whitespace )
      return true;
  }

  return false;
}

//----------------------------------------------------------------------
bool FTermOutput::canClearLeadingWS (uInt& xmin, uInt y)
{
  // Line has leading whitespace
  // => clear from xmin to beginning of line

  auto& vt = vterm;
  const auto& cb = TCAP(t_clr_bol);
  const auto& first_char = vt->data[y * uInt(vt->width)];

  if ( cb && first_char.ch[0] == L' ' )
  {
    uInt leading_whitespace = 1;
    const bool normal = FTerm::isNormal(first_char);
    const bool& ut = FTermcap::background_color_erase;

    for (uInt x{1}; x < uInt(vt->width); x++)
    {
      const auto& ch = vt->data[y * uInt(vt->width) + x];

      if ( first_char == ch )
        leading_whitespace++;
      else
        break;
    }

    if ( leading_whitespace > xmin
      && (ut || normal)
      && clr_bol_length < leading_whitespace )
    {
      xmin = leading_whitespace - 1;
      return true;
    }
  }

  return false;
}

//----------------------------------------------------------------------
bool FTermOutput::canClearTrailingWS (uInt& xmax, uInt y)
{
  // Line has trailing whitespace
  // => clear from xmax to end of line

  auto& vt = vterm;
  const auto& ce = TCAP(t_clr_eol);
  const auto& last_char = vt->data[(y + 1) * uInt(vt->width) - 1];

  if ( ce && last_char.ch[0] == L' ' )
  {
    uInt trailing_whitespace = 1;
    const bool normal = FTerm::isNormal(last_char);
    const bool& ut = FTermcap::background_color_erase;

    for (uInt x = uInt(vt->width) - 1; x >  0 ; x--)
    {
      const auto& ch = vt->data[y * uInt(vt->width) + x];

      if ( last_char == ch )
        trailing_whitespace++;
      else
        break;
    }

    if ( trailing_whitespace > uInt(vt->width) - xmax
      && (ut || normal)
      && clr_bol_length < trailing_whitespace )
    {
      xmax = uInt(vt->width) - trailing_whitespace;
      return true;
    }
  }

  return false;
}

//----------------------------------------------------------------------
bool FTermOutput::skipUnchangedCharacters (uInt& x, uInt xmax, uInt y)
{
  // Skip characters without changes if it is faster than redrawing

  auto& vt = vterm;
  auto& print_char = vt->data[y * uInt(vt->width) + x];
  print_char.attr.bit.printed = true;

  if ( print_char.attr.bit.no_changes )
  {
    uInt count{1};

    for (uInt i = x + 1; i <= xmax; i++)
    {
      const auto& ch = vt->data[y * uInt(vt->width) + i];

      if ( ch.attr.bit.no_changes )
        count++;
      else
        break;
    }

    if ( count > cursor_address_length )
    {
      setCursor (FPoint{int(x + count), int(y)});
      x = x + count - 1;
      return true;
    }
  }

  return false;
}

//----------------------------------------------------------------------
void FTermOutput::printRange ( uInt xmin, uInt xmax, uInt y
                             , bool draw_trailing_ws )
{
  for (uInt x = xmin; x <= xmax; x++)
  {
    const auto& ec = TCAP(t_erase_chars);
    const auto& rp = TCAP(t_repeat_char);
    auto& print_char = vterm->data[y * uInt(vterm->width) + x];
    print_char.attr.bit.printed = true;
    replaceNonPrintableFullwidth (x, print_char);

    // skip character with no changes
    if ( skipUnchangedCharacters(x, xmax, y) )
      continue;

    // Erase character
    if ( ec && print_char.ch[0] == L' ' )
    {
      PrintState erase_state = \
          eraseCharacters(x, xmax, y, draw_trailing_ws);

      if ( erase_state == PrintState::LineCompletelyPrinted )
        break;
    }
    else if ( rp )  // Repeat one character n-fold
    {
      repeatCharacter(x, xmax, y);
    }
    else  // General character output
    {
      bool min_and_not_max( x == xmin && xmin != xmax );
      printCharacter (x, y, min_and_not_max, print_char);
    }
  }
}

//----------------------------------------------------------------------
inline void FTermOutput::replaceNonPrintableFullwidth ( uInt x
                                                      , FChar& print_char )
{
  // Replace non-printable full-width characters that are truncated
  // from the right or left terminal side

  if ( x == 0 && isFullWidthPaddingChar(print_char) )
  {
    print_char.ch[0] = wchar_t(UniChar::SingleLeftAngleQuotationMark);  // ‹
    print_char.ch[1] = L'\0';
    print_char.attr.bit.fullwidth_padding = false;
  }
  else if ( x == uInt(vterm->width - 1)
         && isFullWidthChar(print_char) )
  {
    print_char.ch[0] = wchar_t(UniChar::SingleRightAngleQuotationMark);  // ›
    print_char.ch[1] = L'\0';
    print_char.attr.bit.char_width = 1;
  }
}

//----------------------------------------------------------------------
void FTermOutput::printCharacter ( uInt& x, uInt y, bool min_and_not_max
                                 , FChar& print_char)
{
  // General character output on terminal

  if ( x < uInt(vterm->width - 1) && isFullWidthChar(print_char) )
  {
    printFullWidthCharacter (x, y, print_char);
  }
  else if ( x > 0 && x < uInt(vterm->width - 1)
         && isFullWidthPaddingChar(print_char)  )
  {
    printFullWidthPaddingCharacter (x, y, print_char);
  }
  else if ( x > 0 && min_and_not_max )
  {
    printHalfCovertFullWidthCharacter (x, y, print_char);
  }
  else
  {
    // Print a half-width character
    appendCharacter (print_char);
    markAsPrinted (x, y);
  }
}

//----------------------------------------------------------------------
void FTermOutput::printFullWidthCharacter ( uInt& x, uInt y
                                          , FChar& print_char )
{
  auto& next_char = vterm->data[y * uInt(vterm->width) + x + 1];

  if ( print_char.attr.byte[0] == next_char.attr.byte[0]
    && print_char.attr.byte[1] == next_char.attr.byte[1]
    && print_char.fg_color == next_char.fg_color
    && print_char.bg_color == next_char.bg_color
    && isFullWidthChar(print_char)
    && isFullWidthPaddingChar(next_char) )
  {
    // Print a full-width character
    appendCharacter (print_char);
    markAsPrinted (x, y);
    skipPaddingCharacter (x, y, print_char);
  }
  else
  {
    // Print ellipses for the 1st full-width character column
    appendAttributes (print_char);
    appendOutputBuffer (FTermChar{wchar_t(UniChar::HorizontalEllipsis)});
    term_pos->x_ref()++;
    markAsPrinted (x, y);

    if ( isFullWidthPaddingChar(next_char) )
    {
      // Print ellipses for the 2nd full-width character column
      x++;
      appendAttributes (next_char);
      appendOutputBuffer (FTermChar{wchar_t(UniChar::HorizontalEllipsis)});
      term_pos->x_ref()++;
      markAsPrinted (x, y);
    }
  }
}

//----------------------------------------------------------------------
void FTermOutput::printFullWidthPaddingCharacter ( uInt& x, uInt y
                                                 , FChar& print_char)
{
  auto& prev_char = vterm->data[y * uInt(vterm->width) + x - 1];

  if ( print_char.attr.byte[0] == prev_char.attr.byte[0]
    && print_char.attr.byte[1] == prev_char.attr.byte[1]
    && print_char.fg_color == prev_char.fg_color
    && print_char.bg_color == prev_char.bg_color
    && isFullWidthChar(prev_char)
    && isFullWidthPaddingChar(print_char) )
  {
    // Move cursor one character to the left
    const auto& le = TCAP(t_cursor_left);
    const auto& LE = TCAP(t_parm_left_cursor);

    if ( le )
      appendOutputBuffer (FTermControl{le});
    else if ( LE )
      appendOutputBuffer (FTermControl{FTermcap::encodeParameter(LE, 1)});
    else
    {
      skipPaddingCharacter (x, y, prev_char);
      return;
    }

    // Print a full-width character
    x--;
    term_pos->x_ref()--;
    appendCharacter (prev_char);
    markAsPrinted (x, y);
    skipPaddingCharacter (x, y, prev_char);
  }
  else
  {
    // Print ellipses for the 1st full-width character column
    appendAttributes (print_char);
    appendOutputBuffer (FTermChar{wchar_t(UniChar::HorizontalEllipsis)});
    term_pos->x_ref()++;
    markAsPrinted (x, y);
  }
}

//----------------------------------------------------------------------
void FTermOutput::printHalfCovertFullWidthCharacter ( uInt& x, uInt y
                                                    , FChar& print_char )
{
  auto& prev_char = vterm->data[y * uInt(vterm->width) + x - 1];

  if ( isFullWidthChar(prev_char) && ! isFullWidthPaddingChar(print_char) )
  {
    // Move cursor one character to the left
    const auto& le = TCAP(t_cursor_left);
    const auto& LE = TCAP(t_parm_left_cursor);

    if ( le )
      appendOutputBuffer (FTermControl{le});
    else if ( LE )
      appendOutputBuffer (FTermControl{FTermcap::encodeParameter(LE, 1)});

    if ( le || LE )
    {
      // Print ellipses for the 1st full-width character column
      x--;
      term_pos->x_ref()--;
      appendAttributes (prev_char);
      appendOutputBuffer (FTermChar{wchar_t(UniChar::HorizontalEllipsis)});
      term_pos->x_ref()++;
      markAsPrinted (x, y);
      x++;
    }
  }

  // Print a half-width character
  appendCharacter (print_char);
  markAsPrinted (x, y);
}

//----------------------------------------------------------------------
inline void FTermOutput::skipPaddingCharacter ( uInt& x, uInt y
                                              , const FChar& print_char )
{
  if ( isFullWidthChar(print_char) )  // full-width character
  {
    x++;  // Skip the following padding character
    term_pos->x_ref()++;
    markAsPrinted (x, y);
  }
}

//----------------------------------------------------------------------
FTermOutput::PrintState FTermOutput::eraseCharacters ( uInt& x, uInt xmax, uInt y
                                                     , bool draw_trailing_ws )
{
  // Erase a number of characters to draw simple whitespaces

  const auto& vt = vterm;
  const auto& ec = TCAP(t_erase_chars);
  auto& print_char = vt->data[y * uInt(vt->width) + x];

  if ( ! ec || print_char.ch[0] != L' ' )
    return PrintState::NothingPrinted;

  uInt whitespace{1};

  for (uInt i = x + 1; i <= xmax; i++)
  {
    const auto& ch = vt->data[y * uInt(vt->width) + i];

    if ( print_char == ch )
      whitespace++;
    else
      break;
  }

  if ( whitespace == 1 )
  {
    appendCharacter (print_char);
    markAsPrinted (x, y);
  }
  else
  {
    const uInt start_pos = x;
    const bool& ut = FTermcap::background_color_erase;
    const bool normal = FTerm::isNormal(print_char);

    if ( whitespace > erase_char_length + cursor_address_length
      && (ut || normal) )
    {
      appendAttributes (print_char);
      appendOutputBuffer (FTermControl{FTermcap::encodeParameter(ec, whitespace)});

      if ( x + whitespace - 1 < xmax || draw_trailing_ws )
        setCursor (FPoint{int(x + whitespace), int(y)});
      else
        return PrintState::LineCompletelyPrinted;

      x = x + whitespace - 1;
    }
    else
    {
      x--;

      for (uInt i{0}; i < whitespace; i++)
      {
        appendCharacter (print_char);
        x++;
      }
    }

    markAsPrinted (start_pos, x, y);
  }

  return PrintState::WhitespacesPrinted;
}

//----------------------------------------------------------------------
FTermOutput::PrintState FTermOutput::repeatCharacter (uInt& x, uInt xmax, uInt y)
{
  // Repeat one character n-fold

  const auto& vt = vterm;
  const auto& rp = TCAP(t_repeat_char);
  auto& print_char = vt->data[y * uInt(vt->width) + x];

  if ( ! rp )
    return PrintState::NothingPrinted;

  uInt repetitions{1};

  for (uInt i = x + 1; i <= xmax; i++)
  {
    const auto& ch = vt->data[y * uInt(vt->width) + i];

    if ( print_char == ch )
      repetitions++;
    else
      break;
  }

  if ( repetitions == 1 )
  {
    appendCharacter (print_char);
    markAsPrinted (x, y);
  }
  else
  {
    const uInt start_pos = x;

    if ( repetitions > repeat_char_length
      && is7bit(print_char.ch[0]) && print_char.ch[1] == L'\0' )
    {
      newFontChanges (print_char);
      charsetChanges (print_char);
      appendAttributes (print_char);
      appendOutputBuffer (FTermControl{FTermcap::encodeParameter(rp, print_char.ch[0], repetitions)});
      term_pos->x_ref() += int(repetitions);
      x = x + repetitions - 1;
    }
    else
    {
      x--;

      for (uInt i{0}; i < repetitions; i++)
      {
        appendCharacter (print_char);
        x++;
      }
    }

    markAsPrinted (start_pos, x, y);
  }

  return PrintState::RepeatCharacterPrinted;
}

//----------------------------------------------------------------------
inline bool FTermOutput::isFullWidthChar (const FChar& ch) const
{
  return ch.attr.bit.char_width == 2;
}

//----------------------------------------------------------------------
inline bool FTermOutput::isFullWidthPaddingChar (const FChar& ch) const
{
  return ch.attr.bit.fullwidth_padding;
}

//----------------------------------------------------------------------
void FTermOutput::cursorWrap() const
{
  // Wrap the cursor

  if ( term_pos->getX() >= vterm->width )
  {
    if ( term_pos->getY() == vterm->height - 1 )
      term_pos->x_ref()--;
    else
    {
      if ( FTermcap::eat_nl_glitch )
      {
        term_pos->setPoint(-1, -1);
      }
      else if ( FTermcap::automatic_right_margin )
      {
        term_pos->setX(0);
        term_pos->y_ref()++;
      }
      else
        term_pos->x_ref()--;
    }
  }
}

//----------------------------------------------------------------------
bool FTermOutput::updateTerminalLine (uInt y)
{
  // Updates pending changes from line y to the terminal

  bool ret{false};
  const auto& vt = vterm;
  uInt& xmin = vt->changes[y].xmin;
  uInt& xmax = vt->changes[y].xmax;

  if ( xmin <= xmax )  // Line has changes
  {
    ret = true;
    bool draw_leading_ws = false;
    bool draw_trailing_ws = false;
    const auto& ce = TCAP(t_clr_eol);

    // Clear rest of line
    bool is_eol_clean = canClearToEOL (xmin, y);

    if ( ! is_eol_clean )
    {
      // leading whitespace
      draw_leading_ws = canClearLeadingWS (xmin, y);

      // trailing whitespace
      draw_trailing_ws = canClearTrailingWS (xmax, y);
    }

    setCursor (FPoint{int(xmin), int(y)});

    if ( is_eol_clean )
    {
      auto& min_char = vt->data[y * uInt(vt->width) + xmin];
      appendAttributes (min_char);
      appendOutputBuffer (FTermControl{ce});
      markAsPrinted (xmin, uInt(vt->width - 1), y);
    }
    else
    {
      if ( draw_leading_ws )
      {
        const auto& cb = TCAP(t_clr_bol);
        auto& first_char = vt->data[y * uInt(vt->width)];
        appendAttributes (first_char);
        appendOutputBuffer (FTermControl{cb});
        markAsPrinted (0, xmin, y);
      }

      printRange (xmin, xmax, y, draw_trailing_ws);

      if ( draw_trailing_ws )
      {
        auto& last_char = vt->data[(y + 1) * uInt(vt->width) - 1];
        appendAttributes (last_char);
        appendOutputBuffer (FTermControl{ce});
        markAsPrinted (xmax + 1, uInt(vt->width - 1), y);
      }
    }

    // Reset line changes
    xmin = uInt(vt->width);
    xmax = 0;
  }

  cursorWrap();
  return ret;
}

//----------------------------------------------------------------------
bool FTermOutput::updateTerminalCursor()
{
  // Updates the input cursor visibility and the position
  if ( vterm && vterm->input_cursor_visible )
  {
    const int x = vterm->input_cursor_x;
    const int y = vterm->input_cursor_y;

    if ( getFVTerm().isInsideTerminal(FPoint{x, y}) )
    {
      setCursor (FPoint{x, y});
      showCursor();
      return true;
    }
  }
  else
    hideCursor();

  return false;
}

//----------------------------------------------------------------------
inline void FTermOutput::flushTimeAdjustment()
{
  const auto now = FObject::getCurrentTime();
  const auto diff = now - time_last_flush;

  if ( diff > milliseconds(400) )
  {
    flush_wait = MIN_FLUSH_WAIT;  // Reset to minimum values after 400 ms
    flush_average = MIN_FLUSH_WAIT;
    flush_median = MIN_FLUSH_WAIT;
  }
  else
  {
    auto usec = uInt64(duration_cast<microseconds>(diff).count());

    if ( usec < MIN_FLUSH_WAIT )
      usec = MIN_FLUSH_WAIT;
    else if ( usec > MAX_FLUSH_WAIT )
      usec = MAX_FLUSH_WAIT;

    if ( usec >= flush_average )
      flush_average += (usec - flush_average) / 10;
    else
    {
      uInt64 delta = (flush_average - usec) / 10;

      if ( flush_average >= delta )  // Avoid uInt64 underflow
        flush_average -= delta;
    }

    if ( usec >= flush_median )
      flush_median += flush_average / 5;
    else
    {
      uInt64 delta = flush_average / 5;

      if ( flush_median >= delta )  // Avoid uInt64 underflow
        flush_median -= delta;
    }

    flush_wait = flush_median;
  }
}

//----------------------------------------------------------------------
inline bool FTermOutput::isFlushTimeout() const
{
  return FObject::isTimeout (time_last_flush, flush_wait);
}

//----------------------------------------------------------------------
inline void FTermOutput::markAsPrinted (uInt pos, uInt line)
{
  // Marks a character as printed

  vterm->data[line * uInt(vterm->width) + pos].attr.bit.printed = true;
}

//----------------------------------------------------------------------
inline void FTermOutput::markAsPrinted (uInt from, uInt to, uInt line)
{
  // Marks characters in the specified range [from .. to] as printed

  for (uInt x = from; x <= to; x++)
    vterm->data[line * uInt(vterm->width) + x].attr.bit.printed = true;
}

//----------------------------------------------------------------------
inline void FTermOutput::newFontChanges (FChar& next_char)
{
  // NewFont special cases
  if ( ! FTerm::isNewFont() )
    return;

  if ( next_char.ch[0] == UniChar::LowerHalfBlock )
  {
    next_char.ch[0] = wchar_t(UniChar::UpperHalfBlock);
    next_char.attr.bit.reverse = true;
  }
  else if ( isReverseNewFontchar(next_char.ch[0]) )
    next_char.attr.bit.reverse = true;  // Show in reverse video
}

//----------------------------------------------------------------------
inline void FTermOutput::charsetChanges (FChar& next_char)
{
  const wchar_t& ch = next_char.ch[0];
  std::copy( next_char.ch.begin()
           , next_char.ch.end()
           , next_char.encoded_char.begin() );

  if ( FTerm::getEncoding() == Encoding::UTF8 )
    return;

  const wchar_t ch_enc = FTerm::charEncode(ch);

  if ( ch_enc == ch )
    return;

  if ( ch_enc == 0 )
  {
    next_char.encoded_char[0] = wchar_t(FTerm::charEncode(ch, Encoding::ASCII));
    return;
  }

  next_char.encoded_char[0] = ch_enc;

  if ( FTerm::getEncoding() == Encoding::VT100 )
    next_char.attr.bit.alt_charset = true;
  else if ( FTerm::getEncoding() == Encoding::PC )
  {
    next_char.attr.bit.pc_charset = true;

    if ( FTerm::isPuttyTerminal() )
      return;

    if ( FTerm::isXTerminal() && ch_enc < 0x20 )  // Character 0x00..0x1f
    {
      if ( FTerm::hasUTF8() )
        next_char.encoded_char[0] = int(FTerm::charEncode(ch, Encoding::ASCII));
      else
      {
        next_char.encoded_char[0] += 0x5f;
        next_char.attr.bit.alt_charset = true;
      }
    }
  }
}

//----------------------------------------------------------------------
inline void FTermOutput::appendCharacter (FChar& next_char)
{
  const int term_width = vterm->width - 1;
  const int term_height = vterm->height - 1;

  if ( term_pos->getX() == term_width
    && term_pos->getY() == term_height )
    appendLowerRight (next_char);
  else
    appendChar (next_char);

  term_pos->x_ref()++;
}

//----------------------------------------------------------------------
inline void FTermOutput::appendChar (FChar& next_char)
{
  newFontChanges (next_char);
  charsetChanges (next_char);
  appendAttributes (next_char);
  characterFilter (next_char);

  for (auto&& ch : next_char.encoded_char)
  {
    if ( ch != L'\0')
      appendOutputBuffer (FTermChar{ch});

    if ( ! combined_char_support )
      return;
  }
}

//----------------------------------------------------------------------
inline void FTermOutput::appendAttributes (FChar& next_attr)
{
  // generate attribute string for the next character
  const auto& attr_str = FTerm::changeAttribute (term_attribute, next_attr);

  if ( ! attr_str.empty() )
    appendOutputBuffer (FTermControl{attr_str});
}

//----------------------------------------------------------------------
void FTermOutput::appendLowerRight (FChar& last_char)
{
  const auto& SA = TCAP(t_enter_am_mode);
  const auto& RA = TCAP(t_exit_am_mode);

  if ( ! FTermcap::automatic_right_margin )
  {
    appendChar (last_char);
  }
  else if ( SA && RA )
  {
    appendOutputBuffer (FTermControl{RA});
    appendChar (last_char);
    appendOutputBuffer (FTermControl{SA});
  }
  else
  {
    const auto& IC = TCAP(t_parm_ich);
    const auto& im = TCAP(t_enter_insert_mode);
    const auto& ei = TCAP(t_exit_insert_mode);
    const auto& ip = TCAP(t_insert_padding);
    const auto& ic = TCAP(t_insert_character);

    const int x = int(getColumnNumber()) - 2;
    const int y = int(getLineNumber()) - 1;
    setCursor (FPoint{x, y});
    appendChar (last_char);
    term_pos->x_ref()++;

    setCursor (FPoint{x, y});
    FChar& second_last = *(&last_char - 1);

    if ( IC )
    {
      appendOutputBuffer (FTermControl{FTermcap::encodeParameter(IC, 1)});
      appendChar (second_last);
    }
    else if ( im && ei )
    {
      appendOutputBuffer (FTermControl{im});
      appendChar (second_last);

      if ( ip )
        appendOutputBuffer (FTermControl{ip});

      appendOutputBuffer (FTermControl{ei});
    }
    else if ( ic )
    {
      appendOutputBuffer (FTermControl{ic});
      appendChar (second_last);

      if ( ip )
        appendOutputBuffer (FTermControl{ip});
    }
  }
}

//----------------------------------------------------------------------
inline void FTermOutput::characterFilter (FChar& next_char)
{
  auto& sub_map = getFTerm().getCharSubstitutionMap();
  const auto& entry = sub_map[next_char.encoded_char[0]];

  if ( entry )
    next_char.encoded_char[0] = entry;
}

//----------------------------------------------------------------------
inline bool FTermOutput::isOutputBufferLimitReached() const
{
  return output_buffer->size() >= TERMINAL_OUTPUT_BUFFER_LIMIT;
}

//----------------------------------------------------------------------
inline void FTermOutput::appendOutputBuffer (const FTermControl& ctrl)
{
  output_buffer->emplace(std::make_tuple(OutputType::Control, TermString(ctrl.string)));

  if ( isOutputBufferLimitReached() )
    flush();
}

//----------------------------------------------------------------------
inline void FTermOutput::appendOutputBuffer (const FTermChar& c)
{
  if ( c.ch != L'\0' )
    appendOutputBuffer(FTermString{std::wstring(1, c.ch)});
}

//----------------------------------------------------------------------
void FTermOutput::appendOutputBuffer (const FTermString& str)
{
  if ( ! output_buffer->empty()
    && std::get<0>(output_buffer->back()) == OutputType::String )
  {
    // Append string data to the back element
    auto& string_buf = std::get<1>(output_buffer->back());
    std::transform ( str.string.begin()
                   , str.string.end()
                   , std::back_inserter(string_buf.wstring)
                   , [] (wchar_t ch)
                     {
                       return ch;
                     }
                   );
  }
  else
    output_buffer->emplace(std::make_tuple(OutputType::String, TermString(str.string)));

  if ( isOutputBufferLimitReached() )
    flush();
}

}  // namespace finalcut
