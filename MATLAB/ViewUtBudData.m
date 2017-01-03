function [AvPopulationDataOut, AvHVelocityDataOut, AvVVelocityDataOut,...
    AvHAgeDataOut, AvVAgeDataOut] = ViewUtBudData(TotalJobs, ...
    BinSize_ux, BinSize_vx, BinSize_agex, BinSize_agey, max_x, max_y)
%ViewUtBudData: Produces data plots for batch data from UtericBudSimulation
%   Inputs: 
%       TotalJobs = total number of sims, match 'num_sums' in .sh file.
%       BinSize_ux = size of partition elements of x in 'Mean u vs x'
%       BinSize_vx = size of partition elements of x in 'Mean v vs x'
%       BinSize_agex = size of partition elements of x in 'Mean age vs x'
%       BinSize_agey = size of partition elements of x in 'Mean age vs y'
%       max_x = Right endpoint of largest partition element will be the
%           first integer multiple after this
%       max_y = Right endpoint of largest partition element will be the
%           first integer multiple after this
% 
%   Outputs:
%       AvPopulationDataOut = cell containing plot data for 'Mean no. 
%           cells vs time', values as specified in plot code.
%       AvHVelocityDataOut = cell containing plot data for 'Mean u vs x'.
%       AvVVelocityDataOut = cell containing plot data for 'Mean v vs x'.
%       AvHAgeDataOut = cell containing plot data for 'Mean age vs x'.
%       AvVAgeDataOut = cell containing plot data for 'Mean age vs y'.
%
% Default Args : ViewUtBudData(5, 2, 2, 1, 1, 40, 10)
% 
%% Add paths and set up

close all;
fontopt = {'FontSize',50,'FontWeight','bold'};

addpath(genpath('/home/blake/Workspace/Chaste/anim/'));
addpath(genpath('testoutput/'));

%% Mean no. cells vs time
% Plots time series of the population count by ProliferativeType

figure('units', 'normalized', 'position', [.3 .3 0.12 0.4]);

PopulationData = importdata('testoutput/UtericBudSimulation_0/results_from_time_0/celltypes.dat');
MeanPopulationData = PopulationData;
for k = 2:TotalJobs
    PopulationData = importdata(['testoutput/UtericBudSimulation_' num2str(k-1) '/results_from_time_0/celltypes.dat']);
    MeanPopulationData = ((k-1)*MeanPopulationData + PopulationData)./k;
end

AvPopulationDataOut{1} = 120*(MeanPopulationData(:,1));
AvPopulationDataOut{2} = MeanPopulationData(:,4);
AvPopulationDataOut{3} = MeanPopulationData(:,3);
AvPopulationDataOut{4} = MeanPopulationData(:,4) + MeanPopulationData(:,3);

SimTime = AvPopulationDataOut{1};
DiffCells = AvPopulationDataOut{2};
TransitCells = AvPopulationDataOut{3};
TotalCells = AvPopulationDataOut{4};

plot(SimTime, TotalCells, 'k', SimTime, DiffCells, 'r',...
    SimTime, TransitCells, 'b');
legend('Total', 'Differentiated', 'Transit', 'Location', 'Best');
title(['Average number of cells over ', num2str(TotalJobs), ' simulations'], fontopt{:});
xlabel('simulation time'); ylabel('no. of cells');


%% Mean u vs x
% Plots bar graph of horizonal velocities averaged over a partition of x on
% the last frame of the simulation.
%   x = horizontal position
%   u = velocity in x direction
% Size of partition elements specified by 'BinSize_ux' input.
% Maximum x specified by 'max_x' input.

figure('units', 'normalized', 'position', [.3 .3 0.12 0.4]);

VelocityData = LoadNonConstantLengthData('testoutput/UtericBudSimulation_0/results_from_time_0/cellvelocities.dat');
x = VelocityData{end}(3:5:end-3);
u = VelocityData{end}(5:5:end-1);
av_udata = barplotdatagen(x, u, BinSize_ux, max_x);

MeanHVelocityData = av_udata;

for k = 2:TotalJobs
    VelocityData = LoadNonConstantLengthData(['testoutput/UtericBudSimulation_', num2str(k-1) '/results_from_time_0/cellvelocities.dat']);
    x = VelocityData{end}(3:5:end-3);
    u = VelocityData{end}(5:5:end-1);
    av_udata = barplotdatagen(x, u, BinSize_ux, max_x);
    
    MeanHVelocityData = ((k-1)*MeanHVelocityData + av_udata)./k;
end

AvHVelocityDataOut{1} = BinSize_ux*(0:numel(MeanHVelocityData));
AvHVelocityDataOut{2} = MeanHVelocityData;

bar(BinSize_ux*(1:numel(MeanHVelocityData)) - (BinSize_ux/2), MeanHVelocityData);
title(['Average horizontal velocity vs. horizontal position over ', num2str(TotalJobs), ' simulations'], fontopt{:});
xlabel('x'); ylabel('u');


%% Mean v vs x
% Plots bar graph of vertical velocities averaged over a partition of x on
% the last frame of the simulation.
%   x = horizontal position
%   u = velocity in x direction
% Size of partition elements specified by 'BinSize_vx' input.
% Maximum x specified by 'max_x' input.


