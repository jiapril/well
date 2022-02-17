/**************************************************************************/
/*  TIGER - Hydro-thermal sImulator GEothermal Reservoirs                 */
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

#include "ConvectiveHeatFluxInterface.h"

// MOOSE includes
#include "MooseVariable.h"

registerMooseObject("TigerApp", ConvectiveHeatFluxInterface);

template <>
InputParameters
validParams<ConvectiveHeatFluxInterface>()
{
  InputParameters params = validParams<InterfaceKernel>();
  params.addClassDescription("lateral heat flux from fluid to the surrouding solid structures,"
                             "i.e. casings and formation");
  return params;
}

ConvectiveHeatFluxInterface::ConvectiveHeatFluxInterface(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _h(getMaterialProperty<Real>("heat_transfer_coefficient"))
{

}

Real
ConvectiveHeatFluxInterface::computeQpResidual(Moose::DGResidualType type)
{
   Real r=0;
  switch (type)
  {
    case Moose::Element:
         r = _test[_i][_qp] * _h[_qp] * (_u[_qp] - _neighbor_value[_qp]);
         break;

    case Moose::Neighbor:
         r = _test_neighbor[_i][_qp] * _h[_qp] * (-_u[_qp]  + _neighbor_value[_qp]);
         break;
  }
    return r;
}

Real ConvectiveHeatFluxInterface::computeQpJacobian(Moose::DGJacobianType type)
{
  Real jac = 0;

  switch (type)
  {
    case Moose::ElementElement:
      jac = _test[_i][_qp] * _h[_qp] * _phi[_j][_qp];
      break;
    case Moose::NeighborNeighbor:
      jac = _test_neighbor[_i][_qp] * _h[_qp] * _phi_neighbor[_j][_qp];
      break;
    case Moose::NeighborElement:
      jac = -_test_neighbor[_i][_qp] * _h[_qp] * _phi[_j][_qp];
      break;

    case Moose::ElementNeighbor:
      jac = -_test[_i][_qp] * _h[_qp]  * _phi_neighbor[_j][_qp];
      break;
  }

  return jac;
}
