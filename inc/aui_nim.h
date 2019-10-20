/**
<!--
Notice:             This file mainly contains Doxygen-style comments to generate
                    the HTML help file (.chm) provided by ALi Corporation to
                    give ALi Customers a quick support for using ALi AUI API
                    Interface.\n
                    Furthermore, the comments have been wrapped at about 80 bytes
                    in order to avoid the horizontal scrolling when splitting
                    the display in two part for coding, testing, debugging
                    convenience
Current ALi Author: Oscar.Shi
Last update:        2017.08.17
-->

@file   aui_nim.h

@brief  Net Interface (NIM) Module

        Net Interface (NIM) Module is used to control the network Interfaces
        available on the targeted ALi board.\n
        In fact, an ALi board may have different network interfaces supporting
        different standards then NIM Module provides support for
        - @b DVBS/S2
        - @b DVBC
        - @b DVBT/T2/ISDBT

        NIN Module allows controlling
        - @b LNB (only for DVBS)
        - @b Tuner
        - @b Demodulator

        as the blocks of the reception chain after the antenna. Furthermore
        NIM Module allows to select a given channel in the systems
        - @b DVBS/S2
        - @b DVB-C
        - @b DVB-T

        and also to launch an auto-scan procedure but only for DVBS/S2.\n\n

        The ALi chipset allows multiple streams reception. One chain uses the
        embedded demodulator, and it is connected through the internal parallel
        interface.
        The other chain requires an external demodulator which can be connected
        through one serial interface supporting different modes SSI, SPI, ASSI.

        With the exception of the initialization phase, the use of the embedded
        or external demodulator is transparent for NIM Module.

@note For further details, please refer to ALi document
      <b><em>
      ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Net Interface Module"
      (NIM) Module"
      </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_NIM_H_

#define _AUI_NIM_H_

/*************************Included Header File List***************************/

#include "aui_common.h"

#ifdef __cplusplus

