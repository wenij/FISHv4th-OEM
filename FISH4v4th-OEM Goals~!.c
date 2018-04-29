FISHv4th_OEM goals:
  HI Vic~!!!
    I'm spouting the same old goal<s>~!
  Create the best merge of v4th and FISH we can.
    It may buy us residuals from the BIG project,
    as a owned IP licensed for BIG's PSTAT.
      We'll see. I'd take a buyout, too~! lol.
        Anywho...

To clean this up I will endeavoir to eleminate the
asm preprocessor #defines except for chip (407/205) ones.
The asm preprocessor #defines mechanism
is the only way to control the IAR configurations.
IAR ships with debug and Release Configuration defaulted.

We use configurations to run mostly the same code on STM32F0-F4 Chips. 
This is a STM32F407vg Discovery Boards and STM32F205rc example.

The branches we create need to be assigned 2 tasks,
create a supporting feature for FISH or a v4th enhancencment
OR
Create branchs maintained and customized for money or love.
The branches are pruned off into the product pipeline.

Money love and recognition come back...

Then drop most obsolete config defines stuff.
  Like TRUE is -1. Done deal. Drop the chatter.

So after a few other things we come up with....    

  BIG PSTAT version.
  allways use .h files for this type of notes, just don't include em~!
    
    NEED TO redo Wordcat / word adding process.
      for now #define WORDS_START as the definative method.
        New words and wordcats are the beneficiaries~!
          
    SIMPLIFY THIS CRAP for any pro/commercial wordset you develop~!
      Craft it inline as a tool~!

EXPECT_FILE ( --) Simple high speed expect

GPIO Wordcat: 1rst whack will be pstat specific
MAKE all needed WORDCATS and define insertion of words in the cat~!
We want the 1rs whack to be them made our OEM version
and then make BIG PSTAT a branch of that we just tear off for BIG.

  Chip_Select_ADC ( on=!0/off-0 -- <signal is active low>),
  DAC, etc for test and production.
    Smartio_LED ( on=!0/off-0 -- )

Tread carefully~!!!
I made a few too many shortcuts in FISH.
The following goals may have issues with my implementation:

Cache TOS and NOS: I hope this isn't a bear~!

VIC: I need help getting the hi-level single stepping working,
      and IAR debugging best practices~!
      

