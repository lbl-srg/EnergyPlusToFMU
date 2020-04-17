// Methods for Functional Mock-up Unit Export of EnergyPlus.


///////////////////////////////////////////////////////
/// \file   utilSocket.c
///
/// \brief  Methods for interfacing FMU
///         using BSD sockets.
///
/// \author Thierry S Nouidui
///         Simulation Research Group,
///         LBNL,
///         TSNouidui@lbl.gov
///
/// \date   2012-12-01
///
///
/// This file provides methods that allow clients to
/// establish a socket connection. This file has been
/// derived from utilSocket.h that is used for BCVTB
///
///////////////////////////////////////////////////////
#ifndef _UTILSOCKET_H_
#define _UTILSOCKET_H_
#ifdef _MSC_VER // Microsoft compiler
#include <windows.h>
#include <winsock.h>

//#include <winsock2.h>
//#include <ws2tcpip.h> // this gives compile error due to bug in .h file
#else
// Include arpa/inet.h so that inet_ntoa is defined for 64bit linux
// and on Mac OS X 10.6.1 Snow Leopard
#include <arpa/inet.h>
#include <sys/socket.h>
//#include <netinet/in.h>
#include <netdb.h> 
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <errno.h>
#include "defines.h"

////////////////////////////////////////////////////////////////
/// Assembles the buffer that will be exchanged through the IPC.
///
///\param flag The communication flag.
///\param nDbl The number of double values.
///\param nInt The number of integer values.
///\param nBoo The number of boolean values.
///\param dblVal The array that stores the double values.
///\param intVal The array that stores the integer values.
///\param booVal The array that stores the boolean values.
///\param buffer The buffer into which the values will be written.
///\param bufLen The buffer length prior and after the call.
///\return 0 if no error occurred.
int assembleBufferFMU(int flag,
		   int nDbl, int nInt, int nBoo,
		   double curSimTim,
		   double dblVal[], int intVal[], int booVal[],
		   char* *buffer, int *bufLen);

/////////////////////////////////////////////////////////////////
/// Gets an integer and does the required error checking.
///
///\param nptr Pointer to character buffer that contains the number.
///\param endptr After return, this variable contains a pointer to the 
///            character after the last character of the number.
///\param base Base for the integer.
///\param The value contained in the character buffer.
///\return 0 if no error occurred.
int getIntCheckErrorFMU(const char *nptr, char **endptr, const int base,
		     int* val);

/////////////////////////////////////////////////////////////////
/// Gets a double and does the required error checking.
///
///\param nptr Pointer to character buffer that contains the number.
///\param endptr After return, this variable contains a pointer to the 
///            character after the last character of the number.
///\param The value contained in the character buffer.
///\return 0 if no error occurred.
int getDoubleCheckErrorFMU(const char *nptr, char **endptr, 
			double* val);


/////////////////////////////////////////////////////////////////
/// Disassembles the header of the buffer that has been received through the IPC.
///
/// This method is separated from disassemblebuffer since in the
/// first call, we only need to peek at the header to assign
/// a long enough buffer for the read operation.
///
///\param buffer The buffer that contains the values to be parsed.
///\param flag The communication flag.
///\param nDbl The number of double values received.
///\param nInt The number of integer values received.
///\param nBoo The number of boolean values received.
///\return 0 if no error occurred.
int disassembleHeaderBufferFMU(const char* buffer,
			    char **endptr, const int base,
			    int *fla, int *nDbl, int *nInt, int *nBoo);

/////////////////////////////////////////////////////////////////
/// Disassembles the buffer that has been received through the IPC.
///
///\param buffer The buffer that contains the values to be parsed.
///\param flag The communication flag.
///\param nDbl The number of double values received.
///\param nInt The number of integer values received.
///\param nBoo The number of boolean values received.
///\param dblVal The array that stores the double values.
///\param intVal The array that stores the integer values.
///\param booVal The array that stores the boolean values.
///\return 0 if no error occurred.
int disassembleBufferFMU(const char* buffer,
		      int *fla,
		      int *nDbl, int *nInt, int *nBoo,
		      double *curSimTim,
		      double dblVal[], int intVal[], int booVal[]);

/////////////////////////////////////////////////////////////////////
/// Gets the port number for the BSD socket communication.
///
/// This method parses the xml file for the socket number.
/// \param docname Name of xml file.
/// \return the socket port number if successful, or -1 if an error occured.
int getsocketportnumber(const char *const docname);

