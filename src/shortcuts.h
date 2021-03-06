// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * TODO: insert short description here
 *//*
 * Authors: see git history
 *
 * Copyright (C) 2017 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
#ifndef SEEN_SP_SHORTCUTS_H
#define SEEN_SP_SHORTCUTS_H

/*
 * Keyboard shortcut processing
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <vector>
#include <gdkmm/types.h>
#include <gdk/gdktypes.h>


typedef struct _GtkAccelGroup GtkAccelGroup;
typedef struct _GtkWidget     GtkWidget;

namespace Inkscape {
    class Verb;
    namespace UI {
        namespace View {
            class View;
        }
    }
}

/* We define high-bit mask for packing into single int */

#define SP_SHORTCUT_SHIFT_MASK GDK_MODIFIER_RESERVED_20_MASK
#define SP_SHORTCUT_CONTROL_MASK GDK_MODIFIER_RESERVED_21_MASK
#define SP_SHORTCUT_ALT_MASK GDK_MODIFIER_RESERVED_22_MASK
#define SP_SHORTCUT_SUPER_MASK GDK_MODIFIER_RESERVED_23_MASK
#define SP_SHORTCUT_HYPER_MASK GDK_MODIFIER_RESERVED_24_MASK
#define SP_SHORTCUT_META_MASK GDK_MODIFIER_RESERVED_25_MASK
#define SP_SHORTCUT_MODIFIER_MASK (SP_SHORTCUT_SHIFT_MASK|SP_SHORTCUT_CONTROL_MASK|SP_SHORTCUT_ALT_MASK|SP_SHORTCUT_SUPER_MASK|SP_SHORTCUT_HYPER_MASK|SP_SHORTCUT_META_MASK)


/* Returns true if action was performed */
bool sp_shortcut_invoke (unsigned int shortcut, Inkscape::UI::View::View *view);

void sp_shortcut_init();
Inkscape::Verb * sp_shortcut_get_verb (unsigned int shortcut);
unsigned int sp_shortcut_get_primary (Inkscape::Verb * verb); // Returns GDK_VoidSymbol if no shortcut is found.
char* sp_shortcut_get_label (unsigned int shortcut); // Returns the human readable form of the shortcut (or NULL), for example Shift+Ctrl+F. Free the returned string with g_free.
void sp_shortcut_set(unsigned int const shortcut, Inkscape::Verb *const verb, bool const is_primary, bool const is_user_set=false);
void sp_shortcut_unset(unsigned int const shortcut);
void sp_shortcut_add_to_file(char const *action, unsigned int const shortcut);
void sp_shortcut_delete_from_file(char const *action, unsigned int const shortcut);
void sp_shortcuts_delete_all_from_file();
Glib::ustring sp_shortcut_to_label(unsigned int const shortcut);
unsigned int sp_shortcut_get_from_event(GdkEventKey const *event);
unsigned int sp_shortcut_get_from_gdk_event(unsigned int accel_key, Gdk::ModifierType accel_mods, unsigned int hardware_keycode);
unsigned int sp_shortcut_get_for_event(GdkEventKey const *event);
void sp_shortcut_get_file_names(std::vector<Glib::ustring> *names, std::vector<Glib::ustring> *paths);
bool sp_shortcut_is_user_set(Inkscape::Verb *verb);
void sp_shortcut_file_export();
bool sp_shortcut_file_import();
void sp_shortcut_file_import_do(char const *importname);
void sp_shortcut_file_export_do(char const *exportname);
GtkAccelGroup *sp_shortcut_get_accel_group();
void sp_shortcut_add_accelerator(GtkWidget *item, unsigned int const shortcut);

#endif

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
