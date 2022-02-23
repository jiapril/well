# Copyright 2017 Battelle Energy Alliance, LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import scipy.stats as st
import numpy as np
from sklearn.metrics import mean_squared_error
import pandas as pd

def load_logging_data(filename):
    measurment=pd.read_csv(filename)
    df=pd.DataFrame(measurment)
    measured_temp=df['temp']
    depth_measure=df['depth']
    return measured_temp,depth_measure

def pps_name(pps_number):
    pps=[]
    i=0
    while i < pps_number:
        pps.append('pointvalue'+'{}'.format(i+1))
        i=i+1
    return pps

def run(self,Inputs):
    pps=pps_name(44)
    simulated_temp=[]
    i=0
    while i < len(pps):
        simulated_temp.append(Inputs[pps[i]][0])
        i=i+1
    measured_temp,depth_measure=load_logging_data('../{}'.format('Measured_temperature_log.csv'))
    part1=mean_squared_error(simulated_temp, measured_temp)*len(simulated_temp)
    self.pout=-len(simulated_temp)*0.5*np.log(part1)
