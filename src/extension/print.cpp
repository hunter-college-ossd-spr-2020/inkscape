/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "implementation/implementation.h"
#include "print.h"

/* Inkscape::Extension::Print */

namespace Inkscape {
namespace Extension {

Print::Print (SPRepr * in_repr, Implementation::Implementation * in_imp) : Extension(in_repr, in_imp)
{
    base = NULL;
    arena = NULL;
    root = NULL;
    dkey = 0;

    return;
}

Print::~Print (void)
{
    return;
}

bool
Print::check (void)
{
	return Extension::check();
}

unsigned int
Print::setup (void)
{
    return imp->setup(this);
}

unsigned int
Print::set_preview (void)
{
    return imp->set_preview(this);
}

unsigned int
Print::begin (SPDocument *doc)
{
    return imp->begin(this, doc);
}

unsigned int
Print::finish (void)
{
    return imp->finish(this);
}

unsigned int
Print::bind (const NRMatrix *transform, float opacity)
{
    return imp->bind (this, transform, opacity);
}

unsigned int
Print::release (void)
{
    return imp->release(this);
}

unsigned int
Print::fill (const NRBPath *bpath, const NRMatrix *ctm, const SPStyle *style,
                   const NRRect *pbox, const NRRect *dbox, const NRRect *bbox)
{
    return imp->fill (this, bpath, ctm, style, pbox, dbox, bbox);
}

unsigned int
Print::stroke (const NRBPath *bpath, const NRMatrix *transform, const SPStyle *style,
                 const NRRect *pbox, const NRRect *dbox, const NRRect *bbox)
{
    return imp->stroke (this, bpath, transform, style, pbox, dbox, bbox);
}

unsigned int
Print::image (unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
                const NRMatrix *transform, const SPStyle *style)
{
    return imp->image (this, px, w, h, rs, transform, style);
}

unsigned int
Print::text (const char* text, NR::Point p, const SPStyle* style)
{
    return imp->text (this, text, p, style);
}

bool
Print::textToPath (void)
{
	return imp->textToPath(this);
}

} }  /* namespace Inkscape, Extension */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
