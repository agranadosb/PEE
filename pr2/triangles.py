import os
import time
import shutil
from multiprocessing import Process

def execute(command: str) -> str:
    with open('log-commands.txt', 'a') as logs:
        print(command, file=logs)
        print(command)
    stream = os.popen(command)
    output = stream.read()
    print(output)
    return output

def launch_jobs(processes: list[Process]):
    for i in processes:
        i.start()
    for i in processes:
        if i.is_alive():
            i.join()

# Datos
base_dir = 'PCFGs'
models = ['', '-v']
bracketed = ['', '-b']
iterations = 100

# Tiene que haber una carpeta llamada custom-models
folder_models = os.path.join(base_dir, 'triangles-algorithms')

# copy base grammar
if os.path.exists(folder_models):
    shutil.rmtree(folder_models)
os.mkdir(folder_models)

if os.path.exists('log-commands.txt'):
    os.remove('log-commands.txt')

learn_command = os.path.join(base_dir, 'scfg-toolkit/scfg_learn')
prob_command = os.path.join(base_dir, 'scfg-toolkit/scfg_prob')
confus_command = os.path.join(base_dir, 'scfg-toolkit/confus')
initial_grammar_file = os.path.join(base_dir, 'MODELS/G-0')

assert os.path.exists(initial_grammar_file)

for model in models:
    for brackets in bracketed:
        training_right_data = os.path.join(base_dir, f'DATA/Tr-right{brackets}')
        training_isoc_data = os.path.join(base_dir, f'DATA/Tr-isoc{brackets}')
        training_equil_data = os.path.join(base_dir, f'DATA/Tr-equil{brackets}')

        assert os.path.exists(training_right_data)
        assert os.path.exists(training_isoc_data)
        assert os.path.exists(training_equil_data)

        test_right_data = os.path.join(base_dir, f'DATA/Ts-right{brackets}')
        test_isoc_data = os.path.join(base_dir, f'DATA/Ts-isoc{brackets}')
        test_equil_data = os.path.join(base_dir, f'DATA/Ts-equil{brackets}')

        assert os.path.exists(test_right_data)
        assert os.path.exists(test_isoc_data)
        assert os.path.exists(test_equil_data)

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
        results_confus_file = os.path.join(folder_results, f'confus-results{model}{brackets}.txt')

        # train models
        command = f"{learn_command} -g {initial_grammar_file} -f {out_right_model} -i {iterations} -m {training_right_data} -l {model}"
        p1 = Process(target=execute, args=(command,))
        command = f"{learn_command} -g {initial_grammar_file} -f {out_isoc_model} -i {iterations} -m {training_isoc_data} -l {model}"
        p2 = Process(target=execute, args=(command,))
        command = f"{learn_command} -g {initial_grammar_file} -f {out_equil_model} -i {iterations} -m {training_equil_data} -l {model}"
        p3 = Process(target=execute, args=(command,))

        launch_jobs([p1, p2, p3])

        # classify with the trained models and get results in right
        command = f"{prob_command} -g {out_right_model} -m {test_right_data} > {out_prob_right_model}"
        p1 = Process(target=execute, args=(command,))
        command = f"{prob_command} -g {out_isoc_model} -m {test_right_data} > {out_prob_isoc_model}"
        p2 = Process(target=execute, args=(command,))
        command = f"{prob_command} -g {out_equil_model} -m {test_right_data} > {out_prob_equil_model}"
        p3 = Process(target=execute, args=(command,))

        launch_jobs([p1, p2, p3])

        command = "paste {} {} {} | awk '{}' > {}".format(
            out_prob_right_model,
            out_prob_isoc_model,
            out_prob_equil_model,
            r'{m=$1;argm="right"; if ($2>m) {m=$2;argm="equil";} if ($3>m) {m=$3;argm="isosc";}printf("right %s\n",argm);}',
            results_file
        )
        execute(command)

        # classify with the trained models and get results in equil
        command = f"{prob_command} -g {out_right_model} -m {test_equil_data} > {out_prob_right_model}"
        p1 = Process(target=execute, args=(command,))
        command = f"{prob_command} -g {out_isoc_model} -m {test_equil_data} > {out_prob_isoc_model}"
        p2 = Process(target=execute, args=(command,))
        command = f"{prob_command} -g {out_equil_model} -m {test_equil_data} > {out_prob_equil_model}"
        p3 = Process(target=execute, args=(command,))

        launch_jobs([p1, p2, p3])

        command = "paste {} {} {} | awk '{}' >> {}".format(
            out_prob_right_model,
            out_prob_isoc_model,
            out_prob_equil_model,
            r'{m=$2;argm="equil"; if ($1>m) {m=$1;argm="right";} if ($3>m) {m=$3;argm="isosc";} printf("equil %s\n",argm);}',
            results_file
        )
        execute(command)

        # classify with the trained models and get results in isoc
        command = f"{prob_command} -g {out_right_model} -m {test_isoc_data} > {out_prob_right_model}"
        p1 = Process(target=execute, args=(command,))
        command = f"{prob_command} -g {out_isoc_model} -m {test_isoc_data} > {out_prob_isoc_model}"
        p2 = Process(target=execute, args=(command,))
        command = f"{prob_command} -g {out_equil_model} -m {test_isoc_data} > {out_prob_equil_model}"
        p3 = Process(target=execute, args=(command,))

        launch_jobs([p1, p2, p3])

        command = "paste {} {} {} | awk '{}' >> {}".format(
            out_prob_right_model,
            out_prob_isoc_model,
            out_prob_equil_model,
            r'{m=$3;argm="isosc"; if ($1>m) {m=$1;argm="right";} if ($2>m) {m=$2;argm="equil";} printf("isosc %s\n",argm);}',
            results_file
        )
        execute(command)

        command = f"cat {results_file} | {confus_command}"
        confus_output = execute(command)
        with open(results_confus_file, 'w') as result_confus:
            result_confus.write(confus_output)
