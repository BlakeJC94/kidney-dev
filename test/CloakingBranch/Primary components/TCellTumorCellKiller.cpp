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



#include "TCellTumorCellKiller.hpp"

#include "TCellMutationState.hpp"
#include "TumorCellMutationState.hpp"
#include "DifferentiatedCellProliferativeType.hpp"
#include "SmartPointers.hpp"

#include "Debug.hpp"

TCellTumorCellKiller::TCellTumorCellKiller(AbstractCellPopulation<2>* pCellPopulation)
    : AbstractCellKiller<2>(pCellPopulation),
    mDomainRadius(5.0),
    kill_radius_a(5.15),
    kill_radius_b(6.0)
{
}

double TCellTumorCellKiller::GetDomainRadius()
{
    return mDomainRadius;
}

void TCellTumorCellKiller::SetDomainRadius(double newValue)
{
    mDomainRadius = newValue;
}

void TCellTumorCellKiller::SetKillRadiusA(double newValue)
{
    kill_radius_a = newValue;
}

void TCellTumorCellKiller::SetKillRadiusB(double newValue)
{
    kill_radius_b = newValue;
}


void TCellTumorCellKiller::CheckAndLabelCellsForApoptosisOrDeath()
{
    double node_0_coord = (mDomainRadius + 1.5)*cos(M_PI/4);
    
    // Update CellPopulation and make pointers
    mpCellPopulation->Update(); 
    MAKE_PTR(DifferentiatedCellProliferativeType, p_diff_type);
    MAKE_PTR_ARGS(CellLabel, p_t_cell_label, (2));
    MAKE_PTR_ARGS(CellLabel, p_tumor_cell_label, (4));
    
    
    // Iterate over all cells (cell selected)
    for (AbstractCellPopulation<2>::Iterator cell_iter = this->mpCellPopulation->Begin();
        cell_iter != this->mpCellPopulation->End();
        ++cell_iter)
    {
        // Get node index and node of selected cell
        unsigned node_index = this->mpCellPopulation->GetLocationIndexUsingCell(*cell_iter);
        Node<2>* p_node = this->mpCellPopulation->GetNode(node_index);
        
        // Calculate position coordinates of selected cell using node data
        double x_coordinate = p_node->rGetLocation()[0];
        double y_coordinate = p_node->rGetLocation()[1];
        double r_coordinate = sqrt(pow(x_coordinate, 2) + pow(y_coordinate, 2));
        
        
        /* --==-- 01: T Cell Portal segment --==-- */
        /* Check for newly spawned cells outside "kill-zone" boundary. Daughter cells of the Stem Cell with 
         * TCellMutation will be Transit Cells with TCellMutation. If there is a Transit TCell in the range (r > kill_radius_b), 
         * teleoprt that cell to a random point on the circle with radius of domain and 
         * change the proliferative type into a differentiated T Cell.
         * Spawning new T Cells moves the Stem Cell slightly, so reset position often. */
         
        // Generate random angular coordinate to teleport a cell to
        RandomNumberGenerator* p_gen_theta = RandomNumberGenerator::Instance();
        
        // Filter: Transit T Cell that is in the range r > kill_radius_b (Default: 6.0)
        if (  (r_coordinate > kill_radius_b) && ((cell_iter->GetMutationState()->IsType<TCellMutationState>()) && (cell_iter->GetCellProliferativeType()->IsType<TransitCellProliferativeType>()))  )
        {
            // Teleporter
            double angular_coord = p_gen_theta->ranf() * 2 * M_PI;
            p_node->rGetModifiableLocation()[0] = mDomainRadius * cos(angular_coord); // Default value = 4.9 ...
            p_node->rGetModifiableLocation()[1] = mDomainRadius * sin(angular_coord);
            
            // Adds DifferentiatedCellProliferativeType to new T Cells from node 0
            cell_iter->SetCellProliferativeType(p_diff_type);
        }
        // Filter: All Stem Cells in simulation (i.e. only the Stem T Cell in simulation) 
        else if (cell_iter->GetCellProliferativeType()->IsType<StemCellProliferativeType>())
        {
            // Keep resetting position 
            p_node->rGetModifiableLocation()[0] = node_0_coord; 
            p_node->rGetModifiableLocation()[1] = node_0_coord;
        }
        
        
        /* --==-- 02: T Cell Killer segment --==-- */
        /* Check and kill any T Cells in an annular region (kill_radius_a < r < kill_radius_b).
         * Inner radius chosen to be slightly above 5 to slightly increase chance of survival for new T Cells.
         * Outer radius chosen to exclude checking brand new T Cells that had not yet teleported. */
        
        // Filter: T Cells in the range kill_radius_a < r < kill_radius_b (Default: 5.15, 6.0)
        if (  !(cell_iter->GetCellProliferativeType()->IsType<StemCellProliferativeType>()) && (r_coordinate > kill_radius_a) && (r_coordinate < kill_radius_b )   )
        {
            cell_iter->Kill();
        }
        
        
        /* --==-- 03: T Cell Labeller and Tumor Killer segment --==-- */
        /* For each tumor cell, check each neighbour cell. If there is at least one neighbour that is an unlabelled T Cell, 
         * then label that neighbouring T Cell. Otherwise if at least one neighbour is a labelled T Cell, then kill both 
         * the labelled T Cell and the selected Tumor cell. */
        
        // Generate random Bernoulli trial parameter for killing Tumor Cells 
        RandomNumberGenerator* p_gen_kill = RandomNumberGenerator::Instance();
        
        // Filter: Tumor Cells
        if (cell_iter->GetMutationState()->IsType<TumorCellMutationState>())
        {
            // Get set of all node indices of neighbouring cells
            std::set<unsigned> neighbour_indices = this->mpCellPopulation->GetNeighbouringLocationIndices(*cell_iter);
            
            // Iterate over all neighbour cell nodes
            for (std::set<unsigned>::iterator iter = neighbour_indices.begin();
                iter != neighbour_indices.end();
                ++iter)
            {
                // Get cell associated with neighbour node index
                unsigned neighbour_index = *(iter);
                CellPtr p_neighbour_cell = this->mpCellPopulation->GetCellUsingLocationIndex(neighbour_index);

                // Filter: Unlabelled T Cell neighbour
                if (  (p_neighbour_cell->GetMutationState()->IsType<TCellMutationState>()) && !(p_neighbour_cell->HasCellProperty<CellLabel>())  )
                {
                    // Label T Cell neighbour                    
                    p_neighbour_cell->AddCellProperty(p_t_cell_label);
                    break;
                }
                // Filter: Labelled T Cell neighbour
                else if (  (p_neighbour_cell->GetMutationState()->IsType<TCellMutationState>()) && (p_neighbour_cell->HasCellProperty<CellLabel>())  )
                {
                    double kill_chance = p_gen_kill->ranf();
                    // Bernoulli trial for Tumor Killer
                    if (kill_chance < 0.002)
                    {
                        // Kills both Tumor Cell and Labelled T Cell neighbour
                        p_neighbour_cell->Kill();
                        cell_iter->Kill();
                        break;
                    }
                }
            }   
        }
        
        
        /* --==-- 04: T Cell Unlabeller segment --==-- */
        /* Check each Labelled T Cell and unlabel if there is no Tumor Cell nearby */
        if (  (cell_iter->GetMutationState()->IsType<TCellMutationState>()) && (cell_iter->HasCellProperty<CellLabel>())  )
        {
            bool condition_satisfied = false;
            
            // Get set of all node indices of neighbouring cells
            std::set<unsigned> neighbour_indices = this->mpCellPopulation->GetNeighbouringLocationIndices(*cell_iter);
            
            // Iterate over all neighbour cell nodes
            for (std::set<unsigned>::iterator iter = neighbour_indices.begin();
                iter != neighbour_indices.end();
                ++iter)
            {
                // Get cell associated with neighbour node index
                unsigned neighbour_index = *(iter);
                CellPtr p_neighbour_cell = this->mpCellPopulation->GetCellUsingLocationIndex(neighbour_index);
                
                // Filter: Tumor Cell neighbour
                if (p_neighbour_cell->GetMutationState()->IsType<TumorCellMutationState>())
                {
                    condition_satisfied = true;
                    break;
                }
            }
            
            if (condition_satisfied == false)
            {
                cell_iter->RemoveCellProperty<CellLabel>();
            }
        }
    }
}

void TCellTumorCellKiller::OutputCellKillerParameters(out_stream& rParamsFile)
{
    AbstractCellKiller<2>::OutputCellKillerParameters(rParamsFile);
}

#include "SerializationExportWrapperForCpp.hpp"
CHASTE_CLASS_EXPORT(TCellTumorCellKiller)

