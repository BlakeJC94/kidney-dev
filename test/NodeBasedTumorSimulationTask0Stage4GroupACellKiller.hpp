/*

Copyright (c) 2005-2015, University of Oxford.
All rights reserved.

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.


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


#include <cxxtest/TestSuite.h>
#include "CheckpointArchiveTypes.hpp"

/* The following header is usually included in all cell-based test suites. It enables us to write tests where the {{{SimulationTime}}} is handled automatically and simplifies the tests.*/
#include "AbstractCellBasedTestSuite.hpp"

#include "PetscSetupAndFinalize.hpp"

/* The remaining header files define classes that will be used in the cell population
 * simulation test. We encountered some of these header files in
 * UserTutorials/RunningMeshBasedSimulations. */
#include "CellsGenerator.hpp"
#include "TransitCellProliferativeType.hpp"
#include "StochasticDurationCellCycleModel.hpp"
#include "HoneycombMeshGenerator.hpp"
#include "GeneralisedLinearSpringForce.hpp"
#include "OffLatticeSimulation.hpp"
#include "SmartPointers.hpp"
/* The next header file defines the class for storing the spatial information of cells. */
#include "NodesOnlyMesh.hpp"
/* The next header file defines a node-based {{{CellPopulation}}} class.*/
#include "NodeBasedCellPopulation.hpp"

//#include "GroupACellProperty.hpp"
#include "GroupACellKiller.hpp"



class NodeBasedTumorSimulation : public AbstractCellBasedTestSuite
{
public:

    void TestBlobSCCNoKiller() throw(Exception)
    {
        HoneycombMeshGenerator generator(4, 4);
        MutableMesh<2,2>* p_generating_mesh = generator.GetCircularMesh(4);
        NodesOnlyMesh<2> mesh;
        mesh.ConstructNodesWithoutMesh(*p_generating_mesh, 1.5);
        
        MAKE_PTR(CellLabel, p_label);
        MAKE_PTR(WildTypeCellMutationState, p_state);
       // MAKE_PTR(GroupACellProperty, p_group_a);
        
        std::vector<CellPtr> cells;
        MAKE_PTR(TransitCellProliferativeType, p_transit_type);
        //CellsGenerator<StochasticDurationCellCycleModel, 2> cells_generator;
        //cells_generator.GenerateBasicRandom(cells, mesh.GetNumNodes(), p_transit_type);
        for (unsigned i=0; i<mesh.GetNumNodes(); i++)
        {
            StochasticDurationCellCycleModel* p_model = new StochasticDurationCellCycleModel();
            CellPropertyCollection collection;
            if (RandomNumberGenerator::Instance()->ranf() < 0.5)
            {
                collection.AddProperty(p_label);
         //       collection.AddProperty(p_group_a);
            }
            CellPtr p_cell(new Cell(p_state, p_model, NULL, false, collection));
            p_cell->SetCellProliferativeType(p_transit_type);
            
            double birth_time = - RandomNumberGenerator::Instance()->ranf() *
                                    (p_model->GetStemCellG1Duration() 
                                        + p_model->GetSG2MDuration());
            
            p_cell->SetBirthTime(birth_time);
            cells.push_back(p_cell);
        }
        
        NodeBasedCellPopulation<2> cell_population(mesh, cells);
        
        OffLatticeSimulation<2> simulator(cell_population);
        simulator.SetOutputDirectory("PropTest");
        simulator.SetSamplingTimestepMultiple(6);
        simulator.SetEndTime(1.0);
        
        MAKE_PTR(GeneralisedLinearSpringForce<2>, p_force);
        simulator.AddForce(p_force);
        
        MAKE_PTR_ARGS(GroupACellKiller, p_killer, (&cell_population));
        simulator.AddCellKiller(p_killer);

        simulator.Solve();
        
    }
};

// Problem isolated to GroupACellKiller.hpp
