#define __MAIN_C__

/*
 * Inkscape - an ambitious vector drawing program
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Davide Puricelli <evo@debian.org>
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Masatake YAMATO  <jet@gyve.org>
 *   F.J.Franklin <F.J.Franklin@sheffield.ac.uk>
 *   Michael Meeks <michael@helixcode.com>
 *   Chema Celorio <chema@celorio.com>
 *   Pawel Palucha
 *   Bryce Harrington <bryce@bryceharrington.com>
 * ... and various people who have worked with various projects
 *
 * Copyright (C) 1999-2004 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "path-prefix.h"

#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif
#include <string.h>
#include <locale.h>

#include <popt.h>
#ifndef POPT_TABLEEND
#define POPT_TABLEEND { NULL, '\0', 0, 0, 0, NULL, NULL }
#endif /* Not def: POPT_TABLEEND */

#include <libxml/tree.h>
#include <glib-object.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkbox.h>

#include <gtk/gtkmain.h>

#include "gc-core.h"

#include "macros.h"
#include "file.h"
#include "document.h"
#include "desktop.h"
#include "sp-object.h"
#include "interface.h"
#include "print.h"
#include "slideshow.h"
#include "color.h"
#include "sp-item.h"
#include "unit-constants.h"

#include "svg/svg.h"

#include "inkscape-private.h"
#include "inkscape-stock.h"
#include "inkscape_version.h"

#include "sp-namedview.h"
#include "sp-guide.h"
#include "sp-object-repr.h"
#include "xml/repr.h"

#include "io/sys.h"

#include <extension/extension.h>
#include <extension/system.h>
#include <extension/db.h>

#ifdef WIN32
#include "extension/internal/win32.h"
#endif
#include "extension/init.h"

#include <glibmm/i18n.h>
#include <gtkmm/main.h>

#ifndef HAVE_BIND_TEXTDOMAIN_CODESET
#define bind_textdomain_codeset(p,c)
#endif
#ifndef HAVE_GTK_WINDOW_SET_DEFAULT_ICON_FROM_FILE
#define gtk_window_set_default_icon_from_file(f,v)
#endif

#include "application/application.h"

enum {
    SP_ARG_NONE,
    SP_ARG_NOGUI,
    SP_ARG_GUI,
    SP_ARG_FILE,
    SP_ARG_PRINT,
    SP_ARG_EXPORT_PNG,
    SP_ARG_EXPORT_DPI,
    SP_ARG_EXPORT_AREA,
    SP_ARG_EXPORT_WIDTH,
    SP_ARG_EXPORT_HEIGHT,
    SP_ARG_EXPORT_ID,
    SP_ARG_EXPORT_ID_ONLY,
    SP_ARG_EXPORT_USE_HINTS,
    SP_ARG_EXPORT_BACKGROUND,
    SP_ARG_EXPORT_BACKGROUND_OPACITY,
    SP_ARG_EXPORT_SVG,
    SP_ARG_SLIDESHOW,
    SP_ARG_BITMAP_ICONS,
    SP_ARG_VERSION,
    SP_ARG_NEW_GUI,
    SP_ARG_LAST
};

int sp_main_gui(int argc, char const **argv);
int sp_main_console(int argc, char const **argv);
static void sp_do_export_png(SPDocument *doc);


static gchar *sp_global_printer = NULL;
static gboolean sp_global_slideshow = FALSE;
static gchar *sp_export_png = NULL;
static gchar *sp_export_dpi = NULL;
static gchar *sp_export_area = NULL;
static gchar *sp_export_width = NULL;
static gchar *sp_export_height = NULL;
static gchar *sp_export_id = NULL;
static gchar *sp_export_background = NULL;
static gchar *sp_export_background_opacity = NULL;
static gboolean sp_export_use_hints = FALSE;
static gboolean sp_export_id_only = FALSE;
static gchar *sp_export_svg = NULL;
static int sp_new_gui = FALSE;

static GSList *sp_process_args(poptContext ctx);
struct poptOption options[] = {
    {"version", 'V', 
     POPT_ARG_NONE, NULL, SP_ARG_VERSION,
     N_("Print the Inkscape version number"), 
     NULL},

