/*

Copyright (c) 2005-2015, University of Oxford.
All rights reserved.

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.
1. Apply a proper Parabollic PDE which only uses Labelled T Cells as sources.

This file is part of Chaste.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the University of Oxford nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


/* Continuing from where we left off with the Vacation Scholars project, we met with Fede and 
 * discussed the next steps foward to making this project worthy of publication. We don't want to 
 * replicate what fede has done with his original model, but we want to add to it.
 * 
 * We want to write a detailed study of different methods of comutation and modelling. This will 
 * involve branching off the model currently and scaling up to the order of 500 cells instead of
 * 20 or so. The first branch will be incorperating a 'cloaking' behaviour of the tumor cells by 
 * modifying the existing PDE to make the attacked tumor cells act as 'sinks'. The second branch 
 * will be adding a recrietment process for the attacking T Cells by modifying the CellCycleModel.  */

/* Task 01. Modify the PDE to get the attacked tumor cells to 'mop up' cytokines (cloaking model) */


#include <cxxtest/TestSuite.h>
#include "CheckpointArchiveTypes.hpp"

#include "AbstractCellBasedTestSuite.hpp"
#include "PetscSetupAndFinalize.hpp"
#include "CellsGenerator.hpp"
#include "DifferentiatedCellProliferativeType.hpp"
#include "OffLatticeSimulation.hpp"
#include "SmartPointers.hpp"
#include "NodesOnlyMesh.hpp"
#include "NodeBasedCellPopulation.hpp"

/* Load Primary components */
#include "CM_CellDiffusionForce.hpp"
//#include "TCellTumorCellKiller.hpp"
#include "UtericBudSpringForce.hpp"
#include "GeneralisedLinearSpringForce.hpp"
#include "UtericBudCellCycleModel.hpp"
//#include "TCellChemotacticForce.hpp"
//#include "AveragedSourceParabolicPde.hpp"

/* Load Secondary components */
#include "UT_CellMutationState.hpp"
#include "CM_CellMutationState.hpp"
//#include "TCellTumorMutationStatesCountWriter.hpp"

#include "PlaneBoundaryCondition.hpp"



#include "CommandLineArguments.hpp"


class UtericBudSimulation : public AbstractCellBasedTestSuite
{
public:

