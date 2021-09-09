// Generated by using Rcpp::compileAttributes() -> do not edit by hand
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include <Rcpp.h>

using namespace Rcpp;

#ifdef RCPP_USE_GLOBAL_ROSTREAM
Rcpp::Rostream<true>&  Rcpp::Rcout = Rcpp::Rcpp_cout_get();
Rcpp::Rostream<false>& Rcpp::Rcerr = Rcpp::Rcpp_cerr_get();
#endif

// cppEAJumpCost
List cppEAJumpCost(int size, int k, double mu, long seed);
RcppExport SEXP _FlowshopSolveR_cppEAJumpCost(SEXP sizeSEXP, SEXP kSEXP, SEXP muSEXP, SEXP seedSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< int >::type size(sizeSEXP);
    Rcpp::traits::input_parameter< int >::type k(kSEXP);
    Rcpp::traits::input_parameter< double >::type mu(muSEXP);
    Rcpp::traits::input_parameter< long >::type seed(seedSEXP);
    rcpp_result_gen = Rcpp::wrap(cppEAJumpCost(size, k, mu, seed));
    return rcpp_result_gen;
END_RCPP
}
// initFactories
void initFactories(std::string data_folder);
RcppExport SEXP _FlowshopSolveR_initFactories(SEXP data_folderSEXP) {
BEGIN_RCPP
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< std::string >::type data_folder(data_folderSEXP);
    initFactories(data_folder);
    return R_NilValue;
END_RCPP
}
// solveFSP
List solveFSP(std::string mh, Rcpp::CharacterVector rproblem, long seed, Rcpp::CharacterVector rparams, bool verbose);
RcppExport SEXP _FlowshopSolveR_solveFSP(SEXP mhSEXP, SEXP rproblemSEXP, SEXP seedSEXP, SEXP rparamsSEXP, SEXP verboseSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< std::string >::type mh(mhSEXP);
    Rcpp::traits::input_parameter< Rcpp::CharacterVector >::type rproblem(rproblemSEXP);
    Rcpp::traits::input_parameter< long >::type seed(seedSEXP);
    Rcpp::traits::input_parameter< Rcpp::CharacterVector >::type rparams(rparamsSEXP);
    Rcpp::traits::input_parameter< bool >::type verbose(verboseSEXP);
    rcpp_result_gen = Rcpp::wrap(solveFSP(mh, rproblem, seed, rparams, verbose));
    return rcpp_result_gen;
END_RCPP
}
// sampleSolutionStatisticsFLA
List sampleSolutionStatisticsFLA(std::string dataFolder, Rcpp::CharacterVector rproblem, long noSamples, long seed);
RcppExport SEXP _FlowshopSolveR_sampleSolutionStatisticsFLA(SEXP dataFolderSEXP, SEXP rproblemSEXP, SEXP noSamplesSEXP, SEXP seedSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< std::string >::type dataFolder(dataFolderSEXP);
    Rcpp::traits::input_parameter< Rcpp::CharacterVector >::type rproblem(rproblemSEXP);
    Rcpp::traits::input_parameter< long >::type noSamples(noSamplesSEXP);
    Rcpp::traits::input_parameter< long >::type seed(seedSEXP);
    rcpp_result_gen = Rcpp::wrap(sampleSolutionStatisticsFLA(dataFolder, rproblem, noSamples, seed));
    return rcpp_result_gen;
END_RCPP
}
// sampleRandomWalk
std::vector<double> sampleRandomWalk(std::string dataFolder, Rcpp::CharacterVector rproblem, int noSamples, std::string samplingStrat, long seed);
RcppExport SEXP _FlowshopSolveR_sampleRandomWalk(SEXP dataFolderSEXP, SEXP rproblemSEXP, SEXP noSamplesSEXP, SEXP samplingStratSEXP, SEXP seedSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< std::string >::type dataFolder(dataFolderSEXP);
    Rcpp::traits::input_parameter< Rcpp::CharacterVector >::type rproblem(rproblemSEXP);
    Rcpp::traits::input_parameter< int >::type noSamples(noSamplesSEXP);
    Rcpp::traits::input_parameter< std::string >::type samplingStrat(samplingStratSEXP);
    Rcpp::traits::input_parameter< long >::type seed(seedSEXP);
    rcpp_result_gen = Rcpp::wrap(sampleRandomWalk(dataFolder, rproblem, noSamples, samplingStrat, seed));
    return rcpp_result_gen;
END_RCPP
}
// adaptiveWalk
List adaptiveWalk(Rcpp::CharacterVector rproblem, Rcpp::CharacterVector rsampling, long seed);
RcppExport SEXP _FlowshopSolveR_adaptiveWalk(SEXP rproblemSEXP, SEXP rsamplingSEXP, SEXP seedSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< Rcpp::CharacterVector >::type rproblem(rproblemSEXP);
    Rcpp::traits::input_parameter< Rcpp::CharacterVector >::type rsampling(rsamplingSEXP);
    Rcpp::traits::input_parameter< long >::type seed(seedSEXP);
    rcpp_result_gen = Rcpp::wrap(adaptiveWalk(rproblem, rsampling, seed));
    return rcpp_result_gen;
END_RCPP
}
// adaptiveWalkLengthFLA
int adaptiveWalkLengthFLA(Rcpp::CharacterVector rproblem, Rcpp::CharacterVector rsampling, long seed);
RcppExport SEXP _FlowshopSolveR_adaptiveWalkLengthFLA(SEXP rproblemSEXP, SEXP rsamplingSEXP, SEXP seedSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< Rcpp::CharacterVector >::type rproblem(rproblemSEXP);
    Rcpp::traits::input_parameter< Rcpp::CharacterVector >::type rsampling(rsamplingSEXP);
    Rcpp::traits::input_parameter< long >::type seed(seedSEXP);
    rcpp_result_gen = Rcpp::wrap(adaptiveWalkLengthFLA(rproblem, rsampling, seed));
    return rcpp_result_gen;
END_RCPP
}
// enumerateAllFitness
std::vector<double> enumerateAllFitness(Rcpp::CharacterVector rproblem);
RcppExport SEXP _FlowshopSolveR_enumerateAllFitness(SEXP rproblemSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< Rcpp::CharacterVector >::type rproblem(rproblemSEXP);
    rcpp_result_gen = Rcpp::wrap(enumerateAllFitness(rproblem));
    return rcpp_result_gen;
END_RCPP
}
// enumerateSolutions
List enumerateSolutions(Rcpp::List fspInstance, Rcpp::CharacterVector fspProblem);
RcppExport SEXP _FlowshopSolveR_enumerateSolutions(SEXP fspInstanceSEXP, SEXP fspProblemSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< Rcpp::List >::type fspInstance(fspInstanceSEXP);
    Rcpp::traits::input_parameter< Rcpp::CharacterVector >::type fspProblem(fspProblemSEXP);
    rcpp_result_gen = Rcpp::wrap(enumerateSolutions(fspInstance, fspProblem));
    return rcpp_result_gen;
END_RCPP
}
// sampleLON
List sampleLON(std::string sampleType, Rcpp::CharacterVector rproblem, Rcpp::CharacterVector rsampling, long seed);
RcppExport SEXP _FlowshopSolveR_sampleLON(SEXP sampleTypeSEXP, SEXP rproblemSEXP, SEXP rsamplingSEXP, SEXP seedSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< std::string >::type sampleType(sampleTypeSEXP);
    Rcpp::traits::input_parameter< Rcpp::CharacterVector >::type rproblem(rproblemSEXP);
    Rcpp::traits::input_parameter< Rcpp::CharacterVector >::type rsampling(rsamplingSEXP);
    Rcpp::traits::input_parameter< long >::type seed(seedSEXP);
    rcpp_result_gen = Rcpp::wrap(sampleLON(sampleType, rproblem, rsampling, seed));
    return rcpp_result_gen;
END_RCPP
}

