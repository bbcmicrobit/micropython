
import os
from microbit import display, Image

#We don't have space for Beowolf, so here's the next best thing...
text = """
’Twas brillig, and the slithy toves
  Did gyre and gimble in the wabe:
All mimsy were the borogoves,
  And the mome raths outgrabe.

“Beware the Jabberwock, my son!
  The jaws that bite, the claws that catch!
Beware the Jubjub bird, and shun
 The frumious Bandersnatch!”

He took his vorpal sword in hand;
  Long time the manxome foe he sought—
So rested he by the Tumtum tree
  And stood awhile in thought.

And, as in uffish thought he stood,
  The Jabberwock, with eyes of flame,
Came whiffling through the tulgey wood,
  And burbled as it came!

One, two! One, two! And through and through
  The vorpal blade went snicker-snack!
He left it dead, and with its head
  He went galumphing back.

“And hast thou slain the Jabberwock?
  Come to my arms, my beamish boy!
O frabjous day! Callooh! Callay!”
  He chortled in his joy.

’Twas brillig, and the slithy toves
  Did gyre and gimble in the wabe:
All mimsy were the borogoves,
  And the mome raths outgrabe.
"""

def test_read_while_writing():
    j1 = open("jabbawocky.txt", "w")
    j1.write(text)
    j2 = open("jabbawocky.txt")
    short = j2.read()
    assert text.startswith(short)
    del j2
    j1.close()
    j2 = open("jabbawocky.txt")
    assert j2.read() == text
    j2.close()
    os.remove("jabbawocky.txt")

def test_removing_mid_read():
    with open("jabbawocky.txt", "w") as j:
        j.write(text)
    j = open("jabbawocky.txt")
    os.remove("jabbawocky.txt")
    try:
        j.read()
        assert False, "Shouldn't reach here"
    except OSError:
        pass

def test_removing_mid_write():
    j = open("jabbawocky.txt", "w")
    os.remove("jabbawocky.txt")
    try:
        j.write(text)
        assert False, "Shouldn't reach here"
    except OSError:
        pass

def test_repeated_write():
    for i in range(40):
        with open("jabbawocky.txt", "w") as j:
            j.write(text)

display.clear()
try:
    test_read_while_writing()
    test_removing_mid_read()
    test_removing_mid_write()
    test_repeated_write()
    print("File test: PASS")
    display.show(Image.HAPPY)
except Exception as ae:
    display.show(Image.SAD)
    raise

