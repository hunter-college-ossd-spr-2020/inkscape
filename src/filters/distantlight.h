#ifndef SP_FEDISTANTLIGHT_H_SEEN
#define SP_FEDISTANTLIGHT_H_SEEN

/** \file
 * SVG <filter> implementation, see sp-filter.cpp.
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *   Jean-Rene Reinhard <jr@komite.net>
 *
 * Copyright (C) 2006,2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

#define SP_FEDISTANTLIGHT(obj) ((SPFeDistantLight*)obj)
#define SP_IS_FEDISTANTLIGHT(obj) (dynamic_cast<const SPFeDistantLight*>((SPObject*)obj))

/* Distant light class */
class SPFeDistantLight : public SPObject, public CObject {
public:
	SPFeDistantLight();
	virtual ~SPFeDistantLight();

    /** azimuth attribute */
    gfloat azimuth;
    guint azimuth_set : 1;
    /** elevation attribute */
    gfloat elevation;
    guint elevation_set : 1;

	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();

	virtual void set(unsigned int key, const gchar* value);

	virtual void update(SPCtx* ctx, unsigned int flags);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, guint flags);
};

#endif /* !SP_FEDISTANTLIGHT_H_SEEN */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
