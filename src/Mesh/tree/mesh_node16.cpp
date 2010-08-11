// $Id$
// See LICENSE_CELLO file for license and copyright information

/// @file     mesh_node16.cpp
/// @author   James Bordner (jobordner@ucsd.edu)
/// @date     2009-09-18
/// @brief    Node class for 4^2-trees Tree16

#include <stdio.h>

#include "cello.hpp"

#include "mesh_node16.hpp"
#include "error.hpp"

//----------------------------------------------------------------------

const bool debug = false;

//----------------------------------------------------------------------

Node16::Node16(int level_adjust) 
  /// @param    level_adjust difference: actual mesh level - tree level
  : level_adjust_(level_adjust)
{ 
  for (int ix=0; ix<4; ix++) {
    neighbor_[ix] = NULL;
    for (int iy=0; iy<4; iy++) {
      child_[ix][iy] = NULL;
    }
  }

  parent_ = NULL;
}

//----------------------------------------------------------------------

Node16::~Node16() 
///
{ 
  // recursively delete children

  for (int ix=0; ix<4; ix++) {
    for (int iy=0; iy<4; iy++) {
      if (child_[ix][iy]) delete child_[ix][iy];
    }
  }

  // update neighbor's neighbors

  if (neighbor_[R]) neighbor_[R]->neighbor_[L] = NULL;
  if (neighbor_[U]) neighbor_[U]->neighbor_[D] = NULL;
  if (neighbor_[L]) neighbor_[L]->neighbor_[R] = NULL;
  if (neighbor_[D]) neighbor_[D]->neighbor_[U] = NULL;

  neighbor_[R] = NULL;
  neighbor_[U] = NULL;
  neighbor_[L] = NULL;
  neighbor_[D] = NULL;

  // Update parent's children

  if (parent_) {
    for (int ix=0; ix<4; ix++) {
      for (int iy=0; iy<4; iy++) {
	if (parent_->child_[ix][iy] == this) parent_->child_[ix][iy] = NULL;
      }
    }
  }

  parent_ = NULL;
}

//----------------------------------------------------------------------

Node16::Node16(const Node16 & node16) throw()
{
  INCOMPLETE_MESSAGE("Node16::Node16","");
}

//----------------------------------------------------------------------

Node16 & Node16::operator= (const Node16 & node16) throw()
{
  INCOMPLETE_MESSAGE("Node16::operator =","");
  return *this;
}

//----------------------------------------------------------------------

int Node16::num_nodes()
{
  int node_count = 0;
  for (int ix=0; ix<4; ix++) {
    for (int iy=0; iy<4; iy++) {
      if (child(ix,iy)) node_count += child(ix,iy)->num_nodes();
    }
  }
  return (1 + node_count);
}

//----------------------------------------------------------------------

inline Node16 * Node16::child (int ix, int iy) 
/// @param    ix        Index 0 <= ix < 3 of cell in grid block
/// @param    iy        Index 0 <= iy < 3 of cell in grid block
{ 
  return child_[ix][iy]; 
}

//----------------------------------------------------------------------

inline Node16 * Node16::neighbor (face_type face) 
/// @param    face      Face 0 <= (face = [XYZ][MP]) < 6
{ 
  return neighbor_[face]; 
}

//----------------------------------------------------------------------

inline Node16 * Node16::cousin (face_type face, int ix, int iy) 
/// @param    face      Face 0 <= (face = [XYZ][MP]) < 6
/// @param    ix        Index 0 <= ix < 3 of cell in grid block
/// @param    iy        Index 0 <= iy < 3 of cell in grid block
{ 
  if (neighbor_[face] && neighbor_[face]->child_[ix][iy]) {
    return neighbor_[face]->child_[ix][iy];
  } else {
    return NULL;
  }
}

//----------------------------------------------------------------------

inline Node16 * Node16::parent () 
///
{ 
  return parent_; 
}

//----------------------------------------------------------------------
// Set two nodes to be neighbors.  Friend function since nodes may be NULL
inline void make_neighbors 
(
 Node16 * node_1, 
 Node16 * node_2, 
 face_type face_1
 )
/// @param    node_1    First neighbor node pointer 
/// @param    node_2    Second neighbor node pointer
/// @param    face_1    Face 0 <= face_1 < 6 of node_1 that is adjacent to node_2
{
  if (node_1 != NULL) node_1->neighbor_[face_1] = node_2;
  if (node_2 != NULL) node_2->neighbor_[(face_1+2)%4] = node_1;
}

