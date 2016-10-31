// axisem.cpp
// created by Kuangdai on 26-Mar-2016 
// main of AxiSEM3D

#include "axisem.h"

int axisem_main(int argc, char *argv[]) {
    
    try {
        
        // variable sets
        PreloopVariables pl;
        SolverVariables sv;
        
        // initialize mpi
        XMPI::initialize(argc, argv);
        
        //////// spectral-element constants
        SpectralConstants::initialize(nPol);  
        
        //////// input parameters 
        int verbose;
        Parameters::buildInparam(pl.mParameters, verbose);
        
        //////// exodus model and attenuation parameters 
        ExodusModel::buildInparam(pl.mExodusModel, *(pl.mParameters), pl.mAttParameters, verbose);
        
        //////// fourier field 
        NrField::buildInparam(pl.mNrField, *(pl.mParameters), pl.mExodusModel->getROuter(), verbose);
        initializeSolverStatic(*(pl.mNrField)); // initialize static variables in solver
        
        //////// 3D models 
        Volumetric3D::buildInparam(pl.mVolumetric3D, *(pl.mParameters), verbose);
        Geometric3D::buildInparam(pl.mGeometric3D, *(pl.mParameters), verbose);
        OceanLoad3D::buildInparam(pl.mOceanLoad3D, *(pl.mParameters), verbose);
        
        //////// source
        Source::buildInparam(pl.mSource, *(pl.mParameters), verbose);
        double srcLat = pl.mSource->getLatitude();
        double srcLon = pl.mSource->getLongitude();
        double srcDep = pl.mSource->getDepth();
        
        //////// mesh, phase 1
        // define mesh
        pl.mMesh = new Mesh(pl.mExodusModel, pl.mNrField, srcLat, srcLon, srcDep, *(pl.mParameters));
        pl.mMesh->setVolumetric3D(pl.mVolumetric3D);
        pl.mMesh->setGeometric3D(pl.mGeometric3D);
        pl.mMesh->setOceanLoad3D(pl.mOceanLoad3D);
        // build unweighted local mesh 
        pl.mMesh->buildUnweighted();
        
        //////// dt
        double dt_mesh = pl.mMesh->getDeltaT();
        double dt_user = pl.mParameters->getValue<double>("TIME_DELTA_T");
        double dt = (dt_user > 0.) ? dt_user : dt_mesh;
        
        //////// attenuation
        AttBuilder::buildInparam(pl.mAttBuilder, *(pl.mParameters), *(pl.mAttParameters), dt, verbose);
        //////// mesh, phase 2
        pl.mMesh->setAttBuilder(pl.mAttBuilder);
        pl.mMesh->buildWeighted();
        
        //////// mesh test 
        // test positive-definiteness and self-adjointness of stiffness and mass matrices
        // better to turn with USE_DOUBLE 
        // pl.mMesh->test();
        // XMPI::barrier();
        // exit(0);
        
        //////// source time function 
        STF::buildInparam(pl.mSTF, *(pl.mParameters), dt, verbose);
        
        //////// receivers
        ReceiverCollection::buildInparam(pl.mReceivers, 
            *(pl.mParameters), srcLat, srcLon, srcDep, verbose);
        
        //////// computational domain
        sv.mDomain = new Domain();
        // release mesh
        pl.mMesh->release(*(sv.mDomain));
        // release source 
        pl.mSource->release(*(sv.mDomain), *(pl.mMesh));
        // release stf 
        pl.mSTF->release(*(sv.mDomain));
        // release receivers
        pl.mReceivers->release(*(sv.mDomain), *(pl.mMesh));
        // verbose domain 
        if (verbose) XMPI::cout << sv.mDomain->verbose();
        
        //////////////////////// PREPROCESS DONE ////////////////////////
        
        //////// Newmark
        int infoInt = pl.mParameters->getValue<int>("OPTION_LOOP_INFO_INTERVAL");
        int stabInt = pl.mParameters->getValue<int>("OPTION_STABILITY_INTERVAL");
        sv.mNewmark = new Newmark(sv.mDomain, infoInt, stabInt);
        
        //////// final preparations
        // finalize preloop variables before time loop starts
        pl.finalize();
        // forbid matrix allocation in time loop
        #ifndef NDEBUG
            Eigen::internal::set_is_malloc_allowed(false);
        #endif
            
        //////// GoGoGo
        XMPI::barrier();
        sv.mNewmark->solve();
        
        //////// finalize solver
        // solver 
        sv.finalize();
        // static variables in solver
        finalizeSolverStatic();
        
        // finalize mpi 
        XMPI::finalize();
        
    } catch (const std::exception &e) {
        // print exception
        XMPI::cout.setp(XMPI::rank());
        XMPI::printException(e);
        
        // abort program
        // TODO 
        // MPI_Abort is necessary here. Otherwise, if an exception
        // is thrown from one of the procs, deadlock will occur.
        // But the problem is, how we free memories on other procs?!
        XMPI::abort();
    }
    
    return 0;
}

#include "SolverFFTW.h"
#include "SolverFFTW_1.h"
#include "SolverFFTW_3.h"
#include "SolverFFTW_N3.h"
#include "SolverFFTW_N6.h"
#include "SolverFFTW_N9.h"
#include "PreloopFFTW.h"
#include "SolidElement.h"
#include "FluidElement.h"

extern void initializeSolverStatic(const NrField &nrf) {
    // fftw
    SolverFFTW::importWisdom();
    int maxNr = ceil(nrf.getMaxNr() * 1.1);
    SolverFFTW_1::initialize(maxNr);
    SolverFFTW_3::initialize(maxNr); 
    SolverFFTW_N3::initialize(maxNr);
    SolverFFTW_N6::initialize(maxNr);
    SolverFFTW_N9::initialize(maxNr);
    SolverFFTW::exportWisdom();
    PreloopFFTW::initialize(maxNr);
    // element
    SolidElement::initWorkspace(maxNr / 2);
    FluidElement::initWorkspace(maxNr / 2);
};

extern void finalizeSolverStatic() {
    // fftw
    SolverFFTW_1::finalize();
    SolverFFTW_3::finalize(); 
    SolverFFTW_N3::finalize();
    SolverFFTW_N6::finalize();
    SolverFFTW_N9::finalize();
    PreloopFFTW::finalize();
};