/*
 * IO functions for svg files.
 *
 * Authors:
 *   Adrian Boguszewski
 *
 * Copyright (C) 2016 Adrian Boguszewski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_REPR_IO_H
#define INKSCAPE_REPR_IO_H

#include "svg-parser.h"
#include <glibmm/ustring.h>

namespace Inkscape {

namespace XML {

class Document;

class IO {
public:
    static Document* read_svg_file(const Glib::ustring& filename, const bool& isInternal = false, const Glib::ustring& defaultNs = "");
    static Document* read_svg_buffer(const Glib::ustring& source, const bool& isInternal = false, const Glib::ustring& defaultNs = "");

private:
    static SVGParser parser;
};

} // namespace XML

} // namespace Inkscape

#endif //INKSCAPE_REPR_IO_H

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
