# This is a test case for modeling lateral heat transfer 
# between a two-dimensional well and its surrouding two-dimensional solid structure.
# The interface-kernel concept is used here.
# The study case is for the RN15/IDDP2 injection well which consists of multiple casings and 
# cement and the surrouding formation (matrix). The well is devided into the upper and lower section, which have different radius. 
# Two interfaces are constructed in the [Mesh] block using the upper well section and its contacting solid wall (casing1), and thermal
# lower well section and its contacting wall (matrix).
# From mesh.msh you can see the well lies in the horizontal direction (x-axis)
# The thermal boundary condition and initial condition (i.e. SFT) are given as a function.

[Mesh]

[file]
  file = mesh.msh
  type = FileMeshGenerator
[]

[interface_1]
  type =SideSetsBetweenSubdomainsGenerator
  input = file
  master_block = well_upper
  new_boundary = master1_interface
  paired_block = casing1
[]
  
[break_1]
  type = BreakBoundaryOnSubdomainGenerator  
  input = interface_1
[]

[interface_2]
  type = SideSetsBetweenSubdomainsGenerator
  input = break_1
  master_block = well_lower
  new_boundary = master2_interface
  paired_block = matrix
[]

[break_2]
  type = BreakBoundaryOnSubdomainGenerator
  input = interface_2
[]

[]

[Problem]
  coord_type = RZ
  rz_coord_axis = X    ##the well is in horizontal direction
[]


[Modules]
  [./FluidProperties]
    [./water]
      type = TigerWaterConst
    [../]
  [../]
[]

[UserObjects]
  [./well_uo]
    k0 = 1.0e-6
    permeability_type = isotropic
    type = TigerPermeabilityConst
  [../]
  [./supg]
    effective_length = average
    supg_coeficient = optimal
    type = TigerSUPG
  [../]
[]

[Functions]
  [./SFT]
    type = ParsedFunction
    value = '7-0.09*x'
  [../]
[]

[Materials]
  [./f_well]
    block = "well_upper well_lower"
    fp_uo = water
    temperature = temperature_f
    type = TigerFluidMaterial
  [../]
  [./f_matrix]
    block = "casing1 casing2 casing3 casing4 cement matrix"
    fp_uo = water
    temperature = temperature_s
    type = TigerFluidMaterial
  [../]
  [./geometry_fluid_flowing_part]
    type = TigerGeometryMaterial
    #block = "well_upper well_lower"
    scale_factor = 1     
  [../]
  [./porosity_fluid_part]
    type = TigerPorosityMaterial
    porosity = 1.0
    specific_density = 1.0
    block = "well_upper well_lower"
  [../]
  [./porosity_cement]
    type = TigerPorosityMaterial
    porosity = 0.0
    specific_density = 2010
    block = "cement"
  [../]
  [./porosity_casing]
    type = TigerPorosityMaterial
    porosity = 0.0
    specific_density =  8000
    block = "casing1 casing2 casing3 casing4"
  [../]
  [./porosity_matrix]
    type = TigerPorosityMaterial
    porosity = 0.0
    specific_density = 2650
    block = "matrix"
  [../]
  [./casing_T]
    type = TigerThermalMaterialT
    advection_type = pure_diffusion
    block = "casing1 casing2 casing3 casing4"
    conductivity_type = isotropic
    lambda = 50
    specific_heat = 500   
  [../]
  [./cement_T]
    type = TigerThermalMaterialT
    advection_type = pure_diffusion
    block = cement
    conductivity_type = isotropic
    lambda = 1.75
    specific_heat = 950
  [../]
  [./matrix_t]
    type = TigerThermalMaterialT
    advection_type = pure_diffusion
    block = matrix
    conductivity_type = isotropic
    lambda  = 3.0
    specific_heat = 900
  [../]
  [./well_upper_t]
    type = WB2DThermalMaterialT
    advection_type = velocity_three_components
    block = well_upper
    has_supg = true
    heat_transfer_direction = y
    lambda = "0.0 50000"   #### ultra-high thermal conductivity in y direction
    specific_heat = 0
    density = 0
    supg_uo = supg
    velocity_component_x = '-0.5'
    well_radius = 0.11024    #### borehole radius of the upper section
  [../]
  [./well_lower_t]
    type = WB2DThermalMaterialT
    advection_type = velocity_three_components
    block = well_lower
    has_supg = true
    heat_transfer_direction = y
    lambda = "0.0 50000"  #### ultra-high thermal conductivity in y direction
    specific_heat = 0
    density = 0
    supg_uo = supg
    velocity_component_x = '-0.5'
    well_radius = 0.107950          #### borehole radius of the lower section
  [../]
