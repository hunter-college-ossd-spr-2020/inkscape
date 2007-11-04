#ifndef INKSCAPE_LPE_PATHALONGPATH_H
#define INKSCAPE_LPE_PATHALONGPATH_H

/*
 * Inkscape::LPEPathAlongPath
 *
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/path.h"
#include "live_effects/parameter/enum.h"
#include "live_effects/parameter/bool.h"

namespace Inkscape {
namespace LivePathEffect {

enum PAPCopyType {
    PAPCT_SINGLE = 0,
    PAPCT_SINGLE_STRETCHED,
    PAPCT_REPEATED,
    PAPCT_REPEATED_STRETCHED,
    PAPCT_END // This must be last
};

class LPEPathAlongPath : public Effect {
public:
    LPEPathAlongPath(LivePathEffectObject *lpeobject);
    virtual ~LPEPathAlongPath();

    Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect (Geom::Piecewise<Geom::D2<Geom::SBasis> > & pwd2_in);

    void resetDefaults(SPItem * item);

private:
    PathParam  bend_path;
    EnumParam<PAPCopyType> copytype;
    ScalarParam  prop_scale;
    BoolParam scale_y_rel;
    ScalarParam  spacing;
    ScalarParam  normal_offset;
    ScalarParam  tang_offset;
    BoolParam    vertical_pattern;

    void on_pattern_pasted();

    LPEPathAlongPath(const LPEPathAlongPath&);
    LPEPathAlongPath& operator=(const LPEPathAlongPath&);
};

}; //namespace LivePathEffect
}; //namespace Inkscape

#endif
