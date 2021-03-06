function AttRep_RadForce1_Data

close all;

addpath(genpath('functions/'));

testoutput_path = ['data/AttRep/RadForce1/RF1_testoutput_dats'];
data_path = [testoutput_path '/UtericBudSimulation_RadForce1_'];
D = dir([data_path '*']);
TotalJobs = length(D(:));


[MeanPopulationData, TotalCellsStd] = meanscellsvstime(data_path);
[totalcount, proportion] = totalcellsteadystatecount(data_path);
onsettime = totalcellsteadystate(data_path);
slope_total = steadystateshape(data_path, 0);
slope_prolif = steadystateshape(data_path, 1);


SimTime = MeanPopulationData(:,1);
TransitCells = MeanPopulationData(:,2);
TotalCells = MeanPopulationData(:,3) + MeanPopulationData(:,2);

AttachedCells = MeanPopulationData(:,5);
RVCells = MeanPopulationData(:,6);

capheight_RF1 = capheight(data_path);

save('MAT/AttRep_RadForce1.mat')

disp('Done!');


end