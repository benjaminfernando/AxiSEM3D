# AxiSEM3D - A Quickstart Guide

I look ugly to you? I am a Markdown file. If you do not have a Markdown reader, read me on [github](https://github.com/kuangdai/AxiSEM3D), or paste me [here](http://dillinger.io/).

Report issues to kuangdal@earth.ox.ac.uk.

## 1 Get AxiSEM3D
```sh
git clone https://github.com/kuangdai/AxiSEM3D
export CURRENT_WORK_DIR=$PWD
```

## 2 Installing dependencies
AxiSEM3D has been built upon a few modern numerical packages for its performance and sustainability, as listed below (newer versions are acceptable):

name | version | build-from-source instructions
--- | --- | --- 
mpi | --- | Common implementations such as [open-mpi](https://www.open-mpi.org/) or [mpich2](http://www.mpich.org/).
boost | 1.60 | Simply [download](https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.tar.bz2) and unzip to your path of treasures. No need to build or install.
eigen3 | 3.3-rc1 | Simply [download](http://bitbucket.org/eigen/eigen/get/3.3-rc1.tar.bz2) and unzip to your path of treasures. No need to build or install.
fftw | 3.3 | Install both double- and single-precision (with _--enable-float_) versions to the same path. See [here](http://www.fftw.org/fftw2_doc/fftw_6.html).
metis | 5.1 | [Download](http://glaros.dtc.umn.edu/gkhome/metis/metis/download) and install following `Install.txt`. Skip step 3 in `Install.txt` to use the 32-bit build. 
HDF5 |---| See [here](https://support.hdfgroup.org/HDF5/).

 Don't panic! All these popular packages may be handily installed with free package management software, such as [Conda](http://conda.pydata.org/docs/). Here we introduce the wizard `axisem3d_depends.sh`. 

* Make sure your MPI works properly. 
* Get [Conda](http://conda.pydata.org/docs/).  
* Edit the first few lines in `axisem3d_depends.sh` and run it.
* Check your `~/.bash_profile` (or `~/.bashrc`) and `~/.axisem3d_roots`. Grats, you are done!
    

## 3 Building AxiSEM3D
* Edit `SOLVER/CMakeLists.txt` if needed (normally not), including

    to-be-edited | notes
    ---|---
    compiler suit | Changes are usually required only for HPC clusters, unless you have a bizarre MPI. 
    dependency roots | No need if you have used `axisem3d_depends.sh` to install the dependencies.
    FFTW_WISDOM_DIR | Just specify any directory you like, or leave it as it. 
 
* Build AxiSEM3D (go step by step to see what's happening)

    * Style 1: work under source 

        ```sh
        cd $CURRENT_WORK_DIR/AxiSEM3D
        # make a simulation directory
        mkdir my_first_run
        cd my_first_run
        # cmake (*** see the notes below ***)
        cmake -DCMAKE_BUILD_TYPE=release ../SOLVER
        # compile and link
        make -j4
        # copy the input folder
        cp -R ../template/input ./
        # run it with any number of processors
        mpirun -np 4 ./axisem3d
        # check the outputs
        ls output
        ```
        
        Read the output of `cmake` carefully, which should end up with (If not, verify dependency installations as prompted)
        
      ```sh
      -- Configuring done
      -- Generating done
      -- Build files have been written to: ...
      ```
    
    * Style 2: keep source clean (suggested)
    
        To offer maximal flexibility for various infrastructures, AxiSEM3D is organized such that the directories of source, build and simulations are fully independent of one another. For exmaple:
        
        ```sh
        ########## build ##########
        cd $CURRENT_WORK_DIR
        # make a build directory
        mkdir my_axisem3d_build
        cd my_axisem3d_build
        # cmake
        cmake -DCMAKE_BUILD_TYPE=release ../AxiSEM3D/SOLVER
        # compile and link
        make -j4
        
        ########## run ##########
        # make a simulation directory
        cd $CURRENT_WORK_DIR
        mkdir my_second_run
        cd my_second_run
        # copy the executable 
        cp ../my_axisem3d_build/axisem3d ./
        # copy the input folder
        cp -R ../AxiSEM3D/template/input ./
        # Though optional, it is always a good practice 
        # to backup the source code for reproducibility:
        cp -r ../AxiSEM3D/SOLVER ./
        # run it with any number of processors
        mpirun -np 4 ./axisem3d
        # check the outputs
        ls output
        ```

## 4 The MESHER
In the above examples, we use the mesh file `AxiSEM_prem_ani_one_crust_50.e` (anisotropic PREM model with one crustal layer and a 50 s period), located at `SOLVER/template/input`. To generate an AxiSEM3D mesh like this, you will need the `salvus_mesher`, a python-based command-line tool to generate several types of 2-D and 3-D finite element meshes. 

Currently, the `salvus_mesher` is not published. Here we provide a stable version for AxiSEM3D:
* Install [pyexodus](https://github.com/SalvusHub/pyexodus) (mainly by Lion Krischer at ETH Zurich)
* Install [salvus_mesher](https://gitlab.com/Salvus/salvus_mesher) (mainly by van Driel Martin at ETH Zurich)

These two packages have their own requirements and manual. 