    {"without-gui", 'z', 
     POPT_ARG_NONE, NULL, SP_ARG_NOGUI,
     N_("Do not use X server (only process files from console)"),
     NULL},

    {"with-gui", 'g', 
     POPT_ARG_NONE, NULL, SP_ARG_GUI,
     N_("Try to use X server (even if $DISPLAY is not set)"),
     NULL},

    {"file", 'f', 
     POPT_ARG_STRING, NULL, SP_ARG_FILE,
     N_("Open specified document(s) (option string may be excluded)"),
     N_("FILENAME")},

    {"print", 'p', 
     POPT_ARG_STRING, &sp_global_printer, SP_ARG_PRINT,
     N_("Print document(s) to specified output file (use '| program' for pipe)"),
     N_("FILENAME")},

    {"export-png", 'e', 
     POPT_ARG_STRING, &sp_export_png, SP_ARG_EXPORT_PNG,
     N_("Export document to a PNG file"),
     N_("FILENAME")},

    {"export-dpi", 'd', 
     POPT_ARG_STRING, &sp_export_dpi, SP_ARG_EXPORT_DPI,
     N_("The resolution used for exporting SVG into bitmap (default 90)"),
     N_("DPI")},

    {"export-area", 'a', 
     POPT_ARG_STRING, &sp_export_area, SP_ARG_EXPORT_AREA,
     N_("Exported area in SVG pixels (default is full document; 0,0 is lower-left corner)"),
     N_("x0:y0:x1:y1")},

    {"export-width", 'w', 
     POPT_ARG_STRING, &sp_export_width, SP_ARG_EXPORT_WIDTH,
     N_("The width of exported bitmap in pixels (overrides export-dpi)"), 
     N_("WIDTH")},

    {"export-height", 'h', 
     POPT_ARG_STRING, &sp_export_height, SP_ARG_EXPORT_HEIGHT,
     N_("The height of exported bitmap in pixels (overrides export-dpi)"), 
     N_("HEIGHT")},

    {"export-id", 'i', 
     POPT_ARG_STRING, &sp_export_id, SP_ARG_EXPORT_ID,
     N_("The ID of the object to export (overrides export-area)"), 
     N_("ID")},

    {"export-id-only", 'j', 
     POPT_ARG_NONE, &sp_export_id_only, SP_ARG_EXPORT_ID_ONLY,
     // TRANSLATORS: this means: "Only export the object whose id is given in --export-id".
     //  See "man inkscape" for details.
     N_("Export just the object with export-id, hide all others (only with export-id)"), 
     N_("ID")},

    {"export-use-hints", 't', 
     POPT_ARG_NONE, &sp_export_use_hints, SP_ARG_EXPORT_USE_HINTS,
     N_("Use stored filename and DPI hints when exporting (only with export-id)"), 
     NULL},

    {"export-background", 'b', 
     POPT_ARG_STRING, &sp_export_background, SP_ARG_EXPORT_BACKGROUND,
     N_("Background color of exported bitmap (any SVG-supported color string)"), 
     N_("COLOR")},

    {"export-background-opacity", 'y', 
     POPT_ARG_STRING, &sp_export_background_opacity, SP_ARG_EXPORT_BACKGROUND_OPACITY,
     N_("Background opacity of exported bitmap (either 0.0 to 1.0, or 1 to 255)"), 
     N_("VALUE")},

    {"export-plain-svg", 'l', 
     POPT_ARG_STRING, &sp_export_svg, SP_ARG_EXPORT_SVG,
     N_("Export document to plain SVG file (no sodipodi or inkscape namespaces)"), 
     N_("FILENAME")},

    {"slideshow", 's', 
     POPT_ARG_NONE, &sp_global_slideshow, SP_ARG_SLIDESHOW,
     N_("Show given files one-by-one, switch to next on any key/mouse event"), 
     NULL},

    {"new-gui", 'G', 
     POPT_ARG_NONE, &sp_new_gui, SP_ARG_NEW_GUI,
     N_("Use the new Gtkmm GUI interface"),
     NULL},

    POPT_AUTOHELP POPT_TABLEEND
};

