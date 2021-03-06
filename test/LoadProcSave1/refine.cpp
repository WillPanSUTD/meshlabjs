/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2004-2012                                           \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/
#include <stdlib.h>
#include <emscripten.h>
#include <vcg/complex/complex.h>
#include <wrap/io_trimesh/import_off.h>
#include <wrap/io_trimesh/export_off.h>

#include <vcg/complex/algorithms/refine.h>
using namespace vcg;
using namespace std;


class MyEdge;
class MyFace;
class MyVertex;
struct MyUsedTypes : public vcg::UsedTypes<	vcg::Use<MyVertex>   ::AsVertexType,
                                        vcg::Use<MyEdge>     ::AsEdgeType,
                                        vcg::Use<MyFace>     ::AsFaceType>{};

class MyVertex  : public vcg::Vertex<MyUsedTypes, vcg::vertex::Coord3f, vcg::vertex::Normal3f, vcg::vertex::Color4b, vcg::vertex::Qualityf, vcg::vertex::BitFlags  >{};
class MyFace    : public vcg::Face< MyUsedTypes,  vcg::face::Normal3f, vcg::face::VertexRef, vcg::face::FFAdj,  vcg::face::BitFlags > {};
class MyEdge    : public vcg::Edge<MyUsedTypes>{};
class MyMesh    : public vcg::tri::TriMesh< std::vector<MyVertex>, std::vector<MyFace> , std::vector<MyEdge>  > {};

char *buf=0;
int size;

extern "C"
{
  char * allocator(size_t _size)
  {
     size = _size;
     buf = (char *) malloc(size);
     return buf;
  }

  char* refine(int num)
  {
    MyMesh m;
    int t0=clock();
    int loadmask;

    printf("Buffer ptr is '%i'\n",buf);
    printf("Buffer[0] is '%c'\n",buf[0]);

    int ret = tri::io::ImporterOFF<MyMesh>::OpenMem(m,buf,strlen(buf),loadmask);
    int t1=clock();
    if(ret != 0)
    {
      printf("Error in opening file\n");
      exit(-1);
    }
    int t2=clock();
    printf("Read mesh %i %i\n",m.FN(),m.VN());
    tri::UpdateTopology<MyMesh>::FaceFace(m);
    tri::EdgeLen<MyMesh,float> edgePred(0);
    tri::MidPoint<MyMesh> midFun(&m);
    tri::RefineE(m,midFun,edgePred);
    int t3=clock();
    printf("Refined mesh %i %i\n",m.FN(),m.VN());
    printf("Opening time %5.2f \n Refinement time %5.2f",float(t1-t0)/CLOCKS_PER_SEC,float(t3-t2)/CLOCKS_PER_SEC);
    char *bufMeshOut = tri::io::ExporterOFF<MyMesh>::SaveStream(m);
    return bufMeshOut;
  }
}
