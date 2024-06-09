import sys
import subprocess
import time
import os
import signal
import csv
from py4j.java_gateway import JavaGateway, GatewayParameters, Py4JNetworkError

# caution: path[0] is reserved for script path (or '' in REPL)
sys.path.insert(1, r'D:\University\Year 3\Thesis\Bachelor Thesis\code\dl-lib-main')

from algorithm import descriptionTreeMappingAlgorithm

problem_types = ['just_based', 'random_entailed', 'random_non_entailed']

def run_command(cmd, cwd=None):
    process = subprocess.run(cmd, shell=True, capture_output=True, text=True, cwd=cwd)
    if process.returncode != 0:
        print(f"Command failed with error: {process.stderr}")
        sys.exit(1)
    return process.stdout, process.stderr

def start_java_gateway(dl_lib_main_path, jar_command):
    java_gateway_process = subprocess.Popen(
        ['java', '-Xms25m', '-Xmx27m', '-jar', jar_command],
        cwd=dl_lib_main_path,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        stdin=subprocess.PIPE
    )
    return java_gateway_process

def close_java_gateway(java_gateway_process):
    try:
        java_gateway_process.terminate()
        java_gateway_process.wait(timeout=5)
    except Exception as e:
        print(f"Error occurred while terminating the Java gateway process: {e}")

def main():
    datasets_folder = r"D:\University\Year 3\Thesis\Bachelor Thesis\code\datasets"
    owl2spass_folder = r'D:\University\Year 3\Thesis\Bachelor Thesis\code\IJCAR-2022-Experiments'
    owl2spass_jar = r".\owl2spass\target\owl2spass-0.1-SNAPSHOT-jar-with-dependencies.jar"
    results_file = r"D:\University\Year 3\Thesis\Bachelor Thesis\code\results.csv"
    dl_lib_main_path = r"D:\University\Year 3\Thesis\Bachelor Thesis\code\dl-lib-main"
    jar_command = 'target/dl-lib-0.1.5-jar-with-dependencies.jar'
    # jar_command = 'java -Xms100m -Xmx100m -jar target/dl-lib-0.1.5-jar-with-dependencies.jar'
    
    with open(results_file, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(["ontology", "iteration", "command_output", "number_of_hypothesis_returned", "algorithm_total_time", "ontology_parsing_time", "observation_time", "gateway_opening_time", "gateway_closing_time"])

    # Run the algorithm for each file in the "Datasets" folder
    # Run the algorithm n times for each file in the "Datasets" folder
    for file_name in os.listdir(datasets_folder):
            file_path = os.path.join(datasets_folder, file_name)
            for i in range(5):
                gate_open_time = time.time()
                java_gateway_process = start_java_gateway(dl_lib_main_path, jar_command)
                gate_open = time.time()
                obs_start = time.time()
                # Run owl2spass command to generate observation
                java_command = f'java -Xmx25m -Xmx27m -jar {owl2spass_jar} "{file_path}" spass.owl just_based'
                # java_command = f'java -jar {owl2spass_jar} "{file_path}" spass.owl just_based'  # just_based/random_entailed/random_non_entailed
                run_command(java_command, cwd=owl2spass_folder)
                stdout, stderr = run_command(java_command, cwd=owl2spass_folder)
                obs_end = time.time()
                # Parse ontology and observation
                gateway = JavaGateway()
                # get a parser from OWL files to DL ontologies
                parse_start = time.time()
                parser = gateway.getOWLParser(True)
                ontology = parser.parseFile(r"D:\University\Year 3\Thesis\Bachelor Thesis\code\IJCAR-2022-Experiments\ontology.owl")
                observation = parser.parseFile(r"D:\University\Year 3\Thesis\Bachelor Thesis\code\IJCAR-2022-Experiments\observation.owl")
                parse_end = time.time()
                start = time.time()
                algo = descriptionTreeMappingAlgorithm(ontology, observation, gateway)
                hypothesis = algo.find_hypothesis(observation, ontology)
                end = time.time()
                num_hyp = len(hypothesis)
                total_algorithm_time = end - start
                gate_close_time = time.time()
                close_java_gateway(java_gateway_process)
                gate_close = time.time()
                with open(results_file, mode='a', newline='') as file:
                    writer = csv.writer(file)
                    writer.writerow([file_name, i+1, stdout, num_hyp, total_algorithm_time, parse_end -parse_start, obs_end-obs_start, gate_open-gate_open_time, gate_close-gate_close_time])
                print(f"Iteration {i+1} for file {file_name}:")
                print(f"Command output: {stdout}")
                print(f"There are {num_hyp} hypotheses returned by the algorithm.")
                print(f"Total time: {total_algorithm_time} seconds")
                print()
                print()
                

if __name__ == "__main__":
    main()