

#### Generate Seed

    "123456"


#### Restore BIP39

     1 mirror
     2 reject
     3 rookie
     4 talk
     5 pudding
     6 throw
     7 happy
     8 era
     9 myth
    10 already
    11 payment
    12 owner


#### Restore SSK (2 out of 3)

Note: Each generation of SSKR shares (using the same binary seed) leads to different combination of words.
However, the restoration of these words should always lead back to the same binary seed.

    Share #1
     1 tuna   6 gyro   11 zone   16 runs   21 waxy   26 buzz
     2 acid   7 able   12 jazz   17 dark   22 zone   27 horn
     3 epic   8 acid   13 draw   18 menu   23 body   28 luau
     4 gyro   9 able   14 pool   19 luau   24 veto   29 tiny
     5 cost   10 news  15 jump   20 hard   25 meow
     
    Share #2
     1 tuna   6 gyro   11 whiz   16 scar   21 gala   26 zone
     2 acid   7 able   12 very   17 also   22 huts   27 cost
     3 epic   8 acid   13 stub   18 jowl   23 what   28 into
     4 gyro   9 acid   14 join   19 idea   24 knob   29 ugly
     5 cost   10 help  15 inky   20 yawn   25 song

    Share #3
     1 tuna   6 gyro   11 visa   16 fuel   21 rock   26 cook
     2 acid   7 able   12 join   17 iris   22 ugly   27 slot
     3 epic   8 acid   13 yurt   18 judo   23 soap   28 next
     4 gyro   9 also   14 echo   19 fizz   24 safe   29 guru
     5 cost   10 axis  15 heat   20 cost   25 dice


## Validation with seedtool

```bash
$ seedtool --version
0.9.0

$ seedtool --in sskr "tuna acid epic gyro cost gyro able acid also axis visa join yurt echo heat fuel iris judo fizz cost rock ugly soap safe dice cook slot next guru" "tuna acid epic gyro cost gyro able acid acid help whiz very stub join inky scar also jowl idea yawn gala huts what knob song zone cost into ugly"
8d969eef6ecad3c29a3a629280e686cf

$ seedtool --in dice 123456
8d969eef6ecad3c29a3a629280e686cf

$ seedtool --in dice 123456 | seedtool --in hex --out bip39
mirror reject rookie talk pudding throw happy era myth already payment owner
```
