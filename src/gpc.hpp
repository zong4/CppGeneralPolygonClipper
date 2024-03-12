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

#ifndef __gpc_h
#define __gpc_h

// TODO:
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string>

#include "gpc_polygon.hpp"

namespace gpc {

/*
===========================================================================
                             Constants
===========================================================================
*/

/* Increase GPC_EPSILON to encourage merging of near coincident edges    */

const double GPC_EPSILON = DBL_EPSILON;

const std::string GPC_VERSION = "2.33";

/*
===========================================================================
                           Public Data Types
===========================================================================
*/

enum class gpc_op /* Set operation type                */
{
  GPC_DIFF, /* Difference                        */
  GPC_INT,  /* Intersection                      */
  GPC_XOR,  /* Exclusive or                      */
  GPC_UNION /* Union                             */
};

struct gpc_tristrip /* Tristrip set structure            */
{
  int num_strips = 0;               /* Number of tristrips               */
  gpc_vertex_list *strip = nullptr; /* Tristrip array pointer            */
};

/*
===========================================================================
                       Public Function Prototypes
===========================================================================
*/

void gpc_polygon_clip(gpc_op set_operation, gpc_polygon *subject_polygon,
                      gpc_polygon *clip_polygon, gpc_polygon *result_polygon);

void gpc_tristrip_clip(gpc_op set_operation, gpc_polygon *subject_polygon,
                       gpc_polygon *clip_polygon,
                       gpc_tristrip *result_tristrip);

void gpc_polygon_to_tristrip(gpc_polygon *polygon, gpc_tristrip *tristrip);

void gpc_free_tristrip(gpc_tristrip *tristrip);

} // namespace gpc

#endif

/*
===========================================================================
                           End of file: gpc.h
===========================================================================
*/
