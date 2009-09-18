#include <stdio.h>
#include "cello.h"
#include "node16.h"
#include <assert.h>

const bool debug = false;


//    3
//
//    2
//
//    1        
//
//iy  0  1  2  3  
// ix

Node16::Node16(int level_adjust) 
  : level_adjust_(level_adjust)

{ 
  ++Node16::num_nodes_;

  for (int ix=0; ix<4; ix++) {
    neighbor_[ix] = NULL;
    for (int iy=0; iy<4; iy++) {
      child_[ix][iy] = NULL;
    }
  }

  parent_ = NULL;
}

// Delete the node and all descendents

Node16::~Node16() 
{ 
  --Node16::num_nodes_;

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

inline Node16 * Node16::child (int ix, int iy) 
{ 
  return child_[ix][iy]; 
}

inline Node16 * Node16::neighbor (face_type face) 
{ 
  return neighbor_[face]; 
}

inline Node16 * Node16::cousin (face_type face, int ix, int iy) 
{ 
  if (neighbor_[face] && neighbor_[face]->child_[ix][iy]) {
    return neighbor_[face]->child_[ix][iy];
  } else {
    return NULL;
  }
}

inline Node16 * Node16::parent () 
{ 
  return parent_; 
}

// Set two nodes to be neighbors.  Friend function since nodes may be NULL
inline void make_neighbors 
(
 Node16 * node_1, face_type face_1,
 Node16 * node_2
 )
{
  if (node_1 != NULL) node_1->neighbor_[face_1] = node_2;
  if (node_2 != NULL) node_2->neighbor_[(face_1+2)%4] = node_1;
}


// Create 16 empty child nodes

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
{
  int depth = 0;
  if ( level < max_level && lowx < upx-1 && lowy < upy-1 ) {

    // determine whether to refine the node

    int dx = (upx + lowx)/4;
    int dy = (upy + lowy)/4;
    int ix4[5] = {lowx, lowx+dx, lowx+2*dx,lowx+3*dx,upx};
    int iy4[5] = {lowy, lowy+dy, lowy+2*dy,lowy+3*dy,upy};

    int depth_child[4][4] = {0};

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
	       level+1,max_level,full_nodes);
	  }
	}
      }

//     } else {

//       // Refine each child separately if any bits in the level_array
//       // are in in them

//       bool refine_child[4][4] = {false};

//       for (int iy=lowy; iy<midy; iy++) {
// 	for (int ix=lowx; ix<midx; ix++) {
// 	  if (level_array[ix + ndx * iy] >= level) refine_child[UL] = true;
// 	}
//       }
//       for (int iy=lowy; iy<midy; iy++) {
// 	for (int ix=midx; ix<upx; ix++) {
// 	  if (level_array[ix + ndx * iy] >= level) refine_child[DL] = true;
// 	}
//       }
//       for (int iy=midy; iy<upy; iy++) {
// 	for (int ix=lowx; ix<midx; ix++) {
// 	  if (level_array[ix + ndx * iy] >= level) refine_child[UR] = true;
// 	}
//       }
//       for (int iy=midy; iy<upy; iy++) {
// 	for (int ix=midx; ix<upx; ix++) {
// 	  if (level_array[ix + ndx * iy] >= level) refine_child[DR] = true;
// 	}
//       }

//       // refine each child if needed

//       if (refine_child[UL]) {
// 	create_child_(UL);
// 	update_child_(UL);
// 	depth_child[UL] = child_[UL]->refine 
// 	  (level_array,ndx,ndy,lowx,midx,lowy,midy,level+1,max_level,full_nodes);
//       }

//       if (refine_child[DL]) {
// 	create_child_(DL);
// 	update_child_(DL);
// 	depth_child[DL] = child_[DL]->refine 
// 	  (level_array,ndx,ndy,midx,upx,lowy,midy,level+1,max_level,full_nodes);
//       }

//       if (refine_child[UR]) {
// 	create_child_(UR);
// 	update_child_(UR);
// 	depth_child[UR] = child_[UR]->refine 
// 	  (level_array,ndx,ndy,lowx,midx,midy,upy,level+1,max_level,full_nodes);
//       }

//       if (refine_child[DR]) {
// 	create_child_(DR);
// 	update_child_(DR);
// 	depth_child[DR] = child_[DR]->refine 
// 	  (level_array,ndx,ndy,midx,upx,midy,upy,level+1,max_level,full_nodes);
//       }

    }

    for (int ix=0; ix<4; ix++) {
      for (int iy=0; iy<4; iy++) {
	depth = (depth_child[ix][iy] > depth) ? depth_child[ix][iy] : depth;
      }
    }
    ++depth;

  } // if not at bottom of recursion

  return depth;
}