int
main(int argc, char **argv)
{
#ifdef HAVE_FPSETMASK
    /* This is inherited from Sodipodi code, where it was in #ifdef __FreeBSD__.  It's probably
       safe to remove: the default mask is already 0 in C99, and in current FreeBSD according to
       the fenv man page on www.freebsd.org, and in glibc according to (libc)FP Exceptions. */
    fpsetmask(fpgetmask() & ~(FP_X_DZ | FP_X_INV));
#endif

#ifdef ENABLE_NLS
#ifdef WIN32
    gchar *pathBuf = g_strconcat(g_path_get_dirname(argv[0]), "\\", PACKAGE_LOCALE_DIR, NULL);
    bindtextdomain(GETTEXT_PACKAGE, pathBuf);
    g_free(pathBuf);
#else
    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
#endif
#endif

    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

#ifdef ENABLE_NLS
    textdomain(GETTEXT_PACKAGE);
#endif

    LIBXML_TEST_VERSION

    Inkscape::GC::init();

    gboolean use_gui;
#ifndef WIN32
    use_gui = (getenv("DISPLAY") != NULL);
#else
    /*
      Set the current directory to the directory of the
      executable.  This seems redundant, but is needed for
      when inkscape.exe is executed from another directory.
      We use relative paths on win32.
      HKCR\svgfile\shell\open\command is a good example
    */
    // FIXME BROKEN - non-UTF-8 sneaks in here.
    char *homedir = g_path_get_dirname(argv[0]);
    SetCurrentDirectory(homedir);
    g_free(homedir);

    use_gui = TRUE;
#endif
    /* Test whether with/without GUI is forced */
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-z")
            || !strcmp(argv[i], "--without-gui")
            || !strcmp(argv[i], "-p")
            || !strncmp(argv[i], "--print", 7)
            || !strcmp(argv[i], "-e")
            || !strncmp(argv[i], "--export-png", 12)
            || !strcmp(argv[i], "-l")
            || !strncmp(argv[i], "--export-plain-svg", 12)
            || !strcmp(argv[i], "-i")
            || !strncmp(argv[i], "--export-id", 12)
           )
        {
            /* main_console handles any exports -- not the gui */
            use_gui = FALSE;
            break;
        } else if (!strcmp(argv[i], "-g") || !strcmp(argv[i], "--with-gui")) {
            use_gui = TRUE;
            break;
        } else if (!strcmp(argv[i], "-G") || !strcmp(argv[i], "--new-gui")) {
            sp_new_gui = TRUE;
            break;
        }
    }

    // TODO:  Should this be a static object (see inkscape.cpp)?
    Inkscape::NSApplication::Application app(argc, argv, use_gui, sp_new_gui);

    return app.run();
}

GSList *fixupFilenameEncoding( GSList* fl )
{
    GSList *newFl = NULL;
    int count = 0;
    while ( fl ) {
        gchar *fn = static_cast<gchar*>(fl->data);
        fl = g_slist_remove( fl, fl->data );

        if ( !g_utf8_validate(fn, -1, NULL) ) {
            g_message( "file #%d is not valid UTF-8", count);

            GError *error = NULL;
            gchar *newFileName = g_filename_to_utf8(fn, -1, NULL, NULL, &error);
            if ( newFileName ) {
                if ( !g_utf8_validate(newFileName, -1, NULL) ) {
                    g_warning( "input filename did not yield UTF-8 for #%d", count );
                }
                g_free( newFileName );
                newFileName = 0;
            } else {
                g_warning( "input filename conversion failed for #%d", count );
            }
        }
        newFl = g_slist_append( newFl, fn );

        count++;
    }
    return newFl;
}

int sp_common_main( int argc, char const **argv, GSList **flDest )
{
    /* fixme: Move these to some centralized location (Lauris) */
    sp_object_type_register("sodipodi:namedview", SP_TYPE_NAMEDVIEW);
    sp_object_type_register("sodipodi:guide", SP_TYPE_GUIDE);


    // temporarily switch gettext encoding to locale, so that help messages can be output properly
    gchar const *charset;
    g_get_charset(&charset);
    bind_textdomain_codeset(GETTEXT_PACKAGE, charset);

    poptContext ctx = poptGetContext(NULL, argc, argv, options, 0);
    poptSetOtherOptionHelp(ctx, _("[OPTIONS...] [FILE...]\n\nAvailable options:"));
    g_return_val_if_fail(ctx != NULL, 1);

    /* Collect own arguments */
    GSList *fl = sp_process_args(ctx);
    poptFreeContext(ctx);

    // now switch gettext back to UTF-8 (for GUI)
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

    // Now let's see if the file list still holds up
    fl = fixupFilenameEncoding( fl );

    if ( flDest ) {
        *flDest = fl;
        fl = 0;
    } else {
        while ( fl ) {
            g_free( fl->data );
            fl = g_slist_remove( fl, fl->data );
        }
    }
    return 0;
}

