/***********************************************************************
* flistbox.h - Widget FListBox and FListBoxItem                        *
*                                                                      *
* This file is part of the Final Cut widget toolkit                    *
*                                                                      *
* Copyright 2014-2018 Markus Gans                                      *
*                                                                      *
* The Final Cut is free software; you can redistribute it and/or       *
* modify it under the terms of the GNU Lesser General Public License   *
* as published by the Free Software Foundation; either version 3 of    *
* the License, or (at your option) any later version.                  *
*                                                                      *
* The Final Cut is distributed in the hope that it will be useful,     *
* but WITHOUT ANY WARRANTY; without even the implied warranty of       *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
* GNU Lesser General Public License for more details.                  *
*                                                                      *
* You should have received a copy of the GNU Lesser General Public     *
* License along with this program.  If not, see                        *
* <http://www.gnu.org/licenses/>.                                      *
***********************************************************************/

/*  Inheritance diagram
 *  ═══════════════════
 *
 * ▕▔▔▔▔▔▔▔▔▔▏
 * ▕  FTerm  ▏
 * ▕▁▁▁▁▁▁▁▁▁▏
 *      ▲
 *      │
 * ▕▔▔▔▔▔▔▔▔▔▏ ▕▔▔▔▔▔▔▔▔▔▏
 * ▕ FVTerm  ▏ ▕ FObject ▏
 * ▕▁▁▁▁▁▁▁▁▁▏ ▕▁▁▁▁▁▁▁▁▁▏
 *      ▲           ▲
 *      │           │
 *      └─────┬─────┘
 *            │
 *       ▕▔▔▔▔▔▔▔▔▔▏
 *       ▕ FWidget ▏
 *       ▕▁▁▁▁▁▁▁▁▁▏
 *            ▲
 *            │
 *       ▕▔▔▔▔▔▔▔▔▔▔▏1     *▕▔▔▔▔▔▔▔▔▔▔▔▔▔▔▏
 *       ▕ FListBox ▏- - - -▕ FListBoxItem ▏
 *       ▕▁▁▁▁▁▁▁▁▁▁▏       ▕▁▁▁▁▁▁▁▁▁▁▁▁▁▁▏
 *
 */

#ifndef FLISTBOX_H
#define FLISTBOX_H

#if !defined (USE_FINAL_H) && !defined (COMPILE_FINAL_CUT)
  #error "Only <final/final.h> can be included directly."
#endif

#include <vector>

#include "final/fscrollbar.h"
#include "final/fstring.h"
#include "final/fwidget.h"


//----------------------------------------------------------------------
// class FListBoxItem
//----------------------------------------------------------------------

#pragma pack(push)
#pragma pack(1)

class FListBoxItem
{
  public:
    // Constructors
    FListBoxItem ();
    FListBoxItem (const FListBoxItem&);  // copy constructor
    explicit FListBoxItem (const FString&, FWidget::data_ptr = 0);

    // Destructor
    virtual ~FListBoxItem();

    // Assignment operator (=)
    FListBoxItem& operator = (const FListBoxItem&);

    // Accessors
    virtual FString& getText();
    virtual FWidget::data_ptr getData() const;

    // Mutators
    void setText (const FString&);
    void setData (FWidget::data_ptr);

    // Methods
    void clear();

  private:
    // Friend classes
    friend class FListBox;

    // Data Members
    FString           text;
    FWidget::data_ptr data_pointer;
    fc::brackets_type brackets;
    bool              selected;
};
#pragma pack(pop)


// FListBoxItem inline functions
//----------------------------------------------------------------------
inline FString& FListBoxItem::getText()
{ return text; }

//----------------------------------------------------------------------
inline FWidget::data_ptr FListBoxItem::getData() const
{ return data_pointer; }

//----------------------------------------------------------------------
inline void FListBoxItem::setText (const FString& txt)
{ text = txt; }

//----------------------------------------------------------------------
inline void FListBoxItem::setData (FWidget::data_ptr data)
{ data_pointer = data; }

