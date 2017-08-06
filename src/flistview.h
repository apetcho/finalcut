// File: flistview.h
// Provides: class FListViewItem
//           class FListView
//
//  Inheritance diagram
//  ═══════════════════
//
// ▕▔▔▔▔▔▔▔▔▔▏ ▕▔▔▔▔▔▔▔▔▔▏
// ▕ FObject ▏ ▕  FTerm  ▏
// ▕▁▁▁▁▁▁▁▁▁▏ ▕▁▁▁▁▁▁▁▁▁▏
//      ▲           ▲
//      │           │
//      └─────┬─────┘
//            │
//       ▕▔▔▔▔▔▔▔▔▏
//       ▕ FVTerm ▏
//       ▕▁▁▁▁▁▁▁▁▏
//            ▲
//            │
//       ▕▔▔▔▔▔▔▔▔▔▏
//       ▕ FWidget ▏
//       ▕▁▁▁▁▁▁▁▁▁▏
//            ▲
//            │
//      ▕▔▔▔▔▔▔▔▔▔▔▔▏1     *▕▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▏
//      ▕ FListView ▏- - - -▕ FListViewItem ▏
//      ▕▁▁▁▁▁▁▁▁▁▁▁▏       ▕▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▏
//

#ifndef FLISTVIEW_H
#define FLISTVIEW_H

#include "fscrollbar.h"
#include "fstring.h"
#include "ftermbuffer.h"
#include "fwidget.h"

// class forward declaration
class FListView;

//----------------------------------------------------------------------
// class FListViewItem
//----------------------------------------------------------------------

#pragma pack(push)
#pragma pack(1)

class FListViewItem : public FObject
{
 public:
   FListViewItem (const FListViewItem&);  // copy constructor
   FListViewItem (FListViewItem*);
   FListViewItem (FListView*);
   FListViewItem ( const std::vector<FString>&
                 , FWidget::data_ptr = 0
                 , FListView* = 0);

  // Destructor
  ~FListViewItem();

   // Assignment operator (=)
   FListViewItem& operator = (const FListViewItem&);

   // Accessors
   const char*  getClassName() const;
   uInt         getCount() const;
   FString      getText (int) const;

   // Mutator
   void         setText (int, const FString&);

 private:
   // Data Member
   std::vector<FString> column_line;
   FWidget::data_ptr    data_pointer;

   // Friend class
   friend class FListView;
};
#pragma pack(pop)


// FListViewItem inline functions
//----------------------------------------------------------------------
inline const char* FListViewItem::getClassName() const
{ return "FListViewItem"; }

//----------------------------------------------------------------------
inline uInt FListViewItem::getCount() const
{ return uInt(column_line.size()); }


//----------------------------------------------------------------------
// class FListView
//----------------------------------------------------------------------

#pragma pack(push)
#pragma pack(1)

class FListView : public FWidget
{
 public:
   // Typedef
   typedef std::vector<FListViewItem*> listViewItems;

   // Using-declaration
   using FWidget::setGeometry;

   // Constructor
   explicit FListView (FWidget* = 0);

   // Destructor
  ~FListView();

   // Accessors
   const char*        getClassName() const;
   fc::text_alignment getColumnAlignment (int) const;
   FString            getColumnText (int) const;
   FListViewItem*     getCurrentItem() const;

   // Mutators
   void               setGeometry (int, int, int, int, bool = true);
   void               setColumnAlignment (int, fc::text_alignment);
   void               setColumnText (int, const FString&);

   // Methods
   virtual int        addColumn (const FString&, int = USE_MAX_SIZE);
   void               insert (FListViewItem*);
   void               insert ( const std::vector<FString>&
                             , data_ptr = 0
                             , FListView* = 0 );
   void               insert ( const std::vector<long>&
                             , data_ptr = 0
                             , FListView* = 0 );

   // Event handlers
   void               onKeyPress (FKeyEvent*);
   void               onMouseDown (FMouseEvent*);
   void               onMouseUp (FMouseEvent*);
   void               onMouseMove (FMouseEvent*);
   void               onMouseDoubleClick (FMouseEvent*);
   void               onWheel (FWheelEvent*);
   void               onTimer (FTimerEvent*);
   void               onFocusIn (FFocusEvent*);
   void               onFocusOut (FFocusEvent*);

 protected:
   // Methods
   void               adjustYOffset();
   void               adjustSize();

 private:
   // Typedef
   struct Header
   {
    public:
      Header()
      : name()
      , width (0)
      , fixed_width (false)
      , alignment (fc::alignLeft)
      { }

     ~Header()
      { }

      FString name;
      int width;
      bool fixed_width;
      fc::text_alignment alignment;
   };

   typedef std::vector<Header> headerItems;

   // Constants
   static const int USE_MAX_SIZE = -1;

   // Disable copy constructor
   FListView (const FListView&);

   // Disable assignment operator (=)
   FListView& operator = (const FListView&);

   // Methods
   void               init();
   uInt               getAlignOffset (fc::text_alignment, uInt, uInt);
   void               draw();
   void               drawColumnLabels();
   void               drawList();
   void               recalculateHorizontalBar (int);
   void               recalculateVerticalBar (int);
   void               processClick();
   void               processChanged();
   listViewItems::iterator index2iterator (int);

   // Callback methods
   void               cb_VBarChange (FWidget*, data_ptr);
   void               cb_HBarChange (FWidget*, data_ptr);

   // Data Members
   listViewItems      data;
   headerItems        header;
   FTermBuffer        headerline;
   FScrollbar*        vbar;
   FScrollbar*        hbar;
   fc::dragScroll     drag_scroll;
   bool               scroll_timer;
   int                scroll_repeat;
   int                scroll_distance;
   int                current;
   int                xoffset;
   int                yoffset;
   int                nf_offset;
   int                max_line_width;

   // Friend class
   friend class FListViewItem;
};
#pragma pack(pop)


// FListView inline functions
//----------------------------------------------------------------------
inline const char* FListView::getClassName() const
{ return "FListView"; }

//----------------------------------------------------------------------
inline FListViewItem* FListView::getCurrentItem() const
{ return data[uInt(current-1)]; }

//----------------------------------------------------------------------
inline FListView::listViewItems::iterator FListView::index2iterator (int index)
{
  listViewItems::iterator iter = data.begin();
  std::advance (iter, index);
  return iter;
}

#endif  // FLISTVIEW_H
