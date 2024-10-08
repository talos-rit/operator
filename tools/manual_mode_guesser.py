import sys

import TTY_Reader

def guess(tty: TTY_Reader):
    #enter trial command
    #listen and log response
    resp = tty.cmd(command)
    #after sleep period, abort sequence
    tty.abort()
    #repeat
    #after permutations have been guessed (or manual mode has been found), end loop and write results to csv/stdout
    pass

def main(tty_name):
    tty = TTY_Reader(tty_name)

if __name__ == "__main__":

    tty_name = '/dev/ttyUSB0'
    if len(sys.argv) > 1:
        tty_name = sys.argv[1]        

    main(tty_name)