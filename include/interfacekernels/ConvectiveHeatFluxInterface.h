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
#ifndef CONVECTIVEHEATFLUXINTERFACE_H
#define CONVECTIVEHEATFLUXINTERFACE_H

#include "InterfaceKernel.h"
#include "Function.h"

class ConvectiveHeatFluxInterface;

template <>
InputParameters validParams<ConvectiveHeatFluxInterface>();

/**
 * InterfaceKernel to enforce a Lagrange-Multiplier based componentwise
 * continuity of a variable gradient.
 */
class ConvectiveHeatFluxInterface : public InterfaceKernel
{
public:
  ConvectiveHeatFluxInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  const MaterialProperty<Real> & _h;
};

#endif // CONVECTIVEHEATFLUXINTERFACE_H
