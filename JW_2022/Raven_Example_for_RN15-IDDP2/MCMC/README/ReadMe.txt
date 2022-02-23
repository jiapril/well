This a showcase for using MCMC method for data inversion.

The example is based on the RN-15/Iddp2 well, where temperature measurements taken from
the borehole (temperature log) are used to determine the static formation temperature (SFT) around
the borehole as well as the unknown flow losses (fls1, fls2, fls3) at three loss zones.

To solve this problem, the forward modeling step uses a trained neural network model (rom.pk) which returns 
the borehole temperature predictions (i.e., temperatures at 44 different depths) given SFT, fls1, fls2, fls3.
For how to train rom.pk? Please go to the other two examples given in folder "Forward_sampling" and "TrainingSurrogate". The sampling step uses Markov chain Monte Carlo (metropolis hasting) method which returns
the probabilistic density distributions of the estimated variables.

############ file description
mcmc.xml: raven input file to perform MCMC

rom.pk: a pre-existing machine learning model (reduced-order model) that predicts borehole temperatures (i.e., temperatures at 44 different depths) given SFT, fls1, fls2, fls3.

Measured_temperature_log.csv: real temperature measurements at 44 depths from the RN-15/Iddp2 well.

likelihood_observed.py: used-defined likelihood function



