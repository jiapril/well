/**************************************************************************/
/*  TIGER - Hydro-thermal sImulator GEothermal Reservoirs                 */
/*                                                                        */
/*  Karlsruhe Institute of Technology, Institute of Applied Geosciences   */
/*  Division of Geothermal Research                                       */
/*                                                                        */
/*  This file is part of TIGER App                                        */
/*                                                                        */
/*  This program is free software: you can redistribute it and/or modify  */
/*  it under the terms of the GNU General Public License as published by  */
/*  the Free Software Foundation, either version 3 of the License, or     */
/*  (at your option) any later version.                                   */
/*                                                                        */
/*  This program is distributed in the hope that it will be useful,       */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          */
/*  GNU General Public License for more details.                          */
/*                                                                        */
/*  You should have received a copy of the GNU General Public License     */
/*  along with this program.  If not, see <http://www.gnu.org/licenses/>  */
/**************************************************************************/

#include "WB2DThermalMaterialT.h"
#include "MooseMesh.h"
#include "libmesh/quadrature.h"
#include <math.h>

#define PI 3.141592653589793238462643383279502884197169399375105820974944592308

registerMooseObject("TigerApp", WB2DThermalMaterialT);

template <>
InputParameters
validParams<WB2DThermalMaterialT>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<Real>("well_radius", "Well inner radius (m)");
  params.addRequiredParam<Real>("specific_heat",
        "Specific heat of rock matrix (J/(kg K))");
  params.addRequiredParam<Real>("density", "density of rock matrix (kg/m^3)");
  params.addRequiredParam<std::vector<Real>>("lambda",
        "Initial thermal conductivity of rock matrix (W/(m K))");
  MooseEnum Advection
        ("pure_diffusion darcy_velocity user_velocity darcy_user_velocities velocity_three_components",
        "darcy_velocity");
  params.addParam<MooseEnum>("advection_type", Advection,
        "Type of the velocity to simulate advection [pure_diffusion "
        "darcy_velocity user_velocity darcy_user_velocities velocity_three_component]");
  MooseEnum Heat_transfer_direction
        ("x y","x");
  params.addParam<MooseEnum>("heat_transfer_direction", Heat_transfer_direction,
        "Heat transfer direction"
        " [x,y]");
  params.addParam<bool>("output_Pe_Cr_numbers", false ,
        "calcuate Peclet and Courant numbers");
  params.addParam<bool>("has_supg", false ,
        "Is Streameline Upwinding / Petrov Galerkin (SU/PG) activated?");
  params.addParam<Real>("supg_coeficient_scale", 1.0 ,
        "The user defined factor to scale SU/PG coefficent (tau)");
  params.addParam<Real>("natural_convection_factor", 1.0, "scale factor for heat transfer coefficient under shut-in");
  params.addParam<FunctionName>("user_velocity", 0.0,
        "a vector function to define the velocity field");
  params.addParam<FunctionName>("velocity_component_x", 0.0, "A scalar to define velocity_x as a function of time and space");
  params.addParam<FunctionName>("velocity_component_y", 0.0, "A scalar to define velocity_y as a function of time and space");
  params.addParam<FunctionName>("velocity_component_z", 0.0, "A scalar to define velocity_z as a function of time and space");
  params.addParam<FunctionName>("fluid_remain_factor", 1.0, "Designed for well where there is drilling loss such as RN15-IDDP2 well,"
  "It means the ratio of the remaining flow below the loss zone to the flow above the loss zone");
  params.addParam<UserObjectName>("supg_uo", "",
        "The name of the userobject for SU/PG");
  params.addClassDescription("Thermal material for thermal kernels. So far it's limited to two-dimensional borehole and two-dimensional "
  "reservior system, but can be modified easily to three-dimensional system");

  return params;
}

