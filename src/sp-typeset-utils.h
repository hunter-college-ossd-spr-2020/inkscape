#ifndef __sp_typeset_layout_utils_H__
#define __sp_typeset_layout_utils_H__

/*
 * layout utilities for the typeset element
 */

#include <config.h>

#include "style.h"
#include "attributes.h"

#include "sp-typeset.h"

#include "display/curve.h"
#include "livarot/Path.h"
#include "livarot/Ligne.h"
#include "livarot/Shape.h"
#include "livarot/LivarotDefs.h"

#include "libnrtype/FontInstance.h"

#include <pango/pango.h>
#include <glib.h>

class text_wrapper;

class dest_col_chunker : public dest_chunker  {
public:
  double     colWidth;
  
  dest_col_chunker(double iWidth);
  virtual ~dest_col_chunker(void);
  
  virtual box_solution   VeryFirst(void);
  virtual box_solution   NextLine(box_solution& after,double asc,double desc,double lead);
  virtual box_solution   NextBox(box_solution& after,double asc,double desc,double lead,bool &stillSameLine);
  virtual double         RemainingOnLine(box_solution& after);
};

class dest_box_chunker : public dest_chunker  {
public:
  int          nbBoxes;
  NR::Rect     *boxes;
  
  dest_box_chunker(void);
  virtual ~dest_box_chunker(void);
  
  void           AppendBox(const NR::Rect &iBox);
  
  virtual box_solution   VeryFirst(void);
  virtual box_solution   NextLine(box_solution& after,double asc,double desc,double lead);
  virtual box_solution   NextBox(box_solution& after,double asc,double desc,double lead,bool &stillSameLine);
  virtual double         RemainingOnLine(box_solution& after);
};


class dest_shape_chunker: public dest_chunker {
public:
	typedef struct cached_line {
		int          date;
		FloatLigne*  theLine;
		int          shNo;
		double       y,a,d;
	} cached_line;
  // data stored for each shape:
  typedef struct one_shape_elem {
    Shape*   theS;     // the shape itself
    double   l,t,r,b;  // the bounding box
    int      curPt;
    float    curY;
  } one_shape_elem;
  
	int                   nbShape,maxShape;
	one_shape_elem        *shapes;
  
	int                   lastDate; // date for the cache
	int                   maxCache,nbCache;
	cached_line*          caches;  
	FloatLigne*           tempLine;
	FloatLigne*           tempLine2;
  
  dest_shape_chunker(void);
  virtual ~dest_shape_chunker(void);
  
	void                  AppendShape(Shape* iShape,Shape* iExcl=NULL);
	void                  ComputeLine(float y,float a,float d,int shNo);
  
  virtual box_solution   VeryFirst(void);
  virtual box_solution   NextLine(box_solution& after,double asc,double desc,double lead);
  virtual box_solution   NextBox(box_solution& after,double asc,double desc,double lead,bool &stillSameLine);
  virtual double         RemainingOnLine(box_solution& after);
};

class dest_path_chunker: public dest_chunker {
public:
  typedef struct one_path_elem {
    Path*    theP;
    double   length;
  } one_path_elem;
  
	int                   nbPath,maxPath;
	one_path_elem         *paths;
  
  
  dest_path_chunker(void);
  virtual ~dest_path_chunker(void);
  
	void                  AppendPath(Path* iPath);
  
  virtual box_solution   VeryFirst(void);
  virtual box_solution   NextLine(box_solution& after,double asc,double desc,double lead);
  virtual box_solution   NextBox(box_solution& after,double asc,double desc,double lead,bool &stillSameLine);
  virtual double         RemainingOnLine(box_solution& after);
};


/*
 *
 */
enum {
  p_t_c_none        = 0,
  p_t_c_mergeWhite  = 1
};

