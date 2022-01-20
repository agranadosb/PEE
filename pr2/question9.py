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

parser = argparse.ArgumentParser(description='Executes question 9 of PEE.')
parser.add_argument('dir', type=str, help='Base directory of PCFG')
parser.add_argument('-i', '--iterations', default=100, help='Iterations number for model generation')
parser.add_argument('-tl','--trainl', nargs='+', type=int, help='List of training length', default=[1000, 5000, 10000, 20000])

args = parser.parse_args()

# Datos
base_dir = args.dir
models = ['', '-v']
iterations = args.iterations
train_lengths = args.trainl

learn_command = os.path.join(base_dir, 'scfg-toolkit/scfg_learn')
prob_command = os.path.join(base_dir, 'scfg-toolkit/scfg_prob')
confus_command = os.path.join(base_dir, 'scfg-toolkit/confus')
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
logging.info(f"{'Iterations':15s}: {iterations}")
logging.info(f"List of lengths: {train_lengths}")

base_folder = os.path.join(base_dir, 'results-question-9')
create_remove_folder(base_folder)

commands_file = os.path.join(base_folder, 'log-commands.txt')
if os.path.exists(commands_file):
    os.remove(commands_file)

initial_grammar_file = os.path.join(base_dir, 'MODELS/G-0')

if not os.path.exists(initial_grammar_file):
    raise ValueError("Inital grammar dont't exists or is not named G-0. THe name of the inital grammar has to be 'G-0'")

test_right_data = os.path.join(base_dir, f'datasets/Ts-right-b')
test_isoc_data = os.path.join(base_dir, f'datasets/Ts-isoc-b')
test_equil_data = os.path.join(base_dir, f'datasets/Ts-equil-b')

if not (os.path.exists(test_right_data) and os.path.exists(test_isoc_data) and os.path.exists(test_equil_data)):
    raise ValueError("Test data not found, execute datasets.py script to create the datasets")

logging.info("Creating datasets")
for length in train_lengths:
    length_folder = os.path.join(base_folder, f'results-{length}')
    create_remove_folder(length_folder)

    training_right_data = os.path.join(length_folder, f'Tr-right-{length}')
    training_equil_data = os.path.join(length_folder, f'Tr-equil-{length}')
    training_isoc_data = os.path.join(length_folder, f'Tr-isoc-{length}')

    processes = []
    command = f"{dataset_creation_command} -F 0 -c {length} -l 2 -L 10 > {training_right_data}"
    processes.append(create_process(command, commands_file, f'Creating right dataset with {length}'))
    logging.debug(command)
    command = f"{dataset_creation_command} -F 1 -c {length} -l 2 -L 10 > {training_equil_data}"
    processes.append(create_process(command, commands_file, f'Creating equi dataset with {length}'))
    logging.debug(command)
    command = f"{dataset_creation_command} -F 2 -c {length} -l 2 -L 10 > {training_isoc_data}"
    processes.append(create_process(command, commands_file, f'Creating isoc dataset with {length}'))
    logging.debug(command)
    launch_jobs(processes)

    assert os.path.exists(training_right_data)
    assert os.path.exists(training_isoc_data)
    assert os.path.exists(training_equil_data)

    for model in models:
        out_right_model = os.path.join(length_folder, f'right-b')
        out_isoc_model = os.path.join(length_folder, f'isoc-b')
        out_equil_model = os.path.join(length_folder, f'equil-b')

        out_prob_right_model = os.path.join(length_folder, f'r-b')
        out_prob_isoc_model = os.path.join(length_folder, f'i-b')
        out_prob_equil_model = os.path.join(length_folder, f'e-b')

        results_file = os.path.join(length_folder, f'results{model}-b.txt')
        if os.path.exists(results_file):
            os.remove(results_file)
        results_confus_file = os.path.join(length_folder, f'confus-results{model}-b.txt')

        def create_line_confus(test_model, name, awk):
            # classify with the trained models and get results in right
            command = f"{prob_command} -g {out_right_model} -m {test_model} > {out_prob_right_model}"
            p1 = Process(target=execute, args=(command, commands_file,), name=f'Getting probabilities of right model using {name} test corpus')
            command = f"{prob_command} -g {out_isoc_model} -m {test_model} > {out_prob_isoc_model}"
            p2 = Process(target=execute, args=(command, commands_file,), name=f'Getting probabilities of isoc model using {name} test corpus')
            command = f"{prob_command} -g {out_equil_model} -m {test_model} > {out_prob_equil_model}"
            p3 = Process(target=execute, args=(command, commands_file,), name=f'Getting probabilities of equil model using {name} test corpus')

            launch_jobs([p1, p2, p3])

            command = "paste {} {} {} | awk '{}' >> {}".format(
                out_prob_right_model,
                out_prob_isoc_model,
                out_prob_equil_model,
                awk,
                results_file
            )
            execute(command, commands_file)

        # train models
        command = f"{learn_command} -g {initial_grammar_file} -f {out_right_model} -i {iterations} -m {training_right_data} -l {model}"
        p1 = Process(target=execute, args=(command, commands_file,), name='Learning right model')
        command = f"{learn_command} -g {initial_grammar_file} -f {out_isoc_model} -i {iterations} -m {training_isoc_data} -l {model}"
        p2 = Process(target=execute, args=(command, commands_file,), name='Learning isoc model')
        command = f"{learn_command} -g {initial_grammar_file} -f {out_equil_model} -i {iterations} -m {training_equil_data} -l {model}"
        p3 = Process(target=execute, args=(command, commands_file,), name='Learning equil model')

        launch_jobs([p1, p2, p3])

        create_line_confus(test_right_data, 'right', r'{m=$1;argm="right"; if ($2>m) {m=$2;argm="equil";} if ($3>m) {m=$3;argm="isosc";}printf("right %s\n",argm);}')
        create_line_confus(test_equil_data, 'equil', r'{m=$2;argm="equil"; if ($1>m) {m=$1;argm="right";} if ($3>m) {m=$3;argm="isosc";} printf("equil %s\n",argm);}')
        create_line_confus(test_isoc_data, 'isoc', r'{m=$3;argm="isosc"; if ($1>m) {m=$1;argm="right";} if ($2>m) {m=$2;argm="equil";} printf("isosc %s\n",argm);}')

        command = f"cat {results_file} | {confus_command}"
        confus_output = execute(command, commands_file)
        with open(results_confus_file, 'w') as result_confus:
            result_confus.write(confus_output)