This is a example showing how to construct a reduced-order model (ROM) in raven.


###description of the files

ConstructROM.xml: raven input file

training.csv: training data set for constructing the ROM which contains the input (grad, fls1, fls2, fls3) and the output (pointvalue1, ..., pointvalue44)

test.csv: test data set for testing the ROM
### the data in training.csv and test.csv can basically be obtained by running the example in the "Forward_sampling" folder.


the folder ROM: folder to save results which includes: the pickled ROM file, the predicted outputs (pointvalue1, ..., pointvalue44) using the ROM for the training data set (rom_on_training.csv) 
                and for the test data set (rom_on_test.csv).

