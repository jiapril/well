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

#include "WBCoupledHeatSourceT.h"
#include "MooseMesh.h"
#include "MooseVariable.h"


// MOOSE
#include "Function.h"

registerMooseObject("TigerApp", WBCoupledHeatSourceT);
template <>
InputParameters
validParams<WBCoupledHeatSourceT>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("T_wall", "Wall temperature");
  return params;
}

WBCoupledHeatSourceT::WBCoupledHeatSourceT(const InputParameters & parameters)
  : Kernel(parameters), _T_var(coupled("T_wall")),
    _T_wall(coupledValue("T_wall")),
    _well_perimeter(getMaterialProperty<Real>("well_perimeter_t")),
    _h(getMaterialProperty<Real>("heat_transfer_coefficient")),
    _SUPG_p(getMaterialProperty<RealVectorValue>("thermal_petrov_supg_p_function")),
    _SUPG_ind(getMaterialProperty<bool>("thermal_supg_indicator"))
{
  if (_var.number() == _T_var)
  mooseError("Coupled variable 'T_wall' needs to be different from 'variable' with CoupledHeatSource");
}

Real
WBCoupledHeatSourceT::computeQpResidual()
{
  Real r = _h[_qp] * (_T_wall[_qp] - _u[_qp]);
  if (_SUPG_ind[_qp])
      r *= -(_test[_i][_qp] + _SUPG_p[_qp] * _grad_test[_i][_qp]);
  else
      r *= -_test[_i][_qp];
  return _well_perimeter[_qp] * r;
}

Real
WBCoupledHeatSourceT::computeQpJacobian()
{
  Real r = -_h[_qp] * _phi[_j][_qp];
  if (_SUPG_ind[_qp])
      r *= -(_test[_i][_qp] + _SUPG_p[_qp] * _grad_test[_i][_qp]);
  else
      r *= -_test[_i][_qp];
  return _well_perimeter[_qp] * r;
}

Real
WBCoupledHeatSourceT::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real r = _h[_qp] * _phi[_j][_qp];
  if (jvar == _T_var)
    { if (_SUPG_ind[_qp])
         r *= -(_test[_i][_qp] + _SUPG_p[_qp] * _grad_test[_i][_qp]);
      else
         r *= -_test[_i][_qp];
      return _well_perimeter[_qp] * r ;
    }
  return 0.0;
}
