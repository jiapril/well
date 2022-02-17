
[Mesh]
   type= FileMesh
   file = matrix.msh
[]

[Problem]
  coord_type = RZ
  rz_coord_axis = X
[]

[Modules]
  [./FluidProperties]
    [./water]
      type =  TigerWaterConst
    [../]
  [../]
[]

[Functions]
  [./temp]
    type= PiecewiseLinear
    data_file =  SFT.csv
    axis= x
    format = columns
  [../]
[]

[Materials]
  [./geometry]
    type = TigerGeometryMaterial
  [../]
  [./rock_p]
    type = TigerPorosityMaterial
    porosity = 0.0
    specific_density = 1.0
  [../]
  [./matrix_casing_H]
    fp_uo = water
    #pressure = 3.3973e7
    temperature = temperature_s
    type = TigerFluidMaterial
  [../]
  [./casing_T]
    type = TigerThermalMaterialT
    conductivity_type = isotropic
    lambda= 50
    density = 8000
    specific_heat = 500
    block = 'casing1 casing2 casing3 casing4'
    advection_type = pure_diffusion
  [../]
  [./cement_T]
    type = TigerThermalMaterialT
    conductivity_type = isotropic
    lambda = 1.2
    density = 1760
    specific_heat = 1000
    block = 'cement'
    advection_type = pure_diffusion
  [../]
  [./matrix_t]
    advection_type = pure_diffusion
    block = 'matrix'
    conductivity_type = isotropic
    density= 2650
    lambda = 2.0
    specific_heat = 900
    type = TigerThermalMaterialT
  [../]
[]

[ICs]
  [./init_temperature_s]
    type = FunctionIC
    function = temp
    variable =  temperature_s
  [../]
  [./init_sub]
    type = FunctionIC
    function = temp
    variable =  from_sub
  [../]
[]
[BCs]
  [./matrix_right_t]
   type= FunctionDirichletBC
   variable = temperature_s
   function= temp
   boundary= 'right'
  [../]
  [./matrix_left_t]
   type = WBCoupledConvectiveFlux
   variable = temperature_s
   boundary = 'left'
   T_fluid = from_sub  # the main var of subapp
   coefficient = from_sub_app_var #heat transfer coeffcient from sub
  [../]
[]

[Variables]
  [./temperature_s]
    scaling = 1e-6
  [../]
[]

[AuxVariables]
  [./from_sub]
  [../]
  [./from_sub_app_var]
  [../]
[]

[Kernels]
  [./T_diff]
    type = TigerThermalDiffusionKernelT
    variable = temperature_s
  [../]
  [./time_derivativeT]
    type =  TigerThermalTimeKernelT
    variable = temperature_s
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
  nl_rel_tol = 1e-15
  nl_abs_tol = 1e-15
  nl_max_its = 15
  nl_rel_step_tol= 1e-15
  petsc_options_iname = '-pc_type -ksp_type -sub_pc_type -snes_type -snes_linesearch_type -pc_asm_overlap -sub_pc_factor_shift_type -ksp_gmres_restart'
  petsc_options_value = 'asm gmres lu newtonls basic 2 NONZERO 51'
  solve_type = 'PJFNK'
  picard_max_its = 40
  # l_abs_step_tol = 1e-10
  # nl_abs_tol = 1e-10
  # petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor -ksp_monitor_true_residual -ksp_monitor_singular_value -snes_view'
  # petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  # petsc_options_value = 'hypre boomeramg 150'
  # petsc_options_iname = '-pc_type -pc_hypre_type'
  # petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = True
[]

[MultiApps]
  [./sub]
    type = TransientMultiApp
    app_type = TigerApp
    execute_on = TIMESTEP_BEGIN
    positions = '0 0.11 0'
    input_files =sub.i
  [../]
[]

[Transfers]
  [./to_sub_1]
    type = MultiAppNearestNodeTransfer
    direction = to_multiapp
    multi_app = sub
    source_variable = temperature_s
    source_boundary = 'left'
    variable = from_master
  [../]
  [./from_sub]
    type = MultiAppNearestNodeTransfer
    direction = from_multiapp
    multi_app = sub
    source_variable = temperature_f
    variable = from_sub
    target_boundary = 'left'
  [../]
  [./layered_transfer_from_sub_app]
    type = MultiAppUserObjectTransfer
    direction = from_multiapp
    user_object = sub_app_uo
    variable = from_sub_app_var
    multi_app = sub
    displaced_source_mesh = false
  [../]
[]
