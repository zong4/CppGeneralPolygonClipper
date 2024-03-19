/*
===========================================================================

Project:   Generic Polygon Clipper

           A new algorithm for calculating the difference, intersection,
           exclusive-or or union of arbitrary polygon sets.

File:      gpc.h
Author:    Alan Murta (email: gpc@cs.man.ac.uk)
Version:   2.33
Date:      21st May 2014

Copyright: (C) Advanced Interfaces Group,
           University of Manchester.

           This software is free for non-commercial use. It may be copied,
           modified, and redistributed provided that this copyright notice
           is preserved on all copies. The intellectual property rights of
           the algorithms used reside with the University of Manchester
           Advanced Interfaces Group.

           You may not use this software, in whole or in part, in support
           of any commercial product without the express consent of the
           author.

           There is no warranty or other guarantee of fitness of this
           software for any purpose. It is provided solely "as is".

===========================================================================
*/

#pragma once

#include "geometry/gpc_lmt.hpp"
#include "geometry/gpc_tristrip.hpp"
#include "geometry/it_node.hpp"
#include "utilis/gpc_constants.hpp"
#include "utilis/gpc_math.hpp"

namespace gpc {

void gpc_polygon_clip(gpc_op set_operation, gpc_polygon &subject_polygon,
                      gpc_polygon &clip_polygon, gpc_polygon &result_polygon);

void gpc_polygon_to_tristrip(gpc_polygon *polygon, gpc_tristrip *tristrip);

} // namespace gpc
