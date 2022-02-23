[Mesh]
  file = mesh.msh
  type = FileMesh
[]
[Problem]
  coord_type = RZ
  rz_coord_axis = X
[]
[MeshModifiers]
  [./interface_1]
    master_block = 'well_upper'
    new_boundary = 'master1_interface'
    paired_block = 'casing1'
    type = SideSetsBetweenSubdomains
  [../]
  [./break_1]
    input = interface_1
    type = BreakBoundaryOnSubdomain
  [../]
  [./interface_2]
    master_block = 'well_lower'
    new_boundary = 'master2_interface'
    paired_block = 'matrix'
    type = SideSetsBetweenSubdomains
  [../]
  [./break_2]
    input = interface_2
    type = BreakBoundaryOnSubdomain
  [../]
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
    k0 = '1.0e-6'
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
    axis = x
    data_file = SFT.csv
    format = columns
    type = PiecewiseLinear
  [../]
  [./vel_well_upper]
    data_file = flow_rate_file_for_moose_input_mp123_10s.csv
    format = columns
    scale_factor = -0.026192184
    type = PiecewiseLinear
  [../]
  [./vel_well_lower]
    data_file = flow_rate_file_for_moose_input_mp123_10s.csv
    format = columns
    scale_factor = -0.02734055
    type = PiecewiseLinear
  [../]
  # Ratio of the remaining flow rate below the first loss zone  at 3400 m 
  # to the flow rate above the the first loss zone
  [./fluid_loss1] 
    data_file = flow_loss1.txt
    direction = 'right right'
    type = PiecewiseMulticonstant
  [../]
  # Ratio of the remaining flow rate below the second loss zone at 4300 m 
  # to the flow rate above the loss zone 
  [./fluid_loss2] 
    data_file = flow_loss2.txt
    direction = 'right right'
    type = PiecewiseMulticonstant
  [../]
  # Ratio of the remaining flow rate below the third loss zone at 4375 m 
  # to the flow rate above the loss zone 
  [./fluid_loss3] 
    data_file = flow_loss3.txt
    direction = 'right right'
    type = PiecewiseMulticonstant
  [../]
  # Multiplication of the three loss ratios, which returns the absolute 
  # ratio of the remaining flow rate after each flow loss zone to the total
  # injected flow at the well-head
  [./fluid_rate_ratio] 
    type=CompositeFunction
    functions='fluid_loss1 fluid_loss2 fluid_loss3'
[../]
[]

[Materials]
  [./f_well]
    block = 'well_upper well_lower'
    fp_uo = water
    temperature = temperature_f
    type = TigerFluidMaterial
  [../]
  [./f_matrix]
    block = 'casing1 casing2 casing3 casing4 cement matrix'
    fp_uo = water
    temperature = temperature_s
    type = TigerFluidMaterial
  [../]
  [./geometry_fluid_flowing_part]
    block = 'well_upper well_lower'
    #porosity = 1.0
    type = TigerGeometryMaterial
  [../]
  [./geometry_solid_part]
    block = 'casing1 casing2 casing3 casing4 cement matrix'
    #porosity = 0.0
    type = TigerGeometryMaterial
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
    advection_type = pure_diffusion
    block = 'casing1 casing2 casing3 casing4'
    conductivity_type = isotropic
    #density_rock = 8000
    lambda = 50
    specific_heat = 500
    type = TigerThermalMaterialT
  [../]
  [./cement_T]
    advection_type = pure_diffusion
    block = 'cement'
    conductivity_type = isotropic
    #density_rock =  2010
    lambda =  1.75
    specific_heat =  950
    type = TigerThermalMaterialT
  [../]
  [./matrix_t]
    advection_type = pure_diffusion
    block = 'matrix'
    conductivity_type = isotropic
    #density_rock = 2650
    lambda = 3.0
    specific_heat = 900
    type = TigerThermalMaterialT
  [../]
  [./well_upper_t]
    type = WB2DThermalMaterialT
    advection_type = velocity_three_components
    block = 'well_upper'
    conductivity_type = orthotropic
    density = 0
    fluid_remain_factor = fluid_rate_ratio
    has_supg = true
    heat_transfer_direction = y
    lambda =  '0.0 10000'
    mean_calculation_type = wellbore_specific
    specific_heat = 0
    supg_uo = supg
    velocity_component_x = vel_well_upper
    well_radius = 0.11024
  [../]
  [./well_lower_t]
    type = WB2DThermalMaterialT
    advection_type = velocity_three_components
    block = 'well_lower'
    conductivity_type = orthotropic
    density = 0
    fluid_remain_factor =  fluid_rate_ratio
    has_supg = true
    heat_transfer_direction = y
    lambda =  '0.0 10000'
    mean_calculation_type = wellbore_specific
    specific_heat = 0
    supg_uo = supg
    velocity_component_x = vel_well_lower
    well_radius = 0.107950
  [../]
[]

