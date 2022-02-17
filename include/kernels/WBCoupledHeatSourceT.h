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


#ifndef WBCOUPLEDHEATSOURCET_H
#define WBCOUPLEDHEATSOURCET_H

#include "Kernel.h"

class WBCoupledHeatSourceT;
class Function;

template <>
InputParameters validParams<WBCoupledHeatSourceT>();

class WBCoupledHeatSourceT : public Kernel
{
public:
  WBCoupledHeatSourceT(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  unsigned int _T_var;
  const VariableValue & _T_wall;
  const MaterialProperty<Real> & _well_perimeter;
  const MaterialProperty<Real> & _h;
  const MaterialProperty<RealVectorValue> & _SUPG_p;
  const MaterialProperty<bool> & _SUPG_ind;

};

#endif  //COUPLEDHEATSOURCET_H
