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
parser.add_argument('-n','--nterminals', nargs='+', type=int, help='List of non terminals for each grammar', default=[5, 10, 15, 20])

args = parser.parse_args()

# Datos
base_dir = args.dir
nterminals = args.nterminals

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
logging.info(f"{'Non terminals':15s}: {nterminals}")

base_folder = os.path.join(base_dir, 'grammars')
create_remove_folder(base_folder)

base_grammar = """
NonTerminals {}
{}

Terminals 3
b
g
d

Rules

"""

for non_terminals in nterminals:
    non_terminals_str = ''
    current_char = ord('A')
    for i in range(non_terminals):
        non_terminals_str += f"{chr(current_char)}\n"
        current_char += 1
    new_grammar = base_grammar.format(non_terminals, non_terminals_str)
    
    grammar_file = os.path.join(base_folder, f'G-triangle-{non_terminals}')
    with open(grammar_file, 'w') as gf:
        gf.write(new_grammar)
