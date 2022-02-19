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

#ifndef WBMOVINGPOINTSOURCEH_H
#define WBMOVINGPOINTSOURCEH_H

#include "DiracKernel.h"
#include "Function.h"


class WBMovingPointSourceH;

template <>
InputParameters validParams<WBMovingPointSourceH>();

/**
 * Moving Point source (or sink) that adds (removes) fluid at defined mass flux rate for times
 * between the specified start and end times. If no start and end times are specified,
 * the source (sink) starts at the start of the simulation and continues to act indefinitely
 */
class WBMovingPointSourceH : public DiracKernel
{
public:
  WBMovingPointSourceH(const InputParameters & parameters);

  virtual void addPoints() override;
  virtual Real computeQpResidual() override;

protected:
  //x,y,z coordinate of the moving point source
  const Function * _x_function;

  const Function * _y_function;

  const Function * _z_function;

  /// The constant mass flux (kg/s)
  const Real _mass_flux;

  // The time at which the point source (sink) starts operating
  const Real _start_time;
  /// The time at which the point source (sink) stops operating
  const Real _end_time;
  /// fluid density
  const MaterialProperty<Real> & _rhof;
  /// flow rate is function of time (kg/s)
  const Function * _mass_flux_function;
};

#endif // WBMOVINGPOINTSOURCEH_H
