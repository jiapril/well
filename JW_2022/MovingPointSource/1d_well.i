# This is a test case for a moving mass point source. The concept is mimic the drilling of a well, which physically means that 
# exposed time to the injected fluid (i.e. drilling mud) at different depth should be different. 
# To simulate such process, a 1-D well is injected with the drilling mud at the well head (0,0,0) at a mass rate of 20 kg/s.
# At t=0, the well has a depth of 5 m. Then it is deepened at a rate of 1e-3 m/s. After 1e3 s, the drilling is stopped.
# The position of the moving point source is then a time function "if(t<=1e3, 5 + 2e-3*t,7.0)".

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 10
  nx = 10
[]

[Modules]
  [./FluidProperties]
    [./water_uo]
      type = TigerWaterConst
    [../]
  [../]
[]

[UserObjects]
  [./rock_uo]
    type =  TigerPermeabilityConst
    permeability_type = isotropic
    k0 = '1.0e-10'
  [../]
[]

[Materials]
  [./rock_g]
    type = TigerGeometryMaterial
  [../]
  [./rock_p]
    type = TigerPorosityMaterial
    porosity = 0
    specific_density = 2500
  [../]
  [./rock_f]
    type = TigerFluidMaterial
    fp_uo = water_uo
  [../]
  [./rock_h]
    type = TigerHydraulicMaterialH
    pressure = pressure
    compressibility = 7.5e-8
    kf_uo = rock_uo
  [../]
[]

[BCs]
  [./right]
    type = DirichletBC
    variable = pressure
    boundary = right
    value = 0.0
  [../]
[]

[Functions]
  [./well_depth]
    type = ParsedFunction
    value = 'if(t<=1e3, 5 + 2e-3*t,7.0)'  
  [../]
[]

[AuxVariables]
  [./vx]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./vx_ker]
    type = TigerDarcyVelocityH
    pressure = pressure
    variable =  vx
    component = x
  [../]
[]

[Variables]
  [./pressure]
  [../]
[]

[DiracKernels]
  [./Injection]
    type = TigerHydraulicPointSourceH
    point = '0.0 0.0 0.0'
    mass_flux_function= -20.0
    variable = pressure
  [../]
  [./dryout]
    type = WBMovingPointSourceH
    mass_flux = -20
    variable = pressure
    x_coord_function =  well_depth   
    y_coord_function = '0'
    z_coord_function = '0'
  [../]
[]


[Kernels]
  [./diff]
    type = TigerHydraulicKernelH
    variable = pressure
  [../]
  [./time]
    type = TigerHydraulicTimeKernelH
    variable = pressure
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 50
  end_time = 5000.0
  l_tol = 1e-10 #difference between first and last linear step
  nl_rel_step_tol = 1e-14 #machine percision
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]



[Outputs]
  exodus = true
  print_linear_residuals = false
[]
