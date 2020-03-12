#include <iostream>

using std::cout;
using std::endl;

#include "define_t.h"
#include "externClass.h"
 //defines the global variables to be defined in the configuration phase
#include "global.h"
 //ver quais dos includes devem sair para rodar no Linux (cluster)
#include "headfile.h"
#include "miscellaneous.h"

using namespace std;

int testing_input_args(int argc, char **argv);

int hyper_PSO(string HyperFileName, int TotRuns, int Run);

int main_Hyper_PSO(int argc, char **argv) {
  Problem_List[0] = "FlShp";
  //  Problem_List[1] = "Queen";
  //  Problem_List[2] = "Mx1s";
  // Problem_List[3] = "Gcol";

  str_N = "";

  //////////////////////////////testing Error of Inputs
  ////////////////////////////////////////////////
  if (testing_input_args(argc, argv) == EXIT_FAILURE) return (EXIT_FAILURE);
  ////////////////////////////////////////////////////////////////////////////////////////////////////

  pargv = argv;  // global var to allow the acess of comand line arguments for
                 // all the modules

  list_mhClass lMH;
  lMH.init_MH_List();
  string HyperFileName = argv[arg_hcName];
  // PRINT mandatory arguments  on Screen
  cout << "\n\n\n\nMAIN CONFIGURATION \n";
  cout << "\nInput File:" << HyperFileName << ".in";
  lMH.print_MH_List(cout);
  if (HPPSOmode == 1)
    cout << "HPPSO running in a Unique Instance Mode ";
  else
    cout << "HPPSO running in Irace Mode ";
  cout << "\nSolving  " << argv[arg_bsProb] << " problem";
  cout << "\nInstance(s):" << argv[arg_IName] << '\n';

  int TotRuns, Run;

  if (HPPSOmode == 1 && argc > (int)arg_totruns_or_run) {
    string tr = argv[arg_totruns_or_run];
    tr.erase(tr.begin());  // erase symbol t or r from tr
    char run_type = argv[arg_totruns_or_run][0];

    switch (run_type) {
      case 't':  // program executes TotRuns starting from Run 1
        TotRuns = atoi(tr.c_str());
        cout << "totruns:" << TotRuns << endl;
        //mypause();
        Run = 1;
        if (TotRuns > 0)
          cout << "\ntotal of Runs: " << TotRuns << "\n\n";
        else {
          Run = 0;  // HPPSO will continue the evol process from an interrupted
                    // point (read vars from file)
          cout << "\nRestarting from a previous execution..." << endl;
        }
        break;
      case 'r':  // program executes only one run (numbered by the user)
        TotRuns = 1;
        Run = atoi(tr.c_str());
        cout << "\nCurrent Run: " << Run << "\n\n";
        break;

      default:
        cout << "Wrong argument " << argv[arg_totruns_or_run] << " at positon "
             << (int)arg_totruns_or_run << endl;
        //<< "The expected argument is tXXX or rXXX where XXX is a number
        //indication TotRuns or Run Number" << endl;
        exit(1);
    }
  } else  // if (HPPSOmode == 1)
  {
    TotRuns = 1;
    Run = 1;
  }

  if (argc > (int)arg_runs_seed_app) {
    string tr = pargv[arg_runs_seed_app];
    tr.erase(tr.begin());  // erase symbol f
    Tot_Exec_Seed_Solv_Prob = atoi(tr.c_str());
  } else
    Tot_Exec_Seed_Solv_Prob = 1;

  // optional arguments

  if (argc > (int)arg_stopcond) {
    string s_stopcond;
    s_stopcond = pargv[arg_stopcond];
    Lim_MHEval = atoi(s_stopcond.c_str());
    cout << "\nMaxEval:" << Lim_MHEval << endl;
  } else
    Lim_MHEval = 0;  // it will read from the input file

  //////////////////////////////////////////////////

  if (hyper_PSO(HyperFileName, TotRuns, Run) == (EXIT_FAILURE)) {
    cerr << "\nError in Hyper_PSO(..)  ... Exiting main_Hyper_PSO(..) ....\n";
    return (EXIT_FAILURE);
  }
  return (EXIT_SUCCESS);
}

