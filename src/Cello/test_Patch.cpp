// See LICENSE_CELLO file for license and copyright information

/// @file     test_Patch.cpp
/// @author   James Bordner (jobordner@ucsd.edu)
/// @date     2010-05-06
/// @brief    Program implementing unit tests for the Patch class

#include "main.hpp" 
#include "test.hpp"

#include "mesh.hpp"

PARALLEL_MAIN_BEGIN
{

  PARALLEL_INIT;

  GroupProcess * group_process = GroupProcess::create();

  unit_init(0,1);

  unit_class("Patch");

  //======================================================================
  // np = 1
  // ip = 0 [default]
  //======================================================================

  unit_func("Patch");

  FieldDescr * field_descr = new FieldDescr;

  // Set Patch size, offset, and blocking

  int patch_size[] = {12,12,12};

  int patch_offset[] = {5, 2, 9};

  int patch_blocking[] = {3,3,3};

  // Set domain extents

  double domain_lower[] = {0.0, 0.0, 0.0};
  double domain_upper[] = {1.0, 1.0, 1.0};

  Factory * factory = new Factory;

  Patch * patch = factory->create_patch 
    (group_process,
     patch_size[0],     patch_size[1],     patch_size[2],
     patch_offset[0],   patch_offset[1],   patch_offset[2],
     patch_blocking[0], patch_blocking[1], patch_blocking[2],
     domain_lower[0],   domain_lower[1],   domain_lower[2],
     domain_upper[0],   domain_upper[1],   domain_upper[2]);

  unit_assert(patch != NULL);

  //--------------------------------------------------

  unit_func("size");

  int npx,npy,npz;

  patch->size(&npx,&npy,&npz);

  unit_assert(patch_size[0]==npx && 
	      patch_size[1]==npy && 
	      patch_size[2]==npz);

  //--------------------------------------------------

  unit_func("offset");

  int nx0,ny0,nz0;

  patch->offset(&nx0,&ny0,&nz0);

  unit_assert(patch_offset[0]==nx0 && 
	      patch_offset[1]==ny0 && 
	      patch_offset[2]==nz0);

  //--------------------------------------------------

  unit_func("blocking");

  int nbx,nby,nbz;

  patch->blocking(&nbx,&nby,&nbz);

  unit_assert(patch_blocking[0]==nbx && 
	      patch_blocking[1]==nby && 
	      patch_blocking[2]==nbz);

  //--------------------------------------------------

  unit_func("set_lower");

  double xm,ym,zm;
  patch->lower(&xm,&ym,&zm);

  unit_assert(xm==domain_lower[0]);
  unit_assert(ym==domain_lower[1]);
  unit_assert(zm==domain_lower[2]);

  //--------------------------------------------------

  unit_func("set_upper");

  double xp,yp,zp;
  patch->upper(&xp,&yp,&zp);

  unit_assert(xp==domain_upper[0]);
  unit_assert(yp==domain_upper[1]);
  unit_assert(zp==domain_upper[2]);

  //--------------------------------------------------

  unit_func("layout");

  Layout * layout = patch->layout();

  unit_assert(layout != NULL);

  layout->set_process_range(0,1);

  //--------------------------------------------------

  unit_func("allocate_blocks");

  patch->allocate_blocks(field_descr);

  //--------------------------------------------------

#ifdef CONFIG_USE_CHARM
  unit_func("num_blocks");
  unit_assert(patch->num_blocks() == (size_t)nbx*nby*nbz);
#else
  unit_func("num_local_blocks");
  unit_assert(patch->num_local_blocks()==(size_t)nbx*nby*nbz);
#endif

  ItBlock itBlock (patch);

  Block *  block = 0;
  FieldBlock * field_block = 0;

  size_t block_counter = 0;

  while ((block = ++itBlock)) {

  //--------------------------------------------------

    unit_func("allocate_blocks");
    unit_assert_quiet(block != NULL);

  //--------------------------------------------------

    unit_class("Block");
    unit_func("field_block");

    field_block = block ? block->field_block() : NULL;

    unit_assert_quiet(field_block != NULL);

    if (block && field_block) {

      //--------------------------------------------------

      unit_class("FieldBlock");
      unit_func("size");

      int nfx, nfy, nfz;

      field_block->size(&nfx,&nfy,&nfz);

      unit_assert_quiet (nfx == patch_size[0] / patch_blocking[0]);
      unit_assert_quiet (nfy == patch_size[1] / patch_blocking[1]);
      unit_assert_quiet (nfz == patch_size[2] / patch_blocking[2]);

      GroupProcess * group_process = patch->group_process();
      Layout      * layout = patch->layout();

      int ip = group_process->rank();

      int index_local = block_counter;
      int index_global = layout->global_index(ip,index_local);

      //--------------------------------------------------

      unit_class("Layout");
      unit_func("block_indices");

      int ibx,iby,ibz;
      layout->block_indices(index_global,&ibx,&iby,&ibz);

      // Not terribly rigorous
      unit_assert (0 <= ibx && ibx < nbx);
      unit_assert (0 <= iby && iby < nby);
      unit_assert (0 <= ibz && ibz < nbz);

      //--------------------------------------------------

      double xmb,ymb,zmb;
      block->lower (&xmb,&ymb,&zmb);
      double xpb,ypb,zpb;
      block->upper (&xpb,&ypb,&zpb);

      unit_class("Block");
      unit_func("lower");

      unit_assert(cello::err_abs(xm + ibx*(xpb-xmb) , xmb) < 1e-6);
      unit_assert(cello::err_abs(ym + iby*(ypb-ymb) , ymb) < 1e-6);
      unit_assert(cello::err_abs(zm + ibz*(zpb-zmb) , zmb) < 1e-6);

      unit_class("Block");
      unit_func("upper");

      unit_assert(cello::err_abs(xm + (ibx+1)*(xpb-xmb) , xpb) < 1e-6);
      unit_assert(cello::err_abs(ym + (iby+1)*(ypb-ymb) , ypb) < 1e-6);
      unit_assert(cello::err_abs(zm + (ibz+1)*(zpb-zmb) , zpb) < 1e-6);

      //--------------------------------------------------

    }

    //--------------------------------------------------

    ++block_counter;

  }

  //--------------------------------------------------

  unit_class("Block");
  unit_func("index_patch");

  int * b = new int [nbx*nby*nbz];
  int i;
  for (i=0; i<nbx*nby*nbz; i++) b[i]=0;

  while ((block = ++itBlock)) {
    int ibx,iby,ibz;
    block->index_patch(&ibx,&iby,&ibz);
    b[ibx + nbx*(iby + nby*ibz)] = 1;
  }
  for (i=0; i<nbx*nby*nbz; i++) {
    unit_assert_quiet(b[i]==1);
  }

  delete [] b;

  //--------------------------------------------------

#ifndef CONFIG_USE_CHARM
  unit_func("num_local_blocks");
  unit_assert(block_counter == patch->num_local_blocks());
#endif

  //--------------------------------------------------

  unit_func("allocate_blocks");

  //--------------------------------------------------

  unit_func("deallocate_blocks");

  patch->deallocate_blocks();

  //--------------------------------------------------

  delete patch;
  delete field_descr;

  unit_finalize();

  PARALLEL_EXIT;
}

PARALLEL_MAIN_END