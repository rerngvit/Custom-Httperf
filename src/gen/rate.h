/*
    httperf -- a tool for measuring web server performance
    Copyright 2000-2007 Hewlett-Packard Company and Contributors listed in
    AUTHORS file. Originally contributed by David Mosberger-Tang

    This file is part of httperf, a web server performance measurment
    tool.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.
    
    In addition, as a special exception, the copyright holders give
    permission to link the code of this work with the OpenSSL project's
    "OpenSSL" library (or with modified versions of it that use the same
    license as the "OpenSSL" library), and distribute linked combinations
    including the two.  You must obey the GNU General Public License in
    all respects for all of the code used other than "OpenSSL".  If you
    modify this file, you may extend this exception to your version of the
    file, but you are not obligated to do so.  If you do not wish to do
    so, delete this exception statement from your version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
    02110-1301, USA
*/

#ifndef rate_h
#define rate_h

#include <httperf.h>
#include <timer.h>

typedef struct Rate_Generator
  {
    u_short xsubi[3];		/* used for random number generation */
    Rate_Info *rate;
    Time start;
    Time next_time;
    Any_Type arg;
    Timer *timer;
    int (*tick) (Any_Type arg);
    int done;
    Time (*next_interarrival_time) (struct Rate_Generator *rg);
    int num_det_bursts; // count for consecutive det-bursts
  }
Rate_Generator;

extern void rate_generator_start (Rate_Generator *rg,
				  Event_Type completion_event);
extern void rate_generator_stop (Rate_Generator *rg);

#endif /* rate_h */
