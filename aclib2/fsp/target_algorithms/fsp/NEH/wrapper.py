#!/usr/bin/env python3
# encoding: utf-8

import json
import traceback
import os
import sys

from genericWrapper4AC.generic_wrapper import AbstractWrapper

'''
    Example call:

python3 \
aclib2/neh_FSP/target_algorithms/fsp/neh/wrapper.py \
--seed=559369527 \
--instance FSP,MAKESPAN,NOWAIT,FIXEDTIME_15,med,erlang_rand_20_40_30.dat \
--config \
-NEH.Init neh \
-NEH.Init.NEH.Priority ra_c1 \
-NEH.Init.NEH.PriorityOrder hi_hilo \
-NEH.Init.NEH.PriorityWeighted 1 \
-NEH.Init.NEH.Insertion first_best
'''
class MHWrapper(AbstractWrapper):
    '''
        Dummy wrapper for unit testing
    '''
    
    def __init__(self):
        AbstractWrapper.__init__(self)
        self._return_value = None
        self.__script_dir = os.path.abspath(os.path.split(__file__)[0])
        self.mh_name = 'NEH'
    
    def get_command_line_args(self, runargs, config):
        cmd = '/_install/main/fsp_solver --printLastFitness --data_folder=/data --mh=%s --seed=%d ' % (self.mh_name, runargs['seed'])
        inst_splits = runargs['instance'].split('/')[-1].split(',')
        prob_data = {
            'problem': inst_splits[0],
            'objective': inst_splits[1],
            'type': inst_splits[2],
            'stopping_criterion': inst_splits[3],
            'budget': inst_splits[4],
            'instance': inst_splits[5]
        }
        for key, value in prob_data.items() :
            cmd += '--%s=%s ' %(key, value)
        for key, value in config.items():
            cmd += '-%s=%s ' %(key.replace('_', '.'), value)
        return cmd
    
    def process_results(self, filepointer, exit_code):
        '''
        Parse a results file to extract the run's status (SUCCESS/CRASHED/etc) and other optional results.
    
        Args:
            filepointer: a pointer to the file containing the solver execution standard out.
            exit_code : exit code of target algorithm
        '''

        statuses = ['SUCCESS', 'TIMEOUT', 'CRASHED', 'ABORT']

        
        # If something fails, we a serious problem
        output = dict(status='ABORT')
        for line in filepointer:
            try:
                out = str(line.decode('UTF-8')).replace('\n','').split(',')
                output = dict(status='SUCCESS', cost=float(out[0]), quality=float(out[0]), runtime=float(out[0]))
            except ValueError:
                traceback.print_exc()
                pass

        return output

        
if __name__ == '__main__':
    wrapper = MHWrapper()
    wrapper.main()