//----------------------------------------------------------------------
inline void FListBoxItem::clear()
{ text.clear(); }


//----------------------------------------------------------------------
// class FListBox
//----------------------------------------------------------------------

#pragma pack(push)
#pragma pack(1)

class FListBox : public FWidget
{
  public:
    // Typedef
    typedef std::vector<FListBoxItem> listBoxItems;

    // Using-declaration
    using FWidget::setGeometry;

    // Constructor
    explicit FListBox (FWidget* = 0);
    template <class Iterator, class InsertConverter>
    FListBox (Iterator, Iterator, InsertConverter, FWidget* = 0);
    template <class Container, class LazyConverter>
    FListBox (Container, LazyConverter, FWidget* = 0);

    // Destructor
    ~FListBox();

    // Accessors
    const char*  getClassName() const;
    uInt         getCount() const;
    FListBoxItem getItem (int);
    FListBoxItem getItem (listBoxItems::iterator) const;
    int          currentItem() const;
    FString&     getText();

    // Mutators
    void         setCurrentItem (int);
    void         setCurrentItem (listBoxItems::iterator);
    void         selectItem (int);
    void         selectItem (listBoxItems::iterator);
    void         unselectItem (int);
    void         unselectItem (listBoxItems::iterator);
    void         showInsideBrackets (int, fc::brackets_type);
    void         showNoBrackets (int);
    void         showNoBrackets (listBoxItems::iterator);
    void         setGeometry (int, int, int, int, bool = true);
    void         setMultiSelection (bool);
    void         setMultiSelection ();
    void         unsetMultiSelection ();
    bool         setDisable();
    bool         setFocus (bool);
    bool         setFocus();
    bool         unsetFocus();
    void         setText (const FString&);

    // Inquiries
    bool         isSelected (int);
    bool         isSelected (listBoxItems::iterator) const;
    bool         isMultiSelection() const;
    bool         hasBrackets (int);
    bool         hasBrackets (listBoxItems::iterator) const;

    // Methods
    void         hide();
    template <class Iterator, class InsertConverter>
    void         insert (Iterator, Iterator, InsertConverter);
    template <class Container, class LazyConverter>
    void         insert (Container, LazyConverter);
    void         insert (FListBoxItem);
    void         insert ( const FString&
                        , fc::brackets_type = fc::NoBrackets
                        , bool = false
                        , data_ptr = 0 );
    void         insert ( long
                        , fc::brackets_type = fc::NoBrackets
                        , bool = false
                        , data_ptr = 0 );
    void         remove (int);
    void         clear();

    // Event handlers
    void         onKeyPress (FKeyEvent*);
    void         onMouseDown (FMouseEvent*);
    void         onMouseUp (FMouseEvent*);
    void         onMouseMove (FMouseEvent*);
    void         onMouseDoubleClick (FMouseEvent*);
    void         onWheel (FWheelEvent*);
    void         onTimer (FTimerEvent*);
    void         onFocusIn (FFocusEvent*);
    void         onFocusOut (FFocusEvent*);


  protected:
    // Methods
    void adjustYOffset();
    void adjustSize();

  private:
    // Enumeration
    enum convert_type
    {
      no_convert     = 0,
      direct_convert = 1,
      lazy_convert   = 2
    };

    // Disable copy constructor
    FListBox (const FListBox&);

    // Disable assignment operator (=)
    FListBox& operator = (const FListBox&);

    // Accessors
    static FString& getString (listBoxItems::iterator);

