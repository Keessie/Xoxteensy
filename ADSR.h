#include "Arduino.h"

/*Definitions*/
//ADSR
#define NUMBER_OF_ADSR_STEPS  255 //number of steps in the adsr, is affected by the length of the slow interrupt
//lower numbers here can make some interesting effects
#define ADSR_DIVIDER      8 //log of NUMBEROFADSRSTEPS base 2, basically 2 to what power 
#define ADSR_MIN_VALUE      47  //the minimum value of the ADSR, limited by the analog VCA
#define SUSTAIN_MIN_VALUE   47
#define ADSR_ATTACK      0
#define ADSR_DECAY      1
#define  ADSR_RELEASE    3
#define  ADSR_SUSTAIN    2
#define ADSR_LENGTH      4

//ADSR States
enum
{
  ATTACK = 0,
  DECAY,
  SUSTAIN,
  RELEASE
};


static unsigned char uc_adsr_timer = 0;
static unsigned char uc_state = 0;
static unsigned char uc_sustain_level = 127;
static unsigned char uc_velocity = 127;

/*Function Prototypes*/

/*
@brief The adsr function calculates the amplitude envelope for the ADSR function. It's
based around a central timer which is counting down. The number of countdown cycles
determines how long each stage of the envelope takes.
There are different ways to handle the ADSR. This method does not start from zero with
every new note. It's hard with an analog ADSR to avoid pops and clicks with sudden large
changes in amplitude. You're free to try to fix that problem.

@param It takes the global setting structure.

@return It returns nothing, just setting the adsr multiplier variable in the global structure.
*/

class Adsr {
  private:
    unsigned int un_amplitude_temp;
    unsigned int un_sustain_calc;
    unsigned int un_velocity_calc;
    unsigned char uc_index;
    unsigned char uc_decay;
    unsigned char uc_release;
    unsigned char uc_sustain;

    unsigned char uc_adsr_multiplier; //used for the ADSR calculation
    unsigned char uc_note_velocity;   //how hard the key was hit
    volatile unsigned char g_uc_adsr_midi_sync_flag;
    volatile unsigned char g_uc_key_press_flag;         //flag for when a key is pressed
    volatile unsigned char g_uc_note_on_flag;
    unsigned char auc_synth_params[5] = { 100,100,100,100,100};         //array of synth parameters affected by the ADC readings

  public:
    Adsr();
    void work();
    unsigned char decode_adsr_length(unsigned char uc_adsr_length);
    int set_amplitude();
    int    get_adsr_state();
    unsigned char set_adsr_state(unsigned char uc_adsr_state);
};

Adsr::Adsr() {
}

void Adsr::work() {
  if (g_uc_adsr_midi_sync_flag == 1)
  {
    uc_state = ATTACK;
    g_uc_adsr_midi_sync_flag = 0;

    un_velocity_calc = uc_note_velocity << 1;
    un_velocity_calc *= (NUMBER_OF_ADSR_STEPS - ADSR_MIN_VALUE);
    uc_velocity = un_velocity_calc >> 8;
    uc_velocity += ADSR_MIN_VALUE;
  }

  //if the key is released, move directly to release
  //if the key has bee pressed, turn on the note on Flag and the sequence will begin with attack
  if (0 == g_uc_key_press_flag)
  {
    uc_state = RELEASE;
  }
  else
  {
    g_uc_note_on_flag = 1;//turn on the output
  }

  if (uc_adsr_timer > 0)
  {
    uc_adsr_timer--;
  }
  else
  {
    /*Four state for the ADSR envelope (Obviously!)
    Attack is an upward progression.
    Decay brings the amplitude down to the sustain level.
    Release comes in when the key is released and takes the amplitude down to zero.
    If you want to know more, wikipedia awaits your questioning mind.
    I broke up sections of the knob travel to make much longer and much shorter lengths of time
    possible with only 8 bits. That's what all the if/else statements in each phase are about.
    */
    switch (uc_state)
    {
      case ATTACK:

        if (uc_adsr_multiplier < uc_velocity)
        {
          if (auc_synth_params[ADSR_ATTACK] < 192)
          {
            uc_adsr_timer = auc_synth_params[ADSR_ATTACK] >> 2;

            if (uc_adsr_multiplier < 253)
            {
              uc_adsr_multiplier += 2;
            }
            else
            {
              uc_adsr_multiplier = NUMBER_OF_ADSR_STEPS;
            }
          }
          else
          {
            uc_adsr_timer = auc_synth_params[ADSR_ATTACK] >> 1;

            if (uc_adsr_multiplier < 253)
            {
              uc_adsr_multiplier += 1;
            }
            else
            {
              uc_adsr_multiplier = NUMBER_OF_ADSR_STEPS;
            }
          }
        }
        else
        {
          uc_state = DECAY;

          uc_adsr_timer = auc_synth_params[ADSR_DECAY] >> 2;

          /*The minimum is not zero because the VCA has a 1.3V zero point, so we have to shift
          the knob value to be between the VCA zero point and the maximum*/
          un_sustain_calc = uc_velocity * auc_synth_params[ADSR_SUSTAIN];

          un_sustain_calc >>= 8;

          un_sustain_calc *= (NUMBER_OF_ADSR_STEPS - SUSTAIN_MIN_VALUE);
          un_sustain_calc >>= 8;
          uc_sustain_level = un_sustain_calc + SUSTAIN_MIN_VALUE;
        }

        break;

      case DECAY:


        if (uc_adsr_multiplier > uc_sustain_level)
        {
          uc_adsr_multiplier--;

          uc_adsr_timer = auc_synth_params[ADSR_DECAY];

          if (uc_adsr_timer < 48)
          {
            if (uc_adsr_multiplier > 4)
            {
              uc_adsr_multiplier -= 4;
            }
            else
            {
              uc_adsr_multiplier = 0;
            }
          }
          else if (uc_adsr_timer < 96)
          {
            if (uc_adsr_multiplier > 2)
            {
              uc_adsr_multiplier -= 2;
            }
            else
            {
              uc_adsr_multiplier = 0;
            }
          }
          else
          {
            uc_adsr_multiplier--;
          }

        }
        else
        {
          uc_state = SUSTAIN;

        }

        break;

      case SUSTAIN:

        break;

      case RELEASE:

        //if the note got pressed again, start over
        if (1 == g_uc_key_press_flag)
        {

          uc_state = ATTACK;
          uc_adsr_timer = auc_synth_params[ADSR_ATTACK];
        }
        else//otherwise keep going with the release
        {
          /*There is a non-zero minimum caused by the analog voltage-controlled
            amplifier*/
          if (uc_adsr_multiplier > ADSR_MIN_VALUE)
          {
            uc_adsr_multiplier--;

            uc_adsr_timer = auc_synth_params[ADSR_RELEASE] >> 2;

          }
          else
          {
            //when the envelope has been completed, return to attack and clear the note on Flag
            uc_state = ATTACK;

            uc_adsr_timer = auc_synth_params[ADSR_ATTACK];

            g_uc_note_on_flag = 0;//end of that note

          }
        }
        break;

      default:

        break;
    }
  }

}