class pango_text_chunker : public text_chunker {
public:
  typedef struct elem_box {
    double       x_pos,y_ascent,y_descent,x_length;
    int          t_first,t_last;
    bool         is_white,is_return,end_of_word;
  } elem_box;

  // source data
  text_wrapper* theText;
  bool            own_text;
  char*           theFace;
  double          theSize;

  // tempStuff
  int             textLength;
  double          baselineY;
  PangoAttrList*  theAttrs;
    
  elem_box*       words;
  int             nbWord,maxWord;
    
  pango_text_chunker(text_wrapper* inText,char* font_family,double font_size,int flags);
  virtual ~pango_text_chunker(void);
  
  virtual void                 SetText(char* inText,int flags);
  virtual int                  MaxIndex(void);
  
  virtual void                 InitialMetricsAt(int startPos,double &ascent,double &descent);
  virtual text_chunk_solution* StuffThatBox(int start_ind,double minLength,double nominalLength,double maxLength,bool strict);
  
  virtual void                 GlyphsAndPositions(int start_ind,int end_ind,to_SVG_context *hungry);
  virtual void                 GlyphsInfo(int start_ind,int end_ind,int &nbG,double &totLength);

  void                         SetTextWithAttrs(text_wrapper* inText,int flags);
  void                         AddDullGlyphs(to_SVG_context *hungry,double &cumul,int c_st,int c_en);
};


class box_to_SVG_context : public to_SVG_context{
public:
  double           box_y,box_start,box_end;
  
  double           cur_x,cur_y,cur_next,cur_spacing;
  double           cur_start,cur_by;
  
  double           *dxs;
  double           *dys;
  double           *xs;
  double           *ys;
  double           letter_spacing;
  text_wrapper		 *text;
  int              st,en,orig_st;
  
  SPRepr           *text_repr;
  
  SPCSSAttr        *style_repr;
  SPRepr           *span_repr;
  
  box_to_SVG_context(SPRepr* in_repr,double y,double x_start,double x_end);
  virtual ~box_to_SVG_context(void);
  
  virtual void            Finalize(void);
  
  virtual void            ResetStyle(void);
  virtual void            AddFontFamily(char* n_fam);
  virtual void            AddFontSize(double n_siz);
  virtual void            AddFontVariant(int n_var);
  virtual void            AddFontStretch(int n_str);
  virtual void            AddFontWeight(int n_wei);
  virtual void            AddLetterSpacing(double n_spc);
  
  virtual void            SetText(text_wrapper*  n_txt,int n_len);
  virtual void            SetLetterSpacing(double n_spc);
  
  void                    Flush(void);
  virtual void            AddGlyph(int a_g,int f_c,int l_c,const NR::Point &at,double advance);
  void                    SetY(int a_g,int f_c,int l_c,double to);
};

class path_to_SVG_context : public to_SVG_context{
public:
  Path*            thePath;
  double           path_length;
  double           path_delta;
  
  double           cur_x;
  
  double           letter_spacing;
  text_wrapper		 *text;
  int              st,en,orig_st;
  
  SPRepr           *text_repr;
  
  SPCSSAttr        *style_repr;
  SPRepr           *span_repr;
  
  path_to_SVG_context(SPRepr* in_repr,Path *iPath,double iLength,double iDelta);
  virtual ~path_to_SVG_context(void);
  
  virtual void            Finalize(void);
  
  virtual void            ResetStyle(void);
  virtual void            AddFontFamily(char* n_fam);
  virtual void            AddFontSize(double n_siz);
  virtual void            AddFontVariant(int n_var);
  virtual void            AddFontStretch(int n_str);
  virtual void            AddFontWeight(int n_wei);
  virtual void            AddLetterSpacing(double n_spc);
  
  virtual void            SetText(text_wrapper*  n_txt,int n_len);
  virtual void            SetLetterSpacing(double n_spc);
  
  virtual void            AddGlyph(int a_g,int f_c,int l_c,const NR::Point &at,double advance);
};

#endif

