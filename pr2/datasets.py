import os
import time
import shutil
from multiprocessing import Process
import logging
import argparse

logging.basicConfig(
    format="%(asctime)s %(levelname)-8s %(message)s",
    level=logging.DEBUG,
    datefmt="%Y-%m-%d %H:%M:%S",
)

parser = argparse.ArgumentParser(description='Creates datasets for question 9 for PEE.')
parser.add_argument('dir', type=str, help='Base directory of PCFG')
parser.add_argument('-l', '--length', default=1000, help='Length of the dataset')

args = parser.parse_args()

# Datos
base_dir = args.dir
length = args.length
dataset_creation_command = os.path.join(base_dir, 'scfg-toolkit/genFig')

def execute(command: str, log_file: str) -> str:
    with open(log_file, 'a') as logs:
        print(command, file=logs)
        logging.info(f"Executing command: {command}")
    stream = os.popen(command)
    output = stream.read()
    return output

def launch_jobs(processes: list[Process]):
    for i in processes:
        i.start()
    for i in processes:
        init = time.time()
        while i.is_alive():
            logging.info(f'Waiting to {i.name} during {time.time() - init:10.2f} seconds')
            i.join(timeout=120)
        logging.info(f'Process {i.name} finished in {time.time() - init:10.2f} seconds')

def create_process(command: str, file: str, name: str) -> Process:
    return Process(target=execute, args=(command, file), name=name)

def create_remove_folder(folder):
    if os.path.exists(folder):
        logging.info(f"Removing {folder}")
        shutil.rmtree(folder)
    logging.info(f"Creating {folder}")
    os.mkdir(folder)

logging.info(f"{'Base directory':15s}: {base_dir}")

base_folder = os.path.join(base_dir, 'datasets')
create_remove_folder(base_folder)

logging.info("Creating datasets")
training_right_data_b = os.path.join(base_folder, f'Tr-right-b')
training_equil_data_b = os.path.join(base_folder, f'Tr-equil-b')
training_isoc_data_b = os.path.join(base_folder, f'Tr-isoc-b')

test_right_data_b = os.path.join(base_folder, f'Ts-right-b')
test_equil_data_b = os.path.join(base_folder, f'Ts-equil-b')
test_isoc_data_b = os.path.join(base_folder, f'Ts-isoc-b')

training_right_data = os.path.join(base_folder, f'Tr-right')
training_equil_data = os.path.join(base_folder, f'Tr-equil')
training_isoc_data = os.path.join(base_folder, f'Tr-isoc')

test_right_data = os.path.join(base_folder, f'Ts-right')
test_equil_data = os.path.join(base_folder, f'Ts-equil')
test_isoc_data = os.path.join(base_folder, f'Ts-isoc')

datasets_b = [
    (training_right_data_b, test_right_data_b),
    (training_equil_data_b, test_equil_data_b),
    (training_isoc_data_b, test_isoc_data_b),
]

datasets = [
    (training_right_data, test_right_data),
    (training_equil_data, test_equil_data),
    (training_isoc_data, test_isoc_data),
]

commands_file = os.path.join(base_folder, 'log-commands.txt')

for tr, ts in datasets:
    processes = []
    command = f"{dataset_creation_command} -F 0 -c {length} -l 2 -L 10 > {tr}"
    processes.append(create_process(command, commands_file, f'Creating right dataset with {length}'))
    logging.debug(command)
    command = f"{dataset_creation_command} -F 1 -c {length} -l 2 -L 10 > {ts}"
    processes.append(create_process(command, commands_file, f'Creating equi dataset with {length}'))
    logging.debug(command)
    launch_jobs(processes)

    with open(tr, 'r') as tr_file, open(ts, 'r') as ts_file:
        tr_data = tr_file.read()
        ts_data = ts_file.read()
    
    tr_data = tr_data.replace('[ ', '').replace(' ]', '')
    ts_data = ts_data.replace('[ ', '').replace(' ]', '')

    with open(tr, 'w') as tr_file, open(ts, 'w') as ts_file:
        tr_file.write(tr_data)
        ts_file.write(ts_data)

for tr, ts in datasets_b:
    processes = []
    command = f"{dataset_creation_command} -F 0 -c {length} -l 2 -L 10 > {tr}"
    processes.append(create_process(command, commands_file, f'Creating right dataset with {length}'))
    logging.debug(command)
    command = f"{dataset_creation_command} -F 1 -c {length} -l 2 -L 10 > {ts}"
    processes.append(create_process(command, commands_file, f'Creating equi dataset with {length}'))
    logging.debug(command)
    launch_jobs(processes)
