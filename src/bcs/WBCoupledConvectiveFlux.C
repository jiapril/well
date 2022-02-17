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

#include "WBCoupledConvectiveFlux.h"

#include "Function.h"

registerMooseObject("TigerApp", WBCoupledConvectiveFlux);

template <>
InputParameters
validParams<WBCoupledConvectiveFlux>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredCoupledVar("T_fluid", "bulk fluid temperature");
  params.addRequiredCoupledVar("coefficient", "heat transfer coefficient");
  return params;
}

WBCoupledConvectiveFlux::WBCoupledConvectiveFlux(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _T_var(coupled("T_fluid")),
    _T_fluid(coupledValue("T_fluid")),
    _coefficient(coupledValue("coefficient"))
{
 if (_var.number() == _T_var)
  mooseError("Coupled variable 'T_fluid' needs to be different from 'variable' of TigerCoupledConvectiveFlux");
}

Real
WBCoupledConvectiveFlux::computeQpResidual()
{
  return _test[_i][_qp] * _coefficient[_qp] * (_u[_qp] - _T_fluid[_qp]);
}

Real
WBCoupledConvectiveFlux::computeQpJacobian()
{
  return _test[_i][_qp] * _coefficient[_qp] * _phi[_j][_qp];
}

Real
WBCoupledConvectiveFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _T_var)
    {
      return -_test[_i][_qp] * _coefficient[_qp] * _phi[_j][_qp];
    }
  return 0.0;
}
