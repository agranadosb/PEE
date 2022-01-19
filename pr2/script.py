import os
import time
import shutil

# Datos
base_dir = 'PCFGs'
non_terminals = 20
iterations = 500

# Tiene que haber una carpeta llamada custom-models
folder_models = os.path.join(base_dir, 'custom-models')

folder_base_models = os.path.join(base_dir, 'MODELS')
folder_data = os.path.join(base_dir, 'DATA')
result_folder = os.path.join(folder_models, f'result-{non_terminals}')

result_file = os.path.join(result_folder, f"results-{non_terminals}.txt")

if os.path.exists(result_file):
  os.remove(result_file)

samples = 'SampleTriangle-10K'
samples_file = os.path.join(folder_data, samples)

learn_command = os.path.join(base_dir, 'scfg-toolkit/scfg_learn')
generate_first_grammar_command = os.path.join(base_dir, 'scfg-toolkit/scfg_cgr')
test_grammar_command = os.path.join(base_dir, 'scfg-toolkit/scfg_gstr')
check_triangle_command = os.path.join(base_dir, 'scfg-toolkit/checkTriangle')

model = 'Gt'
base_grammar = os.path.join(folder_models, f'G-triangle-{non_terminals}')
base_grammar_custom_non_terminals = os.path.join(result_folder, model)

# copy base grammar
if os.path.exists(result_folder):
    shutil.rmtree(result_folder)
os.mkdir(result_folder)

command = f"{generate_first_grammar_command} -g {base_grammar} -f {base_grammar_custom_non_terminals}"
print(command)
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
    print(command)
    stream = os.popen(command)
    log_liklyhood = stream.read()
    fin = time.time() - init

    command = f"{test_grammar_command} -g {output_model_file} -c 1000 > tri-test-{iteration}"
    stream = os.popen(command)
    output = stream.read()
    command = f"awk -f {check_triangle_command} tri-test-{iteration} | grep Y | wc -l"
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