unsigned char Adsr::decode_adsr_length(unsigned char uc_adsr_length)                 // return the the adsr length
{


  /*We break it up into regions. Divide by 32 to get 4 regions*/
  uc_index = uc_adsr_length >> 6;

  switch (uc_index)
  {
    case 0:
      uc_decay = uc_adsr_length >> 1;
      uc_sustain = SUSTAIN_MIN_VALUE;
      uc_release = 0;
      break;
    case 1:
      uc_decay = uc_adsr_length >> 1;
      uc_sustain = 92;
      uc_release = uc_adsr_length >> 1;
      break;
    case 2:
      uc_decay = uc_adsr_length;
      uc_sustain = 127;
      uc_release = uc_adsr_length >> 1;
      break;
    case 3:
      uc_decay = uc_adsr_length;
      uc_sustain = 164;
      uc_release = uc_adsr_length;
      break;
    default:
      break;
  }

  auc_synth_params[ADSR_DECAY] = uc_decay;
  auc_synth_params[ADSR_RELEASE] = uc_release;
  auc_synth_params[ADSR_SUSTAIN] = uc_sustain;

}
/*
@brief This function sets the overall amplitude of the synth.
    It blends together amplitude data from the LFO, MIDI, and the ADSR envelope.
@param It takes the global setting structure as input

@return It doesn't return anything. It sets the amplitude by setting the output
    which is a PWM output for the transconductance amplifier based voltage-controlled
    amplifier.
*/

//////////////// set amplitude , maybe incorporate the LFO value in here  /////
int Adsr::set_amplitude() {        // set amplitude gain of adsr


  un_amplitude_temp = uc_adsr_multiplier;      //From the ADSR
  //  un_amplitude_temp *= auc_synth_params[AMPLITUDE];//This parameter comes from the LFO or Drone Mode.

  un_amplitude_temp >>= 8;

  /*The ADSR has a minimum value which is not zero. This is an artifact of the
  transconductance amplifier and the way that it works. Two Base-Emitter junction drops
  if you really want to know. Check out the datasheet for serious details.*/
  if (un_amplitude_temp < ADSR_MIN_VALUE)
  {
    un_amplitude_temp = ADSR_MIN_VALUE;
  }

  /*Set the PWM duty cycle which sets the amplitude of the voltage-controlled amplifier.*/
  //  OCR1AL = (unsigned char)un_amplitude_temp;
 Serial.println("Amplitude");
 Serial.println(un_amplitude_temp);
  return un_amplitude_temp;
}

/////////////////////////////////////////////////


// DONE
int Adsr::get_adsr_state(void) {      // return the state of the adsr

  return uc_state;
}

unsigned char  Adsr::set_adsr_state(unsigned char uc_adsr_state) {                 // set the state of the adsr

  uc_state = uc_adsr_state;
}
