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

#include "WBPointHeatSource.h"
#include "FEProblemBase.h"

registerMooseObject("TigerApp", WBPointHeatSource);

template <>
InputParameters
validParams<WBPointHeatSource>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<Point>("point", "The x,y,z coordinates of the "
        "source/sink point");
  params.addParam<Real>("weighting_factor", 1.0, "The weighting factor of source/sink heat flux.");
  params.addParam<Real>("heat_flux", 0.0, "The source/sink heat flux.");
  params.addParam<FunctionName>("function", "The function describing the source/sink heat flux.");
  params.addParam<Real>("start_time", 0.0, "The starting time.");
  params.addParam<Real>("end_time", 1.0e30, "The time at which the source will "
        "end");
  params.addClassDescription("Point heat source/sink");
  return params;
}

WBPointHeatSource::WBPointHeatSource(const InputParameters & parameters)
  : DiracKernel(parameters),
    _weight_factor(getParam<Real>("weighting_factor")),
    _heat_flux(getParam<Real>("heat_flux")),
    _p(getParam<Point>("point")),
    _start_time(getParam<Real>("start_time")),
    _end_time(getParam<Real>("end_time"))
{
  _heat_flux_function = isParamValid("heat_flux_function") ?
                        &getFunction("heat_flux_function") : NULL;

  // Sanity check to ensure that the end_time is greater than the start_time
  if (_end_time <= _start_time)
    mooseError("Start time for WBPointHeatSource is ",_start_time,
                " but it must be less than end time ",_end_time);
}

void
WBPointHeatSource::addPoints()
{
  addPoint(_p);
}

Real
WBPointHeatSource::computeQpResidual()
{
  // make it default a sourceterm
  Real factor = -1.0;

  if (isParamValid("heat_flux_function"))
      factor *= _heat_flux_function->value(_t, Point());
  else
  {
    /**
     * There are six cases for the start and end time in relation to t-dt and t.
     * If the interval (t-dt,t) is only partly but not fully within the (start,
     * end) interval, then the  heat_flux is scaled so that the total heat added
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
      factor *=_heat_flux;
  }
  // Negative sign to make a positive heat_flux as a source
  //std::cout <<"heat_flux"<<_heat_flux <<std::endl;
  return -_test[_i][_qp] * _weight_factor * factor;
}
