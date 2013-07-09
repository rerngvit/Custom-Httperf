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


#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/types.h>

#include <httperf.h>
#include <call.h>
#include <core.h>
#include <event.h>

static char *fbase, *fend, *fcurrent;

static void
set_uri (Event_Type et, Call * c)
{
  char *uri;
  int uri_len, did_wrap = 0, range_len, input_len;
  const char *input_line;

  assert (et == EV_CALL_NEW && object_is_call (c));

  do
    {
      if (fcurrent >= fend)
		{
    	  if (did_wrap)
			panic ("%s: %s does not contain any valid URIs\n",
			   prog_name, param.wrangelog.file);
		  did_wrap = 1;

		  /* We reached the end of the uri list so wrap around to the
			 beginning.  If not looping, also ask for the test to stop
			 as soon as possible (the current request will still go
			 out, but httperf won't wait for its reply to show up).  */
		  fcurrent = fbase;
		  if (!param.wrangelog.do_loop)
			core_exit ();

		}
        input_line = fcurrent;

        input_len = strlen (fcurrent);


      /*****************************/
//        int j;
//        while (input_line[j] != ' ') {
//        	j++;
//        	if (j > input_len)
//        	{
//        		panic ("Error: illegal format for wrangelog\n");
//        	}
//        }
//
//        uri_len = j;

        uri_len = strcspn( input_line, " ");

        uri = fcurrent;

        /***************************/
        int range_size = input_len - uri_len - 1;

        char prefix_extra_hdrs[] = "Range: bytes=";
		size_t prefix_size = strlen(prefix_extra_hdrs);
		char suffix_extra_hdrs[] = "\r\n";
		size_t suffix_size = strlen(suffix_extra_hdrs);

		size_t  extra_header_len = prefix_size + range_size + suffix_size;
		c->extra_header = (char*) malloc( sizeof(char) * extra_header_len);



		// Copy the prefix
   	    strncpy(c->extra_header, prefix_extra_hdrs, prefix_size);

   	    // Copy the range value
   	    strncpy(c->extra_header + prefix_size, fcurrent + uri_len + 1, range_size);

   	    // Copy the suffix
   	    strncpy(c->extra_header + prefix_size + range_size, suffix_extra_hdrs, suffix_size);

   	    call_set_uri (c, uri, uri_len);
   	 	fcurrent += input_len + 1;

   	 	call_append_request_header (c, c->extra_header,extra_header_len);



    }
  while (input_len == 0);

  if (verbose)
    printf ("%s: accessing URI `%s'\n", prog_name, uri);
}



static void
uri_wrange_call_done (Event_Type et, Object *obj, Any_Type regarg, Any_Type callarg)
{

  assert (object_is_call (c));
  Call *call = (Call *) obj;

  // this cleaning part is not actually called
  free(call->extra_header);

}


void
init_wrangelog (void)
{
  struct stat st;
  Any_Type arg;
  int fd;


  fd = open (param.wrangelog.file, O_RDONLY, 0);
  if (fd == -1)
    panic ("%s: can't open %s\n", prog_name, param.wrangelog.file);



  fstat (fd, &st);
  if (st.st_size == 0)
    panic ("%s: file %s is empty\n", prog_name, param.wrangelog.file);

  /* mmap anywhere in address space: */
  fbase = (char *) mmap (0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (fbase == (char *) 0 - 1)
    panic ("%s: can't mmap the file: %s\n", prog_name, strerror (errno));

  close (fd);

  /* set the upper boundary: */
  fend = fbase + st.st_size;
  /* set current entry: */
  fcurrent = fbase;



  arg.l = 0;
  event_register_handler (EV_CALL_NEW, (Event_Handler) set_uri, arg);
  event_register_handler (EV_CONN_CLOSE, (Event_Handler) uri_wrange_call_done, arg);
}

static void
stop_wrangelog (void)
{
 munmap (fbase, fend - fbase);
}

Load_Generator uri_wrangelog =
  {
    "Generates URIs based on a predetermined list and range",
    init_wrangelog,
    no_op,
    stop_wrangelog
  };