    // Methods
    void        init();
    void        draw();
    void        drawLabel();
    void        drawList();
    void        drawListLine (int, listBoxItems::iterator, bool);
    void        printLeftBracket (fc::brackets_type);
    void        printRightBracket (fc::brackets_type);
    void        drawListBracketsLine (int, listBoxItems::iterator, bool);
    void        setLineAttributes (int, bool, bool, bool&);
    void        unsetAttributes();
    void        updateDrawing (bool, bool);
    void        recalculateHorizontalBar (int, bool);
    void        recalculateVerticalBar (int);
    void        getWidgetFocus();
    void        multiSelection (int);
    void        multiSelectionUpTo (int);
    void        wheelUp (int);
    void        wheelDown (int);
    bool        dragScrollUp();
    bool        dragScrollDown();
    void        dragUp (int);
    void        dragDown (int);
    void        stopDragScroll();
    void        prevListItem (int);
    void        nextListItem (int);
    void        scrollToX (int);
    void        scrollToY (int);
    void        scrollLeft (int);
    void        scrollRight (int);
    void        keyUp();
    void        keyDown();
    void        keyLeft();
    void        keyRight();
    void        keyPgUp();
    void        keyPgDn();
    void        keyHome();
    void        keyEnd();
    bool        keyEsc();
    void        keyEnter();
    bool        keySpace();
    bool        keyInsert();
    bool        keyBackspace();
    bool        keyIncSearchInput (int);
    void        processClick();
    void        processSelect();
    void        processChanged();
    void        lazyConvert (listBoxItems::iterator, int);
    listBoxItems::iterator index2iterator (int);

    // Callback methods
    void         cb_VBarChange (FWidget*, data_ptr);
    void         cb_HBarChange (FWidget*, data_ptr);

    // Function Pointer
    void        (*convertToItem) ( FListBoxItem&
                                 , FWidget::data_ptr
                                 , int index );

    // Data Members
    listBoxItems      itemlist;
    FWidget::data_ptr source_container;
    convert_type      conv_type;
    FScrollbar*       vbar;
    FScrollbar*       hbar;
    FString           text;
    FString           inc_search;
    bool              multi_select;
    bool              mouse_select;
    fc::dragScroll    drag_scroll;
    bool              scroll_timer;
    int               scroll_repeat;
    int               scroll_distance;
    int               current;
    int               last_current;
    int               secect_from_item;
    int               xoffset;
    int               yoffset;
    int               last_yoffset;
    int               nf_offset;
    int               max_line_width;
};
#pragma pack(pop)


// FListBox inline functions
//----------------------------------------------------------------------
template <class Iterator, class InsertConverter>
inline FListBox::FListBox ( Iterator first
                          , Iterator last
                          , InsertConverter convert
                          , FWidget* parent )
  : FWidget(parent)
  , convertToItem(0)
  , itemlist()
  , source_container(0)
  , conv_type(FListBox::no_convert)
  , vbar(0)
  , hbar(0)
  , text()
  , inc_search()
  , multi_select(false)
  , mouse_select(false)
  , drag_scroll(fc::noScroll)
  , scroll_timer(false)
  , scroll_repeat(100)
  , scroll_distance(1)
  , current(0)
  , last_current(-1)
  , secect_from_item(-1)
  , xoffset(0)
  , yoffset(0)
  , last_yoffset(-1)
  , nf_offset(0)
  , max_line_width(0)
{
  init();

  while ( first != last )
  {
    insert(convert(first), fc::NoBrackets, false, &(*first));
    ++first;
  }
}

//----------------------------------------------------------------------
template <class Container, class LazyConverter>
inline FListBox::FListBox ( Container container
                          , LazyConverter convert
                          , FWidget* parent )
  : FWidget(parent)
  , convertToItem(0)
  , itemlist()
  , source_container(0)
  , conv_type(FListBox::no_convert)
  , vbar(0)
  , hbar(0)
  , text()
  , inc_search()
  , multi_select(false)
  , mouse_select(false)
  , drag_scroll(fc::noScroll)
  , scroll_timer(false)
  , scroll_repeat(100)
  , scroll_distance(1)
  , current(0)
  , last_current(-1)
  , secect_from_item(-1)
  , xoffset(0)
  , yoffset(0)
  , last_yoffset(-1)
  , nf_offset(0)
  , max_line_width(0)
{
  init();
  insert (container, convert);
}