void Node16::create_children_()
{
  for (int ix=0; ix<4; ix++) {
    for (int iy=0; iy<4; iy++) {
      child_[ix][iy]->create_child_(ix,iy);
    }
  }
}

void Node16::update_children_()
{
  for (int ix=0; ix<4; ix++) {
    for (int iy=0; iy<4; iy++) {
      child_[ix][iy]->update_child_(ix,iy);
    }
  }
}

void Node16::create_child_(int ix, int iy)
{
  child_[ix][iy] = new Node16();
}

void Node16::update_child_ (int ix, int iy)
{
  if (child(ix,iy)) {

    child(ix,iy)->parent_ = this;

    // Right neighbor

    if (ix < 3) {
      make_neighbors (child (ix,iy),R,child (ix+1,iy));
    } else {
      make_neighbors (child (ix,iy),R,cousin (R,0,iy));
    }

    // Left neighbor

    if (ix > 0) {
      make_neighbors (child (ix,iy),L,child (ix-1,iy));
    } else {
      make_neighbors (child (ix,iy),L,cousin (L,3,iy));
    }

    // Up neighbor

    if (iy < 3) {
      make_neighbors (child (ix,iy),U,child (ix,iy+1));
    } else {
      make_neighbors (child (ix,iy),U,cousin (U,ix,0));
    }

    // Down neighbor

    if (iy > 0) {
      make_neighbors (child (ix,iy),D,child (ix,iy-1));
    } else {
      make_neighbors (child (ix,iy),D,cousin (D,ix,3));
    }
  }
}

// Perform a pass of trying to remove level-jumps 
void Node16::normalize_pass(bool & refined_tree, bool full_nodes)
{

  if (full_nodes) {

    // is a leaf
    if (! any_children()) {

      bool refine_node = false;

      // Check for adjacent nodes two levels finer

      for (int i=0; i<4; ! refine_node && i++) {
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
	    child(ix,iy)->normalize_pass(refined_tree,full_nodes);
	  }
	}
      }
    }

//   } else { 

//     // not full node refinement

//     if (! all_children ()) {


//       bool refine_child[4][4] = {false};

//       if (! child(UL)) {

// 	refine_child[UL] = 
// 	  (cousin(U,DL) && 
// 	   ( cousin(U,DL)->child(DL) ||
// 	     cousin(U,DL)->child(DR))) ||
// 	  (cousin(L,UR) && 
// 	   ( cousin(L,UR)->child(UR) ||
// 	     cousin(L,UR)->child(DR))) ||
// 	  (child(UR) && 
// 	   ( child(UR)->child(UL) ||
// 	     child(UR)->child(DL))) ||
// 	  (child(DL) && 
// 	   ( child(DL)->child(UL) ||
// 	     child(DL)->child(UR)));

// 	if (refine_child[UL]) {
// 	  create_child_(UL); 
// 	  update_child_(UL);
// 	  child(UL)->normalize_pass(refined_tree,full_nodes);
// 	  refined_tree = true;
// 	}

//       }
	 
//       if ( ! child(DL) ) {

// 	refine_child[DL] =
// 	  (cousin(L,DR) && 
// 	   ( cousin(L,DR)->child(UR) ||
// 	     cousin(L,DR)->child(DR))) ||
// 	  (cousin(D,UL) && 
// 	   ( cousin(D,UL)->child(UL) ||
// 	     cousin(D,UL)->child(UR))) ||
// 	  (child(DR) && 
// 	   ( child(DR)->child(UL) ||
// 	     child(DR)->child(DL))) ||
// 	  (child(UL) && 
// 	   ( child(UL)->child(DL) ||
// 	     child(UL)->child(DR)));

// 	if (refine_child[DL]) {
// 	  create_child_(DL); 
// 	  update_child_(DL);
// 	  child(DL)->normalize_pass(refined_tree,full_nodes);
// 	  refined_tree = true;
// 	}
//       }

//       if ( ! child(UR) ) {

