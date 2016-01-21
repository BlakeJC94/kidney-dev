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


TCellTumorCellKiller::TCellTumorCellKiller(AbstractCellPopulation<2>* pCellPopulation)
    : AbstractCellKiller<2>(pCellPopulation)
{
}

void TCellTumorCellKiller::CheckAndLabelCellsForApoptosisOrDeath()
{
    mpCellPopulation->Update();
    MAKE_PTR(DifferentiatedCellProliferativeType, p_diff_type);
    
    for (AbstractCellPopulation<2>::Iterator cell_iter = this->mpCellPopulation->Begin();
        cell_iter != this->mpCellPopulation->End();
        ++cell_iter)
    {
        unsigned node_index = this->mpCellPopulation->GetLocationIndexUsingCell(*cell_iter);
        Node<2>* p_node = this->mpCellPopulation->GetNode(node_index);
        double x_coordinate = p_node->rGetLocation()[0];
        double y_coordinate = p_node->rGetLocation()[1];
        double r_coordinate = sqrt(pow(x_coordinate, 2) + pow(y_coordinate, 2));
        
        /* Code that was previously in BoundaryCondition component */
        RandomNumberGenerator* p_gen_theta = RandomNumberGenerator::Instance();
        double angular_coord = p_gen_theta->ranf() * 6.283185307;
        
        if (  (r_coordinate > 6.0) && ((cell_iter->GetMutationState()->IsType<TCellMutationState>()) && (cell_iter->GetCellProliferativeType()->IsType<TransitCellProliferativeType>()))  )
        {
            p_node->rGetModifiableLocation()[0] = 4.9 * cos(angular_coord); // Default value = 4.9
            p_node->rGetModifiableLocation()[1] = 4.9 * sin(angular_coord);
            
            // Adds DifferentiatedCellProliferativeType to new T Cells from node 0
            cell_iter->SetCellProliferativeType(p_diff_type);
        }
        
        
        
        /* Check and kill any T Cells in an annular region (5.05 < r < 6.00).
         * Inner radius chosen to be slightly above 5 to slightly increase chance of survival for new T Cells.
         * Outer radius chosen to exclude checking brand new T Cells that had not yet teleported. */
        if (  (cell_iter->GetMutationState()->IsType<TCellMutationState>()) && (r_coordinate > 5.05) && (r_coordinate < 6.0 )   )
        {
            cell_iter->Kill();
        }
        
        /* Check all Tumor Cells. For each case, check if any neighbouring node index is associated with a cell
         * that posesses a TCellMutationState. 
         * If one neighbour node is paired with a T Cell, kill the selected Tumor Cell. */
        if (cell_iter->GetMutationState()->IsType<TumorCellMutationState>())
        {
            // Get set of all node indices of neighbouring cells
            std::set<unsigned> neighbour_indices = this->mpCellPopulation->GetNeighbouringLocationIndices(*cell_iter);
            
            for (std::set<unsigned>::iterator iter = neighbour_indices.begin();
                iter != neighbour_indices.end();
                ++iter)
            {
                unsigned neighbour_index = *(iter);

                // Get cell associated with this node index
                CellPtr p_neighbour_cell = this->mpCellPopulation->GetCellUsingLocationIndex(neighbour_index);

                // Check if neighbouring cell is a T Cell
                if (p_neighbour_cell->GetMutationState()->IsType<TCellMutationState>())
                {
                    cell_iter->Kill();
                    break;
                }
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

