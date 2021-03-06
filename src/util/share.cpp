// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Inkscape::Util::ptr_shared<T> - like T const *, but stronger.
 * Used to hold c-style strings for objects that are managed by the gc.
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2006 MenTaLguY
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "util/share.h"
#include <glib.h>

namespace Inkscape {
namespace Util {

ptr_shared share_string(char const *string) {
    g_return_val_if_fail(string != nullptr, share_unsafe(nullptr));
    return share_string(string, std::strlen(string));
}

ptr_shared share_string(char const *string, std::size_t length) {
    g_return_val_if_fail(string != nullptr, share_unsafe(nullptr));
    char *new_string=new (GC::ATOMIC) char[length+1];
    std::memcpy(new_string, string, length);
    new_string[length] = 0;
    return share_unsafe(new_string);
}

}
}

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
