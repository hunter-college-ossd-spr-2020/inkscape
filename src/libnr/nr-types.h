#ifndef __NR_TYPES_H__
#define __NR_TYPES_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Class-ifying NRPoint, Nathan Hurst <njh@mail.csse.monash.edu.au>
 *
 * This code is in public domain
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>

#include <libnr/nr-coord.h>
#include <libnr/nr-dim2.h>
#include <libnr/nr-i-coord.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-point.h>
#include <libnr/nr-point-l.h>
#include <libnr/nr-point-ops.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-rect-l.h>

namespace NR {

class Rect;
class Matrix;

} /* namespace NR */


#endif /* !__NR_TYPES_H__ */