/////////////////////////////////////////////////////////////////////
/// Gets the hostname for the BSD socket communication.
///
/// This method parses the xml file for the socket host name.
/// \param docname Name of xml file.
/// \param hostname The hostname will be written to this argument.
/// \return 0 if successful, or -1 if an error occured.
int getsockethost(const char *const docname, char *const hostname);


/////////////////////////////////////////////////////////////////
/// Writes data to the socket.
///
/// Clients can call this method to write data to the socket.
///\param sockfd Socket file descripter
///\param flaWri Communication flag to write to the socket stream.
///\param nDblWri Number of double values to write.
///\param nIntWri Number of integer values to write.
///\param nBooWri Number of boolean values to write.
///\param curSimTim Current simulation time in seconds.
///\param dblValWri Double values to write.
///\param intValWri Integer values to write.
///\param boolValWri Boolean values to write.
///\sa int establishclientsocket(uint16_t *portNo)
///\return The exit value of \c send, or a negative value if an error occured.
int writetosocketFMU(const int *sockfd, 
		  const int *flaWri,
		  const int *nDblWri, const int *nIntWri, const int *nBooWri,
		  double *curSimTim,
		  double dblValWri[], int intValWri[], int booValWri[]);

/////////////////////////////////////////////////////////////////
/// Returns the required socket buffer length by reading from
/// the socket how many data it contains.
/// This method also set the global variable \c SERVER_VERSION
///
///\param sockfd Socket file descripter
///\return nCha The nunber of characters needed to store the buffer
int getRequiredReadBufferLengthFMU(const int *sockfd);

/////////////////////////////////////////////////////////////////
/// Returns the required socket buffer length.
///
///\param nDbl Number of double values to read or write.
///\param nInt Number of integer values to read or write.
///\param nBoo Number of boolean values to read or write.
int getrequiredbufferlengthFMU(const int nDbl, const int nInt, const int nBoo);

/////////////////////////////////////////////////////////////////
/// Reads data from the socket.
//
/// Clients can call this method to exchange data through the socket.
///
///\param sockfd Socket file descripter
///\param flaRea Communication flag read from the socket stream.
///\param nDblRea Number of double values to read.
///\param nIntRea Number of integer values to read.
///\param nBooRea Number of boolean values to read.
///\param curSimTim Current simulation time in seconds read from socket.
///\param dblValRea Double values read from socket.
///\param intValRea Integer values read from socket.
///\param boolValRea Boolean values read from socket.
///\sa int establishclientsocket(uint16_t *portNo)
int readfromsocketFMU(const int *sockfd, int *flaRea, 
		   int *nDblRea, int *nIntRea, int *nBooRea,
		   double *curSimTim,
		   double dblValRea[], int intValRea[], int booValRea[]);

/////////////////////////////////////////////////////////////////
/// Reads a character buffer from the socket.
///
/// This method is called by \c readfromsocket.
///
///\param sockfd The socket file descripter.
///\param buffer The buffer into which the values will be written.
///\param bufLen The buffer length prior to the call.
///\return The exit value of the \c read command.
int readbufferfromsocketFMU(const int *sockfd,
			 char *buffer, int *bufLen);

/////////////////////////////////////////////////////////////////
/// Exchanges data with the socket.
///
/// Clients can call this method to exchange data through the socket.
///\param sockfd Socket file descripter
///\param flaWri Communication flag to write to the socket stream.
///\param flaRea Communication flag read from the socket stream.
///\param nDblWri Number of double values to write.
///\param nIntWri Number of integer values to write.
///\param nBooWri Number of boolean values to write.
///\param nDblRea Number of double values to read.
///\param nIntRea Number of integer values to read.
///\param nBooRea Number of boolean values to read.
///\param simTimWri Current simulation time in seconds to write.
///\param dblValWri Double values to write.
///\param intValWri Integer values to write.
///\param boolValWri Boolean values to write.
///\param simTimRea Current simulation time in seconds read from socket.
///\param dblValRea Double values read from socket.
///\param intValRea Integer values read from socket.
///\param boolValRea Boolean values read from socket.
///\sa int establishclientsocket(uint16_t *portNo)
///\return The exit value of \c send or \c read, or a negative value if an error occured.
int exchangewithsocketFMUex(const int *sockfd, 
		       const int *flaWri, int *flaRea,
		       const int *nDblWri, const int *nIntWri, const int *nBooWri,
		       int *nDblRea, int *nIntRea, int *nBooRea,
		       double *simTimWri,
		       double dblValWri[], int intValWri[], int booValWri[],
		       double *simTimRea,
		       double dblValRea[], int intValRea[], int booValRea[]);