int
sp_main_gui(int argc, char const **argv)
{
    Gtk::Main main_instance (&argc, const_cast<char ***>(&argv));

    GSList *fl = NULL;
    int retVal = sp_common_main( argc, argv, &fl );
    g_return_val_if_fail(retVal == 0, 1);

    inkscape_gtk_stock_init();

    /* Set default icon */
    gchar *filename = (gchar *) g_build_filename (INKSCAPE_APPICONDIR, "inkscape.png", NULL);
    if (Inkscape::IO::file_test(filename, (GFileTest)(G_FILE_TEST_IS_REGULAR | G_FILE_TEST_IS_SYMLINK))) {
        gtk_window_set_default_icon_from_file(filename, NULL);
    }
    g_free (filename);
    filename = 0;

    if (!sp_global_slideshow) {
        gboolean create_new = TRUE;

        // FIXME BROKEN - non-UTF-8 sneaks in here.
        inkscape_application_init(argv[0], true);

        while (fl) {
            if (sp_file_open((gchar *)fl->data,NULL)) {
                create_new=FALSE;
            }
            fl = g_slist_remove(fl, fl->data);
        }
        if (create_new) {
            sp_file_new_default();
        }
    } else {
        if (fl) {
            GtkWidget *ss;
            // FIXME BROKEN - non-UTF-8 sneaks in here.
            inkscape_application_init(argv[0], true);
            ss = sp_slideshow_new(fl);
            if (ss) gtk_widget_show(ss);
        } else {
            g_warning ("No slides to display");
            exit(0);
        }
    }

    main_instance.run();

#ifdef WIN32
    //We might not need anything here
    //sp_win32_finish(); <-- this is a NOP func
#endif

    return 0;
}

int
sp_main_console(int argc, char const **argv)
{
    /* We are started in text mode */

    /* Do this g_type_init(), so that we can use Xft/Freetype2 (Pango)
     * in a non-Gtk environment.  Used in libnrtype's
     * FontInstance.cpp and FontFactory.cpp.
     * http://mail.gnome.org/archives/gtk-list/2003-December/msg00063.html
     */
    g_type_init();

    GSList *fl = NULL;
    int retVal = sp_common_main( argc, argv, &fl );
    g_return_val_if_fail(retVal == 0, 1);

    if (fl == NULL) {
        g_print("Nothing to do!\n");
        exit(0);
    }

    inkscape_application_init(argv[0], false);

    while (fl) {
        SPDocument *doc;

        doc = Inkscape::Extension::open(NULL, (gchar *)fl->data);
        if (doc == NULL) {
            doc = Inkscape::Extension::open(Inkscape::Extension::db.get(SP_MODULE_KEY_INPUT_SVG), (gchar *)fl->data);
        }
        if (doc == NULL) {
            g_warning("Specified document %s cannot be opened (is it valid SVG file?)", (gchar *) fl->data);
        } else {
            if (sp_global_printer) {
                sp_print_document_to_file(doc, sp_global_printer);
            }
            if (sp_export_png || sp_export_id) {
                sp_do_export_png(doc);
            }
            if (sp_export_svg) {
                Inkscape::XML::Document *rdoc;
                Inkscape::XML::Node *repr;
                rdoc = sp_repr_document_new("svg:svg");
                repr = rdoc->root();
                repr = sp_document_root(doc)->updateRepr(repr, SP_OBJECT_WRITE_BUILD);
                sp_repr_save_file(repr->document(), sp_export_svg, SP_SVG_NS_URI);
            }
        }
        fl = g_slist_remove(fl, fl->data);
    }

    inkscape_unref();

    return 0;
}