static const R_CallMethodDef CallEntries[] = {
    {"_FlowshopSolveR_cppEAJumpCost", (DL_FUNC) &_FlowshopSolveR_cppEAJumpCost, 4},
    {"_FlowshopSolveR_initFactories", (DL_FUNC) &_FlowshopSolveR_initFactories, 1},
    {"_FlowshopSolveR_solveFSP", (DL_FUNC) &_FlowshopSolveR_solveFSP, 5},
    {"_FlowshopSolveR_sampleSolutionStatisticsFLA", (DL_FUNC) &_FlowshopSolveR_sampleSolutionStatisticsFLA, 4},
    {"_FlowshopSolveR_sampleRandomWalk", (DL_FUNC) &_FlowshopSolveR_sampleRandomWalk, 5},
    {"_FlowshopSolveR_adaptiveWalk", (DL_FUNC) &_FlowshopSolveR_adaptiveWalk, 3},
    {"_FlowshopSolveR_adaptiveWalkLengthFLA", (DL_FUNC) &_FlowshopSolveR_adaptiveWalkLengthFLA, 3},
    {"_FlowshopSolveR_enumerateAllFitness", (DL_FUNC) &_FlowshopSolveR_enumerateAllFitness, 1},
    {"_FlowshopSolveR_enumerateSolutions", (DL_FUNC) &_FlowshopSolveR_enumerateSolutions, 2},
    {"_FlowshopSolveR_sampleLON", (DL_FUNC) &_FlowshopSolveR_sampleLON, 4},
    {NULL, NULL, 0}
};

RcppExport void R_init_FlowshopSolveR(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
