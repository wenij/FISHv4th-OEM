FISHv4th_OEM goals:
  Initially this is a merge with v4th.
    Vic and I will enrich the master branch
      create a PSTAT branch using the
        IAR Configuration I made called
          FISHv4th 205 PSTAT

ISSUES:
  
  HI Vic~!!!
    I'm spouting the same old goal<s>~!
  Create the best merge of v4th and FISH we can.
    It may buy us residuals from the BIG project,
    as a owned IP licensed for BIG's PSTAT.
      We'll see. I'd take a buyout, too~! lol.
        Anywho...

          Add to test file .TIB and .PAD

// Made changes in all configurations~!
/* Define TOP of Dictionary:  
The top of the dictionary is defined in these places
In SLIBS SV_INIT_VALUES:
//        DC32    LIT, WC_FISH_PubRel_NFA // FISH in flash starts here
        DC32    LIT, WC_FISH_GPIO_NFA // FISH in flash starts here
IN FLASH RAMWORDS:
//        DC32    LIT, WC_FISH_PubRel_NFA // FISH in flash starts here
        DC32    LIT, WC_FISH_GPIO_NFA // FISH in flash starts here
In IAR FISH_ONLY:
//        LDR     n, = WC_FISH_PubRel_NFA
        LDR     n, = WC_FISH_GPIO_NFA

*/

asm preprocessor #define entries are for
chip specs and IAR configurations ONLY~!
(407/205) ones:
  STM32F4_XRC08_168MHZ          // Disco Board. Should rename.
  UNDEF_VTOR_PATCH              // 205 issue only?
  STM32F205RC                   // wanna say it's redundant but its foundation.
  STM32F205RC_SRAM128           //
  STM32F205RC_FLASH_1_MB        // Simon selected~!
  STM32F205RC_XRC10_118MHZ      // Best internal xtal.

The asm preprocessor #defines mechanism
is the only way to control the IAR configurations.
IAR ships with debug and Release Configuration defaulted.

We use configurations to run mostly the same code on STM32F0-F4 Chips. 
This is a STM32F407vg Discovery Boards and STM32F205rc example.

The branches we create need to be assigned 2 tasks,
create a supporting feature for FISH or a v4th enhancencment,
to be be merged back into the OEM master,
AND~!
Create branchs maintained and customized for money or love.
The branches are pruned off into the product pipelines.

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
      
NEST initSPI1
 DC32 rmwamd, RCC_CFGR,0F8000000h,20000000h    ; MCO2 = SYSCLK/2
 DC32 atk,RCC_AHB1ENR, ork,5h, strk,RCC_AHB1ENR    ; port A,C clk enable
 DC32 atk,RCC_APB2ENR, ork,1000h, strk,RCC_APB2ENR    ; SPI1 clk enable 84MHz

 DC32 strkk,55550000h, GPIOA_AFRL    ; PA4..7 --> as nSS, SCK, miso, MOSI
 DC32 rmwamd, GPIOA_MODER,0FF00h,0AA00h    ; PA4..7 as AF
 
 DC32 strkk,4h, SPI1_CR2        ; nSS output (I think)
 DC32 strkk,007Dh, SPI1_CR1    ;

 DC32 nexit
