close all;
clear all;

total_jobs = 3;

addpath(genpath('/home/blake/Workspace/Chaste/anim/'));

% To pull data from many simulations, loop over each simulation and use a 
% running average process to calculate the mean data
%   a_(n+1) = (n*a_(n) + x_(n+1))/(n+1), a_1 = x_1;

%% no. cells vs time

addpath(genpath('testoutput/'));

% populationdata = cell(1,total_jobs);
% populationdata{1} = importdata('testoutput/UtericBudSimulation_0/results_from_time_0/celltypes.dat');
% meanpopulationdata = populationdata{1};

populationdata = importdata('testoutput/UtericBudSimulation_0/results_from_time_0/celltypes.dat');
meanpopulationdata = populationdata;

for k = 2:total_jobs
    populationdata = importdata(['testoutput/UtericBudSimulation_' num2str(k-1) '/results_from_time_0/celltypes.dat']);
    meanpopulationdata = ((k-1)*meanpopulationdata + populationdata)./k;
end


figure('units', 'normalized', 'position', [.3 .3 0.12 0.4]);

simtime = 120*(populationdata(:,1)); 
diffcells = populationdata(:,4);
transitcells = populationdata(:,3);

totalcells = transitcells + diffcells;
plot(simtime, totalcells, 'k', simtime, diffcells, 'r',...
    simtime, transitcells, 'b');
legend('Total', 'Differentiated', 'Transit');
title('Number of cells in simulation');
xlabel('simulation time'); ylabel('no. of cells');