//----------------------------------------------------------------------
int Node16::refine 
(
 const int * level_array, 
 int ndx,  int ndy,
 int lowx, int upx,  
 int lowy, int upy,
 int level, 
 int max_level,
 bool full_nodes
 )
/// @param    level_array Array of levels to refine to
/// @param    ndx       x-dimension of level_array[]
/// @param    ndy       y-dimension of level_array[]
/// @param    lowx      Lowest x-index of array for this node
/// @param    upx       Upper bound on x-index of array for this node
/// @param    lowy      Lowest y-index of array for this node
/// @param    upy       Upper bound on y-index of array for this node
/// @param    level     Level of this node
/// @param    max_level Maximum refinement level
/// @param    full_nodes Whether nodes always have a full complement of children
{
  int depth = 0;
  if ( level < max_level && lowx < upx-1 && lowy < upy-1 ) {

    // determine whether to refine the node

    int dx = (upx - lowx)/4;
    int dy = (upy - lowy)/4;
    int ix4[5] = {lowx, lowx+dx, lowx+2*dx, lowx+3*dx, upx};
    int iy4[5] = {lowy, lowy+dy, lowy+2*dy, lowy+3*dy, upy};

    int depth_child[4][4] = {{0}};

    if (full_nodes) {

      // Refine if any bits in the level_array are in this node

      bool refine_node = false;
      for (int iy=lowy; iy<upy && !refine_node; iy++) {
	for (int ix=lowx; ix<upx && !refine_node; ix++) {
	  if (level_array[ix + ndx * iy] >= level) refine_node = true;
	}
      }

      // refine the node if needed

      if (refine_node) {

	create_children_();

	update_children_();

	for (int ix=0; ix<4; ix++) {
	  for (int iy=0; iy<4; iy++) {
	    depth_child[ix][iy] = child_[ix][iy]->refine 
	      (level_array,ndx,ndy,ix4[ix],ix4[ix+1],iy4[iy],iy4[iy+1],
	       level+2,max_level,full_nodes);
	  }
	}
      }

    } else {

      // Refine each child separately if any bits in the level_array
      // are in in them

      bool refine_child[4][4] = {{false}};

      // loop over children

      for (int ix=0; ix<4; ix++) {
	for (int iy=0; iy<4; iy++) {

	  // loop over values in the level array

	  for (int ly=iy4[iy]; ly<iy4[iy+1]; ly++) {
	    for (int lx=ix4[ix]; lx<ix4[ix+1]; lx++) {

	      if (level_array[lx + ndx * ly] >= level) {
		refine_child[ix][iy] = true;

	      }
	    }
	  }

	}
      }

      // refine each child if needed

      for (int ix=0; ix<4; ix++) {
	for (int iy=0; iy<4; iy++) {
	  if (refine_child[ix][iy]) {
	    create_child_(ix,iy);
	    update_child_(ix,iy);
	    depth_child[ix][iy] = child_[ix][iy]->refine 
	      (level_array,ndx,ndy,ix4[ix],ix4[ix+1],iy4[iy],iy4[iy+1],
	       level+2,max_level,full_nodes);
	  }
	}
      }

    }

    for (int ix=0; ix<4; ix++) {
      for (int iy=0; iy<4; iy++) {
	depth = (depth_child[ix][iy] > depth) ? depth_child[ix][iy] : depth;
      }
    }
    depth += 2;

  } // if not at bottom of recursion

  return depth;
}

//----------------------------------------------------------------------
void Node16::create_children_()
///
{
  for (int ix=0; ix<4; ix++) {
    for (int iy=0; iy<4; iy++) {
      create_child_(ix,iy);
    }
  }
}

//----------------------------------------------------------------------
void Node16::update_children_()
///
{
  for (int ix=0; ix<4; ix++) {
    for (int iy=0; iy<4; iy++) {
      update_child_(ix,iy);
    }
  }
}

//----------------------------------------------------------------------
void Node16::create_child_(int ix, int iy)
/// @param    ix        Index 0 <= ix < 3 of cell in grid block
/// @param    iy        Index 0 <= iy < 3 of cell in grid block
{
  child_[ix][iy] = new Node16();
}

