import sys
from collections import defaultdict
import csv
#Use idf.py flash monitor | python3 debug_extraction.py
event_patterns = [
    "TASK_CREATE",
    "TASK_CREATE_FAILED",
    "TASK_DELETE",
    "TASK_DELAY",
    "TASK_DELAY_UNTIL",
    "TASK_SWITCHED_IN",
    "TASK_SWITCHED_OUT",
    "TICK_INCREMENT",
    "RECEIVE",
    "RECEIVE_FAILED",
    "RECEIVE_FROM_ISR",
    "RECEIVE_FROM_ISR_FAILED",
    "SEND",
    "SEND_FAILED",
    "SEND_FROM_ISR",
    "SEND_FROM_ISR_FAILED"
]
log_entry_counter = 0
event_counter = {event: 0 for event in event_patterns}
output_file = 'log_entries.csv'

with open(output_file, 'w', newline='') as csvfile:
    fieldnames = ['eventtype', 'tick', 'timestamp', 'taskid', 'task', 'newtick', 'tickstodelay']
    writer = csv.DictWriter(csvfile, fieldnames)
    writer.writeheader()
    for line in sys.stdin:
        print(line.rstrip())
        if "DEBUG:" in line:
            split = line.split(": ", 1)[1].split(", ")
            split = split[0:len(split)-1]
            my_dict = {item.split(':', 1)[0]: item.split(':', 1)[1].strip() for item in split}
            event_counter[my_dict["Event"]] +=1
            log_entry_counter += 1
            writer.writerow({
                    'eventtype': "trace" + my_dict.get('Event', ''),
                    'tick': my_dict.get('Tick', ''),
                    'timestamp': my_dict.get('Timestamp', ''),
                    'taskid': my_dict.get('TaskHandle', ''),
                    'task': my_dict.get('Task', ''),
                    'newtick': my_dict.get('New_Tick', ''),
                    'tickstodelay': my_dict.get('TicksToDelay', '')
                })
        if "END OF TRACE" in line:
            print(event_counter)
            print("Total number of debug logs:", log_entry_counter)
            break
    