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

#include "UtericBudCellCycleModel.hpp"
#include "Exception.hpp"
#include "StemCellProliferativeType.hpp"
#include "TransitCellProliferativeType.hpp"
#include "DifferentiatedCellProliferativeType.hpp"

#include "UT_CellMutationState.hpp"
#include "TumorCellMutationState.hpp"

UtericBudCellCycleModel::UtericBudCellCycleModel()
{
}

AbstractCellCycleModel* UtericBudCellCycleModel::CreateCellCycleModel()
{
    // Create a new cell-cycle model
    UtericBudCellCycleModel* p_model = new UtericBudCellCycleModel();

    /*
     * Set each member variable of the new cell-cycle model that inherits
     * its value from the parent.
     *
     * Note 1: some of the new cell-cycle model's member variables (namely
     * mBirthTime, mCurrentCellCyclePhase, mReadyToDivide) will already have been
     * correctly initialized in its constructor.
     *
     * Note 2: one or more of the new cell-cycle model's member variables
     * may be set/overwritten as soon as InitialiseDaughterCell() is called on
     * the new cell-cycle model.
     *
     * Note 3: the member variable mDimension remains unset, since this cell-cycle
     * model does not need to know the spatial dimension, so if we were to call
     * SetDimension() on the new cell-cycle model an exception would be triggered;
     * hence we do not set this member variable.
     */
    p_model->SetBirthTime(mBirthTime);
    p_model->SetMinimumGapDuration(mMinimumGapDuration);
    p_model->SetStemCellG1Duration(mStemCellG1Duration);
    p_model->SetTransitCellG1Duration(mTransitCellG1Duration);
    p_model->SetSDuration(mSDuration);
    p_model->SetG2Duration(mG2Duration);
    p_model->SetMDuration(mMDuration);
    p_model->SetGeneration(mGeneration);
    p_model->SetMaxTransitGenerations(mMaxTransitGenerations);

    return p_model;
}

void UtericBudCellCycleModel::SetSpawnRate(double newValue)
{
    mSpawnRate = newValue;
}

void UtericBudCellCycleModel::SetG1Duration()
{
    RandomNumberGenerator* p_gen = RandomNumberGenerator::Instance();
    //double lambda = 100; // Exponential dist. param for Stem T Cell proliferation
    
        
    /* --==-- 01: UT Cell division options --==-- */
    if (mpCell->GetMutationState()->IsType<UT_CellMutationState>())
    {
        if (mpCell->GetCellProliferativeType()->IsType<StemCellProliferativeType>())
        {
            //mG1Duration = (-log(p_gen->ranf())*GetStemCellG1Duration())/lambda; // E[lambda]
            //mG1Duration = GetStemCellG1Duration() + 0.2*p_gen->ranf(); // U[0,0.2]
            /*mMDuration = 0.05;
            mG1Duration = 0.01;
            mSDuration = 0.01;
            mG2Duration = 0.01;*/
            mMDuration = mSpawnRate/4;
            mG1Duration = mSpawnRate/4;
            mSDuration = mSpawnRate/4;
            mG2Duration = mSpawnRate/4;
            
        }
        else if (mpCell->GetCellProliferativeType()->IsType<TransitCellProliferativeType>())
        {
            mG1Duration = DBL_MAX;
        }
        else if (mpCell->GetCellProliferativeType()->IsType<DifferentiatedCellProliferativeType>())
        {
            mG1Duration = DBL_MAX;
        }
        else
        {
        NEVER_REACHED;
        }
    }
    
    
    /* --==-- 02: CM Cell division options --==-- */
    // (Increase SetMaxTransitGenerations if there's any issues here)
    else if (mpCell->GetMutationState()->IsType<TumorCellMutationState>())
    {
        if (mpCell->GetCellProliferativeType()->IsType<StemCellProliferativeType>())
        {
            NEVER_REACHED;
        }
        else if (mpCell->GetCellProliferativeType()->IsType<TransitCellProliferativeType>())
        {
            //mG1Duration = GetTransitCellG1Duration() + 2*p_gen->ranf() + 4; // U[4,6]
            mG1Duration = GetTransitCellG1Duration() + 5*p_gen->ranf(); // U[0,5]
            //mG1Duration = GetTransitCellG1Duration() + 2*p_gen->ranf(); // U[0,2]
        }
        else if (mpCell->GetCellProliferativeType()->IsType<DifferentiatedCellProliferativeType>())
        {
            NEVER_REACHED; 
        }
        else
        {
            NEVER_REACHED;
        }
    }
    
     
}

void UtericBudCellCycleModel::OutputCellCycleModelParameters(out_stream& rParamsFile)
{
    // No new parameters to output

    // Call method on direct parent class
    AbstractSimpleCellCycleModel::OutputCellCycleModelParameters(rParamsFile);
}

// Serialization for Boost >= 1.36
#include "SerializationExportWrapperForCpp.hpp"
CHASTE_CLASS_EXPORT(UtericBudCellCycleModel)
