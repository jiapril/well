This is a test case for modeling lateral heat transfer between one-dimensional well and its surrouding two-dimensional solid structure.
The multi-app concept is used here.
main.i solves temperature for the two-dimensional solid structure.
sub.i solves temperature for the one-dimensional borehole.
matrix.msh: input mesh for main.i
well.msh: input mesh for sub.i
SFT.csv: read-in file for the static formation temperature (dirichlet boundary and initial conditions)
