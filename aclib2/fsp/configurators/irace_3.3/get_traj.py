#!/usr/bin/env python3

import json
import argparse
import csv


def extract_irace_traj(fn_in:str, fn_pcs_irace:str, fn_out:str="traj_irace"):
    '''
        reads irace stdout log file and extracts the trajectory info
        
        Arguments
        ---------
            fn_in: str
                filename of irace log
            fn_out: str:
                filename of trajectory file to be written
    '''
    
    header = ["CPU Time Used","Estimated Training Performance","Wallclock Time","Incumbent ID","Automatic Configurator (CPU) Time","Configuration..."]
    
    trajectory_wc = []
    trajectory_n_eva = []
    id = 1
    cost = None
    wc_time = None
    target_time = None
    config = None
    name_mapping = {}
    with open(fn_pcs_irace) as fp:
        # skip first line
        fp.readline()
        for line in fp:
            splits = line.split(" ")
            irace_name = splits[0]
            original_name = splits[1].strip("\"").strip()
            name_mapping[irace_name] = original_name

    with open(fn_in) as fp:
        while True:
            try:
                line = next(fp)
            except StopIteration:
                break

            if line.startswith("# wctimeUsed:"):
                wc_time = float(line.split(":")[-1].strip(" ").replace("\n", ""))

            if line.startswith("# timeUsed:"):
                target_time = float(line.split(":")[-1].strip(" ").replace("\n", ""))

            if line.startswith("Best-so-far configuration:"):
                cost = float(line.split(":")[-1].strip(" ").replace("\n",""))

            if line.startswith("Description of the best-so-far configuration:"):
                params = list(filter(lambda x: x != "", next(fp).replace("\n", "").split(" ")))[
                         2:-1]  # ignore id, real_target_runner
                print('params', params)
                config = list(filter(lambda x: x != "", next(fp).replace("\n", "").split(" ")))[2:-1]
                config = ["%s='%s'" % (name_mapping[k].replace('-', ''), v.strip()) for k, v in zip(params, config) if v != "<NA>" and v != "NA"]

                if wc_time is not None and cost is not None:
                    add_to_traj = [0, cost, wc_time, id, 0]
                    add_to_traj.extend(config)
                    trajectory_wc.append(add_to_traj)
                    cost = None
                    wc_time = None
                    config = None
                    target_time = None
                    id += 1
                else:
                    print("WARNING: Could not fully parse irace stdout (%s, %s, %s)" %(wc_time, cost, str(config)))


    with open(fn_out+"_wc.csv", "w") as fp:
        cw = csv.writer(fp)
        cw.writerow(header)
        for entry in trajectory_wc:
            cw.writerow(entry)

if __name__ == '__main__':
    
    argparse = argparse.ArgumentParser()
    argparse.add_argument("-f","--file", required=True, help="output log file of irace (stdout)")
    argparse.add_argument("-p", "--irace_pcs", default="paramfile.irace", help="irace pcs file")
    
    args = argparse.parse_args()
    
    extract_irace_traj(fn_in=args.file, fn_pcs_irace=args.irace_pcs)

