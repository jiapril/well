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

#ifndef WBCOUPLEDCONVECTIVEFLUX_H
#define WBCOUPLEDCONVECTIVEFLUX_H

#include "IntegratedBC.h"

class WBCoupledConvectiveFlux : public IntegratedBC
{
public:
   WBCoupledConvectiveFlux(const InputParameters & parameters);
  virtual ~ WBCoupledConvectiveFlux() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  unsigned int _T_var;
  const VariableValue & _T_fluid;
  const VariableValue & _coefficient;

};

template <>
InputParameters validParams< WBCoupledConvectiveFlux>();

#endif // WBCOUPLEDCONVECTIVEFLUX_H
