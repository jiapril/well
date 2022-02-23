import pandas as pd
import os
import csv
import matplotlib.pyplot as plt
import numpy as np
from sklearn.metrics import mean_squared_error
import math

  
def run(self, Input):
        self.grad= Input.get("grad")
        self.temp=287+2000*self.grad
