import matplotlib.pyplot as plt
import re

thread_number = []
eval_rate = []

with open("./ollama-models/data.txt", "r") as file:

    for line in file:

        match = re.search(r'Thread #(\d+) completed', line)
        if match:
            thread_num = int(match.group(1))
            thread_number.append(thread_num)

        eval_rate_match = re.search(r'Eval Rate: ([\d.]+)', line)
        if eval_rate_match:
            eval_rate_value = float(eval_rate_match.group(1))
            eval_rate.append(eval_rate_value)

thread_numbers = thread_number
eval_rates = eval_rate

# Create the plot
plt.plot(thread_numbers, eval_rates, 'o')

plt.xlabel("Thread #")
plt.ylabel("Tokens/s")
plt.title("Model: Eval Rate by Thread #")
plt.show()
