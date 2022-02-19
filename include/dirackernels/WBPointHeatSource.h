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

#ifndef WBPOINTHEATSOURCE_H
#define WBPOINTHEATSOURCE_H
#include "DiracKernel.h"
#include "Function.h"

class WBPointHeatSource;

template <>
InputParameters validParams<WBPointHeatSource>();

/*
 * Point source (or sink) that adds (removes) heat at constant rate
 * for times between the specified start and end times. If no start and end
 * times are specified, the source (sink) starts at the start of the simulation
 * and continues to act indefinitely.
 */
class WBPointHeatSource : public DiracKernel
{
public:
  WBPointHeatSource(const InputParameters & parameters);

  virtual void addPoints() override;
  virtual Real computeQpResidual() override;

protected:
  // weighting factor of point
  const Real _weight_factor;
  // userdefined heat flux (W/m^3)
  const Real _heat_flux;
  // The location of the point source (sink)
  const Point _p;
  // The time at which the point source (sink) starts operating
  const Real _start_time;
  // The time at which the point source (sink) stops operating
  const Real _end_time;
  // flow rate is function of time (kg/s)
  const Function * _heat_flux_function;
};

#endif // WBPOINTHEATSOURCE_H
