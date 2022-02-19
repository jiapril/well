/**************************************************************************/
/*  TIGER - THMC sImulator for GEoscience Research                        */
/*                                                                        */
/*  Copyright (C) 2017 by Maziar Gholami Korzani                          */
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

#include "WB1DThermalMaterialT.h"
#include "MooseMesh.h"
#include "libmesh/quadrature.h"

registerMooseObject("TigerApp", WB1DThermalMaterialT);

template <>
InputParameters
validParams<WB1DThermalMaterialT>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<Real>("specific_heat",
        "Specific heat of rock matrix (J/(kg K))");
  params.addRequiredParam<Real>("density", "density of rock matrix (kg/m^3)");
  params.addRequiredParam<std::vector<Real>>("lambda",
        "Initial thermal conductivity of rock matrix (W/(m K))");
  MooseEnum Advection
        ("pure_diffusion darcy_velocity user_velocity darcy_user_velocities ",
        "darcy_velocity");
  params.addParam<MooseEnum>("advection_type", Advection,
        "Type of the velocity to simulate advection [pure_diffusion "
        "darcy_velocity user_velocity darcy_user_velocities]");
  MooseEnum CT("isotropic orthotropic anisotropic");
  params.addRequiredParam<MooseEnum>("conductivity_type", CT,
        "Thermal conductivity distribution type [isotropic, orthotropic, anisotropic]");
  MooseEnum Mean("arithmetic geometric", "arithmetic");
  params.addParam<MooseEnum>("mean_calculation_type", Mean,
        "Solid-liquid mixture thermal conductivity calculation method "
        "[arithmetic, geometric]");
  params.addParam<bool>("output_Pe_Cr_numbers", false ,
        "calcuate Peclet and Courant numbers");
  params.addParam<bool>("has_supg", false ,
        "Is Streameline Upwinding / Petrov Galerkin (SU/PG) activated?");
  params.addParam<Real>("supg_coeficient_scale", 1.0 ,
        "The user defined factor to scale SU/PG coefficent (tau)");
  params.addParam<FunctionName>("user_velocity", 0.0,
        "a vector function to define the velocity field");
  params.addParam<Real>("natural_convection_factor", 1.0, "Scale factor for heat transfer coefficient due to natural convection " 
  "under shut-in condition. The default 1.0 corresponds to when there is no natural convection and only conduction exisiting. "
  "Refer to Wang et.al 2019 for the reason of using this factor");
  params.addParam<FunctionName>("fluid_remain_factor", 1.0, "Designed for well where there is drilling loss such as RN15-IDDP2 well, "
  "It means the ratio of the remaining flow below the loss zone to the flow above the loss zone");
  params.addParam<UserObjectName>("supg_uo", "",
        "The name of the userobject for SU/PG");
  params.addClassDescription("Thermal material for thermal kernels (ONLY for 1-D well in XYZ coordinate system)");

  return params;
}

WB1DThermalMaterialT::WB1DThermalMaterialT(const InputParameters & parameters)
  : Material(parameters),
    _at(getParam<MooseEnum>("advection_type")),
    _ct(getParam<MooseEnum>("conductivity_type")),
    _mean(getParam<MooseEnum>("mean_calculation_type")),
    _lambda0(getParam<std::vector<Real>>("lambda")),
    _cp0(getParam<Real>("specific_heat")),
    _rho0(getParam<Real>("density")),
    _has_PeCr(getParam<bool>("output_Pe_Cr_numbers")),
    _has_supg(getParam<bool>("has_supg")),
    _supg_scale(getParam<Real>("supg_coeficient_scale")),
    _scale_factor_natural_convection(getParam<Real>("natural_convection_factor")),
    _Re(declareProperty<Real>("reynold_number")),
    _Pr(declareProperty<Real>("prandtl_number")),
    _Nu(declareProperty<Real>("nusselt_number")),
    _h(declareProperty<Real>("heat_transfer_coefficient")),
    _well_perimeter(declareProperty<Real>("well_perimeter_t")),
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
    _scale_factor(getMaterialProperty<Real>("scale_factor")),
    _fluid_remain_factor(declareProperty<Real>("fluid_remain_factor")),
    _fluid_remain_factor0(getFunction("fluid_remain_factor"))
{
  _Pe = (_has_PeCr || _has_supg) ?
              &declareProperty<Real>("thermal_peclet_number") : NULL;
  _Cr = (_has_PeCr || _has_supg) ?
              &declareProperty<Real>("thermal_courant_number") : NULL;
  _vel_func = (_at == AT::user_velocity || _at == AT::darcy_user_velocities) ?
              &getFunction("user_velocity") : NULL;
  _supg_uo = (parameters.isParamSetByUser("supg_uo")) ?
              &getUserObject<TigerSUPG>("supg_uo") : NULL;
  _dv = (_at == AT::darcy_velocity || _at == AT::darcy_user_velocities) ?
              &getMaterialProperty<RealVectorValue>("darcy_velocity") : NULL;
}

