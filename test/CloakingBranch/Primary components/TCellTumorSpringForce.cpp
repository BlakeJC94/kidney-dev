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

#include "TCellTumorSpringForce.hpp"
#include "IsNan.hpp"

#include "TCellMutationState.hpp"
#include "TumorCellMutationState.hpp"

template<unsigned DIM>
TCellTumorSpringForce<DIM>::TCellTumorSpringForce()
   : GeneralisedLinearSpringForce<DIM>()
{
}

template<unsigned DIM>
void TCellTumorSpringForce<DIM>::AddForceContribution(AbstractCellPopulation<DIM>& rCellPopulation)
{
    // Throw an exception message if not using a NodeBasedCellPopulation
    if (dynamic_cast<NodeBasedCellPopulation<DIM>*>(&rCellPopulation) == NULL)
    {
        EXCEPTION("TCellTumorSpringForce is to be used with a NodeBasedCellPopulation only");
    }

    std::vector< std::pair<Node<DIM>*, Node<DIM>* > >& r_node_pairs = (static_cast<NodeBasedCellPopulation<DIM>*>(&rCellPopulation))->rGetNodePairs();
    
    // Iterate over all pairs of nodes
    for (typename std::vector< std::pair<Node<DIM>*, Node<DIM>* > >::iterator iter = r_node_pairs.begin();
        iter != r_node_pairs.end();
        iter++)
    {
        std::pair<Node<DIM>*, Node<DIM>* > pair = *iter;
        Node<DIM>* p_node_a = pair.first;
        Node<DIM>* p_node_b = pair.second;

        // Get the node locations, radii and indices
        c_vector<double, DIM> node_a_location = p_node_a->rGetLocation();
        c_vector<double, DIM> node_b_location =  p_node_b->rGetLocation();
        double node_a_radius = p_node_a->GetRadius();
        double node_b_radius = p_node_b->GetRadius();
        unsigned node_a_index = p_node_a->GetIndex();
        unsigned node_b_index = p_node_b->GetIndex();
        
        // Get the cells associated with these node indices
        CellPtr p_cell_a = rCellPopulation.GetCellUsingLocationIndex(node_a_index);
        CellPtr p_cell_b = rCellPopulation.GetCellUsingLocationIndex(node_b_index);
        
        
        /* --==-- 01: T Cell - T Cell Repulsion segment --==-- */
        /* Applies RepulsionForce between Differentiated T-Cells (labelled and unlabelled) */
        if (  (p_cell_a->GetMutationState()->IsType<TCellMutationState>()) && (p_cell_b->GetMutationState()->IsType<TCellMutationState>()) 
            && (p_cell_a->GetCellProliferativeType()->IsType<DifferentiatedCellProliferativeType>()) && (p_cell_b->GetCellProliferativeType()->IsType<DifferentiatedCellProliferativeType>()) )
        {
            c_vector<double, DIM> unit_difference;
            unit_difference = (static_cast<NodeBasedCellPopulation<DIM>*>(&rCellPopulation))->rGetMesh().GetVectorFromAtoB(node_a_location, node_b_location);

            double rest_length = node_a_radius+node_b_radius;
            
            // Only apply force if the cells are close (repulsion force)
            if (norm_2(unit_difference) < rest_length)
            {
                c_vector<double, DIM> force = this->CalculateForceBetweenNodes(p_node_a->GetIndex(), p_node_b->GetIndex(), rCellPopulation);
                c_vector<double, DIM> negative_force = -1.0 * force;
                for (unsigned j=0; j<DIM; j++)
                {
                    assert(!std::isnan(force[j]));
                }
                
                p_node_a->AddAppliedForceContribution(force);
                p_node_b->AddAppliedForceContribution(negative_force);
            }
        }
        
        
        /* --==-- 02: T Cell - Tumor Cell Springs segment --==-- */
        /* Applies GeneralisedLinearSpringForce between Labelled T-Cells and Tumor Cells */
        if (  (p_cell_a->HasCellProperty<CellLabel>()) && (p_cell_b->GetMutationState()->IsType<TumorCellMutationState>()) 
            || (p_cell_b->HasCellProperty<CellLabel>()) && (p_cell_a->GetMutationState()->IsType<TumorCellMutationState>())  )
        {
        
            // Calculate the force between nodes
            c_vector<double, DIM> force = this->CalculateForceBetweenNodes(p_node_a->GetIndex(), p_node_b->GetIndex(), rCellPopulation);
            c_vector<double, DIM> negative_force = -1.0 * force;
            for (unsigned j=0; j<DIM; j++)
            {
                assert(!std::isnan(force[j]));
            }
            
            // Add the force contribution to each node
            p_node_a->AddAppliedForceContribution(force);
            p_node_b->AddAppliedForceContribution(negative_force);
        }
        
        
        /* --==-- 03: Tumor Cell - Tumor Cell Springs segment --==-- */
        /* Applies GeneralisedLinearSpringForce between Tumor Cells. Check both cells are Tumor Cells */
        if (  (p_cell_a->GetMutationState()->IsType<TumorCellMutationState>()) && (p_cell_b->GetMutationState()->IsType<TumorCellMutationState>())  ) 
        {   
            // Calculate the force between nodes
            c_vector<double, DIM> force = this->CalculateForceBetweenNodes(p_node_a->GetIndex(), p_node_b->GetIndex(), rCellPopulation);
            c_vector<double, DIM> negative_force = -1.0 * force;
            for (unsigned j=0; j<DIM; j++)
            {
                assert(!std::isnan(force[j]));
            }
            
            // Add the force contribution to each node
            p_node_a->AddAppliedForceContribution(force);
            p_node_b->AddAppliedForceContribution(negative_force);
        }
        
        
    }
}

template<unsigned DIM>
void TCellTumorSpringForce<DIM>::OutputForceParameters(out_stream& rParamsFile)
{
    // Call direct parent class
    GeneralisedLinearSpringForce<DIM>::OutputForceParameters(rParamsFile);
}

// Explicit instantiation
template class TCellTumorSpringForce<1>;
template class TCellTumorSpringForce<2>;
template class TCellTumorSpringForce<3>;

// Serialization for Boost >= 1.36
#include "SerializationExportWrapperForCpp.hpp"
EXPORT_TEMPLATE_CLASS_SAME_DIMS(TCellTumorSpringForce)