extern "C" {

#endif

/*******************************Global Macro List*****************************/

/**
Macro to specify that presently the structs contained in this header file are
designed to support up to six (6) NIM devices
*/
#define AUI_NIM_HANDLER_MAX     6

/**
Macro to specify that, when connecting NIM Module in @b DVB-T2 system mode, if
the member @b plp_index of the struct #aui_dvbt2_param is set as
#AUI_NIM_PLP_AUTO then the platform will select the first PLP.\n
After connecting, the signal status information can be gotten then the number
of PLP will be stored in the in the member @b plp_num of the struct
#aui_signal_dvbt2_status
*/
#define AUI_NIM_PLP_AUTO (-1)

/**
Macro to specify that, when connecting NIM Module in @b DVB-T2 system mode, if
the member @b plp_index of the struct #aui_dvbt2_param is set as
#AUI_NIM_PLP_SCAN_NEXT, then the platform will select the next PLP.\n
After connecting, the signal status information can be gotten then the PLP ID
will be stored in the member @b plp_id field of the struct
#aui_signal_dvbt2_status

@warning    The macro #AUI_NIM_PLP_AUTO @a must be used before using this macro
*/
#define AUI_NIM_PLP_SCAN_NEXT (-2)

/**
Macro to specify <b> Frequency Offset </b> in @b QPSK
*/
#define QPSK_CONFIG_FREQ_OFFSET  (1<<0)

/**
Macro to specify <b> External Analog to Digital Converter (ADC) </b> in the
<b> QPSK demodulation </b>
*/
#define QPSK_CONFIG_EXT_ADC      (1<<1)

/**
Macro to specify <b> I/Q Signal Normal </b> in the <b> QPSK demodulation </b>
*/
#define QPSK_CONFIG_IQ_NORMAL    (0<<2)

/**
Macro to specify <b> I/Q Signal Swap </b> in the <b> QPSK demodulation </b>
*/
#define QPSK_CONFIG_IQ_SWAP      (1<<2)

/**
Macro to specify <b> I2C Pass-Through Device </b> in the <b> QPSK demodulation </b>
*/
#define QPSK_CONFIG_I2C_THROUGH  (1<<3)

/**
Macro to specify <b> Polarization Reversed </b> in the <b> QPSK demodulation </b>
*/
#define QPSK_CONFIG_POLAR_REVERT (1<<4)

/**
Macro to specify <b> New Automatic Gain Control (ACG) 1 </b> in the
<b> QPSK demodulation </b>
*/
#define QPSK_CONFIG_NEW_AGC1     (1<<5)

/**
Macro to specify <b> 1 Bit Mode </b> in the <b> QPSK demodulation </b>
*/
#define QPSK_CONFIG_MODE_1BIT    (0<<6)

/**
Macro to specify <b> 2 bit Mode </b> in the <b> QPSK demodulation </b>
*/
#define QPSK_CONFIG_MODE_2BIT    (1<<6)

/**
Macro to specify <b> 4 bit Mode </b> in the <b> QPSK demodulation </b>
*/
#define QPSK_CONFIG_MODE_4BIT    (2<<6)

/**
Macro to specify <b> 8 bit Mode </b> in the <b> QPSK demodulation </b>
*/
#define QPSK_CONFIG_MODE_8BIT    (3<<6)

/**
Macro to specify <b> Hardware I2C Bus </b> for the <b> demodulator and tuner </b>
*/
#define I2C_TYPE_SCB    0x00000000

/**
Macro to specify <b> Software I2C Bus </b> for the <b> demodulator and tuner </b>
*/
#define I2C_TYPE_GPIO   0x00010000

/**
Macro to specify <b> RM I2C Bus </b> for the <b> demodulator and tuner </b>

@warning    This macro is no longer supported then is @a deprecated
*/
#define I2C_TYPE_SCB_RM 0x00020000

/**
Macro to specify <b> First Hardware I2C Bus </b>
*/
#define I2C_TYPE_SCB0   (I2C_TYPE_SCB|0)

/**
Macro to specify <b> Second Hardware I2C Bus </b>
*/
#define I2C_TYPE_SCB1   (I2C_TYPE_SCB|1)

/**
Macro to specify <b> Third Hardware I2C Bus </b>
*/
#define I2C_TYPE_SCB2   (I2C_TYPE_SCB|2)

/**
Macro to specify <b> Forth Hardware I2C Bus </b>
*/
#define I2C_TYPE_SCB3   (I2C_TYPE_SCB|3)

/**
Macro to specify <b> First Software I2C Bus </b>
*/
#define I2C_TYPE_GPIO0  (I2C_TYPE_GPIO|0)

/**
Macro to specify <b> Second Software I2C Bus </b>
*/
#define I2C_TYPE_GPIO1  (I2C_TYPE_GPIO|1)

/**
Macro to specify <b> Third Software I2C Bus </b>
*/
#define I2C_TYPE_GPIO2  (I2C_TYPE_GPIO|2)

/**
Macro to specify <b> Fourth Software I2C Bus </b>
*/
#define I2C_TYPE_GPIO3  (I2C_TYPE_GPIO|3)

/*******************************Global Type List*******************************/

/**
Enum to specify specifies the <b> Signal Modulation Type </b>, which @a must be
set when opening and connecting NIM Module
*/
typedef enum aui_nim_demod_type {

    /**
    Value to specify that @b QPSK modulation is used by <b> DVB-S/DVB-S2 </b>
    system
    */
    AUI_NIM_QPSK,

    /**
    Value to specify that @b QAM modulation is used by <b> DVB-C </b> system
    */
    AUI_NIM_QAM,

    /**
    Value to specify that @b OFDM modulation is used by <b> DVB-T/DVB-T2 </b>
    system
    */
    AUI_NIM_OFDM

} aui_nim_demod_type, aui_demod_type;

/**
Enum to specify the <b> Demodulator Lock Status </b>.
*/
typedef enum aui_nim_lock_status {

    /**
    Value to specify that the demodulator is <b> Not Locked </b>
    */
    AUI_NIM_STATUS_UNLOCKED,

    /**
    Value t specify that the demodulator is <b> Locked </b>
    */
    AUI_NIM_STATUS_LOCKED,

    /**
    Value to specify that the demodulator lock status is <b> Unknown </b>
    */
    AUI_NIM_STATUS_UNKNOWN

} aui_nim_lock_status, aui_lock_status;

/**
Enum to specify the <b> Guard Interval Time Period </b> (given as fractions of
a Symbol Period) for <b> DVB-T system </b>
*/
typedef enum aui_nim_guard_inter {

    /**
    Value to specify the <b> auto mode </b> for the Guard Interval Time Period

    @note  This mode can be set when connecting NIM Module
    */
    AUI_NIM_GUARD_INTER_AUTO,

    /**
    Value to specify the fraction @b 1/4
    */
    AUI_NIM_GUARD_INTER_1_4,

    /**
    Value to specify the fraction @b 1/8
    */
    AUI_NIM_GUARD_INTER_1_8,

    /**
    Value to specify the fraction @b 1/16
    */
    AUI_NIM_GUARD_INTER_1_16,

    /**
    Value to specify the fraction @b 1/32
    */
    AUI_NIM_GUARD_INTER_1_32

} aui_nim_guard_inter, aui_guard_inter;

/**
Enum to specify the <b> Fixed-Point Fast Fourier Transform (FFT) Algorithm </b>
for <b> DVB-T system </b>
*/
typedef enum aui_nim_fft_mode {

    /**
    Value to specify the <b> auto mode </b> for the Fixed-Point FFT Algorithm

    @note   This mode can be set when connecting NIM Module
    */
    AUI_NIM_FFT_MODE_AUTO,

    /**
    Value to specify the @b 2K-point FFT Algorithm
    */
    AUI_NIM_FFT_MODE_2K,

    /**
    Value to specify the @b 4K-point FFT Algorithm
    */
    AUI_NIM_FFT_MODE_4K,

    /**
    Value to specify the @b 8K-point FFT Algorithm
    */
    AUI_NIM_FFT_MODE_8K

} aui_nim_fft_mode, aui_fft_mode;

/**
Enum to specify the <b> Coding Rate </b> (given as fractions of a Symbol Period)
for the <b> Forward Error Correction (FEC) </b>
*/
typedef enum aui_nim_fec {

    /**
    Value to specify the <b> auto mode </b> for the Coding Rate for the FEC

    @note   This mode can be set when connecting NIM Module
    */
    AUI_NIM_FEC_AUTO,

    /**
    Value to specify the fraction @b 1/2
    */
    AUI_NIM_FEC_1_2,

    /**
    Value to specify the fraction @b 2/3
    */
    AUI_NIM_FEC_2_3,

    /**
    Value to specify the fraction @b 3/4
    */
    AUI_NIM_FEC_3_4,

    /**
    Value to specify the fraction @b 5/6
    */
    AUI_NIM_FEC_5_6,

    /**
    Value to specify the fraction @b 7/8
    */
    AUI_NIM_FEC_7_8

} aui_nim_fec, aui_fec;

/**
Enum to specify the <b> Modulation Type </b> for <b> DVB-T system </b>
*/
typedef enum aui_nim_modulation_type {

    /**
    Value to specify the <b> auto mode </b> for the modulation type for
    DVB-T system

    @note   This mode can be set when connecting NIM Module
    */
    AUI_NIM_MODUL_AUTO,

    /**
    Value to specify the @b DQPSK modulation
    */
    AUI_NIM_MODUL_DQPSK,

    /**
    Value to specify the @b QPSK modulation
    */
    AUI_NIM_MODUL_QPSK,

    /**
    Value to specify the @b 16QAM modulation
    */
    AUI_NIM_MODUL_16QAM,

    /**
    Value to specify the @b 64QAM modulation
    */
    AUI_NIM_MODUL_64QAM

} aui_nim_modulation_type, aui_modulation;

/**
Enum to specify the <b> Spectrum Type </b> for the <b> Front End </b> of
NIM Module
*/
typedef enum aui_nim_spectrum_type {

    /**
    Value to specify the <b> auto mode </b> for the Spectrum Type for the
    Front End

    @note   This mode can be set when connecting NIM Module
    */
    AUI_NIM_SPEC_AUTO,

    /**
    Value to specify the <b> Normal Spectrum </b>
    */
    AUI_NIM_SPEC_NORM,

    /**
    Value to specify the <b> Inverted Spectrum </b>
    */
    AUI_NIM_SPEC_INVERT

} aui_nim_spectrum_type, aui_spectrum;

/**
Enum to specify the different types of <b> Digital Broadcasting Delivery 
System </b> which are supported by NIM Module
*/
typedef enum aui_nim_std {

    /**
    Value to specify the <b> Integrated Services Digital Broadcasting </b>

    @note   This terrestrial television standard is mostly used in @b Japan
    */
    AUI_NIM_STD_ISDBT,

    /**
    Value to specify the <b> Digital Video Broadcasting </b>.

    @note   This terrestrial television standard is mostly used in
            <b> European countries </b>
    */
    AUI_NIM_STD_DVBT,

    /**
    Value to specify the <b> Digital Video Broadcasting Terrestrial -
    T2 Type </b>

    @note   This terrestrial television standard is mostly used in
            <b> European countries </b>
    */
    AUI_NIM_STD_DVBT2,

    /**
    Value to specify the <b> Digital Video Broadcasting </b> with automatic
    detection between @b DVBT & @b DVBT2
    */
    AUI_NIM_STD_DVBT2_COMBO,

    /**
    Value to specify the <b> Digital Video Broadcasting Satellite </b>.
    */
    AUI_NIM_STD_DVBS,

    /**
    Value to specify the <b> Digital Video Broadcasting Satellite -S2 Type</b>.
    */
    AUI_NIM_STD_DVBS2,

    /**
    Value to specify the <b> Digital Video Broadcasting Cable </b> follows the
    <b> ITU-T J.83 Annex A/C (DVB-C2) </b> standard
    */
    AUI_NIM_STD_DVBC_ANNEX_AC,

    /**
    Value to specify the <b> Digital Video Broadcasting Cable </b> follows the
    <b> ITU-T J.83 Annex B (USA) </b> standard
    */
    AUI_NIM_STD_DVBC_ANNEX_B,
	
    /**
    Value to specify that it is not a delivery type
    */
    AUI_NIM_STD_OTHER = (-1)

} aui_nim_std;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify a list of
        miscellaneous information about the DVB-T/ISDBT signal status
        </div> @endhtmlonly

        Struct to specify the list of <b> miscellaneous information </b> about
        the <b> DVB-T/ISDBT Signal Status </b>
*/
typedef struct aui_signal_dvbt_status {

    /**
    Member to specify the <b> Coding Rate </b> for the <b> Forward Error
    Correction (FEC) Rate </b> as defined in the enum #aui_nim_fec
    */
    aui_nim_fec fec;

    /**
    Member to specify the <b> Fixed-Point Fast Fourier Transform (FFT) Algorithm
    </b> for <b> DVB-T system </b> as defined in the enum #aui_nim_fft_mode
    */
    aui_nim_fft_mode fft_mode;

    /**
    Member to specify the <b> Modulation Type </b> for <b> DVB-T system </b> as
    defined in the enum #aui_nim_modulation_type
    */
    aui_nim_modulation_type modulation;

    /**
    Member to specify the <b> Guard Interval Time Period </b> for <b> DVB-T </b>
    system as defined in the enum #aui_nim_guard_interval
    */
    aui_nim_guard_inter guard_inter;

    /**
    Member to specify the <b> Spectrum Type </b> for the <b> Front End </b> of
    NIM Module as defined in the enum #aui_nim_spectrum_type
    */
    aui_nim_spectrum_type spectrum;

} aui_signal_dvbt_status;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify a list of
        miscellaneous information about DVB-T2 signal status
        </div> @endhtmlonly

        Struct to specify a list of miscellaneous information about DVB-T2
        signal status
*/
typedef struct aui_signal_dvbt2_status {

    /**
    Member to specify the <b> Coding Rate </b> for the <b> Forward Error
    Correction (FEC) Rate </b> as defined in the enum #aui_fec
    */
    aui_fec fec;

    /**
    Member to specify the <b> Fixed-Point Fast Fourier Transform (FFT)
    Algorithm </b> for <b> DVB-T system </b> as defined in the enum
    #aui_fft_mode
    */
    aui_fft_mode fft_mode;

    /**
    Member to specify the <b> modulation type </b> for <b> DVB-T
    system </b> as defined in the enum #aui_modulation
    */
    aui_modulation modulation;

    /**
    Member to specify the <b> Guard Interval Time Period </b> for <b> DVB-T
    system </b> as defined in the enum #aui_nim_guard_inter
    */
    aui_nim_guard_inter guard_inter;

    /**
    Member to specify the <b> Spectrum Type </b> for the <b> Front End </b>
    of NIM Module as defined in the enum #aui_spectrum
    */
    aui_spectrum spectrum;

    /**
    Member to specify the <b> Number of PLP </b> in the stream
    */
    int plp_num;

    /**
    Member to specify the <b> Current PLP ID </b>
    */
    int plp_id;

    /**
    Member to specify the current data PLP index
    */
    int plp_index;

    /**
    Member to specify the <b> T2 system ID </b> to be checked
    */
    int system_id;

    /**
    Member to specify the <b> Transport Stream (TS) ID </b>
    */
    int network_id;

    /**
    Member to specify the <b> Original Network ID (ONID) </b>
    */
    int trans_stream_id;

} aui_signal_dvbt2_status;

/**
@brief      @htmlonly <div class="details">
            Struct of the <b> Net Interface (NIM) Module </b> to specify a list
            of miscellaneous information about signal status
            </div> @endhtmlonly

            Struct to specify a list of miscellaneous information about signal
            status

@warning    This struct can be used after the signal is locked
*/
typedef struct aui_signal_status {

    /**
    Member to specify the <b> Signal Frequency </b>

    @note The unit depends of the system type as below:

    @note @b 1. In @b satellite systems, the unit is <b> 1.0 MHZ </b> \n
                - @b Example: @b 1310 means <b> 1310 x 1.0 MHz </b> = 1310 MHz

    @note @b 2. In @b cable systems, the unit is <b> 0.01 MHz </b> \n
                - @b Example: @b 68200 means <b> 68200 x 0.01 MHz </b> 682.00 MHz

    @note @b 3. In @b terrestrial systems, the unit is <b> 0.001 MHz </b> \n
                - @b Example: @b 546000 means <b> 546000 x 0.001 MHz </b> = 546.000 MHz
    */
    unsigned long ul_freq;

    /**
    Member to specify the <b> Signal Strength </b> in @b % unit \n\n

    @b Example: @b 80 means the signal strenght is @b 80%

    */
    unsigned long ul_signal_strength;

    /**
    Member to specify the <b> Signal Quality </b> in @b % unit \n\n

    @b Example: @b 80 means the signal quality is @b 80%
    */
    unsigned long ul_signal_quality;

    /**
    Member to specify the <b> Bit Error Ratio (BER)</b> in <b> 1.0E-7 </b> (i.e. 10^-7) unit \n\n

    @b Example: @b 47 means the BER is <b> 47 x 1.0E-7 </b> = 4.7E-6
    */
    unsigned long ul_bit_error_rate;

    /**
    Member to specify the <b> RF Signal Level </b> in <b> -0.1dBm </b> unit

    @b Example: @b 302 means the RF signal level is <b> 302 x (-0.1dBm) </b> = -30.2dBm
    */
    unsigned long ul_rf_level;

    /**
    Member to specify the <b> Carrier-to-Noise Ratio (CNR) </b> in <b> -0.01dB </b> unit \n\n

    @b Example: @b 2010 means the CNR is <b> 2010 x (-0.01dB) </b> = 20.10dB
    */
    unsigned long ul_signal_cn;

    /**
    Member to specify the <b> Modulation Error Ratio (MER) </b> in <b> 0.1dB </b> unit \n\n

    @b Example: @b 126 means the MER is <b> 126 x 0.1dB </b> = 12.6dB
     */
    unsigned long ul_mer;

    /**
    Member to specify <b> Digital Terrestrial Television Broadcasting Standard
    </b> as defined in the enum #aui_nim_std

    @warning    This member can be used in case the locked signal is a digital
                terrestrial broadcasting
    */
    aui_nim_std std;

    /**
    Member to specify the whole collection of signal information available,
    i.e. either @b DVB-T/ISDBT or @b  DVB-T2 signal status information
    */
    union {

        /**
        Sub-Member to specify a list of miscellaneous information about the
        @b DVB-T/ISDBT signal status as defined in the struct
        #aui_signal_dvbt_status
        */
        aui_signal_dvbt_status dvbt_isdbt;

        /**
        Sub-Member to specify a list of miscellaneous information about
        @b DVB-T2 signal status as defined in the struct
        #aui_signal_dvbt2_status
        */
        aui_signal_dvbt2_status dvbt2;
    } u;

    /**
    Member to specify the <b> Packet Error Ratio (PER) </b> in <b> 1E-5 unit </b>\n\n

    @b Example: 126 --> 126E-5 = 126/10000 = 0.0126
    */
    unsigned long ul_per;

    /**
    Member to specify the <b> BER befor error correction</b> in <b> 1.0E-7 </b> (i.e. 10^-7) unit \n\n

    @b Example: @b 47 means the pre ber is <b> 47 x 1.0E-7 </b> = 4.7E-6
    */
    unsigned long ul_pre_ber;
} aui_signal_status;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify all
        attributes of NIM Module used when opening that module then configure it
        </div> @endhtmlonly

        Struct to specify all <b> attributes of NIM Module </b> used when opening
        and configure it
*/
typedef struct aui_nim_attr {

    /**
    Member to specify the <b> Device ID </b>, which value belong to the range
    <b> [ 0; #AUI_NIM_HANDLER_MAX - 1] </b>
    */
    unsigned long ul_nim_id;

    /**
    Member to specify the <b> Signal Modulation Type </b> used by NIM Module,
    as defined in the enum #aui_demod_type
    */
    aui_demod_type en_dmod_type;

    /**
    Member to specify the
    <b> Digital Television Broadcasting Standards </b>,
    as defined in the enum #aui_nim_std

    @note  As quick reminder, a NIM device can be initialized to support the
           standards as per the enum values below:
           - @b AUI_STD_ISDBT       = Support ISDB-T Only
           - @b AUI_STD_DVBT        = Support DVB-T only
           - @b AUI_STD_DVBT2       = Support DVB-T2 only
           - @b AUI_STD_DVBT2_COMBO = Support DVB-T and DVB-T2

    @note  For TDS systems, the setting of those standards is performed during
           the hardware initialization phase. Please take a look at the sample
           code for more clarifications.

    @warning   When DVBT is enabled, the flag @b COFDM_TUNER_CONFIG_DATA of the
               funtion @b front_end_t_cfg located in the file @b board_config.c
               @b must be set correctly
    */
    aui_nim_std en_std;

} aui_nim_attr, *aui_pnim_attr;

/**
Enum to specify the <b> Signal Polarization Type </b> of @b DVB-S/S2 system
*/
typedef enum aui_nim_polar {

    /**
    Value to specify <b> Horizontal Polarization (18V) </b>
    */
    AUI_NIM_POLAR_HORIZONTAL = 0,

    /**
    Value to specify <b> Vertical Polarization (13V) </b>
    */
    AUI_NIM_POLAR_VERTICAL,

    /**
    Value to specify <b> Left Hand Polarization (18V) </b>
    */
    AUI_NIM_CPOLAR_LEFT = 0,

    /**
    Value to specify <b> Right Hand Polarization (13V) </b>
    */
    AUI_NIM_CPOLAR_RIGHT

} aui_nim_polar;

/**
Enum to specify whether <b> 22KHz Tone </b> (i.e. @b Ku-Band) of @b DVB-S/S2
system is On or Off.

This enum is used to switch the High/Low band of the dual-band LNB block of
the reception chain.
*/
typedef enum aui_nim_22k_status {

    /**
    Value to specify that the 22KHz Tone is @b Off
    */
    AUI_NIM_22K_OFF = 0,

    /**
    Value to specify that the 22KHz Tone is @b ON
    */
    AUI_NIM_22K_ON   = 1,

} aui_nim_22k_status, aui_nim_22k_e;

/**
Enum to specify the <b> High/Low Band </b> (i.e. @b Ku-Band) of @b DVB-S/S2
system
*/
typedef enum aui_nim_freq_band {

    /**
    Value to specify that the High/Low Band will not be set when connecting
    NIM Module
    */
    AUI_NIM_BAND_INVALID = 0,

    /**
    Value to specify the <b> Low Band </b>
    */
    AUI_NIM_LOW_BAND,

    /**
    Value to specify the <b> High Band </b>
    */
    AUI_NIM_HIGH_BAND

} aui_nim_freq_band;

/**
Enum to specify whether <b> LNB Power </b> of @b DVB-S/S2 system is On or Off.
*/
typedef enum aui_nim_lnb_power_status {

    /**
    Value to specify that the LNB Power is @b Off
    */
    AUI_NIM_LNB_POWER_OFF = 0,

    /**
    Value to specify that the LNB Power is @b ON
    */
    AUI_NIM_LNB_POWER_ON  = 1

} aui_nim_lnb_power_status;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify some
        attributes of DVB-S/S2 system
        </div> @endhtmlonly

        Struct to specify <b> some attributes </b> of @b DVB-S/S2 system
*/
typedef struct aui_sat_param {

    /**
    Member to specify the <b> Symbol Rate </b> (in @a MSps unit)
    */
    unsigned long       ul_symrate;

    /**
    Member to specify the <b> Forward Error Correction (FEC) Rate </b> as defined
    in the enum #aui_fec
    */
    aui_fec     fec;

    /**
    Member to specify <b> High/Low Band </b> of @b DVB-S/S2 system as defined
    in the enum #aui_nim_freq_band

    @note   If this member is set as #AUI_NIM_BAND_INVALID
            - The High/Low Band and Polarity and Satellite Source will not be
              configured when connecting NIM module, i.e in the function
              #aui_nim_connect
            - The 22KHz Tone can be set after opening NIM Module, i.e. after
              using the function #aui_nim_open
            - The Signal Polarity Type can be set after opening NIM Module,
              i.e. after using the function #aui_nim_open
            - The Satellite Source can be set after opening NIM Module, i.e.
              after using the function #aui_nim_open
    */
    aui_nim_freq_band   ul_freq_band;

    /**
    Member to specify <b> Signal Polarization Type </b> of @b DVB-S/S2 system
    as defined in the enum #aui_nim_polar

    @note   This member will be ignored when the member @b ul_freq_band of this
            struct is set as #AUI_NIM_BAND_INVALID
    */
    aui_nim_polar       ul_polar;

    /**
    Member as a @a flag to specify the <b> Satellite Source </b>, in particular
    - @b 0 = Source A
    - @b 1 = Source B

    @note   This member will be ignored when the member @b ul_freq_band of this
            struct is set as #AUI_NIM_BAND_INVALID
    */
    unsigned long       ul_src;

    /**
    Member to specify the <b> Sub-Stream ID </b> of @b DVB-S2 system

    @note   @b 1. DVB-S2 supports the transmission of several streams on a single
                  transport stream.
    @note   @b 2. For DVB-S2, the valid sub-stream ID range is [0,255].
    */
    unsigned long       ul_stream_id;

} aui_sat_param;

/**
Enum to specify the <b> QAM Demodulation Type </b> for @b DVB-C System
*/
typedef enum aui_nim_qam_mode {

    /**
    Value to specify the <b> Auto Mode </b> for the DVB-C Demodulation QAM Type
    that can be used when connecting NIM Module
    */
    AUI_NIM_NOT_DEFINED,

    /**
    Value to specify the @b 16QAM modulation
    */
    AUI_NIM_QAM16,

    /**
    Value to specify the @b 32QAM modulation
    */
    AUI_NIM_QAM32,

    /**
    Value to specify the @b 64QAM modulation
    */
    AUI_NIM_QAM64,

    /**
    Value to specify the @b 128QAM modulation
    */
    AUI_NIM_QAM128,

    /**
    Value to specify the @b 256QAM modulation
    */

    AUI_NIM_QAM256

} aui_nim_qam_mode, aui_qam_mode;

/**
Enum to specify the <b> Frequency Bandwidth </b> of the
either <b> Digital Terrestrial Television Broadcasting Signal </b>
or <b> Digital Cable Television Broadcasting Signal </b>
*/
typedef enum aui_nim_bandwidth {

    /**
    Value to specify the <b> Default Frequency Bandwidth </b> in cable devices

    @warning The auto bandwidth mode is not supported in Terrestrial Devices
    */
    AUI_NIM_BANDWIDTH_AUTO,

    /**
    Value to specify that the Frequency Bandwidth is @b 6MHz
    */
    AUI_NIM_BANDWIDTH_6_MHZ,

    /**
    Value to specify that the Frequency Bandwidth is @b 7MHz
    */

    AUI_NIM_BANDWIDTH_7_MHZ,

    /**
    Value to specify that the Frequency Bandwidth is @b 8MHz
    */
    AUI_NIM_BANDWIDTH_8_MHZ

} aui_nim_bandwidth, aui_bandwidth;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify some
        attributes of DVB-C system
        </div> @endhtmlonly

        Struct to specify <b> some attributes </b> of @b DVB-C system
*/
typedef struct aui_cab_param {

    /**
    Member to specify the <b> Symbol Rate </b> (in @a MSps unit)
    */
    unsigned long ul_symrate;

    /**
    Member to specify the <b> DVB-C Demodulation QAM Type </b> as defined in
    the enum #aui_qam_mode
    */
    aui_qam_mode en_qam_mode;

    /**
    Member to specify the <b> Frequency Bandwidth </b> as defined in the enum
    #aui_bandwidth

    @note This member needs to be set in some special situations, for example in
          J83A with 6MHz band
    */
    aui_bandwidth   bandwidth;

} aui_cab_param;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify some
        attributes of DVB-T2 system
        </div> @endhtmlonly

        Struct to specify <b> some attributes </b> of @b DVB-T2 system
*/
typedef struct aui_dvbt2_param {

    /**
    Member to specify the <b> PLP Index </b> to be locked
    */
    int plp_index;

    /**
    Member to specify the <b> PLP ID </b> to be locked
    */
    int plp_id;

} aui_dvbt2_param;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify some
        attributes of DVB-T/T2 system
        </div> @endhtmlonly

        Struct to specify <b> some attributes </b> of @b DVB-T/T2 system
 */
typedef struct aui_ter_param {

    /**
    Member to specify the <b> Frequency Bandwidth </b> as defined in the enum
    #aui_bandwidth
    */
    aui_bandwidth   bandwidth;

    /**
    Member to specify <b> Forward Error Correction (FEC) Rate </b> as defined
    in the enum #aui_fec
    */
    aui_fec         fec;

    /**
    Member to specify the <b> Guard Interval Time Period </b> as defined in the
    enum #aui_nim_guard_inter
    */
    aui_nim_guard_inter guard_interval;

    /**
    Member to specify the <b> Fixed-Point Fast Fourier Transform (FFT) Algorithm
    </b> as defined in the enum #aui_fft_mode
    */
    aui_fft_mode    fft_mode;

    /**
    Member to specify the <b> Spectrum Type </b> for the <b> Front End </b> of
    NIM Module to check with the enum #aui_spectrum
    */
    int             spectrum;

    /**
    Member to specify the <b> modulation type </b> for @b DVB-T system as
    defined in the enum #aui_modulation
    */
    aui_modulation  modulation;

    /**
    Member to specify the different types of <b> Digital Terrestrial Television
    Broadcasting Standards </b> as defined in the enum #aui_nim_std
    */
    aui_nim_std    std;

    /**
    Member to collect the whole collection of the current PLP Index

    @warning    This member is used @a only when NIM Module is opened in
                @b DVBT2 or @b DVBT2_COMBO system mode
    */
    union {

        /**
        Sub-Member to specify some attributes of @b DVB-T2 system as defined
        in the struct #aui_dvbt2_param
        */
        aui_dvbt2_param dvbt2;

    } u;

} aui_ter_param;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify the
        Connection Attributes of NIM Module
        </div> @endhtmlonly

        Struct to specify the <b> connection attributes </b> of NIM Module
*/
typedef struct aui_nim_connect_param {

    /**
    Member to specify the <b> Signal Frequency </b> (in @b MHz unit)
    */
    unsigned int ul_freq;

    /**
    Member to specify the <b> Signal Modulation Type </b> as defined in the
    enum #aui_demod_type
    */
    aui_demod_type en_demod_type;

    /**
    Member to specify the whole collection of signal attributes available for
    @b DVB-T/ISDBT, @b DVB-C, @b DVB-S/S2 System
    */
    union {

        /**
        Member to specify some signal attributes of @b DVB-C system as defined
        in the struct #aui_cab_param
        */
        aui_cab_param cab;

        /**
        Member to specify some signal attributes of @b DVB-T/T2 and @b ISDB-T
        systems as defined in the struct #aui_ter_param

        @note  When using the function #aui_nim_open, if the NIM device is
               initialized with the Digital Terrestrial Television Broadcasting
               Standard #AUI_STD_DVBT2_COMBO defined in the enum #aui_ter_std,
               this member will specify a certain terrestrial signal to be
               connected for each standards as below:
               - @b AUI_STD_DVBT connect the DVB-T signal only
               - @b AUI_STD_DVBT2 connect the DVB-T/T2 signal only
               - @b AUI_STD_DVBT2_COMBO connect the DVB-T2 (@b Caution: If the connection
                 fails, this standard will connect the DVB-T signal)
        */
        aui_ter_param ter;

        /**
        Member to specify some some signal attributes of @b DVB-S/S2 system as
        defined in the struct #aui_sat_param
        */
        aui_sat_param sat;

    } connect_param;

} aui_nim_connect_param, *aui_p_nim_connect_param;

/**
Function pointer to specify the type of callback function registered with the
function #aui_nim_auto_scan_start to get the Transponder Information when a
transponder is found

@note       Some important details about the input parameters of this function
            pointer are summarized below:
            - The input parameter @a uc_status is a @a flag which can get the
              following value:
              - @b 0 = Unlocked
              - @b 1 = Locked
            - The input parameter @b uc_polar will get the <b> signal polarization
              type </b> as defined in the enum #aui_nim_polar
            - The input parameter @b u_freq is the <b> Transponder Central
              Frequency </b> (in @a MHz unit)
            - The input parameter @b u_sym is the <b> Symbol Rate </b>
              (in @a KS/s unit))
            - The input parameter @b uc_fec is the <b> Forward Error Correction
              (FEC) Rate </b> as defined in the enum #aui_fec.
            - The input parameter @b pv_user_data is the <b> user data </b> set
              in the struct #aui_autoscan_sat

@warning    The type of callback function referred by this function pointer is
            used @a only for DVB-S/S2 system
*/
typedef int (*aui_autoscan_callback) (

    unsigned char uc_status,

    unsigned char uc_polar,

    unsigned int u_freq,

    unsigned int u_sym,

    unsigned char uc_fec,

    void *pv_user_data,

    unsigned int u_stream_id

    );

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify the
        information requested for a DVB-S/S2 Auto-Scan
        </div> @endhtmlonly

        Struct to specify the information requested for a <b> DVB-S/S2 Auto-Scan </b>
*/
typedef struct aui_autoscan_sat {

    /**
    Member to specify the <b> Auto Scan Start Frequency </b> (in @a MHz unit)
    */
    unsigned long       ul_start_freq;

    /**
    Member to specify the <b> Auto Scan Stop Frequency </b> (in @a MHz unit)
    */
    unsigned long       ul_stop_freq;

    /**
    Member to specify the <b> High/Low Band </b> (i.e. @b Ku-Band) of
    @b DVB-S/S2 system as defined in the enum #aui_nim_freq_band

    @note   If that member is set as #AUI_NIM_BAND_INVALID then the frequency
    band can be controlled by using the function #aui_nim_set22k_onoff.
    */
    aui_nim_freq_band   ul_freq_band;

    /**
    Member to specify the <b> Signal Polarization Type </b> of @b DVB-S/S2
    system as defined in the enum #aui_nim_polar.

    @note   If the member @b ul_freq_band of this struct is set as
    #AUI_NIM_BAND_INVALID then this member will be ignored and the function
    #aui_nim_set_polar can be used to control the polarization
    */
    aui_nim_polar       ul_polar;

    /**
    Member as a @a flag to specify the <b> Satellite Source </b>, in particular
    - @b 0 = Source A
    - @b 1 = Source B

    @note   This member will be ignored when the member @b ul_freq_band of this
            struct is set as #AUI_NIM_BAND_INVALID
    */
    unsigned long       ul_src;

    /**
    Member to specify the <b> user data </b> of the DVB-S/S2 callback function
    (i.e. of the member @b aui_as_cb of the struct #aui_autoscan_sat),
    which will be passed as the input parameter @b pv_user_data of the function
    pointer #aui_autoscan_callback
    */
    void                *pv_user_data;

    /**
    Member to specify the <b> DVB-S/S2 Callback Function </b> which is called
    each time a channel is found, as per comment for the function pointer
    #aui_autoscan_callback
    */
    aui_autoscan_callback  aui_as_cb;

} aui_autoscan_sat;

/**
Enum to specify the <b> Digital Satellite Equipment Control (DiSEqC) messages
</b> to send
*/
typedef enum aui_diseqc_modes {

    /**
    Value to specify that the <b> 22KHz Tone Tuning </b> is @b Off
    */
    AUI_DISEQC_MODE_22KOFF = 0,

    /**
    Value to specify that the <b> 22KHz Tone Tuning </b> is @b On
    */
    AUI_DISEQC_MODE_22KON,

    /**
    Value to specify that the <b> '0' Tone-Burst </b> is used for selecting <b>
    Satellite A </b>
    */
    AUI_DISEQC_MODE_BURST0,

    /**
    Value to specify that the <b> '1' Tone-Burst </b> is used for selecting <b>
    Satellite B </b>
    */
    AUI_DISEQC_MODE_BURST1,

    /**
    Value to specify the <b> Byte Mode </b>, i.e. modulated with bytes from <b>
    DiSEqC INSTR </b>
    */
    AUI_DISEQC_MODE_BYTES,

    /**
    Value to specify that the <b> Envelop Mode </b> is @b enabled
    */
    AUI_DISEQC_MODE_ENVELOP_ON,

    /**
    Value to specify that the <b> Envelope Mode </b> is @b disabled and the
    output is a @b 22KHz waveform
    */
    AUI_DISEQC_MODE_ENVELOP_OFF,

    /**
    Value to specify an <b> Undefined Mode </b>
    */
    AUI_DISEQC_MODE_OTHERS,

    /**
    Value to specify the first of two steps in which the <b> Byte Mode </b>
    is split in order to improve the speed to fit some standard specification
    */
    AUI_DISEQC_MODE_BYTES_EXT_STEP1,

    /**
    Value to specify the second of two steps in which the <b> Byte Mode </b>
    is split in order to improve the speed to fit some standard specification
    */
    AUI_DISEQC_MODE_BYTES_EXT_STEP2,

    /**
    Value to specify the total number of DiSEqC types contained in this enum
    */
    AUI_DISEQC_MODE_NUM

} aui_diseqc_modes;

/**
Enum to specify the @b ID for different <b> Model of Demodulators </b> which
can be used by a platform
*/
typedef enum aui_nim_id {

    /**
    Value to specify the @a first <b> ALi M3501 </b> chip demodulator
    */
    AUI_NIM_ID_M3501_0,

    /**
    Value to specify the @a second <b> ALi M3501 </b> chip demodulator
    */
    AUI_NIM_ID_M3501_1,

    /**
    Value to specify the @a first <b> ALi M3503 </b> chip demodulator
    */
    AUI_NIM_ID_M3503_0,

    /**
    Value to specify the @a second <b> ALi M3503 </b> chip demodulator
    */
    AUI_NIM_ID_M3503_1,

    /**
    Value to specify the @a first <b> ALi M3200 </b> chip demodulator
    */
    AUI_NIM_ID_M3200_0,

    /**
    Value to specify the @a second <b> ALi M3200 </b> chip demodulator
    */
    AUI_NIM_ID_M3200_1,

    /**
    Value to specify the @a first <b> ALi M3281 </b> chip demodulator
    */
    AUI_NIM_ID_M3281_0,

    /**
    Value to specify the @a second <b> ALi M3281 </b> chip demodulator
    */
    AUI_NIM_ID_M3281_1,

    /**
    Value to specify the @a first <b> TDA10025 </b> tuner/demodulator
    */
    AUI_NIM_ID_10025_0,

    /**
    Value to specify the @a second <b> TDS10025 </b> tuner/demodulator
    */
    AUI_NIM_ID_10025_1,

    /**
    Value to specify the @b S3821 demodulator
    */
    AUI_NIM_ID_S3821_0,

    /**
    Value to specify the @a first <b> Hercules </b> demodulator
    */
    AUI_NIM_ID_HERCULES_0,

    /**
    Value to specify the @a second <b> Hercules </b> demodulator
    */
    AUI_NIM_ID_HERCULES_1,

    /**
    Value to specify the @a third <b> Hercules </b> demodulator
    */
    AUI_NIM_ID_HERCULES_2,

    /**
    Value to specify the @a fourth <b> Hercules </b> demodulator
    */
    AUI_NIM_ID_HERCULES_3,

    /**
    Value to specify the <b> MXL603 </b> demodulator
    */
    AUI_NIM_ID_HD2818_MXL603_0,

    /**
    Value to specify the @a third <b> ALi M3501 </b> chip demodulator
    */
    AUI_NIM_ID_M3501_2,

    /**
    Value to specify the @a fourth <b> ALi M3501 </b> chip demodulator
    */
    AUI_NIM_ID_M3501_3,

    /**
    Value to specify the @a first <b> MXL214C </b> demodulator
    */
    AUI_NIM_ID_MXL214C_0,

    /**
    Value to specify the @a second <b> MXL214C </b> demodulator
    */
    AUI_NIM_ID_MXL214C_1,

    /**
    Value to specify the @a third <b> MXL214C </b> demodulator
    */
    AUI_NIM_ID_MXL214C_2,

    /**
    Value to specify the @a fourth <b> MXL214C </b> demodulator
    */
    AUI_NIM_ID_MXL214C_3,

    AUI_NIM_ID_C3505_0,

    AUI_NIM_ID_C3505_1,

    AUI_NIM_ID_CXD2837_0,

	AUI_NIM_ID_C3501H_0,
	
	AUI_NIM_ID_CXD2856_0,
    /**
    Value to specify the <b> total number </b> of demodulator models identified
    in this enum #aui_nim_id
    */
    AUI_NIM_NB

} aui_nim_id;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify the NIM
        GPIO connection attributes
        </div> @endhtmlonly

        Struct to specify the <b> NIM GPIO Connection Attributes </b>
*/
typedef struct aui_gpio_info {

    /**
    Member as a @a flag to specify the <b> GPIO polarity </b>, in particular
    - @b 0 = Active Low
    - @b 1 = Active High
    */
    int gpio_val;

    /**
    Member as a @a flag to specify the <b> GPIO direction </b>, in particular
    - @b 0 = Input
    - @b 1 = Output
    */
    int io;

    /**
    Member as @a Index (which integer values belong to the interval <b> [0;64]
    </b>) refer to different <b> GPIO position </b>.

    @note   The value @b -1 is used to specify <b> no GPIO position </b>
    */
    int position;

} aui_gpio_info;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify the DVB-S
        Demodulator Parameters
        </div> @endhtmlonly

        Struct to specify the <b> DVB-S Demodulator Parameters </b>
*/
typedef struct aui_demod_dvbs_config {

    /**
    Member to specify the <b> Demodulator I2C Base Address </b>
    */
    unsigned long i2c_base_addr;

    /**
    Member to specify the @b ID of the <b> Demodulator I2C Bus </b> as per the
    define #I2C_TYPE_SCBx, #I2C_TYPE_GPIOx
    */
    unsigned long i2c_type_id;

    /**
    Member consisting of a @a byte whose bit are @a flags to specify some <b>
    Demodulator Parameters </b> as below:

    - <b> bit 0 </b> = <b> Frequency Offset </b> (as per the define
                       #QPSK_CONFIG_FREQ_OFFSET)
    - <b> bit 1 </b> = <b> External Analog Digital Converter </b> (as per the
                       define #QPSK_CONFIG_EXT_ADC)
    - <b> bit 2 </b> = <b> I/Q Analog - Digital Signal Swap </b> (as per the
                       define #QPSK_CONFIG_IQ_NORMAL and #QPSK_CONFIG_IQ_SWAP)
    - <b> bit 3 </b> = <b> IC2 Pass-Through Device </b> (as per the define
                       #QPSK_CONFIG_I2C_THROUGH)
    - <b> bit 4 </b> = <b> Polarization Reversed </b> (as per the define
                       #QPSK_CONFIG_POLAR_REVERT)
    - <b> bit 5 </b> = <b> New Automatic Gain Control </b> (as per the define
                       #QPSK_CONFIG_NEW_AGC1)
    - <b> bits 6 - 7 </b> = <b> QPSK Mode </b> , as per the logical combination
                            below:
       - @b 00 = <b> 1 bit Mode </b> (as per the define #QPSK_CONFIG_MODE_1BIT)
       - @b 01 = <b> 2 bit Mode </b> (as per the define #QPSK_CONFIG_MODE_2BIT)
       - @b 10 = <b> 4 bit Mode </b> (as per the define #QPSK_CONFIG_MODE_4BIT)
       - @b 11 = <b> 8 bit Mode </b> (as per the define #QPSK_CONFIG_MODE_8BIT)
    */
    unsigned short QPSK_Config;

} aui_demod_dvbs_config;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify the DVB-C
        demodulator parameters
        </div> @endhtmlonly

        Struct to specify the <b> DVB-C Demodulator Parameters </b>
*/
typedef struct aui_demod_dvbc_config {

    /**
    Member to specify the <b> I2C Base Address of the Demodulator </b>
    */
    unsigned long i2c_base_addr;

    /**
    Member to specify the @b ID of the <b> Demodulator I2C Bus </b> as per define
    #I2C_TYPE_SCBx, #I2C_TYPE_GPIOx
    */
    unsigned long i2c_type_id;

    /**
    Member to specify the <b> QAM Demodulator Type </b>
    Example: AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M
    */
    unsigned int qam_mode;

} aui_demod_dvbc_config;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify the DVB-T
        Demodulator Parameters
        </div> @endhtmlonly

        Struct to specify the <b> DVB-T Demodulator Parameters </b>
*/
typedef struct aui_demod_dvbt_config {

    /**
    Member to specify the <b> IC2 Base Address of the Demodulator </b>
    */
    unsigned long i2c_base_addr;

    /**
    Member to specify the @b ID of the <b> Demodulator I2C Bus </b> as per
    define #I2C_TYPE_SCBx, #I2C_TYPE_GPIOx

    */
    unsigned long i2c_type_id;

    /**
    Member to specify the <b> OFDM Configuration </b>
    */
    unsigned short ofdm_config;

} aui_demod_dvbt_config;


///@coding

/**
Enum to specify the @b ID for different <b> Model of Tuner </b> which can be
used by a platform
*/

typedef enum aui_nim_tuner_type {

    /**
    Value to specify the @b DCT70701 Tuner
    */
    AUI_DCT70701 = 1,

    /**
    Value to specify the @b DCT7044 Tuner
    */
    AUI_DCT7044,

    /**
    Value to specify the @b ALPSTDQE Tuner
    */
    AUI_ALPSTDQE,

    /**
    Value to specify the @b ALPSTDAE Tuner
    */
    AUI_ALPSTDAE,

    /**
    Value to specify the @b TDCCG0X1F Tuner
    */
    AUI_TDCCG0X1F,

    /**
    Value to specify the @b DBCTE702F1 Tuner
    */
    AUI_DBCTE702F1,

    /**
    Value to specify the @b CD1616LF Tuner
    */
    AUI_CD1616LF,

    /**
    Value to specify the @b ALPSTDAC Tuner
    */
    AUI_ALPSTDAC,

    /**
    Value to specify the @b RT810 Tuner
    */
    AUI_RT810,

    /**
    Value to specify the @b VRT820C Tuner
    */
    AUI_VRT820C,

    /**
    Value to specify the @b MXL603 Tuner
    */
    AUI_MXL603,

    /**
    Value to specify the @b VZ7306 Tuner
    */
    AUI_SHARP_VZ7306,

    /**
    Value to specify the @b AV_2012 Tuner
    */
    AUI_AV_2012,

    /**
    Value to specify the @b TDA18250 Tuner
    */
    AUI_TDA18250,

    /**
    Value to specify the @b SANYO Tuner
    */
    AUI_SANYO,

    /**
    Value to specify the @b CD1616LF_GIH Tuner
    */
    AUI_CD1616LF_GIH,

    /**
    Value to specify the @b NXP Tuner
    */
    AUI_NXP,

    /**
    Value to specify the @b MAXLINEAR Tuner
    */
    AUI_MAXLINEAR,

    /**
    Value to specify the @b MICROTUNE  Tuner
    */
    AUI_MICROTUNE,

    /**
    Value to specify the @b QUANTEK Tuner
    */
    AUI_QUANTEK,

    /**
    Value to specify the @b RFMAGIC Tuner
    */
    AUI_RFMAGIC,

    /**
    Value to specify the @b 60120-01Angus Tuner
    */
    AUI_ALPS,

    /**
    Value to specify the @b PHILIPS Tuner
    */
    AUI_PHILIPS,

    /**
    Value to specify the @b INFINEON Tuner
    */
    AUI_INFINEON,

    /**
    Value to specify the @b RDA5815M Tuner
    */
    AUI_RDA5815M,

    /**
    Value to specify the @b MXL214C Tuner
    */
    AUI_MXL214C,

    /**
    Value to specify the <b> M3031 (DVB-S2) </b> Tuner
    */
    AUI_M3031,

    /**
    Value to specify the <b> SI2141 (DVB-T) </b> Tuner
    */
    AUI_SI2141,

    /**
    Value to specify the <b> CXD2872 (DVB-T2) </b> Tuner
    */
    AUI_CXD2872,

    /**
    Value to specify the <b> R858 (DVB-C) </b> Tuner
    */
    AUI_R858,

    /**
    Value to specify the @b TDA18250B or @b TDA18250A Tuner
    */
    AUI_TDA18250AB,
    
    /**
    Value to specify the <b> R836 (DVB-C) </b> Tuner
    */
	AUI_R836

} aui_nim_tuner_type;

///@endcoding

/**
Enum to specify the <b> Sample Clock Frequency of the Demodulator </b>
*/
typedef enum aui_nim_sample_clk {

    /**
    Value to specify that the Sample Clock Frequency is @b 27MHz
    */
    AUI_NIM_DEMO_SAMPLE_CLK_27M = 0,

    /**
    Value to specify that the Sample Clock Frequency is @b 54MHz
    */
    AUI_NIM_DEMO_SAMPLE_CLK_54M = 2

} aui_nim_sample_clk;

/**
Enum to specify the <b> Demodulation Type </b> fro @b DVB-C System
*/
typedef enum aui_nim_dvbc_mode {

    /**
    Value to specify that the modulation scheme follows the
    <b> ITU-T J.83 Annex A/C (DVB-C2) </b> standard
    */
    AUI_NIM_DVBC_MODE_J83AC = 0,

    /**
    Value to specify that the modulation scheme follows the
    <b> ITU-T J.83 Annex B (USA) </b> standard
    */
    AUI_NIM_DVBC_MODE_J83B

} aui_nim_dvbc_mode;

/**
Enum to specify the <b> LNB Type </b> for @b DVB-S System
*/
typedef enum aui_nim_lnb_regulator_id {

    /**
    Value to specify not use LNB regulator
    */
    AUI_NIM_LNB_NONE = 0,

    /**
    Value to specify the <b> A8304 </b> LNB regulator
    */
    AUI_NIM_LNB_A8304,

    /**
    Value to specify the <b> TPS65233 </b> LNB regulator
    */
    AUI_NIM_LNB_TPS65233
	
} aui_nim_lnb_regulator_id;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify the DVB-S
        Tuner Parameters
        </div> @endhtmlonly

        Struct to specify the <b> DVB-S Tuner Parameters </b>
*/

typedef struct aui_tuner_dvbs_config {

    /**
    Member to specify the <b> Lower Boundary Frequency </b> of the Tuner
    */
    unsigned short freq_low;

    /**
    Member to specify the <b> Higher Boundary Frequency </b> of the Tuner
    */
    unsigned short freq_high;

    /**
    Member to specify the <b> I2C Base Address </b> of the Tuner
    */
    unsigned long i2c_base_addr;

    /**
    Member to specify the <b> ID </b> of the tuner I2C bus as per define
    #I2C_TYPE_SCBx, #I2C_TYPE_GPIOx
    */
    unsigned long i2c_type_id;

    /**
    Member to specify the <b> Tuner ID </b>
    */

    unsigned long id;

    /**
    Member to specify the <b> Diseqc Polar GPIO Information </b> as defined in 
    the struct #aui_gpio_info

    @note  If user do not configure this parameter, driver will use to read and write 
           registers to set Diseqc Polar
    */
    aui_gpio_info diseqc_polar_gpio;

} aui_tuner_dvbs_config;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify the DVB-C
        Tuner parameters
        </div> @endhtmlonly

        Struct to specify the <b> DVB-C Tuner parameters </b>
*/
typedef struct aui_tuner_dvbc_config {

    /**
    Member to specify the <b> IC2 Base Address of the Tuner </b>
    */

    unsigned int i2c_base_addr;

    /**
    Member to specify the <b> ID </b> of the tuner I2C bus as per define
    #I2C_TYPE_SCBx, #I2C_TYPE_GPIOx
    */
    unsigned int i2c_type_id;

    /**
    Member to specify the <b> Tuner ID </b> as per definition in the enum
    #aui_nim_tuner_type
    */
    unsigned int id;

    /**
    Member to specify the <b> RF AGC Higher Boundary </b> of the Tuner
    */
    unsigned char rf_agc_max;

    /**
    Member to specify the <b> RF AGC Lower Boundary </b> of the Tuner
    */
    unsigned char rf_agc_min;

    /**
    Member to  specify the <b> IF AGC Higher Boundary </b> of the Tuner
    */
    unsigned char if_agc_max;

    /**
    Member to specify the <b> IF AGC Lower Boundary </b> of the Tuner
    */
    unsigned char if_agc_min;

    /**
    Member to specify the <b> Average AGC </b> of the Tuner
    (i.e. the average amplitude to full scale of A/D in percentage)
    */
    unsigned char agc_ref;

    /**
    Member to specify the <b> Reference Frequency </b> of the Tuner
    */
    unsigned char  tuner_crystal;

    /**
    Member to specify the <b> Chip ID </b> of the Tuner
    */
    unsigned char  chip;

    /**
    Member to specify the <b> Special Configuration </b> of the Tuner,
    in particular
    - @b 0x01 = RF AGC disabled
    */
    unsigned char  tuner_special_config;

    /**
    Member to specify the <b> Reference Div Ratio </b> of the Tuner
    */
    unsigned char  tuner_ref_divratio;

    /**
    Member to specify the <b> IF Frequency </b> of the Tuner
    */
    unsigned short wtuner_if_freq;

    /**
    Member to specify the <b> AGC Higher Boundary </b> of the Tuner
    */
    unsigned char  tuner_agc_top;

    /**
    Member to specify the <b> Step Frequency </b> of the Tuner
    */
    unsigned char  tuner_step_freq;

    /**
    Member to specify the <b> RT810 Standard Type </b> as <b> Frequency
    Parameter </b> of the Tuner
    */
    unsigned char  tunner_freq_param;

    /**
    Member to specify the <b> Re-Opening </b> of the Tuner
    */
    unsigned short tuner_reopen;

    /**
    Member to specify the <b> IF Frequency </b> of the Tuner for
    @b DVB-C(J83A) System
    */
    unsigned short tuner_if_freq_J83A;

    /**
    Member to specify the <b> IF Frequency of the Tuner </b> for
    @b DVB-C(J83B) System
    */
    unsigned short tuner_if_freq_J83B;

    /**
    Member to specify the <b> Frequency </b> of the Tuner for
    @b DVB-C(J83C) System
    */
    unsigned short tuner_if_freq_J83C;

    /**
    Member to specify  the <b> DVB-C(J83A/C) System Type </b>,
    in particular
    - @b 0x00 = J83A
    - @b 0x01 = J83C
    */
    unsigned char  tuner_if_J83AC_type;

} aui_tuner_dvbc_config;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify the DVB-T
        Tuner Parameters
        </div> @endhtmlonly

        Struct to specify the <b> DVB-T Tuner Parameters </b>
*/
typedef struct aui_tuner_dvbt_config {

    /**
    Member to specify <b> Lower Boundary Frequency </b> of the Tuner
    */
    unsigned short freq_low;

    /**
    Member to specify the <b> Higher Boundary Frequency </b> of the Tuner
    */
    unsigned short freq_high;

    /**
    Member to specify the <b> I2C Base Address </b> of the Tuner
    */
    unsigned int   i2c_base_addr;

    /**
    Member to specify the <b> ID </b> of the tuner I2C bus as per define
    #I2C_TYPE_SCBx, #I2C_TYPE_GPIOx
    */
    unsigned int   i2c_type_id;

    /**
    Member to specify the <b> Tuner ID </b> as per definition in the enum
    #aui_nim_tunner_type
    */
    unsigned int   id;

    /**
    Member to specify the <b> IF AGC Higher Boundary </b> of the Tuner
    */
    unsigned char  if_agc_max;

    /**
    Member to specify the <b> IF AGC Lower Boundary </b> of the Tuner
    */
    unsigned char  if_agc_min;

    /**
    Member to specify the <b> Average AGC </b> of the Tuner,
    (i.e. the average amplitude to full scale of A/D in percentage)
    */
    unsigned char  agc_ref;

    /**
    Member to specify the <b> TSI Setting </b> of the Tuner

    @warning    This member is no longer supported then is @a deprecated
    */
    unsigned char  tuner_tsi_setting;

    /**
    Member to specify the <b> Reference Frequency </b> of the Tuner
    */
    unsigned char  tuner_crystal;

    /**
    Member to specify the <b> Chip ID </b> of the Tuner
    */
    unsigned char  chip;

    /**
    Member to specify the <b> Special Configuration </b> of the Tuner,
    in particular
    - @b 0x01 = RF AGC Disabled
    */
    unsigned char  tuner_special_config;

    /**
    Member to specify the <b> IF Frequency </b> of the Tuner
    */
    unsigned short wtuner_if_freq;

    /**
    Member to specify the <b> AGC Higher Boundary </b> of the Tuner
    */
    unsigned char  tuner_agc_top;

    /**
    Member to specify the <b> Digital Terrestrial Television Broadcasting
    Standard Type </b> as per definition in the enum #aui_nim_std
    */
    int std;

} aui_tuner_dvbt_config;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify the DVB-S
        LNB Parameters
        </div> @endhtmlonly

        Struct to specify the <b> DVB-S LNB Parameters </b>
        
@note   If there is no LNB chip on the board, do not need to configure this parameter
*/
typedef struct aui_nim_lnb_regulator_config {

    /**
    Member to specify the <b> ID </b> of the LNB I2C bus as per define
    #I2C_TYPE_SCBx, #I2C_TYPE_GPIOx
    */
    unsigned int i2c_type_id;

    /**
    Member to specify the <b> I2C Base Address </b> of the LNB
    */
    unsigned int i2c_base_addr;

    /**
    Member to specify the <b> LNB id </b> as per definition in the enum
    #aui_nim_lnb_regulator_id
    */
    aui_nim_lnb_regulator_id  id;
    
} aui_nim_lnb_regulator_config;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify the DVB-S
        Parameters
        </div> @endhtmlonly

        Struct to specify the <b> DVB-S Parameters </b>
*/
typedef struct aui_nim_dvbs {

    /**
    Member to specify the <b> DVB-S Demodulator Parameters </b> as defined in
    the struct #aui_demo_dvbs_config
    */
    aui_demod_dvbs_config demod;

    /**
    Member to specify the <b> DVB-S Tuner Parameters </b> as defined in the
    struct #aui_tuner_dvbs_config
    */
    aui_tuner_dvbs_config tuner;

    /**
    Member to specify the <b> DVB-S LNB Parameters </b> as defined in the
    struct #aui_nim_lnb_regulator 
    */
    aui_nim_lnb_regulator_config lnb;

} aui_nim_dvbs;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify the DVB-C
        Parameters
        </div> @endhtmlonly

        Struct to specify the <b> DVB-C Parameters </b>
*/
typedef struct aui_nim_dvbc {

    /**
    Member to specify the <b> DVB-C Demodulator Parameters </b>
    */
    aui_demod_dvbc_config demod;

    /**
    Member to specify the <b> DVB-C Tuner Parameters </b>
    */
    aui_tuner_dvbc_config tuner;

} aui_nim_dvbc;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify the DVB-T
        Parameters
        </div> @endhtmlonly

        Struct to specify the <b> DVB-T Parameters </b>
*/
typedef struct aui_nim_dvbt {

    /**
    Member to specify the <b> DVB-T Demodulator Parameters </b> as defined in
    the struct #aui_demod_dvbt_config
    */
    aui_demod_dvbt_config demod;

    /**
    Member to specify the <b> DVB-T Tuner Parameters </b> as defined in the
    struct #aui_tuner_dvbt_config
    */
    aui_tuner_dvbt_config tuner;

} aui_nim_dvbt;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Net Interface (NIM) Module </b> to specify the
        configuration parameter for each reception chains of NIM Module
        </div> @endhtmlonly

        Struct to specify the <b> Configuration Parameter </b> for each
        reception chains of NIM Module

@note   Please refer to the sample code available for this module for more
        information
*/
typedef struct aui_nim_config {

    /**
    Member to specify the demodulator parameters,
    i.e. <b> DVB-S, DVB-T, DVB-C </b>
    */
    union {

        /**
        Sub-Member to specify the <b> DVB-S Parameters </b> as defined in the
        struct #aui_nim_dvbs
        */
        aui_nim_dvbs dvbs;

        /**
        Sub-Member to specify the <b> DVB-T Parameters </b> as defined in the
        struct #aui_nim_dvbt
        */
        aui_nim_dvbt dvbt;

        /**
        Sub-Member to specify the <b> DVB-C Parameters </b> as defined in the
        struct #aui_nim_dvbc
        */
        aui_nim_dvbc dvbc;

    } config;

    /**
    Member to specify the ID for different <b> Model of Demodulators </b> which
    can be used by a platform as per definition in the enum #aui_nim_id
    */
    unsigned long id;

    /**
    Member to specify the <b> Resetting of GPIO Information </b> as defined in
    the struct #aui_gpio_info
    */
    aui_gpio_info nim_reset_gpio;

    /**
    Member to specify the <b> LNB GPIO Information </b> as defined in the struct
    #aui_gpio_info
    */
    aui_gpio_info lnb_power_gpio;

    /**
    Member to specify the <b>Disable LNB power auto control </b> by the function 
    @b aui_nim_open and @b aui_nim_close
    */
    unsigned int disable_auto_lnb_power_control;

} aui_nim_config;

/*****************************Global Function List*****************************/

/**
@brief          Function used to perform the initialization of the NIM Module,
                before its opening by the function @b aui_nim_open

@warning        This function can be used @a only in the <b> Pre-Run Stage </b>
                of the NIM Module

@param[in]      call_back_init      = Callback function used for the
                                      initialization of the NIM Module, as per
                                      comment for the function pointer
                                      @b p_fun_cb

@return         @b AUI_RTN_SUCCESS  = NIM Module initialized successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Initializing of the NIM Module failed for
                                      some reasons

@note           About the callback function @b call_back_init as input parameter,
                it takes a pointer to a struct #aui_nim_config as input parameter
                to assign all proper data according to the different platforms.\n
                The struct #aui_nim_config will be used to configure the NIM
                Module when opening the NIM Module, i.e. when using the function
                #aui_nim_open.\n
                Please refer to the sample code of the initialization of NIM
                Module for more clarifications
*/
AUI_RTN_CODE aui_nim_init (

    p_fun_cb call_back_init

    );

/**
@brief          Function used to perform the de-initialization of the NIM Module,
                after its closing by the function @b aui_nim_close

@param[in]      call_back_init      = Callback function used to de-initialize of
                                      the NIM Module, as per comment for the
                                      function pointer @b p_fun_cb

@return         @b AUI_RTN_SUCCESS  = NIM Module de-initialized successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = De-initializing of the NIM Module failed
                                      for some reasons

@note           About the callback function @b call_back_init as input parameter,
                it takes a pointer to a struct #aui_nim_config as input parameter
                which @a must be the same as the one mentioned in the function
                #aui_nim_init
*/
AUI_RTN_CODE aui_nim_de_init (

    p_fun_cb call_back_init

    );

/**
@brief          Function used to open the NIM Module, get the related handle
                then configure the desired attributes (as well as configure the
                tuner and demodulator associated to the NIM ID)\n
                The desired attributes has already been prepared when initializing
                NIM Module, i.e. when using the function #aui_nim_init \n
                Please refer to the sample code of the NIM Module for more
                clarifications

@param[in]      p_attr              = Pointer to a struct #aui_nim_attr which
                                      collects all the attributes of the NIM
                                      Module to be opened

@param[out]     p_handle            = #aui_hdl pointer to the handle of the NIM
                                      Module just opened

@return         @b AUI_RTN_SUCCESS  = NIM Module opened successfully
@return         @b AUI_RTN_FAIL.    = Opening of the NIM Module failed for some
                                      reasons
*/
AUI_RTN_CODE aui_nim_open (

    aui_nim_attr *p_attr,

    aui_hdl *p_handle

    );

/**
@brief          Function used to close the NIM Module already opened by the
                function #aui_nim_open then the related handle (i.e. the related
                resources such as memory, device) will be released

@param[in]      handle              = #aui_hdl pointer to the handle of the NIM
                                      Module to be closed

@return         @b AUI_RTN_SUCCESS  = NIM Module closed successfully
@return         @b AUI_RTN_FAIL.    = Closing of the NIM Module failed for some
                                      reasons
*/
AUI_RTN_CODE aui_nim_close (

    aui_hdl handle

    );

/**
@brief          Function used to get a NIM handle according to the index number
                of the NIM device

@param[in]      nim_id              = The index number of the NIM handle
                                      (which starts from the value zero (0))

@param[out]     p_handle            = #aui_hdl pointer to the NIM handle just
                                      gotten by the index number of the NIM device

@return         @b AUI_RTN_SUCCESS  = Getting of the NIM handle performed
                                      successfully
@return         @b AUI_RTN_FAIL.    = Getting of the NIM handle failed for some
                                      reasons
*/
AUI_RTN_CODE aui_nim_handle_get_byid (

    unsigned int nim_id,

    aui_hdl *p_handle

    );

/**
@brief          Function used to get an index number of a NIM device according
                to the handle

@param[in]      handle              = #aui_hdl handle of which getting the Index
                                      Number of the NIM device

@param[out]     p_nim_id            = Pointer to the Index Number of the NIM
                                      Device just gotten

@return         @b AUI_RTN_SUCCESS  = Getting of the Index Number of the NIM
                                      Device performed successfully
@return         @b AUI_RTN_FAIL.    = Getting of the Index Number of the NIM
                                      Device failed for some reasons
*/
AUI_RTN_CODE aui_nimid_get_by_handle (

    aui_hdl handle,

    unsigned int *p_nim_id

    );

/**
@brief          Function used to connect a channel (Transponder) to a NIM Device

@param[in]      handle              = #aui_hdl handle of the NIM Device already
                                      opened and to be connected to a channel
@param[in]      p_connect_param     = Pointer to a struct #aui_nim_connect which
                                      collects the connection attributes/parameters

@return         @b AUI_RTN_SUCCESS  = Connecting of the NIM Device performed
                                      successfully
@return         @b AUI_RTN_FAIL.    = Connecting of the NIM Device failed for
                                      some reasons
*/
AUI_RTN_CODE aui_nim_connect (

    aui_hdl handle,

    aui_nim_connect_param *p_connect_param

    );

/**
@brief          Function used to disconnect a channel (Transponder) from a NIM
                Device

@param[in]      handle              = #aui_hdl handle of the NIM Device already
                                      opened and to be disconnected from a channel
                                      (Transponder)

@return         @b AUI_RTN_SUCCESS  = Disconnecting of the NIM Device performed
                                      successfully
@return         @b AUI_RTN_FAIL.    = Disconnecting of the NIM Device failed for
                                      some reasons

@note           This function is implemented by locking a non-existent Transponder
*/
AUI_RTN_CODE aui_nim_disconnect (

    aui_hdl handle

    );

/**
@brief          Function used to get the lock status of NIM Module

@param[in]      handle              = #aui_hdl handle of the NIM Device already
                                      opened and to be managed for getting the
                                      lock status

@param[out]     p_lock              = Pointer to the lock status just gotten

@return         @b AUI_RTN_SUCCESS  = Getting of the lock status of NIM Module
                                      performed successfully
@return         @b AUI_RTN_FAIL.    = Getting of the lock status of NIM Module
                                      failed for some reasons
*/
AUI_RTN_CODE aui_nim_is_lock (

    aui_hdl handle,

    int *p_lock

    );

/**
@brief          Function used to get information about the current Channel
                (Transponder)

@param[in]      handle              = #aui_hdl handle of the NIM Device already
                                      opened and to be managed for getting
                                      information about the current Transponder

@param[out]     p_status            = Pointer to a struct #aui_signal_status
                                      which collects miscellaneous information
                                      about the signal status

@return         @b AUI_RTN_SUCCESS  = Getting of the lock status of NIM Module
                                      performed successfully
@return         @b AUI_RTN_FAIL.    = Getting of the lock status of NIM Module
                                      failed for some reasons
*/
AUI_RTN_CODE aui_nim_signal_info_get (

    aui_hdl handle,

    aui_signal_status *p_status

    );

/**
@brief          Function used to start the DVB-S/S2 blind scanning

@param[in]      handle              = #aui_hdl handle of the NIM Device already
                                      opened and to be managed for starting a
                                      DVB-S/S2 blind scan
@param[in]      *p_as               = Pointer to a struct #aui_autoscan_sat which
                                      collects the information requested for a
                                      DVB-S/S2 Auto-Scan

@return         @b AUI_RTN_SUCCESS  = Starting of the DVB-S/S2 blind scan
                                      performed successfully
@return         @b AUI_RTN_FAIL.    = Starting of the DVB-S/S2 blind scan failed
                                      for some reasons

@note           This function can @a only be used for @b DVB-S/S2 Systems

@warning        This function is a blocking function as will take long time to
                finish the scaning: whenever a TP is found, it will call the function
                #aui_autoscan_callback and wait for the its return value
*/
AUI_RTN_CODE aui_nim_auto_scan_start (

    aui_hdl handle,

    aui_autoscan_sat *p_as

    );

/**
@brief          Function used to stop the DVB-S/S2 blind scanning.

@param[in]      handle              = #aui_hdl handle of the NIM Device already
                                      opened and to be managed for stopping a
                                      DVB-S/S2 blind scan

@return         @b AUI_RTN_SUCCESS  = Stopping of the DVB-S/S2 blind scan
                                      performed successfully
@return         @b AUI_RTN_FAIL.    = Stopping of the DVB-S/S2 blind scan
                                      failed for some reasons

@note           This function is in pair with the function #aui_nim_auto_scan_start
                and can @a only be for @b DVB-S/S2 System

@warning        This function might return a failure if the stop operation cannot
                be performed within 300ms. The stop operation will wait until the
                function #aui_autoscan_callback returns.\n
                If the application blocks the function #aui_autoscan_callback for
                long time, this function will fail. The application should
                - Either reduce the execution time in the function #aui_autoscan_callback
                - Or check whether this function returns succesfully or not
*/
AUI_RTN_CODE aui_nim_auto_scan_stop (

    aui_hdl handle

    );

/**
@brief          Function used to
                - Send <b> DiSEqc 1.0 </b> message when the parameter @b resp
                  is NULL
                - Send/Receive <b> DiSEqC 2.x </b> message when the parameter
                  @b resp is not NULL

@param[in]      handle              = #aui_hdl handle of the NIM Device already
                                      opened and to be managed for sending/
                                      receiving DiSEqC messages
@param[in]      mode                = Type of DiSEqC message as per definition
                                      of the enum #aui_diseqc_modes
@param[in]      puc_cmd             = Pointer to the DiSEqC command string
@param[in]      cnt                 = Pointer to the command size
@param[in]      puc_resp            = Pointer to the DiSEqC response string
@param[in]      pul_resp_cnt        = Pointer to the response size

@return         @b AUI_RTN_SUCCESS  = Sending/Receiving DiSEqC messages performed
                                      successfully
@return         @b AUI_RTN_FAIL.    = Sending/Receiving DiSEqC messages failed
                                      for some reasons

@note           This function can @a only be used for @b DVB-S/S2 System
*/
AUI_RTN_CODE aui_diseqc_oper (

    aui_hdl handle,

    unsigned int mode,

    unsigned char *puc_cmd,

    unsigned int cnt,

    unsigned char *puc_resp,

    unsigned int *pul_resp_cnt

    );

/**
@brief          Function used to select Vertical Polarization (or Right Circular)/
                Horizontal Polarization (or Left Circular) for the signal

@param[in]      handle              = #aui_hdl handle of the NIM Device already
                                      opened and to be managed for selecting a
                                      polarization
@param[in]      polar               = Pointer to the struct #aui_nim_polar which
                                      collects the types of signal polarization

@return         @b AUI_RTN_SUCCESS  = Selecting of the signal polarization
                                      performed successfully
@return         @b AUI_RTN_FAIL.    = Selecting of the signal polarization
                                      failed for some reasons

@note           @a Only for DVB-S/S2 System, this function can be used to set
                the signal polarization if it has not been set when connecting
                a channel, i.e. when using the function #aui_nim_connect
*/
AUI_RTN_CODE aui_nim_set_polar (

    aui_hdl handle,

    aui_nim_polar polar

    );

/**
@brief          Function used to select High/Low Band of the dual-band LNB block
                of the reception chain by the 22kHz Tone

@param[in]      handle              = #aui_hdl handle of the NIM Device already
                                      opened and to be managed for selecting High/
                                      Low Band of the dual-band LNB block
@param[in]      status_22k          = Status of the 22kHz Tone as per definition
                                      of the enum #aui_nim_22k_e

@return         @b AUI_RTN_SUCCESS  = Selecting of the High/Low Band of the
                                      dual-band LNB block performed successfully
@return         @b AUI_RTN_FAIL.    = Selecting of the High/Low Band of the
                                      dual-band LNB block failed for some reasons

@note           @a Only for DVB-S/S2 System, this function can be used to set
                the High/Low Band if it has been not set when connecting a
                channel i.e. when using the function #aui_nim_connect
*/
AUI_RTN_CODE aui_nim_set22k_onoff (

    aui_hdl handle,

    aui_nim_22k_status status_22k

    );

/**
@brief          Function used to set the symbol rate offset limitation. On tuning,
                if the offset between tuning symbol rate and the physical symbol rate
                of the live signal is bigger than the limitation, aui_nim_connect will fail.

                This function is used for tuner sensitiviy test only.
                It should not be set in production FW.

@param[in]      handle              = #aui_hdl handle of the NIM Device already
                                      opened and to be managed for Setting the
                                      symbol rate offset limit

@param[in]      sym_limit           = Symbol rate offset limit (in @a Kbd unit),
                                      it must be less than 150Kbd

@return         @b AUI_RTN_SUCCESS  = Setting the symbol rate offset limit performed
                                      successfully
@return         @b AUI_RTN_FAIL.    = Setting the symbol rate offset limit failed
                                      for some reasons

@note           @a Only for DVB-C System, this function can be used after
                using the function #aui_nim_open.
*/

AUI_RTN_CODE aui_nim_sym_offset_limit_set (

    aui_hdl handle,

    unsigned long ul_sym_limit

    );

/**
@brief          Function used to set the LNB power ON/OFF

@param[in]      handle              = #aui_hdl handle of the NIM Device already
                                      opened and to be managed for set LNB power
                                      ON/OFF

@param[in]      lnb_onoff           = Status of the LNB power as per definition
                                      of the enum #aui_nim_LNB_power_status

@return         @b AUI_RTN_SUCCESS  = Setting the LNB power performed successfully

@return         @b AUI_RTN_FAIL.    = Setting the LNB power failed for some reasons

@note           @a Only for DVB-S/S2 System, this function can be used to set
                the LNB power ON/OFF if it has been not set when open or close a nim
                i.e. when using the function #aui_nim_open and #aui_nim_close

*/

AUI_RTN_CODE aui_nim_lnb_power_set ( 
	
	aui_hdl handle,
	
	aui_nim_lnb_power_status lnb_onoff

);


/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define aui_nim_tunner_type aui_nim_tuner_type

#define pfn_aui_ascallback aui_autoscan_callback

#define aui_as_sat aui_autoscan_sat

#endif

#define AUI_STD_ISDBT AUI_NIM_STD_ISDBT

#define AUI_STD_DVBT AUI_NIM_STD_DVBT

#define AUI_STD_DVBT2 AUI_NIM_STD_DVBT2

#define AUI_STD_DVBT2_COMBO AUI_NIM_STD_DVBT2_COMBO

#define AUI_STD_OTHER AUI_NIM_STD_OTHER

#define aui_std_ter aui_nim_std

#define aui_ter_std aui_nim_std

#define en_ter_std en_std
/// @endcond

#ifdef __cplusplus

}

#endif

#endif

/* END OF FILE */