//----------------------------------------------------------------------
void Node16::update_child_ (int ix, int iy)
/// @param    ix        Index 0 <= ix < 3 of cell in grid block
/// @param    iy        Index 0 <= iy < 3 of cell in grid block
{
  if (child(ix,iy)) {

    child(ix,iy)->parent_ = this;

    // Right neighbor

    if (ix < 3) {
      make_neighbors (child (ix,iy),child (ix+1,iy),R);
    } else {
      make_neighbors (child (ix,iy),cousin (R,0,iy),R);
    }

    // Left neighbor

    if (ix > 0) {
      make_neighbors (child (ix,iy),child (ix-1,iy),L);
    } else {
      make_neighbors (child (ix,iy),cousin (L,3,iy),L);
    }

    // Up neighbor

    if (iy < 3) {
      make_neighbors (child (ix,iy),child (ix,iy+1),U);
    } else {
      make_neighbors (child (ix,iy),cousin (U,ix,0),U);
    }

    // Down neighbor

    if (iy > 0) {
      make_neighbors (child (ix,iy),child (ix,iy-1),D);
    } else {
      make_neighbors (child (ix,iy),cousin (D,ix,3),D);
    }
  }
}

// Perform a pass of trying to remove level-jumps 
//----------------------------------------------------------------------
void Node16::balance_pass(bool & refined_tree, bool full_nodes)
/// @param    refined_tree Whether tree has been refined
/// @param    full_nodes   Whether nodes always have a full complement of children
{
  if (full_nodes) {

    // is a leaf
    if (! any_children()) {

      bool refine_node = false;

      // Check for adjacent nodes two levels finer

      for (int i=0; i<4; i++) {
	
	refine_node = refine_node ||
	  ( cousin(R,0,i) && ( cousin(R,0,i)->child(0,0) ) ) ||
	  ( cousin(L,3,i) && ( cousin(L,3,i)->child(0,0) ) ) ||
	  ( cousin(U,i,0) && ( cousin(U,i,0)->child(0,0) ) ) ||
	  ( cousin(D,i,3) && ( cousin(D,i,3)->child(0,0) ) );
      }

      if (refine_node) {

	refined_tree = true;

	create_children_();
	update_children_();

	for (int ix=0; ix<4; ix++) {
	  for (int iy=0; iy<4; iy++) {
	    if (child(ix,iy)) {
	      child(ix,iy)->balance_pass(refined_tree,full_nodes);
	    }
	  }
	}
      }
    }

  } else { 

    // not full node refinement
    
    if (! all_children ()) {

      bool refine_child[4][4] = {{false}};

      for (int ix=0; ix<4; ix++) {
	for (int iy=0; iy<4; iy++) {
	  if (! child(ix,iy)) {
	    // right neighbor
	    if (ix < 3 && child(ix+1,iy)) {
	      for (int k=0; k<4; k++) {
		refine_child[ix][iy] = refine_child[ix][iy] ||
		  child(ix+1,iy)->child(0,k);
	      }
	    } else if (ix == 3 && cousin(R,0,iy)) {
	      for (int k=0; k<4; k++) {
		refine_child[ix][iy] = refine_child[ix][iy] ||
		  cousin(R,0,iy)->child(0,k);
	      }
	    }
	    // left neighbor
	    if (ix > 0 && child(ix-1,iy)) {
	      for (int k=0; k<4; k++) {
		refine_child[ix][iy] = refine_child[ix][iy] ||
		  child(ix-1,iy)->child(3,k);
	      }
	    } else if (ix == 0 && cousin(L,3,iy)) {
	      for (int k=0; k<4; k++) {
		refine_child[ix][iy] = refine_child[ix][iy] ||
		  cousin(L,3,iy)->child(3,k);
	      }
	    }
	    // up neighbor
	    if (iy < 3 && child(ix,iy+1)) {
	      for (int k=0; k<4; k++) {
		refine_child[ix][iy] = refine_child[ix][iy] ||
		  child(ix,iy+1)->child(k,0);
	      }
	    } else if (iy == 3 && cousin(U,ix,0)) {
	      for (int k=0; k<4; k++) {
		refine_child[ix][iy] = refine_child[ix][iy] ||
		  cousin(U,ix,0)->child(k,0);
	      }
	    }
	    // down neighbor
	    if (iy > 0 && child(ix,iy-1)) {
	      for (int k=0; k<4; k++) {
		refine_child[ix][iy] = refine_child[ix][iy] ||
		  child(ix,iy-1)->child(k,3);
	      }
	    } else if (iy == 0 && cousin(D,ix,3)) {
	      for (int k=0; k<4; k++) {
		refine_child[ix][iy] = refine_child[ix][iy] ||
		  cousin(D,ix,3)->child(k,3);
	      }
	    }
	  } // k
	} // iy
      } // ix

      for (int ix=0; ix<4; ix++) {
	for (int iy=0; iy<4; iy++) {
	  if (refine_child[ix][iy]) {
	    create_child_(ix,iy); 
	    update_child_(ix,iy);
	    child(ix,iy)->balance_pass(refined_tree,full_nodes);
	    refined_tree = true;
	  }
	}
      }
    }
  }

  // not a leaf: recurse

  for (int ix=0; ix<4; ix++) {
    for (int iy=0; iy<4; iy++) {
      if (child(ix,iy)) {
	child(ix,iy)->balance_pass(refined_tree,full_nodes);
      }
    }
  }

    //  }
}
  // Perform a pass of trying to optimize uniformly-refined nodes
