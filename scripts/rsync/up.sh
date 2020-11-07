#!/bin/bash

rsync -avph --exclude-from='scripts/rsync/rsync-exclude.txt' --progress . lucasmp@linode2:dev/FlowshopSolveR
