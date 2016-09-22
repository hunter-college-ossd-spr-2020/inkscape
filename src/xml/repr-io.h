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

#include <glibmm/ustring.h>

namespace Inkscape {

namespace XML {

class Document;

Document* read_svg_file(const Glib::ustring& filename, const bool& isInternal = false, const Glib::ustring& defaultNs = "");
Document* read_svg_buffer(const Glib::ustring& source, const bool& isInternal = false, const Glib::ustring& defaultNs = "");
Glib::ustring save_svg_buffer(Document* doc, const Glib::ustring& defaultNs = "",
                                     const Glib::ustring& oldBase = "", const Glib::ustring& newBase = "");
bool save_svg_file(Document* doc, const Glib::ustring& filename, const Glib::ustring& defaultNs = "",
                          const Glib::ustring& oldBase = "", const Glib::ustring& newBase = "");

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
