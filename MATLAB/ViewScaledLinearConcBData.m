function ViewScaledLinearConcBData
%VIEWSCALEDLINEARCONCBDATA Summary of this function goes here
%   Detailed explanation goes here


close all;

addpath(genpath('/home/blake/Workspace/Chaste/anim/'));
addpath(genpath('testoutput/'));

%p = [0.4 0.5 0.6:0.05:0.9 1]; %1
p = [0.3:0.1:1]; %2
%p = [0.05:0.1:0.5]; %3

data = zeros(length(p), 20, 5); % col = p value, row = run, depth = time
avgdata = zeros(length(p), 5); % row = p value, col = time
sdvdata = zeros(length(p), 5); % row = p value, col = time
steadystateav = zeros(length(p));
TotalJobs = 20;

for k = 1:length(p) % over each value of p
    
    PopulationDataSet = cell(1,TotalJobs);
    
    for j = 1:TotalJobs % over each job
        
        % Load data from run j-1 in folder '0k'
%         PopulationData = importdata(['ScaledLinearConcBData/0' num2str(k-1) '/testoutput/UtericBudSimulation_' num2str(j-1) '/results_from_time_0/celltypescount.dat']);

%         PopulationData = importdata(['DataLinks/Linux-BlakesPC/' ...
%             'ScaledLinearConcBData/0' num2str(k-1) ....
%             '/testoutput/UtericBudSimulation_' num2str(j-1) ...
%             '/results_from_time_0/celltypescount.dat']);
        
%         PopulationData = importdata(['testoutput_CHASTE/UtericBudSimulation_ConBParameterSweep_' num2str(p(k)) '_' num2str(j-1) '/results_from_time_0/celltypescount.dat']);

%        PopulationData = importdata(['testoutput_HD1/UtericBudSimulation_ConBParameterSweep_' num2str(p(k)) '_' num2str(j-1) '/results_from_time_0/celltypescount.dat']);
          PopulationData = importdata(['testoutput_HD2/UtericBudSimulation_ParameterSweep_' num2str(p(k)) '_' num2str(j-1) '/results_from_time_0/celltypescount.dat']);
%         PopulationData = importdata(['testoutput_HD3/UtericBudSimulation_ParameterSweep_step_' num2str(p(k)) '_' num2str(j-1) '/results_from_time_0/celltypescount.dat']);

        
        PopulationDataSet{j} = PopulationData.data;
        
        for i = 1:5 % over each sample step
            
            % Find which row of data to sample from
            sample = find(PopulationData.data(:,1)  <= 100*i, 1, 'last');
            
            % Pull total cell count at this step
            data(k,j,i) = PopulationData.data(sample,3) + PopulationData.data(sample,2);
            
        end
        
    end
    
    steadystateav(k) = totalcellsteadystate2(PopulationDataSet);
   
    for i = 1:5 % over each sample step
        
        % Find the average total population number
        avgdata(k, i) = mean(data(k, :, i));
        
        % Find the stadrand deviation of the total population number
        sdvdata(k, i) = std(data(k, :, i));
        
    end
    
        
end

% % figure;
% % plot(p, avdata, 'b', p, data, 'o');

for i = 1:5 % over each sample step
    
    % make a new figure and plot results
    figure;
    plot(p,avgdata(:,i), 'b', ...
        p, avgdata(:,i) + sdvdata(:,i), 'b--', ...
        p, avgdata(:,i) - sdvdata(:,i), 'b--', ...
        p, data(:,:,i), 'o');
    title(['Total cell count for different parameter vals at t = ' num2str(100*i)]);
    xlabel('\alpha'); ylabel('total cell counts');
    axis([min(p), max(p), 0, 800]);
    
end

figure;
plot(p, steadystateav, 'ko--')
title(['Total cell count steady state onset time']);
xlabel('\alpha'); ylabel('Steady State');
axis([min(p), max(p), 300, 500]);

end

