#!/usr/bin/env python

import json
import os
import requests
import select
import threading
import time
import subprocess
from websockets.sync.client import connect
from websockets.exceptions import ConnectionClosedError

BOT_USERNAME="bot4352"
OPPONENT_USERNAME="hoomin123123"
FIFO_IN_PATH="/tmp/fifo_from_bot"
FIFO_OUT_PATH="/tmp/fifo_to_bot"

with open("config/sample_team_packed.txt") as f:
    SAMPLE_TEAM_PACKED=f.readline()

# Create the FIFO pipe to communicate with other processes. 
def create_fifo():
    if os.path.exists(FIFO_IN_PATH):
        os.remove(FIFO_IN_PATH)
    if os.path.exists(FIFO_OUT_PATH):
        os.remove(FIFO_OUT_PATH)
    os.mkfifo(FIFO_IN_PATH, 0o666)
    os.mkfifo(FIFO_OUT_PATH, 0o666)

def login(challstr: str):
    data = {"name": BOT_USERNAME, "pass": "p", "challstr": challstr}
    result = requests.post("https://play.pokemonshowdown.com/api/login", data=data)
    result_json = json.loads(result.text[1:])
    return result_json["assertion"]

def parse(message):
    print(f"Received: {message}")

def parse_lobby(message):
    json_string = message.split("|")[-1]
    if len(json_string) == 0:
        return None
    if json_string[0] != "{":
        return json_string
    json_obj = json.loads(json_string)
    if "games" not in json_obj:
        return None
    return json_obj["games"]

def initialize(websocket):
        parse(websocket.recv())
        # Receive initial message from websocket
        message = websocket.recv()
        parse(message)
        # Split by "|"
        split_message = message.split("|")
        # Get challenge string (2|3).
        challstr = split_message[2] + "|" + split_message[3]
        # Call "login" assertion string
        assertion = login(challstr)
        print("Assertion")
        print(assertion)
        reply = "|/trn " + BOT_USERNAME + ",0," + assertion 
        websocket.send(reply)
        parse(websocket.recv())
        websocket.send("|/join lobby")
        return websocket
    
def send_team(websocket, team):
    print("Sending team: ", team)
    websocket.send("|/utm " + team)
    websocket.send("|/accept "  + OPPONENT_USERNAME)

# Get team from std pipe.
def get_team():
    print(f"opening {FIFO_IN_PATH}")
    with open(FIFO_IN_PATH, 'r') as in_fd: 
        # Create a poll object
        poller = select.poll()
        
        # Register the FIFO file descriptor for reading events
        poller.register(in_fd, select.POLLIN)
        
        print(f"Polling {FIFO_IN_PATH} for changes. Press Ctrl+C to exit.")
        
        try:
            # Poll for events
            events = poller.poll()
                
            for fd, event in events:
                if event & select.POLLIN:
                    # Data is available to read
                    data = in_fd.read()
                    if not data: 
                        continue
                    print(f"Received data: {data}")
                    json_data = json.loads(data[data.find('{'):])
                    return json_data["team"]
                
        except KeyboardInterrupt:
            print("Interrupted by user")
            return ""
    

def websocket_to_fifo(websocket, message):
    message = None
    try: 
        print(message)
        with open(FIFO_OUT_PATH, 'w') as out_fd:
            out_fd.write(message)
    except KeyboardInterrupt:
        print("Stopped by user")
    except TimeoutError:
        print("Received all data from websocket")
    return message
     
def fifo_to_websocket(websocket, room_id):
    with open(FIFO_IN_PATH, 'r') as in_fd:
        # Create a poll object
        poller = select.poll()
        
        # Register the FIFO file descriptor for reading events
        poller.register(in_fd, select.POLLIN)
        
        print(f"Polling {FIFO_IN_PATH} for changes. Press Ctrl+C to exit.")
        
        try:
            # Poll for events
            events = poller.poll()
                
            for fd, event in events:
                if event & select.POLLIN:
                    # Data is available to read
                    data = in_fd.read()
                    if not data: 
                        continue
                    print(f"Received data from FIFO: {data}")
                    # Publish to websocket
                    out=room_id + "|/" + data
                    websocket.send(out)
                    
        except KeyboardInterrupt:
            print("Interrupted by user")
            return ""

    

if __name__ == "__main__":
    # create_fifo()
    #bot_process = subprocess.Popen(
    #    ["./build/bot"],  # The path to your C++ program
    #    stdin=subprocess.PIPE, 
    #    stdout=subprocess.PIPE,
    #    stderr=subprocess.PIPE,
    #    text=True  # Ensure communication is in text mode
    #)
    with connect("ws://0.0.0.0:8000/showdown/websocket") as websocket:
        initialize(websocket)
        games_result = None
        while type(games_result) != str or "wants to battle!" not in games_result:
            message = websocket.recv()
            games_result = parse_lobby(message)
            print(games_result)
        send_team(websocket, get_team())
        # Print websocket results to FIFO
        room_id=None
        while room_id is None:
            message=websocket_to_fifo(websocket)
            if message[0] == '>':
                eol = message.find('\n')
                room_id = message[1:eol]
                break
        print(f"Received room id {room_id}")
        if room_id is None:
            exit(1)

        def from_fifo_runner(websocket, room_id):
            while True:
                try:
                    fifo_to_websocket(websocket, room_id)
                except KeyboardInterrupt:
                    print("Keyboard interrupt. Exiting program...")
                    break

        def from_websocket_runner(websocket):
            while True:
                try:
                    websocket_to_fifo(websocket)
                except KeyboadInterrupt:
                    print("Keyboard interrupt. Exiting program...")
                    break

        
        thread1 = threading.Thread(target=from_fifo_runner, args=(websocket, room_id))
        thread2 = threading.Thread(target=from_websocket_runner, args=(websocket))

        thread1.start()
        thread2.start()
        thread1.join()
        thread2.join()