figure('units', 'normalized', 'position', [.3 .3 0.12 0.4]);

VelocityData = LoadNonConstantLengthData('testoutput/UtericBudSimulation_0/results_from_time_0/cellvelocities.dat');
x = VelocityData{end}(3:5:end-3);
v = VelocityData{end}(6:5:end);
av_vdata = barplotdatagen(x, v, BinSize_vx, max_x);

MeanVVelocityData = av_vdata;

for k = 2:TotalJobs
    VelocityData = LoadNonConstantLengthData(['testoutput/UtericBudSimulation_', num2str(k-1) '/results_from_time_0/cellvelocities.dat']);
    x = VelocityData{end}(3:5:end-3);
    v = VelocityData{end}(6:5:end);
    av_vdata = barplotdatagen(x, v, BinSize_vx, max_x);
    
    MeanVVelocityData = ((k-1)*MeanVVelocityData + av_vdata)./k;
end

AvVVelocityDataOut{1} = BinSize_vx*(0:numel(MeanVVelocityData));
AvVVelocityDataOut{2} = MeanVVelocityData;

bar(BinSize_vx*(1:numel(MeanVVelocityData)) - (BinSize_vx/2), MeanVVelocityData);
title(['Average vertical velocity vs. horizontal position over ', num2str(TotalJobs), ' simulations'], fontopt{:});
xlabel('x'); ylabel('v');



%% Mean age vs x
% Plots bar graph of cell ages averaged over a partition of x on
% the last frame of the simulation.
%   x = horizontal position
%   age = age of cell
% Size of partition elements specified by 'BinSize_agex' input.
% Maximum x specified by 'max_x' input.
figure('units', 'normalized', 'position', [.3 .3 0.12 0.4]);

agedata = LoadNonConstantLengthData('testoutput/UtericBudSimulation_0/results_from_time_0/cellages.dat');
x = agedata{end}(3:4:end-3);
age = agedata{end}(5:4:end-1);
av_agedata = barplotdatagen(x, age, BinSize_agex, max_x);

MeanAgeXData = av_agedata;

for k = 2:TotalJobs
    agedata = LoadNonConstantLengthData(['testoutput/UtericBudSimulation_', num2str(k-1) '/results_from_time_0/cellages.dat']);
    x = agedata{end}(3:4:end-3);
    age = agedata{end}(5:4:end-1);
    av_agedata = barplotdatagen(x, age, BinSize_agex, max_x);
    
    MeanAgeXData = ((k-1)*MeanAgeXData + av_agedata)./k;
end

AvHAgeDataOut{1} = BinSize_agex*(0:numel(MeanAgeXData));
AvHAgeDataOut{2} = MeanAgeXData;

bar(BinSize_agex*(1:numel(MeanAgeXData)) - (BinSize_agex/2), MeanAgeXData);
title('cell age vs. horizontal position at end of simulation', fontopt{:});
xlabel('x'); ylabel('age');


%% Mean age vs y
% Plots bar graph of cell ages averaged over a partition of y on
% the last frame of the simulation.
%   y = vertical position
%   age = age of cell
% Size of partition elements specified by 'BinSize_agey' input.
% Maximum x specified by 'max_y' input.

figure('units', 'normalized', 'position', [.3 .3 0.12 0.4]);

agedata = LoadNonConstantLengthData('testoutput/UtericBudSimulation_0/results_from_time_0/cellages.dat');
y = agedata{end}(4:4:end-2);
age = agedata{end}(5:4:end-1);
av_agedata = barplotdatagen(y, age, BinSize_agey, max_y);

MeanAgeYData = av_agedata;

for k = 2:TotalJobs
    agedata = LoadNonConstantLengthData(['testoutput/UtericBudSimulation_', num2str(k-1) '/results_from_time_0/cellages.dat']);
    y = agedata{end}(4:4:end-2);
    age = agedata{end}(5:4:end-1);
    av_agedata = barplotdatagen(y, age, BinSize_agey, max_y);
    
    MeanAgeYData = ((k-1)*MeanAgeYData + av_agedata)./k;
end

AvVAgeDataOut{1} = BinSize_agey*(0:numel(MeanAgeYData));
AvVAgeDataOut{2} = MeanAgeYData;

bar(BinSize_agey*(1:numel(MeanAgeYData)) - (BinSize_agey/2), MeanAgeYData);
title('cell age vs. vertical position at end of simulation', fontopt{:});
xlabel('y'); ylabel('age');


end



function av_data = barplotdatagen(x, u, bin_size, max)
% Partitions x into elements [0, bin_size), [bin_size, 2*bin_size), ...
% until after k*bin_size > max. Then isolate values of u that correspond to
% each partition element and average over each of them. Return average u
% value over each partition element as a vector.

partitions_total = ceil(max/bin_size);
av_data = zeros(1, partitions_total);
for k=1:partitions_total
    indicies = (bin_size*(k-1) <= x) .* (x < bin_size*k);
    tmpdata = u .* indicies;
    av_data(k) = sum(tmpdata)/sum(indicies);
    
    % Removing this check makes the data inflated, since this stops the
    % addition of NaNs?
%     if sum(indicies) == 0
%         av_data(k) = 0;
%     end
end

end