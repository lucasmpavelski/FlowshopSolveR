#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

#include <paradiseo/eo/utils/eoParam.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

//#include "meta/Parameter.hpp"
//#include "meta/FSPProblem.hpp"
#include "flowshop-solver/heuristics/all.hpp"

#ifndef DATA_FOLDER
#define DATA_FOLDER "error"
#endif

#define on_error(...)             \
  {                               \
    fprintf(stderr, __VA_ARGS__); \
    fflush(stderr);               \
    exit(1);                      \
  }

auto main(int argc, char* argv[]) -> int {
  using namespace std::string_literals;
  std::ios::sync_with_stdio(true);

  eoParser parser(argc, argv);

  MHParamsSpecsFactory::init(DATA_FOLDER "/specs", true);
  FSPProblemFactory::init(DATA_FOLDER);

  auto addParam = [&parser](auto val, const std::string& desc) {
    return parser.createParam(val, desc, "").value();
  };

  auto seed = addParam(123l, "seed");

  std::unordered_map<std::string, std::string> prob_data;
  auto addProblemParam = [&prob_data, &addParam](const std::string& name,
                                                 const std::string& def) {
    prob_data[name] = addParam(def, name);
  };
  addProblemParam("problem", "FSP");
  addProblemParam("instance", "taill-like_rand_20_20_01.dat"s);
  addProblemParam("objective", "MAKESPAN"s);
  addProblemParam("type", "PERM"s);
  addProblemParam("stopping_criterium", "EVALS"s);
  addProblemParam("budget", "med"s);

  // std::cerr <<
  //     prob_data["budget"] << '\n' <<
  //     prob_data["instance"] << '\n' <<
  //     prob_data["objective"] << '\n' <<
  //     prob_data["type"] << '\n' <<
  //     prob_data["stopping_criterium"] << '\n';

  auto mh = addParam("IHC"s, "mh");
  auto mh_specs = MHParamsSpecsFactory::get(mh);

  std::unordered_map<std::string, double> mh_params;
  for (const auto& spec : mh_specs) {
    mh_params[spec->name] = addParam(1.0, spec->name);
  }

  RNG::seed(seed);

  try {
    // auto res = solveWith(mh, prob_data, mh_params);
    // std::cout << res.fitness << ' ' << res.no_evals << ' ' << res.time << '\n';
  } catch (std::exception& e) {
    std::cerr << e.what() << '\n';
    std::cout << "ERROR\n";
  }

  /*
      //cout << "SEED: " << RNG::seed() << "\n";

      std::string s =
              " --instance=../../data/instances/generated_intances/"
              "generated_instances_taill-like/"
              "taill-like_20_10_2010109.gen"
              " --obj=PERM "
              " --specs=../../data/specs/hc_specs.txt"
              " --budget=low"
              " --serve=1"
              " --HC.Algo=1"
              " --Comp.Strat=1"
              " --Init.Strat=2"
              " --Neighborhood.Size=3.141592"
              " --Neighborhood.Strat=1"
              ;
  //    std::stringstream test_input(s);
  //    parser.readFrom(test_input);



      serve = parser.createParam(serve, "serve", "Server mode", 's').value();


      FSPData data(instance);
      FSPProblem prob(data, obj, "MAKESPAN", budget, "EVALS");

      if (!serve) {
          specs_file = parser.createParam(specs_file, "specs", "Algorithm
  specs", 'A') .value(); std::ifstream specs(specs_file); MHParamsSpecs
  mh_specs; specs >> mh_specs; MHParamsValues values(&mh_specs);
          // get parameters from terminal
          for (auto ps : mh_specs) {
              std::string param_value = "0";
              param_value = parser.createParam(param_value, ps->name, "Parameter
  " + ps->name).value(); values[ps->name] = ps->fromStrValue(param_value);
          }
          std::cout << "params: " << values << '\n';
          std::cout << evaluate(values, prob, totalProcTimes(data));
      } else {
          MHParamsSpecsFactory::init(DATA_FOLDER"/specs");

          const short port = 0;
          const int BUFFER_SIZE = 1024;
          int server_fd, client_fd, err;
          sockaddr_in server, client;
          char buf[BUFFER_SIZE];

          server_fd = socket(AF_INET, SOCK_STREAM, 0);
          if (server_fd < 0) on_error("Could not create socket\n");

          server.sin_family = AF_INET;
          server.sin_port = htons(port);
          server.sin_addr.s_addr = htonl(INADDR_ANY);

          int opt_val = 1;
          setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof
  opt_val);

          err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
          if (err < 0) on_error("Could not bind socket\n");
          err = listen(server_fd, port);
          if (err < 0) on_error("Could not listen on socket\n");

          socklen_t len = sizeof(server);
          if (getsockname(server_fd, (struct sockaddr *)&server, &len) == -1) {
              on_error("getsockname");
          } else {
               printf("port: %5d\n", ntohs(server.sin_port));
          }
          cout << std::flush;

          socklen_t client_len = sizeof(client);
          client_fd = accept(server_fd, (struct sockaddr *) &client,
  &client_len); std::cerr << "accepted" << '\n' << std::flush;

          if (client_fd < 0) on_error("Could not establish new connection\n");

          while (1) {
              int read = recv(client_fd, buf, BUFFER_SIZE, 0);
              if (!read) break; // done reading
              if (read < 0) on_error("Client read failed\n");
              std::cerr << "received: " << buf << '\n' << std::flush;

              std::stringstream net_stream(buf);
              eoParser net_parser(argc, argv);
              net_parser.readFrom(net_stream);

              bool done = net_parser.createParam(false, "done", "stop
  all").value(); if (done) break;

              std::string mh_name = net_parser.createParam(string(), "mh",
  "metaheuristic name").value(); std::transform(mh_name.begin(), mh_name.end(),
  mh_name.begin(), ::toupper); MHParamsSpecs mh_specs =
  MHParamsSpecsFactory::get(mh_name);

              MHParamsValues values(&mh_specs);
              for (auto ps : mh_specs) {
                  std::string param_value = "0";
                  param_value = net_parser.createParam(param_value, ps->name,
  "").value(); values[ps->name] = ps->fromStrValue(param_value);
              }
              std::cerr << "params: " << values << '\n';
              double r = evaluate(values, prob, totalProcTimes(data));

              std::string response = std::to_string(r);
              err = send(client_fd, response.c_str(), response.size(), 0);
              if (err < 0) on_error("Client write failed\n");
          }
          shutdown(client_fd, SHUT_RDWR);
          close(server_fd);
      }*/
  return EXIT_SUCCESS;
}
