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

#ifndef WB1DTHERMALMATERIALT_H
#define WB1DTHERMALMATERIALT_H

#include "Material.h"
#include "RankTwoTensor.h"
#include "TigerSUPG.h"
#include "Function.h"

class WB1DThermalMaterialT;

template <>
InputParameters validParams<WB1DThermalMaterialT>();

class WB1DThermalMaterialT : public Material
{
public:
  WB1DThermalMaterialT(const InputParameters & parameters);

private:
  // enum to select type of advection velocity
  enum AT {pure_diffusion, darcy_velocity, user_velocity, darcy_user_velocities};
  MooseEnum _at;

  // initial thermal conductivity for solid phase
  std::vector<Real> _lambda0;
  // initial specific heat for solid phase
  Real _cp0;
  // initial density for solid phase
  Real _rho0;
  // boolean selecting mode for upwinding and critical numbers output
  bool _has_PeCr;
  bool _has_supg;
  // userdefined factor to manually modify upwinding coefficient
  Real _supg_scale;
  // userdefined factor to manually modify heat transfer coefficient due to natural convection under shut-in condition 
  Real _scale_factor_natural_convection;
  // userdefined velocity vector function for advection
  const Function * _vel_func;

protected:
  virtual void computeQpProperties() override;
  RankTwoTensor Ari_Cond_Calc(Real const & n, Real const & lambda_f, const std::vector<Real> & lambda_s, const int & dim);
  // Peclet number upon request
  MaterialProperty<Real> * _Pe;
  // Courant number upon request
  MaterialProperty<Real> * _Cr;
  MaterialProperty<Real> & _Re;
  MaterialProperty<Real> & _Pr;
  MaterialProperty<Real> & _Nu;
  MaterialProperty<Real> & _h;
  // borehole perimeter
  MaterialProperty<Real> & _well_perimeter;
  // equivalent conductivity of mixture
  MaterialProperty<RankTwoTensor> & _lambda_sf;
  // coefficient for thermal time kernel
  MaterialProperty<Real> & _TimeKernelT;
  // derivative of thermal time kernel coefficient wrt temperature
  MaterialProperty<Real> & _dTimeKernelT_dT;
  // derivative of thermal time kernel coefficient wrt pressure
  MaterialProperty<Real> & _dTimeKernelT_dp;
  // indicator to inform kernels for considering upwinding
  MaterialProperty<bool> & _SUPG_ind;
  // indicator to inform kernels for considering derivative of darcy velocity
  MaterialProperty<bool> & _av_ind;
  // advection velocity
  MaterialProperty<RealVectorValue> & _av;
  // upwinding coefficient
  MaterialProperty<RealVectorValue> & _SUPG_p;

  // imported props from TigerGeometryMaterial
  const MaterialProperty<Real> & _n;
  const MaterialProperty<RankTwoTensor> & _rot_mat;

  // imported props from TigerFluidMaterial
  const MaterialProperty<Real> & _rho_f;
  const MaterialProperty<Real> & _cp_f;
  const MaterialProperty<Real> & _lambda_f;
  const MaterialProperty<Real> & _drho_dT_f;
  const MaterialProperty<Real> & _drho_dp_f;
  const MaterialProperty<Real> & _mu_f;
  const MaterialProperty<Real> & _scale_factor;
  // imported darcy velocity from TigerHydraulicMaterial
  const MaterialProperty<RealVectorValue> * _dv;
  // The ratio of the remaining flow below the loss zone to the flow above the loss zone
  MaterialProperty<Real> & _fluid_remain_factor;
  // function (as of time and space) for the ratio of the remaining flow
  const Function & _fluid_remain_factor0;

  // userobject to calculate upwinding
  const TigerSUPG * _supg_uo;
};

#endif /* TIGERTHERMALMATERIALT_H */
