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

#include "WBMovingPointSourceH.h"
#include "FEProblemBase.h"
registerMooseObject("TigerApp", WBMovingPointSourceH);

template <>
InputParameters
validParams<WBMovingPointSourceH>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addParam<FunctionName>("x_coord_function", "x coordinate of the point as a function of time ");
  params.addParam<FunctionName>("y_coord_function", "y coordinate of the point as a function of time ");
  params.addParam<FunctionName>("z_coord_function", "z coordinate of the point as a function of time ");
  params.addParam<Real>("mass_flux",0.0,"The constant mass flow rate at this point (well bottom) in kg/s (positive is injection, negative is production)");
  params.addParam<FunctionName>("mass_flux_function", "The mass flow rate as a function of time at this point (well bottom) in kg/s"
                                                      "(positive-valued function is injection, negative-valued function is production)");
  params.addParam<Real>("start_time", 0.0, "The time at which the source will start (the case of the constant flow rate)");
  params.addParam<Real>("end_time", 1.0e30, "The time at which the source will end (the case of the constant flow rate)");
  params.addClassDescription("Injection/Production well that adds (removes) fluid at the well point");
  return params;
}

WBMovingPointSourceH::WBMovingPointSourceH(
    const InputParameters & parameters)
  : DiracKernel(parameters),
    _x_function(&getFunction("x_coord_function")),
    _y_function(&getFunction("y_coord_function")),
    _z_function(&getFunction("z_coord_function")),
    _mass_flux(getParam<Real>("mass_flux")),
    _start_time(getParam<Real>("start_time")),
    _end_time(getParam<Real>("end_time")),
    _rhof(getMaterialProperty<Real>("fluid_density")),
    _mass_flux_function(isParamValid("mass_flux_function") ? &getFunction("mass_flux_function") : NULL)
{
  /// Sanity check to ensure that the end_time is greater than the start_time
  if (_end_time <= _start_time)
    mooseError("Start time for TigerPointSourceH is ",_start_time," but it must be less than end time ",_end_time);
}

void
WBMovingPointSourceH::addPoints()
{
    addPoint(Point(_x_function->value(_t, _point_zero), _y_function->value(_t, _point_zero), _z_function->value(_t, _point_zero)));
}

Real
WBMovingPointSourceH::computeQpResidual()
{
  Real factor = 1.0;

  if (isParamValid("mass_flux_function"))
      factor *= _mass_flux_function->value(_t, Point());
  else
  {
    /**
     * There are six cases for the start and end time in relation to t-dt and t.
     * If the interval (t-dt,t) is only partly but not fully within the (start,end)
     * interval, then the  mass_flux is scaled so that the total mass added
     * (or removed) is correct
     */
    if (_t < _start_time || _t - _dt >= _end_time)
      factor = 0.0;
    else if (_t - _dt < _start_time)
    {
      if (_t <= _end_time)
        factor *= (_t - _start_time) / _dt;
      else
        factor *= (_end_time - _start_time) / _dt;
    }
    else
    {
      if (_t <= _end_time)
        factor *= 1.0;
      else
        factor *= (_end_time - (_t - _dt)) / _dt;
    }
      factor *=_mass_flux;
  }
  /// Negative sign to make a positive mass_flux as a source
  return -_test[_i][_qp] * factor /_rhof[_qp];
}
