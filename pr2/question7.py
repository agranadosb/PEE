import os
import time
import shutil
import argparse
import logging

logging.basicConfig(
    format="%(asctime)s %(levelname)-8s %(message)s",
    level=logging.INFO,
    datefmt="%Y-%m-%d %H:%M:%S",
)

parser = argparse.ArgumentParser(description='Executes question 7 of PEE. This scripts generates a results file with the likelyhood, the number of triangles, the time per iteration and the iteration.')
parser.add_argument('dir', type=str, help='Base directory of PCFG')
parser.add_argument('-i', '--iterations', default=500, help='Iterations number for model generation')
parser.add_argument('-nt', '--nterminals', default=20, help='Number of non terminals')

args = parser.parse_args()

# Datos
base_dir = args.dir
non_terminals = args.nterminals
iterations = args.iterations
base_grammar_name = f'G-triangle-{non_terminals}'

def create_remove_folder(folder):
    if os.path.exists(folder):
        logging.info(f"Removing {folder}")
        shutil.rmtree(folder)
    logging.info(f"Creating {folder}")
    os.mkdir(folder)

folder_models = os.path.join(base_dir, 'results-question-7')
if not os.path.exists(folder_models):
    logging.info(f"Creating {folder_models}")
    os.mkdir(folder_models)

folder_data = os.path.join(base_dir, 'DATA')
result_folder = os.path.join(folder_models, f'result-{non_terminals}')
create_remove_folder(result_folder)

commands_file = os.path.join(result_folder, 'log-commands.txt')
if os.path.exists(commands_file):
    os.remove(commands_file)

result_file = os.path.join(folder_models, f"results-{non_terminals}.txt")

if os.path.exists(result_file):
  os.remove(result_file)

samples = 'SampleTriangle-10K'
samples_file = os.path.join(folder_data, samples)

learn_command = os.path.join(base_dir, 'scfg-toolkit/scfg_learn')
generate_first_grammar_command = os.path.join(base_dir, 'scfg-toolkit/scfg_cgr')
test_grammar_command = os.path.join(base_dir, 'scfg-toolkit/scfg_gstr')
check_triangle_command = os.path.join(base_dir, 'scfg-toolkit/checkTriangle')

model = 'Gt'
base_grammar = os.path.join(base_dir, f"grammars/{base_grammar_name}")
if not os.path.exists(base_grammar):
    raise ValueError(f"Grammar {base_grammar} not exists. Execute grammars.py script for generate it")
base_grammar_custom_non_terminals = os.path.join(result_folder, model)

# copy base grammar
command = f"{generate_first_grammar_command} -g {base_grammar} -f {base_grammar_custom_non_terminals}"
print(command)
with open(commands_file, 'a') as logs:
    print(command, file=logs)
    logging.info(f"Executing command: {command}")
stream = os.popen(command)
output = stream.read()

with open(result_file, "a") as myfile:
    myfile.write(f'{"likeliklyhood":>15s} {"time":>7s} {"words":>7s} {"iteration":>11s}\n')

for iteration in range(iterations):
    output_model = f'new-Gt-{iteration}'

    model_file = os.path.join(result_folder, model)
    output_model_file = os.path.join(result_folder, output_model)

    init = time.time()
    command = f"{learn_command} -g {model_file} -f {output_model_file} -i {1} -m {samples_file} -l"
    with open(commands_file, 'a') as logs:
        print(command, file=logs)
        logging.info(f"Executing command: {command}")
    stream = os.popen(command)
    log_liklyhood = stream.read()
    fin = time.time() - init

    command = f"{test_grammar_command} -g {output_model_file} -c 1000 > tri-test-{iteration}"
    with open(commands_file, 'a') as logs:
        print(command, file=logs)
        logging.info(f"Executing command: {command}")
    stream = os.popen(command)
    output = stream.read()

    command = f"awk -f {check_triangle_command} tri-test-{iteration} | grep Y | wc -l"
    with open(commands_file, 'a') as logs:
        print(command, file=logs)
        logging.info(f"Executing command: {command}")
    stream = os.popen(command)
    words = stream.read()
    os.remove(f"tri-test-{iteration}")

    likeliklyhood = float(log_liklyhood.rstrip())
    words = int(words.rstrip())
    result = f"{likeliklyhood:15.2f} {fin:7.2f} {words:7} {iteration:11}\n"
    with open(result_file, "a") as myfile:
        myfile.write(result)
    print(result)
    model = output_model
