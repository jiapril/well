[Mesh]
  type= FileMesh
  file = well.msh
   uniform_refine = 1
[]

[Functions]
  [./temp]
    type= PiecewiseLinear
    data_file = SFT.csv
    axis= x
    format = columns
  [../]
[]

[Modules]
  [./FluidProperties]
    [./water_uo]
      type = TigerWaterConst
    [../]
  [../]
[]

[UserObjects]
  [./well_uo]
    type =  TigerPermeabilityConst
    permeability_type = isotropic
    k0 = '1.0e-6'
  [../]
  [./supg]
    type = TigerSUPG
    effective_length = average
    supg_coeficient = optimal
  [../]
  [./sub_app_uo]
      type = LayeredAverage
      direction = x
      variable = h
      num_layers = 400
      execute_on = TIMESTEP_END
      use_displaced_mesh = false
  [../]
[]

[Materials]
  [./f_well]
    type = TigerFluidMaterial
    fp_uo = water_uo
  [../]
  [./well_geometry]
    type = TigerGeometryMaterial
    scale_factor = 0.038  ## Must be the area of the borehole cross section!!
  [../]
  [./well_porosity]
    type = TigerPorosityMaterial
    porosity = 1.0   ## Must always be 1.0 !!
    specific_density = 0.0
  [../]
  [./well_h]
     type = TigerHydraulicMaterialH
     pressure = pressure
     compressibility = 1e-10
     kf_uo = well_uo
  [../]
  [./well_t] 
    type =  WB1DThermalMaterialT
    lambda= 1.5  # Provided, but has no meaning!
    density = 2650
    specific_heat = 1000
    advection_type = darcy_velocity
    has_supg = true
    supg_uo = supg
 [../]
[]

[ICs]
  [./init_temperature_f]
    type = FunctionIC
    function = temp
    variable = temperature_f
  [../]
  [./init_main]
    type = FunctionIC
    function = temp
    variable = from_master
  [../]
[]

[BCs]
  [./bottom_h]
    type =  DirichletBC
    variable = pressure
    boundary = 'WellBottom'
    value = 1e7
  [../]
  [./well_top_t]
   type = DirichletBC
   variable = temperature_f
   boundary = 'WellHead'
   value = 7
  [../]
[]

[DiracKernels]
  [./pump_in]
    type = TigerHydraulicPointSourceH
    point = '0 0 0'
    mass_flux = -15
    variable = pressure
  [../]
[]

[AuxVariables]
  [./layered_average]
   order = CONSTANT
   family = MONOMIAL
  [../]
  [./from_master]
  [../]
  [./vx]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./Re]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./Pr]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./h]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./Nu]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./layered_average]
  type = SpatialUserObjectAux
  variable = layered_average
  execute_on = timestep_end
  user_object = sub_app_uo
  [../]
  [./vx_ker]
    type =  TigerDarcyVelocityH
    pressure = pressure
    variable =  vx
    component = x
  [../]
  [./Re_ker]
    type = MaterialRealAux
    property = 'reynold_number'
    variable = Re
  [../]
  [./Pr_ker]
    type = MaterialRealAux
    property = 'prandtl_number'
    variable = Pr
  [../]
  [./h_ker]
    type = MaterialRealAux
    property = 'heat_transfer_coefficient'
    variable = h
  [../]
  [./Nu_ker]
    type = MaterialRealAux
    property = 'nusselt_number'
    variable = Nu
  [../]
[]

[Variables]
  [./temperature_f]
    scaling = 1e-10
  [../]
  [./pressure]
  [../]
[]
[Kernels]
  [./H_diff]
    type =  TigerHydraulicKernelH
    variable = pressure
  [../]
  [./T_diff]
    type = TigerThermalDiffusionKernelT
    variable = temperature_f
  [../]
  [./TH_adv_well]
    type = TigerThermalAdvectionKernelT
    variable = temperature_f
    pressure = pressure
  [../]
  [./T_source]
    type = WBCoupledHeatSourceT
    variable = temperature_f
    T_wall = from_master
  [../]
  [./time_derivative_fluid]
    type =  TigerThermalTimeKernelT
    variable = temperature_f
  [../]
[]

[VectorPostprocessors]
[./point_sample]
  type = PointValueSampler
  sort_by = x
  variable = 'temperature_f'
  points = '-10.0 0 0
-500.0 0 0
-1500.0 0 0
-2000.0 0 0
-2500.0 0 0'
[../]
[]


[Preconditioning]
  active = 'p3'
  [./p1]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = 'hypre boomeramg'
  [../]
  [./p2]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -ksp_gmres_restart'
    petsc_options_value = 'asm lu NONZERO 51'
  [../]
  [./p3]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -ksp_type -sub_pc_type -pc_asm_overlap -sub_pc_factor_shift_type -ksp_gmres_restart'
    petsc_options_value = 'asm gmres lu 2 NONZERO 51'
  [../]
[]

[Executioner]
  type = Transient
  [./TimeStepper]
     type =  SolutionTimeAdaptiveDT
      dt = 100
  [../]
  end_time = 5e3
  l_tol = 1e-10
  l_max_its = 50
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  nl_max_its = 50
  nl_rel_step_tol= 1e-15
  solve_type = 'PJFNK'
  #petsc_options = '-snes_ksp_ew'
  # petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -ksp_max_it -sub_pc_type -sub_pc_factor_shift_type'
  # petsc_options_value = 'gmres asm 1E-10 1E-15 200 500 lu NONZERO'
  # petsc_options_iname = '-pc_type -ksp_type -sub_pc_type -snes_type -snes_linesearch_type -pc_asm_overlap -sub_pc_factor_shift_type -ksp_gmres_restart'
  # petsc_options_value = 'asm gmres lu newtonls basic 2 NONZERO 51'
  # petsc_options_iname = '-pc_type -ksp_type -sub_pc_type -snes_type -snes_linesearch_type -pc_asm_overlap -sub_pc_factor_shift_type -ksp_gmres_restart'
  # petsc_options_value = 'asm gmres lu newtonls basic 2 NONZERO 51'

  #petsc_options = ' -snes_converged_reason -snes_error_if_not_converged -ksp_converged_reason -ksp_error_if_not_converged -snes_view'
  # petsc_options_iname = '-pc_type -pc_hypre_type'
  # petsc_options_value = 'hypre boomeramg'
[]


[Outputs]
  exodus = true
[]
