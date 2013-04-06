#ifndef __SP_STRING_H__
#define __SP_STRING_H__

/*
 * string elements
 * extracted from sp-text
 */

#include <glibmm/ustring.h>

#include "sp-object.h"

#define SP_STRING(obj) ((SPString*)obj)
#define SP_IS_STRING(obj) (dynamic_cast<const SPString*>((SPObject*)obj))

class SPString : public SPObject, public CObject {
public:
	SPString();
	virtual ~SPString();

    Glib::ustring  string;

	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();

	virtual void read_content();

	virtual void update(SPCtx* ctx, unsigned int flags);
};

#endif
