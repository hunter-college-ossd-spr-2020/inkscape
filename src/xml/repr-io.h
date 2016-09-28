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

Document* read_svg_file(const Glib::ustring& filename, const bool& is_internal = false, const Glib::ustring& default_ns = "");
Document* read_svg_buffer(const Glib::ustring& source, const bool& is_internal = false, const Glib::ustring& default_ns = "");
Glib::ustring save_svg_buffer(Document* doc, const Glib::ustring& default_ns = "",
                                     const Glib::ustring& old_base = "", const Glib::ustring& new_base = "");
bool save_svg_file(Document* doc, const Glib::ustring& filename, const Glib::ustring& default_ns = "",
                          const Glib::ustring& old_base = "", const Glib::ustring& new_base = "");

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
