// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * @brief Dialog for naming calligraphic profiles
 */
/* Author:
 *   Aubanel MONNIER
 *
 * Copyright (C) 2007 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_DIALOG_CALLIGRAPHIC_PROFILE_H
#define INKSCAPE_DIALOG_CALLIGRAPHIC_PROFILE_H

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>

namespace Gtk {
class Grid;
}

class SPDesktop;

namespace Inkscape {
namespace UI {
namespace Dialog {
      
class CalligraphicProfileRename : public Gtk::Dialog {  
public:
    CalligraphicProfileRename();
    ~CalligraphicProfileRename() override = default;
    Glib::ustring getName() const {
        return "CalligraphicProfileRename";
    }
    
    static void show(SPDesktop *desktop, const Glib::ustring profile_name);
    static bool applied() {
        return instance()._applied;
    }
    static bool deleted() {
        return instance()._deleted;
    }
    static Glib::ustring getProfileName() {
        return instance()._profile_name;
    }

protected:
    void _close();
    void _apply();
    void _delete();

    Gtk::Label        _profile_name_label;
    Gtk::Entry        _profile_name_entry;
    Gtk::Grid*        _layout_table;

    Gtk::Button       _close_button;
    Gtk::Button       _delete_button;
    Gtk::Button       _apply_button;
    Glib::ustring _profile_name;
    bool _applied;
    bool _deleted;
private:
    static CalligraphicProfileRename &instance() {
        static CalligraphicProfileRename instance_;
        return instance_;
    }
    CalligraphicProfileRename(CalligraphicProfileRename const &) = delete; // no copy
    CalligraphicProfileRename &operator=(CalligraphicProfileRename const &) = delete; // no assign
};
 
} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_DIALOG_CALLIGRAPHIC_PROFILE_H

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
