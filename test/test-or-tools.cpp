/*#include <cstdio>
#include <cstdlib>

#include "ortools/base/filelineiter.h"
#include "ortools/base/integral_types.h"
#include "ortools/base/logging.h"
#include "ortools/base/split.h"
#include "ortools/base/stringprintf.h"
#include "ortools/base/strtoint.h"

namespace operations_research {
// ----- JobShopData -----

// A JobShopData parses data files and stores all data internally for
// easy retrieval.
class JobShopData {
 public:
  // A task is the basic block of a jobshop.
  struct Task {
    Task(int j, int m, int d) : job_id(j), machine_id(m), duration(d) {}
    int job_id;
    int machine_id;
    int duration;
  };

  enum ProblemType { UNDEFINED, JSSP, TAILLARD };

  enum TaillardState {
    START,
    JOBS_READ,
    MACHINES_READ,
    SEED_READ,
    JOB_ID_READ,
    JOB_LENGTH_READ,
    JOB_READ
  };

  JobShopData()
      : name_(""),
        machine_count_(0),
        job_count_(0),
        horizon_(0),
        current_job_index_(0),
        problem_type_(UNDEFINED),
        taillard_state_(START) {}

  ~JobShopData() {}

  // Parses a file in jssp or taillard format and loads the model. See the flag
  // --data_file for a description of the format. Note that the format
  // is only partially checked: bad inputs might cause undefined
  // behavior.
  void Load(const std::string& filename) {
    for (const std::string& line : FileLines(filename)) {
      if (line.empty()) {
        continue;
      }
      ProcessNewLine(line);
    }
  }

  // The number of machines in the jobshop.
  int machine_count() const { return machine_count_; }

  // The number of jobs in the jobshop.
  int job_count() const { return job_count_; }

  // The name of the jobshop instance.
  const std::string& name() const { return name_; }

  // The horizon of the workshop (the sum of all durations), which is
  // a trivial upper bound of the optimal make_span.
  int horizon() const { return horizon_; }

  // Returns the tasks of a job, ordered by precedence.
  const std::vector<Task>& TasksOfJob(int job_id) const {
    return all_tasks_[job_id];
  }

 private:
  void ProcessNewLine(const std::string& line) {
    // TODO(user): more robust logic to support single-task jobs.
    const std::vector<std::string> words =
        absl::StrSplit(line, ' ', absl::SkipEmpty());
    switch (problem_type_) {
      case UNDEFINED: {
        if (words.size() == 2 && words[0] == "instance") {
          problem_type_ = JSSP;
          LOG(INFO) << "Reading jssp instance " << words[1];
          name_ = words[1];
        } else if (words.size() == 1 && atoi32(words[0]) > 0) {
          problem_type_ = TAILLARD;
          taillard_state_ = JOBS_READ;
          job_count_ = atoi32(words[0]);
          CHECK_GT(job_count_, 0);
          all_tasks_.resize(job_count_);
        }
        break;
      }
      case JSSP: {
        if (words.size() == 2) {
          job_count_ = atoi32(words[0]);
          machine_count_ = atoi32(words[1]);
          CHECK_GT(machine_count_, 0);
          CHECK_GT(job_count_, 0);
          LOG(INFO) << machine_count_ << " machines and " << job_count_
                    << " jobs";
          all_tasks_.resize(job_count_);
        }

        if (words.size() > 2 && machine_count_ != 0) {
          CHECK_EQ(words.size(), machine_count_ * 2);
          for (int i = 0; i < machine_count_; ++i) {
            const int machine_id = atoi32(words[2 * i]);
            const int duration = atoi32(words[2 * i + 1]);
            AddTask(current_job_index_, machine_id, duration);
          }
          current_job_index_++;
        }
        break;
      }
      case TAILLARD: {
        switch (taillard_state_) {
          case START: {
            LOG(FATAL) << "Should not be here";
            break;
          }
          case JOBS_READ: {
            CHECK_EQ(1, words.size());
            machine_count_ = atoi32(words[0]);
            CHECK_GT(machine_count_, 0);
            taillard_state_ = MACHINES_READ;
            break;
          }
          case MACHINES_READ: {
            CHECK_EQ(1, words.size());
            const int seed = atoi32(words[0]);
            LOG(INFO) << "Taillard instance with " << job_count_
                      << " jobs, and " << machine_count_
                      << " machines, generated with a seed of " << seed;
            taillard_state_ = SEED_READ;
            break;
          }
          case SEED_READ:
          case JOB_READ: {
            CHECK_EQ(1, words.size());
            current_job_index_ = atoi32(words[0]);
            taillard_state_ = JOB_ID_READ;
            break;
          }
          case JOB_ID_READ: {
            CHECK_EQ(1, words.size());
            taillard_state_ = JOB_LENGTH_READ;
            break;
          }
          case JOB_LENGTH_READ: {
            CHECK_EQ(machine_count_, words.size());
            for (int i = 0; i < machine_count_; ++i) {
              const int duration = atoi32(words[i]);
              AddTask(current_job_index_, i, duration);
            }
            taillard_state_ = JOB_READ;
            break;
          }
        }
        break;
      }
    }
  }

  void AddTask(int job_id, int machine_id, int duration) {
    all_tasks_[job_id].push_back(Task(job_id, machine_id, duration));
    horizon_ += duration;
  }

  std::string name_;
  int machine_count_;
  int job_count_;
  int horizon_;
  std::vector<std::vector<Task> > all_tasks_;
  int current_job_index_;
  ProblemType problem_type_;
  TaillardState taillard_state_;
};
}  // namespace operations_research

#include <cstdio>
#include <cstdlib>

#include "ortools/base/commandlineflags.h"
#include "ortools/base/integral_types.h"
#include "ortools/base/join.h"
#include "ortools/base/logging.h"
#include "ortools/base/stringprintf.h"
#include "ortools/constraint_solver/constraint_solver.h"

DEFINE_string(
    data_file, "",
    "Required: input file description the scheduling problem to solve, "
    "in our jssp format:\n"
    "  - the first line is \"instance <instance name>\"\n"
    "  - the second line is \"<number of jobs> <number of machines>\"\n"
    "  - then one line per job, with a single space-separated "
    "list of \"<machine index> <duration>\"\n"
    "note: jobs with one task are not supported");
DEFINE_int32(time_limit_in_ms, 0, "Time limit in ms, 0 means no limit.");

namespace operations_research {
void Jobshop(const JobShopData& data) {
  Solver solver("jobshop");
  const int machine_count = data.machine_count();
  const int job_count = data.job_count();
  const int horizon = data.horizon();

  // ----- Creates all Intervals and vars -----

  // Stores all tasks attached interval variables per job.
  std::vector<std::vector<IntervalVar*> > jobs_to_tasks(job_count);
  // machines_to_tasks stores the same interval variables as above, but
  // grouped my machines instead of grouped by jobs.
  std::vector<std::vector<IntervalVar*> > machines_to_tasks(machine_count);

  // Creates all individual interval variables.
  for (int job_id = 0; job_id < job_count; ++job_id) {
    const std::vector<JobShopData::Task>& tasks = data.TasksOfJob(job_id);
    for (int task_index = 0; task_index < tasks.size(); ++task_index) {
      const JobShopData::Task& task = tasks[task_index];
      CHECK_EQ(job_id, task.job_id);
      const std::string name =
          StringPrintf("J%dM%dI%dD%d", task.job_id, task.machine_id, task_index,
                       task.duration);
      IntervalVar* const one_task = solver.MakeFixedDurationIntervalVar(
          0, horizon, task.duration, false, name);
      jobs_to_tasks[task.job_id].push_back(one_task);
      machines_to_tasks[task.machine_id].push_back(one_task);
    }
  }

  // ----- Creates model -----

  // Creates precedences inside jobs.
  for (int job_id = 0; job_id < job_count; ++job_id) {
    const int task_count = jobs_to_tasks[job_id].size();
    for (int task_index = 0; task_index < task_count - 1; ++task_index) {
      IntervalVar* const t1 = jobs_to_tasks[job_id][task_index];
      IntervalVar* const t2 = jobs_to_tasks[job_id][task_index + 1];
      Constraint* const prec =
          solver.MakeIntervalVarRelation(t2, Solver::STARTS_AFTER_END, t1);
      solver.AddConstraint(prec);
    }
  }

  // Adds disjunctive constraints on unary resources, and creates
  // sequence variables. A sequence variable is a dedicated variable
  // whose job is to sequence interval variables.
  std::vector<SequenceVar*> all_sequences;
  for (int machine_id = 0; machine_id < machine_count; ++machine_id) {
    const std::string name = StringPrintf("Machine_%d", machine_id);
    DisjunctiveConstraint* const ct =
        solver.MakeDisjunctiveConstraint(machines_to_tasks[machine_id], name);
    solver.AddConstraint(ct);
    all_sequences.push_back(ct->MakeSequenceVar());
  }

  // Creates array of end_times of jobs.
  std::vector<IntVar*> all_ends;
  for (int job_id = 0; job_id < job_count; ++job_id) {
    const int task_count = jobs_to_tasks[job_id].size();
    IntervalVar* const task = jobs_to_tasks[job_id][task_count - 1];
    all_ends.push_back(task->EndExpr()->Var());
  }

  // Objective: minimize the makespan (maximum end times of all tasks)
  // of the problem.
  IntVar* const objective_var = solver.MakeMax(all_ends)->Var();
  OptimizeVar* const objective_monitor = solver.MakeMinimize(objective_var, 1);

  // ----- Search monitors and decision builder -----

  // This decision builder will rank all tasks on all machines.
  DecisionBuilder* const sequence_phase =
      solver.MakePhase(all_sequences, Solver::SEQUENCE_DEFAULT);

  // After the ranking of tasks, the schedule is still loose and any
  // task can be postponed at will. But, because the problem is now a PERT
  // (http://en.wikipedia.org/wiki/Program_Evaluation_and_Review_Technique),
  // we can schedule each task at its earliest start time. This is
  // conveniently done by fixing the objective variable to its
  // minimum value.
  DecisionBuilder* const obj_phase = solver.MakePhase(
      objective_var, Solver::CHOOSE_FIRST_UNBOUND, Solver::ASSIGN_MIN_VALUE);

  // The main decision builder (ranks all tasks, then fixes the
  // objective_variable).
  DecisionBuilder* const main_phase = solver.Compose(sequence_phase, obj_phase);

  // Search log.
  const int kLogFrequency = 1000000;
  SearchMonitor* const search_log =
      solver.MakeSearchLog(kLogFrequency, objective_monitor);

  SearchLimit* limit = nullptr;
  if (FLAGS_time_limit_in_ms > 0) {
    limit = solver.MakeTimeLimit(FLAGS_time_limit_in_ms);
  }

  SolutionCollector* const collector = solver.MakeLastSolutionCollector();
  collector->Add(all_sequences);

  // Search.
  if (solver.Solve(main_phase, search_log, objective_monitor, limit,
                   collector)) {
    for (int m = 0; m < machine_count; ++m) {
      SequenceVar* const seq = all_sequences[m];
      LOG(INFO) << seq->name() << ": "
                << absl::StrJoin(collector->ForwardSequence(0, seq), ", ");
    }
  }
}
}  // namespace operations_research

static const char kUsage[] =
    "Usage: see flags.\nThis program runs a simple job shop optimization "
    "output besides the debug LOGs of the solver.";

int main(int argc, char** argv) {
  gflags::SetUsageMessage(kUsage);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  if (FLAGS_data_file.empty()) {
    LOG(FATAL) << "Please supply a data file with --data_file=";
  }
  operations_research::JobShopData data;
  data.Load(FLAGS_data_file);
  operations_research::Jobshop(data);
  return 0;
}
*/