[InterfaceKernels]
  [./interface_1]
    type = ConvectiveHeatFluxInterface
    boundary = 'master1_interface'
    neighbor_var = temperature_s
    variable = temperature_f
  [../]
  [./interface_2]
    type = ConvectiveHeatFluxInterface
    boundary = 'master2_interface'
    neighbor_var = temperature_s
    variable = temperature_f
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
    boundary = 'well_top'
    type = DirichletBC
    value =7.5
    variable = temperature_f
  [../]
  [./matrix_right_t]
    boundary = 'right'
    function = SFT
    type = FunctionDirichletBC
    variable = temperature_s
  [../]
[]

[AuxVariables]
  [./fluid_remain_factor]
    block = 'well_upper well_lower'
    family = MONOMIAL
    order = CONSTANT
  [../]
[]
[AuxKernels]
  [./fluid_remain_factor_ker]
    block = 'well_upper well_lower'
    property = 'fluid_remain_factor'
    type = MaterialRealAux
    variable = fluid_remain_factor
  [../]
[]
[Variables]
  # [./pressure]
  #   block = 'well'
  # [../]
  [./temperature_f]
    block = 'well_upper well_lower'
    scaling = 1e-10
  [../]
  [./temperature_s]
    block = 'casing1 casing2 casing3 casing4 matrix cement'
    scaling = 1e-10
  [../]
[]
[Kernels]
  [./T_diff_well]
    block = 'well_upper well_lower'
    type = TigerThermalDiffusionKernelT
    variable = temperature_f
  [../]
  [./TH_adv_well]
    block = 'well_upper well_lower'
    type = TigerThermalAdvectionKernelT
    variable = temperature_f
  [../]
  [./time_derivativeT_well]
    block = 'well_upper well_lower'
    type = TigerThermalTimeKernelT
    variable = temperature_f
  [../]
  [./T_diff_s]
    block = 'casing1 casing2 casing3 casing4 matrix cement'
    type = TigerThermalDiffusionKernelT
    variable = temperature_s
  [../]
  [./time_derivativeT_s]
    block = 'casing1 casing2 casing3 casing4 matrix cement'
    type = TigerThermalTimeKernelT
    variable = temperature_s
  [../]
[]
[Postprocessors]
[./pointvalue1]
  point='-2503.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue2]
  point='-2597.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue3]
  point='-2691.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue4]
  point='-2786.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue5]
  point='-2869.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue6]
  point='-2947.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue7]
  point='-3031.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue8]
  point='-3117.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue9]
  point='-3202.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue10]
  point='-3288.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue11]
  point='-3399.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue12]
  point='-3406.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue13]
  point='-3487.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue14]
  point='-3567.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue15]
  point='-3648.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue16]
  point='-3727.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue17]
  point='-3807.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue18]
  point='-3886.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue19]
  point='-3965.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue20]
  point='-4043.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue21]
  point='-4121.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue22]
  point='-4199.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue23]
  point='-4203.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue24]
  point='-4220.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue25]
  point='-4238.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue26]
  point='-4255.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue27]
  point='-4272.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue28]
  point='-4289.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue29]
  point='-4306.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue30]
  point='-4323.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue31]
  point='-4340.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue32]
  point='-4357.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue33]
  point='-4374.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue34]
  point='-4378.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue35]
  point='-4390.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue36]
  point='-4402.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue37]
  point='-4414.5 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue38]
  point='-4427.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue39]
  point='-4439.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue40]
  point='-4451.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue41]
  point='-4463.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue42]
  point='-4475.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue43]
  point='-4487.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[./pointvalue44]
  point='-4499.0 0.05 0'
  type = PointValue
  variable = temperature_f
 [../]
[]
[Preconditioning]
  active = 'p3'
  [./p1]
    full = true
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = 'hypre boomeramg'
    type = SMP
  [../]
  [./p2]
    full = true
    petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -ksp_gmres_restart'
    petsc_options_value = 'asm lu NONZERO 51'
    type = SMP
  [../]
  [./p3]
    full = true
    petsc_options_iname = '-pc_type -ksp_type -sub_pc_type -pc_asm_overlap -sub_pc_factor_shift_type -ksp_gmres_restart'
    petsc_options_value = 'lu gmres lu 2 NONZERO 51'
    type = SMP
  [../]
  [./p4]
    full = true
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -snes_linesearch_minlambda -ksp_gmres_restart'
    petsc_options_value = 'lu mumps 1e-3 51'
    type = SMP
  [../]
[]
[Executioner]
  end_time = 12576449
  l_max_its = 15
  l_tol =  1e-13 
  line_search = 'none'
  nl_abs_tol =  1e-10 
  nl_max_its =  15 
  nl_rel_step_tol =  1e-13 
  solve_type = 'NEWTON'
  type = Transient
  [./TimeStepper]
    column_name = time
    file_name = time_step_file_for_moose_input_down_1.csv
    type = CSVTimeSequenceStepper
  [../]
[]
[Outputs]
  exodus = false
[]
