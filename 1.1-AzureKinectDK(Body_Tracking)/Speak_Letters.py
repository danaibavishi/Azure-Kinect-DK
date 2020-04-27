from playsound import playsound
import argparse
import math
import time
from pythonosc import dispatcher
from pythonosc import osc_server
#D = {'a': 1, 'c': 3, 'b': 2, 'e': 5, 'd': 4, 'g': 7, 'f': 6, 'i': 9, 'h': 8, 'k': 11, 'j': 10, 'm': 13, 'l': 12, 'o': 15, 'n': 14, 'q': 17, 'p': 16, 's': 19, 'r': 18, 'u': 21, 't': 20, 'w': 23, 'v': 22, 'y': 25, 'x': 24, 'z': 26}
#D = {chr(i+64):i for i in range(1,27)}
D = {'A': 1, 'B': 2, 'C': 3, 'D': 4, 'E': 5, 'F': 6, 'G': 7, 'H': 8, 'I': 9, 'J': 10, 'K': 11, 'L': 12, 'M': 13, 'N': 14, 'O': 15, 'P': 16, 'Q': 17, 'R': 18, 'S': 19, 'T': 20, 'U': 21, 'V': 22, 'W': 23, 'X': 24, 'Y': 25, 'Z': 26}
mem = 'NUll'

def play(alphabet):
        str = 'Alphabet/%s.wav'%(alphabet)
        playsound(str)
        print(alphabet)

def extletter(misc,number):
        global mem
        letter = list(D.keys())[list(D.values()).index(number)]
        if (letter != mem):
            mem = letter
            play(letter)

        #print("Letter Received: %s"%(letter))

if __name__ == '__main__':
    #play the sound for that key
    #play ('B')

    #get number from osc
    parser = argparse.ArgumentParser()
    parser.add_argument("--ip",default="127.0.0.1", help="The ip to listen on")
    parser.add_argument("--port",type=int, default=7000, help="The port to listen on")
    args = parser.parse_args()

    dispatcher = dispatcher.Dispatcher()
    dispatcher.map("/wek/outputs", extletter)
    server = osc_server.ThreadingOSCUDPServer((args.ip, args.port), dispatcher)
    print("Serving on {}".format(server.server_address))
    server.serve_forever()