// 	refine_child[UR] = 
// 	  (cousin(R,UL) && 
// 	   ( cousin(R,UL)->child(UL) ||
// 	     cousin(R,UL)->child(DL))) ||
// 	  (cousin(U,DR) && 
// 	   ( cousin(U,DR)->child(DL) ||
// 	     cousin(U,DR)->child(DR))) ||
// 	  (child(UL) && 
// 	   ( child(UL)->child(UR) ||
// 	     child(UL)->child(DR))) ||
// 	  (child(DR) && 
// 	   ( child(DR)->child(UL) ||
// 	     child(DR)->child(UR)));

// 	if (refine_child[UR]) {
// 	  create_child_(UR); 
// 	  update_child_(UR);
// 	  child(UR)->normalize_pass(refined_tree,full_nodes);
// 	  refined_tree = true;
// 	}
//       }
       
//       if ( ! child(DR) ) {
// 	refine_child[DR] = 
// 	  (cousin(R,DL) && 
// 	   ( cousin(R,DL)->child(UL) ||
// 	     cousin(R,DL)->child(DL))) ||
// 	  (cousin(D,UR) && 
// 	   ( cousin(D,UR)->child(UL) ||
// 	     cousin(D,UR)->child(UR))) ||
// 	  (child(DL) && 
// 	   ( child(DL)->child(UR) ||
// 	     child(DL)->child(DR))) ||
// 	  (child(UR) && 
// 	   ( child(UR)->child(DL) ||
// 	     child(UR)->child(DR)));

// 	if (refine_child[DR]) {
// 	  create_child_(DR); 
// 	  update_child_(DR);
// 	  child(DR)->normalize_pass(refined_tree,full_nodes);
// 	  refined_tree = true;
// 	}
//       }
//     }
  } else {

    // not a leaf: recurse

    for (int ix=0; ix<4; ix++) {
      for (int iy=0; iy<4; iy++) {
	child(ix,iy)->normalize_pass(refined_tree,full_nodes);
      }
    }
  }

}

  // Perform a pass of trying to optimize uniformly-refined nodes
void Node16::optimize_pass(bool & refined_tree, bool full_nodes)
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

    level_adjust_ += 1 + child_[0][0]->level_adjust_; 

    for (int ix=0; ix<4; ix++) {
      for (int iy=0; iy<4; iy++) {
	delete child_[ix][iy];
      }
    }

    refined_tree = true;

  } else {
    
    // recurse

    for (int ix=0; ix<4; ix++) {
      for (int iy=0; iy<4; iy++) {

	if (child_[ix][iy]) {
	  child_[ix][iy]->optimize_pass(refined_tree,full_nodes);
	}
      }
    }
  }

}

// Fill the image region with values
void Node16::fill_image
(
 float * image,
 int nd0,  int nd1,
 int low0, int high0,  
 int low1, int high1,
 int level,
 int num_levels,
 int line_width
 )
{
  int i0,i1,i;

  level += level_adjust_;

  // Fill interior

  for (i1=low1; i1<=high1; i1++) {
    for (i0=low0; i0<=high0; i0++) {
      i = i0 + nd0 * i1;
      image[i] = 2*num_levels - level;
    }
  }

  // Draw border
  for (i0=low0; i0<=high0; i0++) {
    i1 = low1;
    for (int k=0; k < line_width; k++) {
      image[i0 + nd0*(i1+k)] = 0;
    }
    i1 = high1;
    for (int k=0; k < line_width; k++) {
      image[i0 + nd0*(i1+k)] = 0;
    }
  }

  for (i1=low1; i1<=high1; i1++) {
    i0 = low0;
    for (int k=0; k < line_width; k++) {
      image[(i0+k) + nd0*i1] = 0;
    }
    i0 = high0;
    for (int k=0; k < line_width; k++) {
      image[(i0+k) + nd0*i1] = 0;
    }
  }
    

  // Recurse

  int mid0 = (high0 + low0)/2;
  int mid1 = (high1 + low1)/2;

  if (child_[UL]) {
    child_[UL]->fill_image 
      (image,nd0,nd1,low0,mid0, low1,mid1, level + 1, num_levels,line_width);
  }
  if (child_[DL]) {
    child_[DL]->fill_image 
      (image,nd0,nd1,mid0,high0,low1,mid1, level + 1, num_levels,line_width);
  }
  if (child_[UR]) {
    child_[UR]->fill_image 
      (image,nd0,nd1,low0,mid0, mid1,high1,level + 1, num_levels,line_width);
  }
  if (child_[DR]) {
    child_[DR]->fill_image 
      (image,nd0,nd1,mid0,high0,mid1,high1,level + 1, num_levels,line_width);
  }
}

int Node16::num_nodes_ = 0;