static void
sp_do_export_png(SPDocument *doc)
{
    const gchar *filename = NULL;
    gdouble dpi = 0.0;

    if (sp_export_use_hints && !sp_export_id) {
        g_warning ("--export-use-hints can only be used with --export-id; ignored.");
    }

    GSList *items = NULL;

    NRRect area;
    if (sp_export_id) {
        SPObject *o = doc->getObjectById(sp_export_id);
        if (o) {
            if (!SP_IS_ITEM (o)) {
                g_warning("Object with id=\"%s\" is not a visible item. Nothing exported.", sp_export_id);
                return;
            }
            if (sp_export_area) {
                g_warning ("Object with id=\"%s\" is being exported; --export-area is ignored.", sp_export_id);
            }

            items = g_slist_prepend (items, SP_ITEM(o));

            if (sp_export_id_only) {
                g_print("Exporting only object with id=\"%s\"; all other objects hidden\n", sp_export_id);
            }

            if (sp_export_use_hints) {

                // retrieve export filename hint
                const gchar *fn_hint = SP_OBJECT_REPR(o)->attribute("inkscape:export-filename");
                if (fn_hint) {
                    if (sp_export_png) {
                        g_warning ("Using export filename from the command line (--export-png). Filename hint %s is ignored.", fn_hint);
                        filename = sp_export_png;
                    } else {
                        filename = fn_hint;
                    }
                } else {
                    g_warning ("Export filename hint not found for the object.");
                    filename = sp_export_png;
                }

                // retrieve export dpi hints
                const gchar *dpi_hint = SP_OBJECT_REPR(o)->attribute("inkscape:export-xdpi"); // only xdpi, ydpi is always the same now
                if (dpi_hint) {
                    if (sp_export_dpi || sp_export_width || sp_export_height) {
                        g_warning ("Using bitmap dimensions from the command line (--export-dpi, --export-width, or --export-height). DPI hint %s is ignored.", dpi_hint);
                    } else {
                        dpi = atof(dpi_hint);
                    }
                } else {
                    g_warning ("Export DPI hint not found for the object.");
                }

            }

            // write object bbox to area
            sp_document_ensure_up_to_date (doc);
            sp_item_bbox_desktop((SPItem *) o, &area);
        } else {
            g_warning("Object with id=\"%s\" was not found in the document. Nothing exported.", sp_export_id);
            return;
        }
    } else if (sp_export_area) {
        /* Try to parse area (given in SVG pixels) */
        if (!sscanf(sp_export_area, "%lg:%lg:%lg:%lg", &area.x0, &area.y0, &area.x1, &area.y1) == 4) {
            g_warning("Cannot parse export area '%s'; use 'x0:y0:x1:y1'. Nothing exported.", sp_export_area);
            return;
        }
        if ((area.x0 >= area.x1) || (area.y0 >= area.y1)) {
            g_warning("Export area '%s' has negative width or height. Nothing exported.", sp_export_area);
            return;
        }
    } else {
        /* Export the whole document */
        area.x0 = 0.0;
        area.y0 = 0.0;
        area.x1 = sp_document_width(doc);
        area.y1 = sp_document_height(doc);
    }

    // set filename and dpi from options, if not yet set from the hints
    if (!filename) {
        if (!sp_export_png) {
            g_warning ("No export filename given and no filename hint. Nothing exported.");
            return;
        }
        filename = sp_export_png;
    }

    if (sp_export_dpi && dpi == 0.0) {
        dpi = atof(sp_export_dpi);
        if ((dpi < 0.1) || (dpi > 10000.0)) {
            g_warning("DPI value %s out of range [0.1 - 10000.0]. Nothing exported.", sp_export_dpi);
            return;
        }
        g_print("DPI: %g\n", dpi);
    } 

    // default dpi
    if (dpi == 0.0)
        dpi = PX_PER_IN;

    gint width = 0;
    gint height = 0;

    if (sp_export_width) {
        width = atoi(sp_export_width);
        if ((width < 1) || (width > 65536)) {
            g_warning("Export width %d out of range (1 - 65536). Nothing exported.", width);
            return;
        }
        dpi = (gdouble) width * PX_PER_IN / (area.x1 - area.x0);
    }

    if (sp_export_height) {
        height = atoi(sp_export_height);
        if ((height < 1) || (height > 65536)) {
            g_warning("Export height %d out of range (1 - 65536). Nothing exported.", width);
            return;
        }
        dpi = (gdouble) height * PX_PER_IN / (area.y1 - area.y0);
    }

    if (!sp_export_width) {
        width = (gint) ((area.x1 - area.x0) * dpi / PX_PER_IN + 0.5);
    }

    if (!sp_export_height) {
        height = (gint) ((area.y1 - area.y0) * dpi / PX_PER_IN + 0.5);
    }

    guint32 bgcolor = 0x00000000;
    if (sp_export_background) {
        // override the page color
        bgcolor = sp_svg_read_color(sp_export_background, 0xffffff00);
        bgcolor |= 0xff; // default is no opacity
    } else {
        // read from namedview
        Inkscape::XML::Node *nv = sp_repr_lookup_name (doc->rroot, "sodipodi:namedview");
        if (nv && nv->attribute("pagecolor"))
            bgcolor = sp_svg_read_color(nv->attribute("pagecolor"), 0xffffff00);
        if (nv && nv->attribute("inkscape:pageopacity"))
            bgcolor |= SP_COLOR_F_TO_U(sp_repr_get_double_attribute (nv, "inkscape:pageopacity", 1.0));
    }

    if (sp_export_background_opacity) {
        // override opacity
        gfloat value;
        if (sp_svg_number_read_f (sp_export_background_opacity, &value)) {
            if (value > 1.0) {
                value = CLAMP (value, 1.0f, 255.0f);
                bgcolor &= (guint32) 0xffffff00;
                bgcolor |= (guint32) floor(value);
            } else {
                value = CLAMP (value, 0.0f, 1.0f);
                bgcolor &= (guint32) 0xffffff00;
                bgcolor |= SP_COLOR_F_TO_U(value);
            }
        }
    }

    g_print("Background RRGGBBAA: %08x\n", bgcolor);

    g_print("Area %g:%g:%g:%g exported to %d x %d pixels (%g dpi)\n", area.x0, area.y0, area.x1, area.y1, width, height, dpi);

    g_print("Bitmap saved as: %s\n", filename);

    if ((width >= 1) && (height >= 1) && (width < 65536) && (height < 65536)) {
        sp_export_png_file(doc, filename, area.x0, area.y0, area.x1, area.y1, width, height, bgcolor, NULL, NULL, true, sp_export_id_only ? items : NULL);
    } else {
        g_warning("Calculated bitmap dimensions %d %d are out of range (1 - 65535). Nothing exported.", width, height);
    }

    g_slist_free (items);
}

