// $Id: enzo_EnzoSimulationCharm.cpp 2116 2011-03-17 22:11:13Z bordner $
// See LICENSE_CELLO file for license and copyright information

/// @file     enzo_EnzoSimulationCharm.cpp
/// @author   James Bordner (jobordner@ucsd.edu)
/// @todo     Create specific class for interfacing Cello code with User code
/// @todo     Move timestep reductions into Timestep object
/// @date     2011-03-17
/// @brief    Implementation of EnzoSimulationCharm user-dependent class member functions

#ifdef CONFIG_USE_CHARM

#include "cello.hpp"

#include "enzo.hpp"

extern CProxy_Main proxy_main;

//----------------------------------------------------------------------

EnzoSimulationCharm::EnzoSimulationCharm
(
 const char parameter_file[],
 int        string_length,
 int        index) throw ()
  : EnzoSimulation(parameter_file, new GroupProcessCharm, index)
{

  initialize();

  if (CkMyPe() == 0) run();

}

//----------------------------------------------------------------------

EnzoSimulationCharm::EnzoSimulationCharm
(
 const char * parameter_file,
 GroupProcess * group_process) throw ()
  : EnzoSimulation(parameter_file,group_process)
{
}

//----------------------------------------------------------------------

EnzoSimulationCharm::~EnzoSimulationCharm() throw()
{
}

//----------------------------------------------------------------------

void EnzoSimulationCharm::run() throw()
{
  
  performance_.start();

  ItPatch it_patch(mesh_);

  Patch * patch;

  while (( patch = ++it_patch )) {

    patch->block().p_initial();

  }

  // // EnzoBlock::p_initial();

  // // Perform any scheduled output

  // for (size_t i=0; i<output_list_.size(); i++) {
  //   Output * output = output_list_[i];
  //   output->scheduled_write(field_descr_, mesh_,cycle_,time_);
  // }

  // //--------------------------------------------------
  // // INITIAL STOPPING CRITERIA TEST
  // //--------------------------------------------------

  // int stop_mesh = true;

  // while ((patch = ++it_patch)) {

  //   int    stop_patch  = true;

  //   ItBlockLocal it_block(patch);
  //   Block * block;

  //   while ((block = ++it_block)) {

  //     EnzoBlock * enzo_block = static_cast <EnzoBlock*> (block);

  //     int cycle_block   = enzo_block->CycleNumber;
  //     double time_block =  enzo_block->Time;

  //     int stop_block = stopping_->complete(cycle_block,time_block);

  //     stop_patch = stop_patch && stop_block;

  //   }

  //   stop_mesh = stop_mesh && stop_patch;
		      
  // }

  // //======================================================================
  // // BEGIN MAIN LOOP
  // //======================================================================

  // Monitor * monitor = Monitor::instance();

  // while (! stop_mesh) {

  //   monitor->print("[Simulation %d] cycle %04d time %15.12f", index_, cycle_,time_);

  //   //--------------------------------------------------
  //   // Determine timestep
  //   //--------------------------------------------------

  //   double dt_mesh = std::numeric_limits<double>::max();

  //   // Accumulate Patch-local timesteps

  //   while ((patch = ++it_patch)) {

  //     double dt_patch = std::numeric_limits<double>::max();

  //     ItBlockLocal it_block(patch);
  //     Block * block;

  //     // Accumulate Block-local timesteps

  //     while ((block = ++it_block)) {

  // 	// Accumulate Block-local dt

  // 	double dt_block = timestep_->compute(field_descr_,block);

  // 	// Reduce timestep to coincide with scheduled output if needed

  // 	double time_block = static_cast <EnzoBlock*> (block)->Time;

  // 	for (size_t i=0; i<output_list_.size(); i++) {
  // 	  Output * output = output_list_[i];
  // 	  dt_block = output->update_timestep(time_block,dt_block);
  // 	}

  // 	// Reduce timestep to coincide with end of simulation if needed

  // 	dt_block = MIN(dt_block, (stopping_->stop_time() - time_block));

  // 	// Update patch-level timestep

  // 	dt_patch = MIN(dt_patch,dt_block);

  //     } // ( block = ++it_block )

  //     // Update mesh-level timestep

  //     dt_mesh = MIN(dt_mesh, dt_patch);

  //   } // ( patch = ++it_patch )

  //   ASSERT("EnzoSimulation::run", "dt == 0", dt_mesh != 0.0);

  //   //--------------------------------------------------
  //   // Apply the methods
  //   //--------------------------------------------------

  //   stop_mesh = true;

  //   while ((patch = ++it_patch)) {

  //     int stop_patch = true;

  //     ItBlockLocal it_block(patch);
  //     Block * block;

  //     while ((block = ++it_block)) {

  // 	// Enforce boundary conditions

  // 	boundary_->enforce(field_descr_,block);

  // 	// Refresh ghosts

  // 	block->refresh_ghosts(field_descr_);

  // 	EnzoBlock * enzo_block = static_cast <EnzoBlock*> (block);

  // 	int        & cycle_block    = enzo_block->CycleNumber;
  // 	enzo_float & time_block     = enzo_block->Time;
  // 	enzo_float & dt_block       = enzo_block->dt;
  // 	enzo_float & old_time_block = enzo_block->OldTime;

  // 	// UNIFORM TIMESTEP OVER ALL BLOCKS IN MESH

  // 	dt_block = dt_mesh;

  // 	// Loop through methods

  // 	for (size_t i = 0; i < method_list_.size(); i++) {

  // 	  Method * method = method_list_[i];

  // 	  method -> compute_block (block,time_block,dt_block);

  // 	}

  // 	// Update enzo_block values

  // 	old_time_block  = time_block;
  // 	time_block     += dt_block;
  // 	cycle_block    += 1;

  // 	// Global cycle and time reduction
	
  // 	int stop_block = stopping_->complete(cycle_block,time_block);

  // 	// Update stopping criteria for patch

  // 	stop_patch = stop_patch && stop_block;

  //     } // (block = ++it_block)

  //     // Update stopping criteria for mesh

  //     stop_mesh = stop_mesh && stop_patch;

  //   } // (patch = ++it_patch)

  //   cycle_ ++;
  //   time_ += dt_mesh;

  //   // Perform any scheduled output

  //   for (size_t i=0; i<output_list_.size(); i++) {
  //     Output * output = output_list_[i];
  //     output->scheduled_write(field_descr_, mesh_,cycle_,time_);
  //   }

  // } // while (! stop_mesh)

  // //======================================================================
  // // END MAIN LOOP
  // //======================================================================

  // monitor->print("[Simulation %d] cycle %04d time %15.12f", 
  // 		 index_, cycle_, time_);

  // performance_.stop();

  // performance_.print();


}

//======================================================================

#endif /* CONFIG_USE_CHARM */