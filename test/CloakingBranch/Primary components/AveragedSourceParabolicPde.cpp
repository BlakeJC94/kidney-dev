/*

Copyright (c) 2005-2016, University of Oxford.
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

#include "AveragedSourceParabolicPde.hpp"

#include "AbstractCentreBasedCellPopulation.hpp"
#include "VertexBasedCellPopulation.hpp"
#include "PottsBasedCellPopulation.hpp"
#include "CaBasedCellPopulation.hpp"
#include "ApoptoticCellProperty.hpp"
#include "TCellMutationState.hpp"
#include "TumorCellMutationState.hpp"
#include "CellLabel.hpp"

#include "Exception.hpp"

#include "Debug.hpp"

template<unsigned DIM>
AveragedSourceParabolicPde<DIM>::AveragedSourceParabolicPde(AbstractCellPopulation<DIM,DIM>& rCellPopulation,
                                                            double duDtCoefficient,
                                                            double diffusionCoefficient,
                                                            double uptakeCoefficient,
                                                            double decayCoefficient,
                                                            double cloakCoefficient)
    : mrCellPopulation(rCellPopulation),
      mDuDtCoefficient(duDtCoefficient),
      mDiffusionCoefficient(diffusionCoefficient),
      mUptakeCoefficient(uptakeCoefficient),
      mDecayCoefficient(decayCoefficient),
      mCloakCoefficient(cloakCoefficient)
{
}

template<unsigned DIM>
const AbstractCellPopulation<DIM,DIM>& AveragedSourceParabolicPde<DIM>::rGetCellPopulation() const
{
    return mrCellPopulation;
}

template<unsigned DIM>
void AveragedSourceParabolicPde<DIM>::SetupSourceTerms(TetrahedralMesh<DIM,DIM>& rCoarseMesh, std::map< CellPtr, unsigned >* pCellPdeElementMap) // must be called before solve
{
    // Allocate memory
    mTCellDensityOnCoarseElements.resize(rCoarseMesh.GetNumElements());
    mTumorCellDensityOnCoarseElements.resize(rCoarseMesh.GetNumElements());
    for (unsigned elem_index=0; elem_index < mTCellDensityOnCoarseElements.size(); elem_index++)
    {
        mTCellDensityOnCoarseElements[elem_index] = 0.0;
        mTumorCellDensityOnCoarseElements[elem_index] = 0.0;
    }

    // Loop over cells, find which coarse element it is in, and add 1 to mSourceTermOnCoarseElements[elem_index]
    for (typename AbstractCellPopulation<DIM>::Iterator cell_iter = mrCellPopulation.Begin();
         cell_iter != mrCellPopulation.End();
         ++cell_iter)
    {
        unsigned elem_index = 0;
        const ChastePoint<DIM>& r_position_of_cell = mrCellPopulation.GetLocationOfCellCentre(*cell_iter);

        if (pCellPdeElementMap != NULL)
        {
            elem_index = (*pCellPdeElementMap)[*cell_iter];
        }
        else
        {
            elem_index = rCoarseMesh.GetContainingElementIndex(r_position_of_cell);
        }

        // Update element map if cell has moved
        bool cell_is_apoptotic = cell_iter->template HasCellProperty<ApoptoticCellProperty>();
        bool cell_is_labelled = cell_iter->template HasCellProperty<CellLabel>();
        bool cell_is_t_cell = cell_iter->GetMutationState()->template IsType<TCellMutationState>();
        bool cell_is_tumor_cell = cell_iter->GetMutationState()->template IsType<TumorCellMutationState>();
        
        if (  (!cell_is_apoptotic) && (cell_is_labelled) && (cell_is_t_cell)  )
        {
            mTCellDensityOnCoarseElements[elem_index] += 1.0;
        }
        else if (  (!cell_is_apoptotic) && (cell_is_tumor_cell)  )
        {
            mTumorCellDensityOnCoarseElements[elem_index] += 1.0;
        }
    }

    // Then divide each entry of mSourceTermOnCoarseElements by the element's area
    c_matrix<double, DIM, DIM> jacobian;
    double det;
    for (unsigned elem_index=0; elem_index<mTCellDensityOnCoarseElements.size(); elem_index++)
    {
        rCoarseMesh.GetElement(elem_index)->CalculateJacobian(jacobian, det);
        mTCellDensityOnCoarseElements[elem_index] /= rCoarseMesh.GetElement(elem_index)->GetVolume(det);
    }
    
}

template<unsigned DIM>
double AveragedSourceParabolicPde<DIM>::ComputeDuDtCoefficientFunction(const ChastePoint<DIM>& )
{
    return mDuDtCoefficient;
}


template<unsigned DIM>
double AveragedSourceParabolicPde<DIM>::ComputeSourceTerm(const ChastePoint<DIM>& rX, double u, Element<DIM,DIM>* pElement)
{
    assert(!mTCellDensityOnCoarseElements.empty());
    double TCellcoefficient = mUptakeCoefficient * mTCellDensityOnCoarseElements[pElement->GetIndex()];
    
    assert(!mTumorCellDensityOnCoarseElements.empty());
    double TumorCellcoefficient = mUptakeCoefficient * mTumorCellDensityOnCoarseElements[pElement->GetIndex()];

    // The source term is C-d*u (TODO: update)
    //return coefficient-5.0*u;
    //return TCellcoefficient - mDecayCoefficient * u;
    //return TCellcoefficient - mDecayCoefficient * u - 10 * TumorCellcoefficient * u;
    
    //return TCellcoefficient - mDecayCoefficient * u;
    
    double source_term = TCellcoefficient - (mDecayCoefficient + 10 * TumorCellcoefficient) * u;
    if (u > 0.01)
    {
        return source_term;
    }
    else if ((u > 0) && (u <= 0.01))
    {
        return TCellcoefficient - mDecayCoefficient * u ;
    }
    else 
    {
        return TCellcoefficient;
    }
}

template<unsigned DIM>
double AveragedSourceParabolicPde<DIM>::ComputeSourceTermAtNode(const Node<DIM>& rNode, double u)
{
    NEVER_REACHED;
    return 0.0;
}

template<unsigned DIM>
c_matrix<double,DIM,DIM> AveragedSourceParabolicPde<DIM>::ComputeDiffusionTerm(const ChastePoint<DIM>& rX, Element<DIM,DIM>* pElement)
{
    return mDiffusionCoefficient*identity_matrix<double>(DIM);
}

template<unsigned DIM>
double AveragedSourceParabolicPde<DIM>::GetUptakeRateForElement(unsigned elementIndex)
{
    return this->mTCellDensityOnCoarseElements[elementIndex];
}

/////////////////////////////////////////////////////////////////////////////
// Explicit instantiation
/////////////////////////////////////////////////////////////////////////////

template class AveragedSourceParabolicPde<1>;
template class AveragedSourceParabolicPde<2>;
template class AveragedSourceParabolicPde<3>;

// Serialization for Boost >= 1.36
#include "SerializationExportWrapperForCpp.hpp"
EXPORT_TEMPLATE_CLASS_SAME_DIMS(AveragedSourceParabolicPde)