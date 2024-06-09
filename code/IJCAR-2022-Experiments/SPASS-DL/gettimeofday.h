/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                 GETTIMEOFDAY                           * */
/* *                                                        * */
/* *  $Module:   GETTIMEOFDAY                               * */ 
/* *                                                        * */
/* *  Copyright (C) 2006                                    * */
/* *  MPI fuer Informatik                                   * */
/* *                                                        * */
/* *  This program is free software; you can redistribute   * */
/* *  it and/or modify it under the terms of the FreeBSD    * */
/* *  Licence.                                              * */
/* *                                                        * */
/* *  This program is distributed in the hope that it will  * */
/* *  be useful, but WITHOUT ANY WARRANTY; without even     * */
/* *  the implied warranty of MERCHANTABILITY or FITNESS    * */
/* *  FOR A PARTICULAR PURPOSE.  See the LICENCE file       * */
/* *  for more details.                                     * */
/* *                                                        * */
/* *                                                        * */
/* $Revision: 1.2 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2010-02-22 14:09:58 $                             * */
/* $Author: weidenb $                                       * */
/* *                                                        * */
/* *             Contact:                                   * */
/* *             Christoph Weidenbach                       * */
/* *             MPI fuer Informatik                        * */
/* *             Stuhlsatzenhausweg 85                      * */
/* *             66123 Saarbruecken                         * */
/* *             Email: spass@mpi-inf.mpg.de                * */
/* *             Germany                                    * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/

/* A wrapper header for gettimeofday function on Windows. 
 * It does nothing on Unix platforms. 
 */
