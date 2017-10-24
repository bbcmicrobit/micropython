# test the random module

import random

result = ('FAIL', 'PASS')

# test getrandbits
hist = 8 * [0]
for i in range(300):
    r = random.getrandbits(3)
    hist[r] += 1
ok = all(h >= 10 for h in hist)
print('getrandbits:', result[ok])

# test seed
random.seed(0x1234567)
r1 = random.getrandbits(10)
random.seed(0x1234567)
r2 = random.getrandbits(10)
print('seed:       ', result[r1 == r2])

# test randint is within the given range
ok = True
for i in range(100):
    if not 0 <= random.randint(0, 9) <= 9:
        ok = False
print('randint:    ', result[ok])
