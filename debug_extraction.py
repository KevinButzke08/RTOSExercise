import sys
from collections import defaultdict
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

for line in sys.stdin:
    print(line)
    for event in event_patterns:
        if "END OF TRACE" in line:
            print(event_counter)
            print ("Total number of debug logs:", log_entry_counter)
            break
        if event in line:
            event_counter[event] +=1
            log_entry_counter += 1
    
       