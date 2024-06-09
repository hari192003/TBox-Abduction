/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *                       GRAPHS                           * */
/* *                                                        * */
/* *  $Module:   GRAPH                                      * */ 
/* *                                                        * */
/* *  Copyright (C) 1998, 2001 MPI fuer Informatik          * */
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
/* $Revision: 1.3 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2011-05-22 13:27:17 $                             * */
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


/* $RCSfile: graph.h,v $ */

#ifndef _GRAPH_
#define _GRAPH_

#include "list.h"

typedef struct {
  NAT     number;
  int     dfs_num;
  int     comp_num;  /* completion number */
  POINTER info;      /* user defined information */
  LIST    neighbors;
} GRAPHNODE_STRUCT, *GRAPHNODE;

typedef struct {
  NAT  size;       /* number of nodes */
  LIST nodes;      /* list of GRAPHNODES */
  NAT  dfscount;   /* used for DFS */
  NAT  compcount;  /* used for DFS */
} GRAPH_STRUCT, *GRAPH;

NAT graph_NodeNumber(GRAPHNODE Node);

int graph_NodeDfsNum(GRAPHNODE Node);

void graph_NodeSetDfsNum(GRAPHNODE Node, int Number);

int graph_NodeCompNum(GRAPHNODE Node);

void graph_NodeSetCompNum(GRAPHNODE Node, int Number);

LIST graph_NodeNeighbors(GRAPHNODE Node);

POINTER graph_NodeInfo(GRAPHNODE Node);

void graph_NodeSetInfo(GRAPHNODE Node, POINTER Info);

NAT graph_NodeOutdegree(GRAPHNODE Node);

BOOL graph_NodeVisited(GRAPHNODE Node);

BOOL graph_NodeCompleted(GRAPHNODE Node);

NAT graph_Size(GRAPH Graph);

LIST graph_Nodes(GRAPH Graph);

GRAPH     graph_Create(void);
void      graph_Delete(GRAPH);

GRAPHNODE graph_GetNode(GRAPH, NAT);
GRAPHNODE graph_AddNode(GRAPH, NAT);

void      graph_AddEdge(GRAPHNODE, GRAPHNODE);
void      graph_DeleteEdge(GRAPHNODE, GRAPHNODE);
void      graph_DeleteDuplicateEdges(GRAPH);

void      graph_SortNodes(GRAPH, BOOL (*)(GRAPHNODE, GRAPHNODE));
NAT       graph_StronglyConnectedComponents(GRAPH);

void      graph_NodePrint(GRAPHNODE Node);
void      graph_Print(GRAPH);

#endif