WB2DThermalMaterialT::WB2DThermalMaterialT(const InputParameters & parameters)
  : Material(parameters),
    _at(getParam<MooseEnum>("advection_type")),
    _heat_transfer_direction(getParam<MooseEnum>("heat_transfer_direction")),
    _lambda0(getParam<std::vector<Real>>("lambda")),
    _cp0(getParam<Real>("specific_heat")),
    _rho0(getParam<Real>("density")),
    _well_radius(getParam<Real>("well_radius")),
    _has_PeCr(getParam<bool>("output_Pe_Cr_numbers")),
    _has_supg(getParam<bool>("has_supg")),
    _supg_scale(getParam<Real>("supg_coeficient_scale")),
    _scale_factor_natural_convection(getParam<Real>("natural_convection_factor")),
    _Re(declareProperty<Real>("reynold_number")),
    _Pr(declareProperty<Real>("prandtl_number")),
    _Nu(declareProperty<Real>("nusselt_number")),
    _h(declareProperty<Real>("heat_transfer_coefficient")),
    _lambda_sf(declareProperty<RankTwoTensor>("thermal_conductivity_mixture")),
    _TimeKernelT(declareProperty<Real>("TimeKernel_T")),
    _dTimeKernelT_dT(declareProperty<Real>("dTimeKernelT_dT")),
    _dTimeKernelT_dp(declareProperty<Real>("dTimeKernelT_dp")),
    _SUPG_ind(declareProperty<bool>("thermal_supg_indicator")),
    _av_ind(declareProperty<bool>("thermal_av_dv_indicator")),
    _av(declareProperty<RealVectorValue>("thermal_advection_velocity")),
    _SUPG_p(declareProperty<RealVectorValue>("thermal_petrov_supg_p_function")),
    _n(getMaterialProperty<Real>("porosity")),
    _rot_mat(getMaterialProperty<RankTwoTensor>("lowerD_rotation_matrix")),
    _rho_f(getMaterialProperty<Real>("fluid_density")),
    _cp_f(getMaterialProperty<Real>("fluid_specific_heat")),
    _lambda_f(getMaterialProperty<Real>("fluid_thermal_conductivity")),
    _drho_dT_f(getMaterialProperty<Real>("fluid_drho_dT")),
    _drho_dp_f(getMaterialProperty<Real>("fluid_drho_dp")),
    _mu_f(getMaterialProperty<Real>("fluid_viscosity")),
    _fluid_remain_factor(declareProperty<Real>("fluid_remain_factor")),
    _fluid_remain_factor0(getFunction("fluid_remain_factor"))
{
  _Pe = (_has_PeCr || _has_supg) ?
              &declareProperty<Real>("thermal_peclet_number") : NULL;
  _Cr = (_has_PeCr || _has_supg) ?
              &declareProperty<Real>("thermal_courant_number") : NULL;
  _vel_func = (_at == AT::user_velocity || _at == AT::darcy_user_velocities) ?
              &getFunction("user_velocity") : NULL;
  _vel_func_x = (_at == AT::velocity_three_components) ?
                          &getFunction("velocity_component_x") : NULL;
  _vel_func_y = (_at == AT::velocity_three_components) ?
                          &getFunction("velocity_component_y") : NULL;
  _vel_func_z = (_at == AT::velocity_three_components) ?
                          &getFunction("velocity_component_z") : NULL;
  _supg_uo = (parameters.isParamSetByUser("supg_uo")) ?
              &getUserObject<TigerSUPG>("supg_uo") : NULL;
  _dv = (_at == AT::darcy_velocity || _at == AT::darcy_user_velocities) ?
              &getMaterialProperty<RealVectorValue>("darcy_velocity") : NULL;
}