/////////////////////////////////////////////////////////////////
/// Exchanges data with the socket.
///
/// Clients can call this method to exchange data through the socket.
///\param sockfd Socket file descripter
///\param flaWri Communication flag to write to the socket stream.
///\param flaRea Communication flag read from the socket stream.
///\param nDblWri Number of double values to write.
///\param nDblRea Number of double values to read.
///\param simTimWri Current simulation time in seconds to write.
///\param dblValWri Double values to write.
///\param simTimRea Current simulation time in seconds read from socket.
///\param dblValRea Double values read from socket.
///\sa int establishclientsocket(uint16_t *portNo)
///\return The exit value of \c send or \c read, or a negative value if an error occured.
int exchangedoubleswithsocketFMUex(const int *sockfd, 
			      const int *flaWri, int *flaRea,
			      const int *nDblWri,
			      int *nDblRea,
			      double *simTimWri,
			      double dblValWri[],
			      double *simTimRea,
			      double dblValRea[]);

///////////////////////////////////////////////////////////
/// Closes the inter process communication socket.
///
///\param sockfd Socket file descripter.
///\return The return value of the \c close function.
int closeipcFMU(int* sockfd);

#endif /* _UTILSOCKET_H_ */

/*

***********************************************************************************
Copyright Notice
----------------

Functional Mock-up Unit Export of EnergyPlus ©2013, The Regents of 
the University of California, through Lawrence Berkeley National 
Laboratory (subject to receipt of any required approvals from 
the U.S. Department of Energy). All rights reserved.
 
If you have questions about your rights to use or distribute this software, 
please contact Berkeley Lab's Technology Transfer Department at 
TTD@lbl.gov.referring to "Functional Mock-up Unit Export 
of EnergyPlus (LBNL Ref 2013-088)".
 
NOTICE: This software was produced by The Regents of the 
University of California under Contract No. DE-AC02-05CH11231 
with the Department of Energy.
For 5 years from November 1, 2012, the Government is granted for itself
and others acting on its behalf a nonexclusive, paid-up, irrevocable 
worldwide license in this data to reproduce, prepare derivative works,
and perform publicly and display publicly, by or on behalf of the Government.
There is provision for the possible extension of the term of this license. 
Subsequent to that period or any extension granted, the Government is granted
for itself and others acting on its behalf a nonexclusive, paid-up, irrevocable 
worldwide license in this data to reproduce, prepare derivative works, 
distribute copies to the public, perform publicly and display publicly, 
and to permit others to do so. The specific term of the license can be identified 
by inquiry made to Lawrence Berkeley National Laboratory or DOE. Neither 
the United States nor the United States Department of Energy, nor any of their employees, 
makes any warranty, express or implied, or assumes any legal liability or responsibility
for the accuracy, completeness, or usefulness of any data, apparatus, product, 
or process disclosed, or represents that its use would not infringe privately owned rights.
 
 
Copyright (c) 2013, The Regents of the University of California, Department
of Energy contract-operators of the Lawrence Berkeley National Laboratory.
All rights reserved.
 
1. Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:
 
(1) Redistributions of source code must retain the copyright notice, this list 
of conditions and the following disclaimer.
 
(2) Redistributions in binary form must reproduce the copyright notice, this list
of conditions and the following disclaimer in the documentation and/or other 
materials provided with the distribution.
 
(3) Neither the name of the University of California, Lawrence Berkeley 
National Laboratory, U.S. Dept. of Energy nor the names of its contributors 
may be used to endorse or promote products derived from this software without 
specific prior written permission.
 
2. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
 
3. You are under no obligation whatsoever to provide any bug fixes, patches, 
or upgrades to the features, functionality or performance of the source code
("Enhancements") to anyone; however, if you choose to make your Enhancements
available either publicly, or directly to Lawrence Berkeley National Laboratory, 
without imposing a separate written license agreement for such Enhancements, 
then you hereby grant the following license: a non-exclusive, royalty-free 
perpetual license to install, use, modify, prepare derivative works, incorporate
into other computer software, distribute, and sublicense such enhancements or 
derivative works thereof, in binary and source code form.
 
NOTE: This license corresponds to the "revised BSD" or "3-clause BSD" 
License and includes the following modification: Paragraph 3. has been added.


***********************************************************************************
*/