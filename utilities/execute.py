import time
import traceback
import datetime


def execute_at(what, when, retries=2, delay_seconds=5):
    time_object = datetime.datetime.strptime(when, "%H:%M")

    prev_time = datetime.datetime.now()

    while True:
        today = datetime.datetime.today()
        deadline = datetime.datetime.combine(today, time_object.time())

        current_time = datetime.datetime.now()
        time_left = deadline - current_time

        # Check if the deadline has been passed
        if prev_time < deadline and current_time >= deadline:
            print("\nTime to execute script!")
            break

        # Print the deadline, current time, and time left
        print(f"\rDeadline: {deadline.strftime('%H:%M:%S')} | Current Time: {current_time.strftime('%H:%M:%S')} | Time Left: {int(time_left.total_seconds())} seconds", end="")

        # Update prev_time
        prev_time = current_time

        # Wait for 1 second
        time.sleep(1)

    result = False
    for i in range(0, retries + 1):
        try:
            result = what()
        except Exception as e:
            print(f"An error occurred: {e}")
            traceback.print_exc()

        if result or i == retries:
            break

        for j in range(delay_seconds, 0, -1):
            print(f"\rNo success! Retry {i + 1}/{retries} in {j} seconds", end="")
            time.sleep(1)

        print("")

    if result:
        print("Success!")
    else:
        print("No success!")


def run_every_hour(what, retries=2, delay_seconds=5):
    now = datetime.datetime.now()
    next_hour = now.replace(minute=0, second=0, microsecond=0) + datetime.timedelta(hours=1)

    if next_hour <= now:
        next_hour += datetime.timedelta(hours=1)

    prev_time = datetime.datetime.now()

    while True:
        current_time = datetime.datetime.now()
        time_left = next_hour - current_time

        # Check if the deadline has been passed
        if prev_time < next_hour and current_time >= next_hour:
            print("\nTime to execute script!")
            break

        # Print the deadline, current time, and time left
        print(f"\rDeadline: {next_hour.strftime('%H:%M:%S')} | Current Time: {current_time.strftime('%H:%M:%S')} | Time Left: {int(time_left.total_seconds())} seconds", end="")

        # Update prev_time
        prev_time = current_time

        # Wait for 1 second
        time.sleep(1)

    result = False
    for i in range(0, retries + 1):
        try:
            result = what()
        except Exception as e:
            print(f"An error occurred: {e}")
            traceback.print_exc()

        if result or i == retries:
            break

        for j in range(delay_seconds, 0, -1):
            print(f"\rNo success! Retry {i + 1}/{retries} in {j} seconds", end="")
            time.sleep(1)

        print("")

    if result:
        print("Success!")
    else:
        print("No success!")