//----------------------------------------------------------------------
inline const char* FListBox::getClassName() const
{ return "FListBox"; }

//----------------------------------------------------------------------
inline uInt FListBox::getCount() const
{ return uInt(itemlist.size()); }

//----------------------------------------------------------------------
inline FListBoxItem FListBox::getItem (int index)
{
  listBoxItems::iterator iter = index2iterator(index - 1);
  return *iter;
}

//----------------------------------------------------------------------
inline FListBoxItem FListBox::getItem (listBoxItems::iterator iter) const
{ return *iter; }

//----------------------------------------------------------------------
inline int FListBox::currentItem() const
{ return current; }

//----------------------------------------------------------------------
inline FString& FListBox::getText()
{ return text; }

//----------------------------------------------------------------------
inline void FListBox::selectItem (int index)
{ index2iterator(index - 1)->selected = true; }

//----------------------------------------------------------------------
inline void FListBox::selectItem (listBoxItems::iterator iter)
{ iter->selected = true; }

//----------------------------------------------------------------------
inline void FListBox::unselectItem (int index)
{ index2iterator(index - 1)->selected = false; }

//----------------------------------------------------------------------
inline void FListBox::unselectItem (listBoxItems::iterator iter)
{ iter->selected = false; }

//----------------------------------------------------------------------
inline void FListBox::showNoBrackets (int index)
{ index2iterator(index - 1)->brackets = fc::NoBrackets; }

//----------------------------------------------------------------------
inline void FListBox::showNoBrackets (listBoxItems::iterator iter)
{ iter->brackets = fc::NoBrackets; }

//----------------------------------------------------------------------
inline void FListBox::setMultiSelection (bool on)
{ multi_select = on; }

//----------------------------------------------------------------------
inline void FListBox::setMultiSelection()
{ setMultiSelection(true); }

//----------------------------------------------------------------------
inline void FListBox::unsetMultiSelection()
{ setMultiSelection(false); }

//----------------------------------------------------------------------
inline bool FListBox::setDisable()
{ return setEnable(false); }

//----------------------------------------------------------------------
inline bool FListBox::setFocus()
{ return setFocus(true); }

//----------------------------------------------------------------------
inline bool FListBox::unsetFocus()
{ return setFocus(false); }

//----------------------------------------------------------------------
inline bool FListBox::isSelected (int index)
{ return index2iterator(index - 1)->selected; }

//----------------------------------------------------------------------
inline bool FListBox::isSelected (listBoxItems::iterator iter) const
{ return iter->selected; }

//----------------------------------------------------------------------
inline bool FListBox::isMultiSelection() const
{ return multi_select; }

//----------------------------------------------------------------------
inline bool FListBox::hasBrackets(int index)
{ return bool(index2iterator(index - 1)->brackets > 0); }

//----------------------------------------------------------------------
inline bool FListBox::hasBrackets(listBoxItems::iterator iter) const
{ return bool(iter->brackets > 0); }

//----------------------------------------------------------------------
template <class Iterator, class InsertConverter>
inline void FListBox::insert ( Iterator first
                             , Iterator last
                             , InsertConverter convert )
{
  conv_type = direct_convert;

  while ( first != last )
  {
    insert (convert(first), fc::NoBrackets, false, &(*first));
    ++first;
  }
}

//----------------------------------------------------------------------
template <class Container, class LazyConverter>
void FListBox::insert (Container container, LazyConverter convert)
{
  conv_type = lazy_convert;
  source_container = container;
  convertToItem = convert;
  std::size_t size = container->size();

  if ( size > 0 )
    itemlist.resize(size);

  recalculateVerticalBar(int(size));
}

//----------------------------------------------------------------------
inline FListBox::listBoxItems::iterator FListBox::index2iterator (int index)
{
  listBoxItems::iterator iter = itemlist.begin();
  std::advance (iter, index);
  return iter;
}

#endif  // FLISTBOX_H
