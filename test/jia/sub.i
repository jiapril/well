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
    scale_factor = 0.038
  [../]
  [./well_porosity]
    type = TigerPorosityMaterial
    porosity = 1.0
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
    conductivity_type = isotropic
    lambda= 1.5
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
    variable =   from_master
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
-20.0 0 0
-30.0 0 0
-40.0 0 0
-50.0 0 0
-60.0 0 0
-70.0 0 0
-80.0 0 0
-90.0 0 0
-100.0 0 0
-110.0 0 0
-120.0 0 0
-130.0 0 0
-140.0 0 0
-150.0 0 0
-160.0 0 0
-170.0 0 0
-180.0 0 0
-190.0 0 0
-200.0 0 0
-210.0 0 0
-220.0 0 0
-240.0 0 0
-250.0 0 0
-270.0 0 0
-280.0 0 0
-290.0 0 0
-300.0 0 0
-310.0 0 0
-320.0 0 0
-330.0 0 0
-340.0 0 0
-350.0 0 0
-360.0 0 0
-370.0 0 0
-400.0 0 0
-410.0 0 0
-420.0 0 0
-430.0 0 0
-440.0 0 0
-450.0 0 0
-460.0 0 0
-470.0 0 0
-480.0 0 0
-490.0 0 0
-500.0 0 0
-510.0 0 0
-520.0 0 0
-530.0 0 0
-540.0 0 0
-550.0 0 0
-560.0 0 0
-570.0 0 0
-590.0 0 0
-600.0 0 0
-610.0 0 0
-620.0 0 0
-630.0 0 0
-640.0 0 0
-650.0 0 0
-660.0 0 0
-670.0 0 0
-680.0 0 0
-690.0 0 0
-700.0 0 0
-710.0 0 0
-720.0 0 0
-730.0 0 0
-740.0 0 0
-750.0 0 0
-760.0 0 0
-770.0 0 0
-780.0 0 0
-790.0 0 0
-810.0 0 0
-840.0 0 0
-850.0 0 0
-860.0 0 0
-900.0 0 0
-910.0 0 0
-930.0 0 0
-940.0 0 0
-960.0 0 0
-980.0 0 0
-990.0 0 0
-1000.0 0 0
-1020.0 0 0
-1040.0 0 0
-1060.0 0 0
-1070.0 0 0
-1090.0 0 0
-1100.0 0 0
-1120.0 0 0
-1130.0 0 0
-1150.0 0 0
-1160.0 0 0
-1180.0 0 0
-1190.0 0 0
-1200.0 0 0
-1230.0 0 0
-1240.0 0 0
-1250.0 0 0
-1260.0 0 0
-1270.0 0 0
-1280.0 0 0
-1320.0 0 0
-1330.0 0 0
-1340.0 0 0
-1350.0 0 0
-1360.0 0 0
-1370.0 0 0
-1380.0 0 0
-1400.0 0 0
-1410.0 0 0
-1420.0 0 0
-1430.0 0 0
-1440.0 0 0
-1460.0 0 0
-1470.0 0 0
-1480.0 0 0
-1490.0 0 0
-1520.0 0 0
-1530.0 0 0
-1540.0 0 0
-1550.0 0 0
-1560.0 0 0
-1570.0 0 0
-1580.0 0 0
-1590.0 0 0
-1610.0 0 0
-1620.0 0 0
-1630.0 0 0
-1640.0 0 0
-1650.0 0 0
-1660.0 0 0
-1670.0 0 0
-1690.0 0 0
-1700.0 0 0
-1710.0 0 0
-1730.0 0 0
-1740.0 0 0
-1760.0 0 0
-1770.0 0 0
-1780.0 0 0
-1790.0 0 0
-1810.0 0 0
-1820.0 0 0
-1840.0 0 0
-1850.0 0 0
-1870.0 0 0
-1880.0 0 0
-1900.0 0 0
-1910.0 0 0
-1920.0 0 0
-1930.0 0 0
-1940.0 0 0
-1950.0 0 0
-1970.0 0 0
-1990.0 0 0
-2010.0 0 0
-2030.0 0 0
-2040.0 0 0
-2050.0 0 0
-2060.0 0 0
-2070.0 0 0
-2080.0 0 0
-2090.0 0 0
-2100.0 0 0
-2120.0 0 0
-2140.0 0 0
-2160.0 0 0
-2170.0 0 0
-2180.0 0 0
-2190.0 0 0
-2200.0 0 0
-2210.0 0 0
-2220.0 0 0
-2230.0 0 0
-2240.0 0 0
-2250.0 0 0
-2270.0 0 0
-2280.0 0 0
-2290.0 0 0
-2310.0 0 0
-2320.0 0 0
-2330.0 0 0
-2340.0 0 0
-2350.0 0 0
-2360.0 0 0
-2370.0 0 0
-2380.0 0 0
-2390.0 0 0
-2400.0 0 0
-2420.0 0 0
-2430.0 0 0
-2440.0 0 0
-2450.0 0 0
-2470.0 0 0
-2480.0 0 0
-2490.0 0 0
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