static GSList *
sp_process_args(poptContext ctx)
{
    GSList *fl = NULL;

    gint a;
    while ((a = poptGetNextOpt(ctx)) >= 0) {
        switch (a) {
            case SP_ARG_FILE: {
                gchar const *fn = poptGetOptArg(ctx);
                if (fn != NULL) {
                    GError *error = NULL;
                    gchar *newFileName = NULL;
                    if ( g_utf8_validate(fn, -1, NULL) ) {
                        newFileName = g_strdup(fn);
                    } else {
                        newFileName = g_filename_to_utf8(fn, -1, NULL, NULL, &error);
                        if ( newFileName && !g_utf8_validate(newFileName, -1, NULL) ) {
                            g_warning( "input filename conversion failed" );
                        }
                    }
                    fl = g_slist_append(fl, ( (newFileName != NULL)
                                              ? newFileName
                                              : g_strdup(fn) ));
                }
                break;
            }
            case SP_ARG_VERSION: {
                printf("Inkscape %s (%s)\n", INKSCAPE_VERSION, __DATE__);
                exit(0);
                break;
            }
            default: {
                break;
            }
        }
    }

    gchar const ** const args = poptGetArgs(ctx);
    if (args != NULL) {
        for (unsigned i = 0; args[i] != NULL; i++) {
            // TODO: bulia, please look over
            gsize bytesRead = 0;
            gsize bytesWritten = 0;
            GError *error = NULL;
            // fixme: check to see if this will leak args[i]
            gchar *newFileName = g_filename_to_utf8(args[i],
                                                    -1,
                                                    &bytesRead,
                                                    &bytesWritten,
                                                    &error);
            fl = g_slist_append(fl, ( (newFileName != NULL)
                                      ? newFileName
                                      : (gpointer) args[i]));
        }
    }

    return fl;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
