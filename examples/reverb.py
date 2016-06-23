import audio

def from_file(file, frame):
    ln = -1
    while ln:
        ln = file.readinto(frame)
        yield frame

def reverb_gen(src, buckets, reflect, fadeout):
    bucket_count = len(buckets)
    bucket = 0
    for frame in src:
        echo = buckets[bucket]
        echo *= reflect
        echo += frame
        yield echo
        buckets[bucket] = echo
        bucket += 1
        if bucket == bucket_count:
            bucket = 0
    while fadeout:
        fadeout -= 1
        echo = buckets[bucket]
        echo *= reflect
        yield echo
        buckets[bucket] = echo
        bucket += 1
        if bucket == bucket_count:
            bucket = 0

def reverb(src, delay, reflect):
    #Do all allocation up front, so we don't need to do any in the generator.
    bucket_count = delay>>2
    buckets = [ None ] * bucket_count
    for i in range(bucket_count):
        buckets[i] = audio.AudioFrame()
    vol = 1.0
    fadeout = 0
    while vol > 0.05:
        fadeout += bucket_count
        vol *= reflect
    return reverb_gen(src, buckets, reflect, fadeout)

def play_file(name, delay=80, reflect=0.5):
    #Do allocation here, as we can't do it in an interrupt.
    frame = audio.AudioFrame()
    with open(name) as file:
        gen = from_file(file, frame)
        r = reverb(gen, delay, reflect)
        audio.play(r)