    void TestUtericBudSimulation() throw(Exception)
    {
        /* Initial cell count options */
        unsigned num_ut_cells = 4; // Default = 4
        unsigned num_cm_cells = 8; // Default = 8
        
        /* Simulation options */
        double domain_height = 12.5; // Default = 5 
        double domain_width = 20; // Default = 5 
        unsigned simulation_time = 45; // Default = 45
        unsigned sampling_timestep_multiple = 6; //Default = 6
        //double t_cell_spawn_rate = 10; // Default = 25 (units: cells/hour)
	    double diffusion_intensity = 0.25; // Default = 0.1, 
        //double chemotactic_strength = 0.8; // Default = 1.0
        

	    //double sim_index = (double) atof(CommandLineArguments::Instance()->GetStringCorrespondingToOption("-sim_index").c_str());
	    double sim_index = 0;
	    RandomNumberGenerator::Instance()->Reseed(100.0*sim_index);        

	    std::stringstream out;
	    out << sim_index;
	    std::string output_directory = "ThyroidTumorSimulation" + out.str();
        
        
        /* Generate nodes */
        /* 01. Initial UT nodes: 
         *         index range = [0, num_ut_cells] 
         *         position = (2.5 + 5k, 0), k = 0, ... 3
         * 02. Initial Tumor Cell nodes: 
         *         index range = [num_ut_cells + 1, num_ut_cells + num_cm_cells] 
         *         radial range = (0, domain_radius/5)) */
        std::vector<Node<2>*> nodes;
        RandomNumberGenerator* p_gen_x = RandomNumberGenerator::Instance();
        RandomNumberGenerator* p_gen_y = RandomNumberGenerator::Instance();
        
        for (unsigned index = 0; index < num_ut_cells; index++) // 01.
        {   
            nodes.push_back(new Node<2>(index, false, 2.5 + index*5.0 , 0));
        }
        
        for (unsigned index = 0; index < num_cm_cells; index++) // 02.
        {
            double x_coord = 12.5 * p_gen_x->ranf();
            double y_coord = 5 * p_gen_y->ranf();
            
            nodes.push_back(new Node<2>(index, false, x_coord , y_coord));
        }
        
        
        /* Generate mesh */
        NodesOnlyMesh<2> mesh;
        mesh.ConstructNodesWithoutMesh(nodes, 2.5);
        
        
        /* Generate cells */
        std::vector<CellPtr> cells;
        
        MAKE_PTR(StemCellProliferativeType, p_stem_type);
        MAKE_PTR(TransitCellProliferativeType, p_transit_type);
        MAKE_PTR(DifferentiatedCellProliferativeType, p_diff_type);

        MAKE_PTR(UT_CellMutationState, p_ut_cell_state); 
        MAKE_PTR(CM_CellMutationState, p_cm_cell_state);
        
        for (unsigned i=0; i<mesh.GetNumNodes(); i++) 
        {
            /* Add cell cycle model for all cells
             * 01. CM Cells: 
             *       
             * 02. UT Cells:
             *               */
            UtericBudCellCycleModel* p_model = new UtericBudCellCycleModel();
            p_model->SetSpawnRate(25);
            
            double birth_time = - RandomNumberGenerator::Instance()->ranf() *
                (p_model->GetStemCellG1Duration()
                    + p_model->GetSG2MDuration());
            
            CellPtr p_cell(new Cell(p_ut_cell_state, p_model, NULL, false));

            p_cell->SetBirthTime(birth_time);
            

            if (i < num_ut_cells) // 01.
            {
               p_cell->SetCellProliferativeType(p_diff_type);   
            }
            else // 02.
            {
                //p_model->SetMaxTransitGenerations(100000000);
                p_cell->SetMutationState(p_cm_cell_state);
                p_cell->SetCellProliferativeType(p_transit_type);
            }
            cells.push_back(p_cell);
        }
        
        
        /* Generate Cell Population */
        NodeBasedCellPopulation<2> cell_population(mesh, cells);

        //cell_population.AddCellPopulationCountWriter<TCellTumorMutationStatesCountWriter>();
        
        
        /* Begin OffLatticeSimulation */
        OffLatticeSimulation<2> simulator(cell_population);
        simulator.SetOutputDirectory("UtericBudSimulation");
	    //simulator.SetOutputDirectory(output_directory);
        simulator.SetSamplingTimestepMultiple(sampling_timestep_multiple);
        simulator.SetEndTime(simulation_time);

        
        /* Add cell killer component */
        //MAKE_PTR_ARGS(TCellTumorCellKiller, p_cell_killer, (&cell_population));
        //p_cell_killer->SetDomainRadius(domain_radius);
        //p_cell_killer->SetKillRadiusA(kill_radius_a);
        //p_cell_killer->SetKillRadiusB(kill_radius_b);
        //simulator.AddCellKiller(p_cell_killer);
        
        
        /* Add generalised linear spring forces between cells
         *   01: T Cell + T Cell --> Repulsion
         *   02: Labelled T-Cells + Tumor Cells --> Attraction & Repulsion
         *   03: Tumor Cell + Tumor Cell --> Attraction and Repulsion */
        MAKE_PTR(UtericBudSpringForce<2>, p_spring_force);
        simulator.AddForce(p_spring_force);
        
        /* Add diffusion force component to Unlabelled Differentiated T Cells */
        MAKE_PTR(CM_CellDiffusionForce<2>, p_diffusion_force);
        p_diffusion_force->SetDiffusionIntensity(diffusion_intensity);
        simulator.AddForce(p_diffusion_force);

        /* Call boundary condition */
        c_vector<double, 2> point = zero_vector<double>(2);
        c_vector<double, 2> normal = zero_vector<double>(2);
        normal(1) = -1.0;
        MAKE_PTR_ARGS(PlaneBoundaryCondition<2>, p_bc, (&cell_population, point, normal));
        simulator.AddCellPopulationBoundaryCondition(p_bc);
        
        /* Call solve */
        simulator.Solve();
        
    }
};

// Pass