[]

## Set up interfaceKernel to solve borehole fluid temperature
[InterfaceKernels]
  [./interface_1]
    type = ConvectiveHeatFluxInterface
    boundary = master1_interface
    variable = temperature_f
    neighbor_var = temperature_s
  [../]
  [./interface_2]
    type = ConvectiveHeatFluxInterface
    boundary = master2_interface
    variable = temperature_f
    neighbor_var = temperature_s
  [../]
[]

[ICs]
  [./init_temperature_fluid]
    function = SFT
    type = FunctionIC
    variable = temperature_f
  [../]
  [./init_temperature_solid]
    function = SFT
    type = FunctionIC
    variable = temperature_s
  [../]
[]

[BCs]
  [./well_top_t]
    type = DirichletBC
    boundary = well_top
    value = 7.5
    variable = temperature_f
  [../]
  [./matrix_right_t]
    type = FunctionDirichletBC
    boundary = right
    function = SFT
    variable = temperature_s
  [../]
[]

[Variables]
  [./temperature_f]
    block = "well_upper well_lower"
    scaling = 1e-10
  [../]
  [./temperature_s]
    block = "casing1 casing2 casing3 casing4 matrix cement"
    scaling = 1e-10
  [../]
[]

[Kernels]
  [./T_diff_well]  # Thermal diffusion kernel for the upper and lower borehole section
    block = "well_upper well_lower"
    type = TigerThermalDiffusionKernelT
    variable = temperature_f
  [../]
  [./TH_adv_well]  # Thermal advection kernel for the upper and lower borehole section
    block = "well_upper well_lower"
    type = TigerThermalAdvectionKernelT
    variable = temperature_f
  [../]
  [./time_derivativeT_well] # Thermal Transient kernel for the upper and lower borehole section
    block = "well_upper well_lower"
    type = TigerThermalTimeKernelT
    variable = temperature_f
  [../]
  [./T_diff_s]  # Thermal diffusion kernel for the solid structures (casings, cement and matrix)
    block = "casing1 casing2 casing3 casing4 matrix cement"
    type = TigerThermalDiffusionKernelT
    variable = temperature_s
  [../]
  [./time_derivativeT_s] # Thermal transient kernel for the solid structures (casings, cement and matrix)
    block = "casing1 casing2 casing3 casing4 matrix cement"
    type = TigerThermalTimeKernelT
    variable = temperature_s
  [../]
[]

[Preconditioning]
  active = p3
  [./p1]
    full = true
    petsc_options_iname = "-pc_type -pc_hypre_type"
    petsc_options_value = "hypre boomeramg"
    type = SMP
  [../]
  [./p2]
    full = true
    petsc_options_iname = "-pc_type -sub_pc_type -sub_pc_factor_shift_type -ksp_gmres_restart"
    petsc_options_value = "asm lu NONZERO 51"
    type = SMP
  [../]
  [./p3]
    full = true
    petsc_options_iname = "-pc_type -ksp_type -sub_pc_type -pc_asm_overlap -sub_pc_factor_shift_type -ksp_gmres_restart"
    petsc_options_value = "lu gmres lu 2 NONZERO 51"
    type = SMP
  [../]
  [./p4]
    full = true
    petsc_options_iname = "-pc_type -pc_factor_mat_solver_package -snes_linesearch_minlambda -ksp_gmres_restart"
    petsc_options_value = "lu mumps 1e-3 51"
    type = SMP
  [../]
[]

[Executioner]
  type = Transient
  end_time = 3600
  l_max_its = 15
  l_tol = 1e-13
  line_search = none
  nl_abs_tol = 1e-10
  nl_max_its = 15
  nl_rel_step_tol = 1e-13
  solve_type = NEWTON
  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 10
  [../]
[]

[Outputs]
  exodus = true
[]
