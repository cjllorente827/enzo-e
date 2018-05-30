// See LICENSE_CELLO file for license and copyright information

/// @file     compute_Solver.hpp 
/// @author   James Bordner (jobordner@ucsd.edu) 
/// @date     2014-10-27 22:37:41
/// @brief    [\ref Compute] Declaration for the Solver class

#ifndef COMPUTE_SOLVER_HPP
#define COMPUTE_SOLVER_HPP

#include <cstring>

class Refresh;
class Solver : public PUP::able 
{
  /// @class    Solver
  /// @ingroup  Compute
  /// @brief    [\ref Solver] Interface to a linear solver

public: // interface

  /// Create a new Solver
  Solver (int monitor_iter,
	  int restart_cycle,
	  int min_level = 0,
	  int max_level = std::numeric_limits<int>::max()) throw()
    : PUP::able(),
      refresh_list_(),
      monitor_iter_(monitor_iter),
      restart_cycle_(restart_cycle),
      callback_(0),
      index_(0),
      min_level_(min_level),
      max_level_(max_level),
      id_sync_(0)
  {}

  /// Create an uninitialized Solver
  Solver () throw()
  : PUP::able(),
    refresh_list_(),
    monitor_iter_(0),
    restart_cycle_(1),
    callback_(0),
    index_(0),
    min_level_(0),
    max_level_(std::numeric_limits<int>::max()),
    id_sync_(0)

  {}

  /// Destructor
  virtual ~Solver() throw();

  /// Charm++ PUP::able declarations
  PUPable_abstract(Solver);

  Solver (CkMigrateMessage *m)
    : PUP::able (m),
      refresh_list_(),
      monitor_iter_(0),
      restart_cycle_(1),
      callback_(0),
      index_(0),
      min_level_(- std::numeric_limits<int>::max()),
      max_level_(  std::numeric_limits<int>::max()),
      id_sync_(0)
  { }
  
  /// CHARM++ Pack / Unpack function
  void pup (PUP::er &p)
  {
    TRACEPUP;
    
    PUP::able::pup(p);
    
    p | refresh_list_;
    p | monitor_iter_;
    p | restart_cycle_;
    p | callback_;
    p | index_;
    p | min_level_;
    p | max_level_;
    p | id_sync_;
  }

  Refresh * refresh(size_t index=0) ;

  void set_callback (int callback)
  { callback_ = callback; }

  void set_index (int index)
  { index_ = index; }

  int index() const { return index_; }

  void set_min_level (int min_level)
  { min_level_ = min_level; }

  const int min_level() { return min_level_; }
  const int max_level() { return max_level_; }

  void set_max_level (int max_level)
  { max_level_ = max_level; }

  void set_sync_id (int sync_id)
  { id_sync_ = sync_id; }
  
  /// Type of neighbor: level if min_level == max_level, else leaf
  int neighbor_type_() const throw() {
    return (min_level_ == max_level_) ? neighbor_level : neighbor_leaf;
  }

  /// Type of synchronization: sync_face if min_level == max_level,
  /// else sync_neighbor
  int sync_type_() const throw() {
    return (min_level_ == max_level_) ? sync_face : sync_neighbor;
  }

  /// Whether Block is active
  bool is_active_(Block * block);

public: // virtual functions

  /// Solve the linear system Ax = b
  virtual void apply ( std::shared_ptr<Matrix> A,
		       int ix, int ib, Block * block) throw() = 0;

  /// Return the name of this solver
  virtual std::string name () const
  { return "UNKNOWN"; }

protected: // functions

  /// Initialize a solve
  void begin_(Block * block);
  
  /// Clean up after a solver is done and returning to its callback_
  void end_(Block * block);

  void monitor_output_(Block * block, int iter,
		       double rr0=0.0,
		       double rr_min=0.0, double rr=0.0, double rr_max=0.0,
		       bool final = false) throw();
  
  int add_refresh (int ghost_depth, 
		   int min_face_rank, 
		   int neighbor_type, 
		   int sync_type,
		   int sync_id);

  /// Perform vector copy X <- Y
  template <class T>
  void copy_ (T * X, const T * Y,
	      int mx, int my, int mz,
	      bool active = true) const throw()
  {
    if (! active ) return;
    const int m = mx*my*mz;
    std::memcpy(X,Y,m*sizeof(T));
  }

  int sync_id_() const throw()
  { return this->id_sync_; }

  bool reuse_solution_ (int cycle) const throw();
    
protected: // attributes

  ///  Refresh object
  std::vector<Refresh *> refresh_list_;

  /// How often to write output
  int monitor_iter_;

  /// Whether to reuse the previous solution as the initial guess
  int restart_cycle_;

  /// Callback id
  int callback_;

  /// Index of this solver
  int index_;

  /// Minimum mesh level
  int min_level_;
  
  /// Maximum mesh level
  int max_level_;

  /// Sync id
  int id_sync_;
};

#endif /* COMPUTE_SOLVER_HPP */