//----------------------------------------------------------------------
void Node16::optimize_pass(bool & refined_tree)
/// @param    refined_tree Whether tree has been refined
{

  bool single_children = true;
  bool same_level = true;

  for (int ix=0; ix<4; ix++) {
    for (int iy=0; iy<4; iy++) {
      single_children = single_children && 
	child_[ix][iy] && (! child_[ix][iy]->any_children());
    }
  }

  if (single_children) {

    // assert: all children exist

    int level = child(0,0)->level_adjust_;
    for (int ix=0; ix<4; ix++) {
      for (int iy=0; iy<4; iy++) {
	same_level = same_level &&
	  (child(ix,iy)->level_adjust_ == level);
      }
    }
  }

  if (single_children && same_level) {

    // adjust effective resolution

    level_adjust_ += 2 + child_[0][0]->level_adjust_; 

    for (int ix=0; ix<4; ix++) {
      for (int iy=0; iy<4; iy++) {
	delete child_[ix][iy];
	child_[ix][iy]=NULL;
      }
    }

    refined_tree = true;

  } else {
    
    // recurse

    for (int ix=0; ix<4; ix++) {
      for (int iy=0; iy<4; iy++) {
	if (child_[ix][iy]) {
	  child_[ix][iy]->optimize_pass(refined_tree);
	}
      }
    }
  }

}

// Fill the image region with values
//----------------------------------------------------------------------
void Node16::fill_image
(
 float * image,
 int ndx,  int ndy,
 int lowx, int upx,  
 int lowy, int upy,
 int level,
 int num_levels,
 int line_width
 )
/// @param    image     Array of colormap indices
/// @param    ndx       x-dimension of image[]
/// @param    ndy       y-dimension of image[]
/// @param    lowx      Lowest x-index of image[] for this node
/// @param    upx       Upper bound on x-index of image[] for this node
/// @param    lowy      Lowest y-index of image[] for this node
/// @param    upy       Upper bound on y-index of image[] for this node
/// @param    level     Level of this node
/// @param    num_levels Total number of levels
/// @param    line_width Width of lines bounding nodes
{
  int ix,iy,i;

  level += level_adjust_;

  // Fill interior

  for (iy=lowy; iy<=upy; iy++) {
    for (ix=lowx; ix<=upx; ix++) {
      i = ix + ndx * iy;
      image[i] = 2*num_levels - level; 
    }
  }

  // Draw border
  for (ix=lowx; ix<=upx; ix++) {
    iy = lowy;
    for (int k=0; k < line_width; k++) {
      image[ix + ndx*(iy+k)] = 0;
    }
    iy = upy;
    for (int k=0; k < line_width; k++) {
      image[ix + ndx*(iy+k)] = 0;
    }
  }

  for (iy=lowy; iy<=upy; iy++) {
    ix = lowx;
    for (int k=0; k < line_width; k++) {
      image[(ix+k) + ndx*iy] = 0;
    }
    ix = upx;
    for (int k=0; k < line_width; k++) {
      image[(ix+k) + ndx*iy] = 0;
    }
  }

  // Recurse

  int dx = (upx - lowx)/4;
  int dy = (upy - lowy)/4;
  int ix4[5] = {lowx, lowx+dx, lowx+2*dx,lowx+3*dx,upx};
  int iy4[5] = {lowy, lowy+dy, lowy+2*dy,lowy+3*dy,upy};

  for (int ix=0; ix<4; ix++) {
    for (int iy=0; iy<4; iy++) {
      if (child_[ix][iy]) {
	child_[ix][iy]->fill_image 
	  (image,ndx,ndy,
	   ix4[ix],ix4[ix+1], 
	   iy4[iy],iy4[iy+1], 
	   level + 1, num_levels, line_width);
      }
    }
  }
}