int testing_input_args(int argc, char **argv) {
  int min_arg;

  if (argc > 1)
    HPPSOmode = atoi(argv[arg_mode]);  //(1) 1 instance / whole Exec (2) two or
                                       //more instances per time - like Irace
  else {
    cerr << "\n\nPlease provide a valid HPPSO mode argument : 1 (inst/run) or "
            "2 (iRace) \n";
    exit(1);
  }

  if (HPPSOmode == 1) {
    min_arg = 6;
  }  // 1+5(min) + 3(opt)}
  else if (HPPSOmode == 2) {
    min_arg = 6;
  }  // 1+5(min) + 1(opt)
  else {
    cerr << atoi(argv[arg_mode]) << endl;
    cerr << "\n\nPlease provide a valid HPPSO mode argument : 1 (inst/run) or "
            "2 (iRace) \n";
    return (EXIT_FAILURE);
  }

  if (argc < min_arg) {
    if (argc > 1)
      cerr << "\n\nError!!! Less than " << min_arg
           << " parameters in the comand line\n";

    cerr << "\n\nTo execute the program, you must specify (at least "
         << min_arg - 1 << " arguments besides the exe name):\n";
    int i = 1;
    cerr << i++ << ") HPPSO mode (1) one instance for a complete execution "
                   "(totRuns) (2) like Irace\n";
    cerr << i++
         << ") the initial running condition  (e.g (b)eginning or (r)estart) "
         << endl;
    cerr << i++ << ") the problem name (e.g. FlShp) " << endl;
    cerr << i++ << ") HC + the name of Hyper Prob PSO configuration file (e.g. "
                   "HC100_20b1_p1_s1)"
         << endl;

    if (HPPSOmode == 1) {
      cerr << i++
           << ") I + the name of the Instance file   (e.g. I20_5/20_5_01)"
           << endl;

      cerr << i++ << ") t + tot of runs of HPPSO (e.g. t10)  or r + run number "
                     "(e.g. r22), as the 4th argument\n\n"
           << endl;
      cerr << "\n and one optional argument " << endl;
      cerr << " f + tot of executions (for fitness computation) (e.g. f10) \n\n"
           << endl;
    } else {
      cerr << i++ << ") I + the set of file Instances used for training  +  "
                     "(e.g. IsetTr1 or jcorr or mcorr)"
           << endl;
    }
    // cerr << " the stopping condition to be considered by all the
    // Metaheuristics, as the 6th argument (default maxEval=20000 evs)\n\n" <<
    // endl; //esta vai ser calculada com base na dificuldade do problema
    return (EXIT_FAILURE);
  }

  // error for init condidition (b)eginning or (r)estart
  if (argc > (int)arg_init_cond) {
    char init = argv[arg_init_cond][0];
    if (init != 'b' && init != 'r' && init != 'w') {
      cerr << "\n\nPlease provide a valid initial running condition b/r "
              "(b)eginning or (r)estart (w)inners \n\n";
      return (EXIT_FAILURE);
    }
  }

  // error for Problem name
  if (argc > (int)arg_bsProb) {
    int ok = 0;
    for (int i = 0; i < Problem_List.size(); i++)
      if ((string)argv[arg_bsProb] == Problem_List[i]) {
        ok = 1;
        break;
      }
    if (ok == 0) {
      cerr << "\n\nPlease provide a valid problem name as the first argument\n";
      cerr << "list of valid names:\n";
      for (int i = 0; i < Problem_List.size(); i++)
        cerr << Problem_List[i] << endl;
      return (EXIT_FAILURE);
    }
  }

  // error for HC name
  if (argc > (int)arg_hcName) {
    if (argv[arg_hcName][0] != 'H' || argv[arg_hcName][1] != 'C') {
      cerr << "\n\n\n\nPlease provide HC + the name of Hyper Prob PSO "
              "configuration file, as the 3th argument (e.g. "
              "HC100_b1_p1_s1)\n\n";
      return (EXIT_FAILURE);
    }
  }

  // error for (set of) Instance(s) name
  if (argc > (int)arg_IName)
    if (argv[arg_IName][0] != 'I' && HPPSOmode == 1) {
      cerr << "\n\nPlease provide  I + name of the Instance file (e.g. "
              "I20_5/20_5_01) for HPPSOmode == 1 (Instance Mode) \n\n";
      return (EXIT_FAILURE);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A main that catches the exceptions

int main(int argc, char **argv) {
  try {
    main_Hyper_PSO(argc, argv);
  } catch (exception &e) {
    cout << "Exception: " << e.what() << '\n';
    return 1;
  }
  return 0;
}
