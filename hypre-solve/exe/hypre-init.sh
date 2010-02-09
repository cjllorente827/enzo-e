#!/bin/tcsh -f

#-----------------------------------------------------------------------
# Generate an input file for hypre-solve
#
# Usage: 
#
#   Unigrid of global size N0**3 distributed along <np0,np1,np2> processor grid
#
#       run unigrid N0 np0 np1 np2
#
#   Simple nested grids in L levels with global root-grid size N0**3
#   distributed along <np0,np1,np2> processor grid
#
#       run nested  N0 np0 np1 np2 L
#
#   (note that "run nested  N0 np0 np1 np2 0" is the same as
#              "run unigrid N0 np0 np1 np2")
#
#-----------------------------------------------------------------------

set boundary = "dirichlet"

#-----------------------------------------------------------------------
# Parse command-line arguments
#-----------------------------------------------------------------------

set type = "$1"
if ($type == "unigrid") then
   set N0  = "$2"
   set np0 = "$3"
   set np1 = "$4"
   set np2 = "$5"
   set L   = "1"
else if ($type == "nested") then
   set N0  = "$2"
   set np0 = "$3"
   set np1 = "$4"
   set np2 = "$5"
   set L   = "$6"
else
   echo
   echo "Usage: $0:t unigrid N0 np0 np1 np2"
   echo "   or"
   echo "       $0:t nested  N0 np0 np1 np2 L"
   echo
   exit 1
endif

#-----------------------------------------------------------------------
# Generate input file
#-----------------------------------------------------------------------

@ np = $np0 * $np1 * $np2

# Get input, output, and directory names

set dir = "$type.L1.G${np}.P${np0}${np1}${np2}.N${N0}.L${L}"
set infile  = "in.$dir"
set outfile = "out.$dir"

printf "Running ${dir}: "

# Blow away directory if it exists; create new directory, and change to it

rm -rf $dir
mkdir $dir
cd $dir

# Copy required files to new directory

ln -s ../hypre-solve .

# Compute local grid sizes
# WARNING: assumes # processors along each dimension evenly divides problem size

@ n0 = $N0 / $np0
@ n1 = $N0 / $np1
@ n2 = $N0 / $np2

# check evenly divisibility
@ t0 = $np0 * $n0
if ($t0 != $N0) then
    echo "Problem size $N0 not evenly divisible by np0 = $np0"
    exit 1
endif
@ t1 = $np1 * $n1
if ($t1 != $N0) then
    echo "Problem size $N0 not evenly divisible by np1 = $np1"
    exit 1
endif
@ t2 = $np2 * $n2
if ($t2 != $N0) then
    echo "Problem size $N0 not evenly divisible by np2 = $np2"
    exit 1
endif

# Loop though processors, generating a grid per processor in each level
set level = 0

while ($level < $L)

   set r = `echo "scale = 1; 0.5^$level" | bc`

   set ip = 0
   set ip0 = 0
   while ($ip0 < $np0)

      set lp0 = `echo "scale=0; $r*(-4*10^9 +  $ip0    * 8*10^9 / $np0)" | bc`
      set up0 = `echo "scale=0; $r*(-4*10^9 + ($ip0+1) * 8*10^9 / $np0)" | bc`
      set li0 = `echo "scale=0; -$N0/2 + $ip0 * $n0" | bc`
   
      set ip1 = 0
      while ($ip1 < $np1)

         set lp1 = `echo "scale=0; $r*(-4*10^9 +  $ip1    * 8*10^9 / $np1)" | bc`
         set up1 = `echo "scale=0; $r*(-4*10^9 + ($ip1+1) * 8*10^9 / $np1)" | bc`
         set li1 = `echo "scale=0; -$N0/2 + $ip1 * $n1" | bc`

         set ip2 = 0
         while ($ip2 < $np2)

            set lp2 = `echo "scale=0; $r*(-4*10^9 +  $ip2    * 8*10^9 / $np2)" | bc`
            set up2 = `echo "scale=0; $r*(-4*10^9 + ($ip2+1) * 8*10^9 / $np2)" | bc`
            set li2 = `echo "scale=0; -$N0/2 + $ip2 * $n2" | bc`

	    # Define a grid in level $level with 

            @ id = $ip + $np * $level
            if ($level == 0) then
               set iparent = -1
            else
               @ iparent = $id - $np
            endif

            echo "grid $id $iparent $ip ${lp0} $lp1 $lp2 $up0 $up1 $up2 $li0 $li1 $li2 $n0 $n1 $n2" >> $infile

	    # ASSUME POINT IS CONTAINED IN LAST GRID
            set iPoint = $id

            @ ip = $ip + 1

            @ ip2 = $ip2 + 1
        end

        @ ip1 = $ip1 + 1
      end

      @ ip0 = $ip0 + 1
   end  

   @ level = $level + 1
end

# Create problem

echo "dimension 3"                                  >> $infile
echo "domain    3 -4e9 -4e9 -4e9  4e9 4e9 4e9"      >> $infile
echo "boundary  $boundary"                          >> $infile
echo "sphere    5.993985e27 6.378137e8 0.0 0.0 0.0" >> $infile
echo "point     100.0   1e3 1e3 1e3   $iPoint"      >> $infile

if ($HOST == "ds100") then
   @ nodes = ($np - 1) / 8 + 1
   poe hypre-solve $infile -nodes 1 -tasks_per_node 8 -rmpool 1
else
   mpirun -np $np hypre-solve $infile >& outfile
endif

set success = `grep "End hypre-solve" outfile | wc`
set success = $success[1]
if ($success == 1) then
   echo "Pass"
else
   echo "Fail"
endif
