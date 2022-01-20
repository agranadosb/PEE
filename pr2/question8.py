import os
import sys
import time
import shutil
from multiprocessing import Process
import logging
import argparse

logging.basicConfig(
    format="%(asctime)s %(levelname)-8s %(message)s",
    level=logging.INFO,
    datefmt="%Y-%m-%d %H:%M:%S",
)

def execute(command: str) -> str:
    with open('log-commands.txt', 'a') as logs:
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

parser = argparse.ArgumentParser(description='Executes question 8 of PEE. This script generates 4 results files, on per execution. The results will be stores in dir/triangles-algorithms/results[algorithm][bracketed]')
parser.add_argument('dir', type=str, help='Base directory of PCFG')
parser.add_argument('-i', '--iterations', default=100, help='Iterations number for model generation')

args = parser.parse_args()

# Datos
base_dir = args.dir
models = ['', '-v']
bracketed = ['', '-b']
iterations = args.iterations

# Tiene que haber una carpeta llamada custom-models
folder_models = os.path.join(base_dir, 'results-question-8')

# copy base grammar
if os.path.exists(folder_models):
    print('Restart model? The folder results-question-8 will be removed')
    print('Press y/Y for restart')
    restart = input()
    if not restart or restart.lower() != 'y':
        sys.exit(0)
    shutil.rmtree(folder_models)
os.mkdir(folder_models)

if os.path.exists('log-commands.txt'):
    os.remove('log-commands.txt')

learn_command = os.path.join(base_dir, 'scfg-toolkit/scfg_learn')
prob_command = os.path.join(base_dir, 'scfg-toolkit/scfg_prob')
confus_command = os.path.join(base_dir, 'scfg-toolkit/confus')
initial_grammar_file = os.path.join(base_dir, 'MODELS/G-0')

if not os.path.exists(initial_grammar_file):
    raise ValueError("Inital grammar dont't exists or is not named G-0. THe name of the inital grammar has to be 'G-0'")

for model in models:
    for brackets in bracketed:
        training_right_data = os.path.join(base_dir, f'datasets/Tr-right{brackets}')
        training_isoc_data = os.path.join(base_dir, f'datasets/Tr-isoc{brackets}')
        training_equil_data = os.path.join(base_dir, f'datasets/Tr-equil{brackets}')

        if not (os.path.exists(training_right_data) and os.path.exists(training_isoc_data) and os.path.exists(training_equil_data)):
            raise ValueError("Training data not found, execute datasets.py script to create the datasets")

        test_right_data = os.path.join(base_dir, f'datasets/Ts-right{brackets}')
        test_isoc_data = os.path.join(base_dir, f'datasets/Ts-isoc{brackets}')
        test_equil_data = os.path.join(base_dir, f'datasets/Ts-equil{brackets}')

        if not (os.path.exists(test_right_data) and os.path.exists(test_isoc_data) and os.path.exists(test_equil_data)):
            raise ValueError("Test data not found, execute datasets.py script to create the datasets")

        folder_results = os.path.join(folder_models, f'results{model}{brackets}')
        if os.path.exists(folder_results):
            shutil.rmtree(folder_results)
        os.mkdir(folder_results)

        out_right_model = os.path.join(folder_results, f'right{brackets}')
        out_isoc_model = os.path.join(folder_results, f'isoc{brackets}')
        out_equil_model = os.path.join(folder_results, f'equil{brackets}')

        out_prob_right_model = os.path.join(folder_results, f'r{brackets}')
        out_prob_isoc_model = os.path.join(folder_results, f'i{brackets}')
        out_prob_equil_model = os.path.join(folder_results, f'e{brackets}')

        results_file = os.path.join(folder_results, f'results{model}{brackets}.txt')
        if os.path.exists(results_file):
            os.remove(results_file)
        results_confus_file = os.path.join(folder_results, f'confus-results{model}{brackets}.txt')

        def create_line_confus(test_model, name, awk):
            # classify with the trained models and get results in right
            command = f"{prob_command} -g {out_right_model} -m {test_model} > {out_prob_right_model}"
            p1 = Process(target=execute, args=(command,), name=f'Getting probabilities of right model using {name} test corpus')
            command = f"{prob_command} -g {out_isoc_model} -m {test_model} > {out_prob_isoc_model}"
            p2 = Process(target=execute, args=(command,), name=f'Getting probabilities of isoc model using {name} test corpus')
            command = f"{prob_command} -g {out_equil_model} -m {test_model} > {out_prob_equil_model}"
            p3 = Process(target=execute, args=(command,), name=f'Getting probabilities of equil model using {name} test corpus')

            launch_jobs([p1, p2, p3])

            command = "paste {} {} {} | awk '{}' >> {}".format(
                out_prob_right_model,
                out_prob_isoc_model,
                out_prob_equil_model,
                awk,
                results_file
            )
            execute(command)

        # train models
        command = f"{learn_command} -g {initial_grammar_file} -f {out_right_model} -i {iterations} -m {training_right_data} -l {model}"
        p1 = Process(target=execute, args=(command,), name='Learning right model')
        command = f"{learn_command} -g {initial_grammar_file} -f {out_isoc_model} -i {iterations} -m {training_isoc_data} -l {model}"
        p2 = Process(target=execute, args=(command,), name='Learning isoc model')
        command = f"{learn_command} -g {initial_grammar_file} -f {out_equil_model} -i {iterations} -m {training_equil_data} -l {model}"
        p3 = Process(target=execute, args=(command,), name='Learning equil model')

        launch_jobs([p1, p2, p3])

        create_line_confus(test_right_data, 'right', r'{m=$1;argm="right"; if ($2>m) {m=$2;argm="equil";} if ($3>m) {m=$3;argm="isosc";}printf("right %s\n",argm);}')
        create_line_confus(test_equil_data, 'equil', r'{m=$2;argm="equil"; if ($1>m) {m=$1;argm="right";} if ($3>m) {m=$3;argm="isosc";} printf("equil %s\n",argm);}')
        create_line_confus(test_equil_data, 'isoc', r'{m=$3;argm="isosc"; if ($1>m) {m=$1;argm="right";} if ($2>m) {m=$2;argm="equil";} printf("isosc %s\n",argm);}')

        command = f"cat {results_file} | {confus_command}"
        confus_output = execute(command)
        with open(results_confus_file, 'w') as result_confus:
            result_confus.write(confus_output)