void
WB2DThermalMaterialT::computeQpProperties()
{
  Real rho_m = _n[_qp] * _rho_f[_qp] + (1.0 - _n[_qp]) * _rho0;

  Real mass_frac;

  _fluid_remain_factor[_qp]= _fluid_remain_factor0.value(_t, _q_point[_qp]);

  if (_n[_qp] ==0.0 || _n[_qp] == 1.0)
    mass_frac =  _n[_qp];
  else
  {
    if ((_rho0 - _rho_f[_qp]) == 0.0 || rho_m == 0.0)
      mooseError("Rock density and fluid density are either equal or zero in Thermal Material");
    else
      mass_frac = (_rho0 - rho_m) * _rho_f[_qp] / rho_m / (_rho0 - _rho_f[_qp]);
  }
  Real c_p_m = mass_frac * _cp_f[_qp] + (1.0 - mass_frac) * _cp0;

  _TimeKernelT[_qp] = rho_m * c_p_m;
  _dTimeKernelT_dT[_qp] = _n[_qp] * _drho_dT_f[_qp] * c_p_m;
  _dTimeKernelT_dp[_qp] = _n[_qp] * _drho_dp_f[_qp] * c_p_m;
 
 _lambda_sf[_qp] = Borehole_Thermal_Conductivity(_n[_qp], _lambda_f[_qp], _lambda0, _current_elem->dim());

  if (_current_elem->dim() < _mesh.dimension())
    _lambda_sf[_qp].rotate(_rot_mat[_qp]);

/*
Velocity in the well, but the use of it has strong limitations! 
    o Only for the "velocity_three_compents type". This means: no hydraulic equations coupled here!
    o Only for one-dimensional flow! This means: only one velocity component out of _vel_func_x, _vel_func_y,_vel_func_z is one-zero (this component
      is in the direction of injection), the other two components are zero.
    o That non-zero component (e.g. _vel_func_x) is the well flow velocity when there is no flow loss (e.g., velocity calculated at well-head)
    o The velocity of the flow anywhere is defined as _vel_func_x * _fluid_remain_factor. Herein, the _fluid_remain_factor is a function that can
      both depend on time and space. If no loss presents, _fluid_remain_factor should be 1.0.
  The three velocity component is assigned separately because I would like to make each component a function of time and space.
*/
  switch (_at)  
  {
    case AT::pure_diffusion:
      _av[_qp].zero();
      _av_ind[_qp] = false;
    break;
    case AT::darcy_velocity:
      _av[_qp] = (*_dv)[_qp]*_fluid_remain_factor[_qp];
      _av_ind[_qp] = true;
      break;
    case AT::user_velocity:
      _av[_qp] = _vel_func->vectorValue(_t, _q_point[_qp])*_fluid_remain_factor[_qp];
      _av_ind[_qp] = false;
      break;
    case AT::darcy_user_velocities:
      _av[_qp] =( (*_dv)[_qp] + _vel_func->vectorValue(_t, _q_point[_qp]))*_fluid_remain_factor[_qp];
      _av_ind[_qp] = true;
      break;
    case AT:: velocity_three_components:
      _av[_qp](0) = _vel_func_x->value(_t, _q_point[_qp])*_fluid_remain_factor[_qp];
      _av[_qp](1) = _vel_func_y->value(_t, _q_point[_qp])*_fluid_remain_factor[_qp];
      _av[_qp](2) = _vel_func_z->value(_t, _q_point[_qp])*_fluid_remain_factor[_qp];
      _av_ind[_qp] = false;
      break;
  }

  Real lambda = _lambda_sf[_qp].trace() / (_current_elem->dim() * _TimeKernelT[_qp]);

  _Pr[_qp]= _mu_f[_qp]*_cp_f[_qp]/_lambda_f[_qp];

  _Re[_qp]= 2 * _av[_qp].norm() * _rho_f[_qp] * _well_radius/_mu_f[_qp];

  //caculation of Nusselt number 
   if (_Re[_qp]>1e4)
     _Nu[_qp]=0.023*pow(_Re[_qp],0.8)*pow(_Pr[_qp],0.3);   //Dittus-Boelter correlation for turbulent flow
  else if (_av[_qp].norm()<1e-6 ||_Re[_qp]<1e-6)           //shut-in condition
     _Nu[_qp]=2.0*_scale_factor_natural_convection;
  else if (_Re[_qp]< 2300)
     _Nu[_qp]=4.36;                                        //laminar flow
  else
     _Nu[_qp]=4.36+(_Re[_qp]-2300)*(36.45*pow(_Pr[_qp],0.3)-4.36)/7700;     //linear interpolation between laminar and turbulent flow

   _h[_qp]= _Nu[_qp]*_lambda_f[_qp]/(2*_well_radius);

  if (_has_PeCr && !_has_supg)
    _supg_uo->PeCrNrsCalculator(lambda, _dt, _current_elem, _av[_qp], (*_Pe)[_qp], (*_Cr)[_qp]);

  if (_has_supg)
  {
    // should be multiplied by the gradient of the test function to build the Petrov Galerkin P function
    _supg_uo->SUPGCalculator(lambda, _dt, _current_elem, _av[_qp], _SUPG_p[_qp], (*_Pe)[_qp], (*_Cr)[_qp]);
    _SUPG_p[_qp] *= _supg_scale;

    if (_SUPG_p[_qp].norm() != 0.0)
      _SUPG_ind[_qp] = true;
    else
      _SUPG_ind[_qp] = false;
  }
  else
    _SUPG_ind[_qp] = false;
}


/*
The values for borehole thermal conductivity are assigned in two directions!
This is essentially a trick that makes the thermal modeling in the borehole one-dimensional, despite that the
mesh is two-dimensional. To do this, the thermal conductivity in the flow direction is the same as the thermal conductivity
of the fluid. The "_heat_transfer_direction" is the direction of the lateral heat transfer, which is perpendicular to the
flow direction. The user can decide this direction according to the coordinate of the mesh. The thermal conductivity in the direction of lateral 
heat transfer needs to be given. This vaule should be large enough such that the temperature at the borehole cross section is homongenous.
A figure to explain this idea is also given in folder "supplementary_material".
*/

RankTwoTensor
WB2DThermalMaterialT::Borehole_Thermal_Conductivity(Real const & n, Real const & lambda_f, const std::vector<Real> & lambda_s, const int & dim)
{
  RankTwoTensor lambda = RankTwoTensor();
  RealVectorValue lambda_x;
  RealVectorValue lambda_y;
  RealVectorValue lambda_z;
  lambda_x.zero();
  lambda_y.zero();
  lambda_z.zero();

 if (dim !=2)
     mooseError("Only two-dimensional elements can use type of material.\n");

 else
  {
    switch (_heat_transfer_direction)
    {
      case HTD::x:
          if (lambda_s.size() != 2)
             mooseError("Two input values are needed for orthotropic distribution of thermal conductivity in two dimensional elements! You provided ", lambda_s.size(), " values.\n");
          lambda  = (1.0 - n)    * RankTwoTensor(lambda_f, lambda_s[1], 0., 0., 0., 0.);
          lambda += n * RankTwoTensor(lambda_s[0], lambda_f, 0., 0., 0., 0.);       // Arithmetic mean    
          break;

      case HTD::y:
          if (lambda_s.size() != 2)
             mooseError("Two input values are needed for orthotropic distribution of thermal conductivity in two dimensional elements! You provided ", lambda_s.size(), " values.\n");
          lambda  = (1.0 - n)    * RankTwoTensor(lambda_s[0], lambda_f, 0., 0., 0., 0.);
          lambda += n * RankTwoTensor(lambda_f, lambda_s[1], 0., 0., 0., 0.);      // Arithmetic mean  
          break;
    }
  }
  return lambda;
}
