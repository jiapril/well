#!/bin/bash
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=10
#SBATCH --time=00:30:00
#SBATCH --export=ALL,PATH_TO=/projects/raven/framework/,EXECUTABLE=Driver.py,INPUT=ConstructROM.xml

#module load ${MPI_MODULE}
#module load numlib/petsc/3.8.3-openmpi-3.1-gnu-8.2 # Not needed, if custom PETSc installation used 

cd ${SLURM_SUBMIT_DIR}
startexe="python ${HOME}${PATH_TO}${EXECUTABLE} ${SLURM_SUBMIT_DIR}/${INPUT}"

echo $startexe
exec $startexe

exit
