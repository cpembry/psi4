/*! \file
    \ingroup LMP2
    \brief Enter brief description of file here
*/
#ifndef _psi_src_bin_lmp2_h_
#define _psi_src_bin_lmp2_h_

#include <string>

namespace boost {
template<class T> class shared_ptr;
}

namespace psi{

class BasisSet;
class Options;
class PSIO;
class Chkpt;

namespace lmp2{

class LMP2 {

  private:
    int nso;			/* no. of symmetry adapted atomic orbitals */
    int natom;			/* no. of atoms */
    int nocc;			/* no. of occupied orbitals */
    int nshell; 		/* no. of angular momentum shells */
    int nirreps;		/* no. of irreducible representations */
    double **C;			/* SCF MO Coefficients in the AO basis */
    double **loevals;
    double **D;			/* Density matrix in the AO basis */
    double **aoovlp;		/* Overlap matrix  in the AO basis */
    double **paoovlp;		/* Overlap matrix in the PAO basis */
    double **aoF;		/* Fock Matrix in the AO basis */
    double **loF;               /* Fock Matrix in the LO basis */
    double **paoF;		/* Fock Matrix in the PAO basis */
    int *aostart;
    int *aostop;
    int *aostart_shell;
    int *aostop_shell;
    int *ao2atom;
    int puream;
    int *l_length;
    int **domain;		/* Orbital domains */
    int *domain_len;		/* Size of each domain */
    int *pairdom_exist;          /* Wether or not the pairdomain exists */
    int *pairdom_nrlen;		/* Size of each pair domain after orthogonalization */
    int ij_pairs;		/* no. of ij pairs */
    int pairs_per_proc;         /* no. of ij pairs owned by each process */
//    double *fR;
    double **Rt_full;		/* Full AO to PO matrix */
    double ***W;
    boost::shared_ptr<PSIO> psio;
    boost::shared_ptr<Chkpt> chkpt;
    int ref;
    char *wfn;
    char *jobtype;
    int print;
    int semicanonical;
    int relax_opdm;
    long int memory;
    int num_threads;
    double rmsconv;
    double econv;
    double escreen;
    int screen_int;
    int maxiter;
    int iter;
    int conv;
    int diis;
    int ndiis;
    int neglectdp;
    double dpcutoff;
    int it_diis;
    double cutoff;
    double fskip;
    int iconv;
    int rconv;
    int fs;
    int div;
    int matsize;
    int dmat1;
    int dmat2;
    int omat;
    int nmat;
    int RILMP2;
    int schwarz_tol;
    double tol;
    double scs_scale_os;
    double scs_scale_ss;
    int rinshell;

    double ****error;
    double ****T_ext;
    double ****T;
    double ***Ktilde;      /* ERI transformed to (ai|bj) where ij are occupied localized orbitals and ab are projected ao's */
    int num_unique_shells;
    double Emp2;           /* LMP2 energy */
    double E_OS;           /* Opposite spin MP2 energy */
    double E_SS;           /* Same spin MP2 energy */
    double Emp2_old;       /* LMP2 energy from previous iteration */
    double Escf;
    double DEmp2;
    double Drms;
    std::string ri_basis, orbital_basis;

  public:
    /* Default constructr/destructor, does nothing */
    LMP2();
    ~LMP2();

    /* Constructor for the main process (myid == 0) */
    LMP2(boost::shared_ptr<PSIO> psio_o, boost::shared_ptr<Chkpt> chkpt_o);

    /* Constructor for all other processes */
    LMP2(boost::shared_ptr<PSIO> psio_o);

    // Creates a chkpt object and reads data from it
    void get_moinfo();
    int get_moinfo(char s);

    // Get input parameters
    void get_params(Options &options);

    // Computes the one-particle density matrix
    void opdm();

    // Prints the data obtained from the checkpoint file
    void print_moinfo();

    // Print the input parameters
    void print_params();

    // Localize the MO coefficients
    void localize();

    // Get the fock matrix from the checkpoint file
    void get_fock();

    // Computes the i and ij pair domains
    void domains();

    // Prints the domains
    void print_domains(double *s);

    // Compute the projection matrix and the W matrices
    void projection();

    // Performs the integral direct transformation
    void direct_transformation();

    // Sorts the integral shells into decending order
    void sort_shell(int **A, int n);

    // Sorts the Schwarz integrals into decending order
    int *sort_schwarz_MN(double **A, int M_, int n_);

    // Get the maximum angular momentum shell
    int get_max_shell();

    // Send the overlap matrix to all of the nodes
    void send_overlap(double **ovlp);

    // allocate the arrays for the amplitudes and the error matrices
    void allocate_T();

    // Gets the diis parameters for each iteration
    void get_diis_params();

    // iteration loop
    void iterate();

    // Computes the amplitudes
    void amplitudes();

    // Performs the diis extapolation
    void diis_ext();

    // Computes the LMP2 energy
    void energy();

    // Prints the information for each iteration
    void print_iteration();

    // Checks to see if the energy and Amplitudes are converged
    void check_conv();

    // Construct MO integrals from density-fitted 3-center quantities
    void direct_df_transformation();
    void direct_df_transformation3();

    // These member functions read data from the checkpoint file
    int  get_nso();
    int  get_natom();
    int  get_nirreps();
    int  get_nshell();
    int  get_puream();
    int* get_doccpi();
    int* get_soccpi();
    int  get_frdocc();
    int* get_stype();
    int* get_aux_stype(char *basisname, int rinshell);
    int* get_snuc();
    int* get_aux_snuc(char *basisname, int rishell);
    int* get_orbspi();
    int* get_frzvpi();
    double  get_enuc();
    double  get_escf();
    double* get_evals();
    double** get_MOC();
    double** get_geom();

    // These member function compute which proc owns each ij pair
    // and determines the absolute ij value of the local proc
    int *get_ij_local();
    int *get_ij_owner();
    int *get_mn_owner(int n);
    int get_mn_pairs(int n);
    int get_num_unique_shells();
    int **get_MN_shell(boost::shared_ptr<BasisSet> basisset);
    int **get_ij_map();
    int *original_ij_map();

    // These member functions get the pairdomain and pairdomain lengths
    int **compute_pairdomain(int **ij_map_);
    int *compute_pairdomlen(int **ij_map_);

    // These member functions compute the required prescreening quantities
    double **get_T_Schwarz_NS(int num_unique_shells_);

};

}}

#endif /* Header guard */