void
WB1DThermalMaterialT::computeQpProperties()
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

  switch (_mean)
  {
    case M::arithmetic:
      _lambda_sf[_qp] = Ari_Cond_Calc(_n[_qp], _lambda_f[_qp], _lambda0, _current_elem->dim());
      break;
    case M::geometric:
      _lambda_sf[_qp] = Geo_Cond_Calc(_n[_qp], _lambda_f[_qp], _lambda0, _current_elem->dim());
      break;
  }

  if (_current_elem->dim() < _mesh.dimension())
    _lambda_sf[_qp].rotate(_rot_mat[_qp]);

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
      _av[_qp] = (*_dv)[_qp] + _vel_func->vectorValue(_t, _q_point[_qp]);
      _av_ind[_qp] = true;
      break;
  }

  Real lambda = _lambda_sf[_qp].trace() / (_current_elem->dim() * _TimeKernelT[_qp]);

  _Pr[_qp]= _mu_f[_qp]*_cp_f[_qp]/_lambda_f[_qp];

  _coord_type = _subproblem.getCoordSystem(_current_elem->subdomain_id());

  _well_perimeter[_qp] = 2 * pow(_scale_factor[_qp]/PI, 0.5) * PI;
  _Re[_qp]= 2*_av[_qp].norm()*_rho_f[_qp]*pow(_scale_factor[_qp]/PI, 0.5)/_mu_f[_qp];
  
  //caculation of Nusselt number 
  if (_Re[_qp]>1e4)
       _Nu[_qp]=0.023*pow(_Re[_qp],0.8)*pow(_Pr[_qp],0.4);   //Dittus-Boelter correlation for turbulent flow
  else if (_av[_qp].norm()<1e-6 ||_Re[_qp]<1e-6)            //shut-in condition
       _Nu[_qp]=2.0*_scale_factor_natural_convection;  
  else if (_Re[_qp]< 2300)                                 //laminar flow
       _Nu[_qp]=4.36;
  else
       _Nu[_qp]=4.36+(_Re[_qp]-2300)*(36.45*pow(_Pr[_qp],0.3)-4.36)/7700; //linear interpolation between laminar and turbulent flow
       
  _h[_qp]= _Nu[_qp]*_lambda_f[_qp]/(pow(_scale_factor[_qp]/PI, 0.5)*2);
          
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

RankTwoTensor
WB1DThermalMaterialT::Ari_Cond_Calc(Real const & n, Real const & lambda_f, const std::vector<Real> & lambda_s, const int & dim)
{
  RankTwoTensor lambda = RankTwoTensor();
  RealVectorValue lambda_x;
  RealVectorValue lambda_y;
  RealVectorValue lambda_z;
  lambda_x.zero();
  lambda_y.zero();
  lambda_z.zero();

  switch (_ct)  //modified from TigerThermalMaterialT, but _ct is not necessary here.
  {
      case CT::isotropic:
        if (lambda_s.size() != 1)
          mooseError("One input value is needed for isotropic distribution of thermal conductivity! You provided ", lambda_s.size(), " values.\n");
        lambda = ((1.0 - n) * lambda_s[0] + n * lambda_f) * RankTwoTensor(1., 0., 0., 0., 0., 0.);
        break;
      case CT::orthotropic ... CT::anisotropic:
        mooseError("One dimensional elements cannot have non-isotropic thermal conductivity values.\n");
        break;
    }

  return lambda;
}

RankTwoTensor
WB1DThermalMaterialT::Geo_Cond_Calc(Real const & n, Real const & lambda_f, const std::vector<Real> & lambda_s, const int & dim)
{
  RankTwoTensor lambda = RankTwoTensor();
  RealVectorValue lambda_x;
  RealVectorValue lambda_y;
  RealVectorValue lambda_z;
  lambda_x.zero();
  lambda_y.zero();
  lambda_z.zero();

  switch (_ct)
  {
      case CT::isotropic:
        if (lambda_s.size() != 1)
          mooseError("One input value is needed for isotropic distribution of thermal conductivity! You provided ", lambda_s.size(), " values.\n");
        lambda = RankTwoTensor(1., 0., 0., 0., 0., 0.) * std::pow(lambda_f,n) * std::pow(lambda_s[0],(1.-n));
      break;
      case CT::orthotropic ... CT::anisotropic:
        mooseError("One dimensional elements cannot have non-isotropic thermal conductivity values.\n");
        break;
    }

  return lambda;
}
