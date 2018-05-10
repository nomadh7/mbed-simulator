/**
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech
 ___ _____ _   ___ _  _____ ___  ___  ___ ___
/ __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
\__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
|___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
embedded.connectivity.solutions===============

Description: LoRaWAN stack layer that controls both MAC and PHY underneath

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis ( Semtech ), Gregory Cristian ( Semtech ) and Daniel Jaeckle ( STACKFORCE )


Copyright (c) 2017, Arm Limited and affiliates.

SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef SX1276_LORARADIO_H_
#define SX1276_LORARADIO_H_

#include "PinNames.h"
#include "InterruptIn.h"
#include "DigitalOut.h"
#include "DigitalInOut.h"
#include "Timeout.h"
#include "platform/PlatformMutex.h"
#ifdef MBED_CONF_RTOS_PRESENT
 #include "rtos/Thread.h"
#endif

#include "lorawan/LoRaRadio.h"

#ifdef MBED_CONF_SX1276_LORA_DRIVER_BUFFER_SIZE
#define MAX_DATA_BUFFER_SIZE_SX1276                        MBED_CONF_SX1276_LORA_DRIVER_BUFFER_SIZE
#else
#define MAX_DATA_BUFFER_SIZE_SX1276                        256
#endif

/**
 * Fake SX1276 Radio implementation.
 */
class SX1276_LoRaRadio: public LoRaRadio {
public:
    /**
     * Use this constructor if pin definitions are provided manually.
     * The pins that are marked NC are optional. It is assumed that these
     * pins are not connected until/unless configured otherwise.
     *
     * Note: Pin ant_switch is equivalent to RxTx pin at
     * https://developer.mbed.org/components/SX1276MB1xAS/.
     * Reading the state of this pin indicates if the radio module type is
     * SX1276MB1LAS(North American frequency band supported) or SX1276MAS
     * (European frequency band supported).
     * Pin dio4 can be mapped to multiple pins on the board, please refer to
     * schematic of your board. For reference look at
     * https://developer.mbed.org/components/SX1276MB1xAS/
     *
     * Most of the radio module control pins are not being used at the moment as
     * the SX1276MB1xAS shield has not connected them. For consistency and future
     * use we are leaving the pins in the constructor. For example, if in some
     * setting SX1276 radio module gets connected to an external power amplifier
     * or radio  latch controls are connected.
     */
    SX1276_LoRaRadio(PinName mosi,
                     PinName miso,
                     PinName sclk,
                     PinName nss,
                     PinName reset,
                     PinName dio0,
                     PinName dio1,
                     PinName dio2,
                     PinName dio3,
                     PinName dio4,
                     PinName dio5,
                     PinName rf_switch_ctl1 = NC,
                     PinName rf_switch_ctl2 = NC,
                     PinName txctl = NC,
                     PinName rxctl = NC,
                     PinName ant_switch = NC,
                     PinName pwr_amp_ctl = NC,
                     PinName tcxo = NC);

    /**
     * Destructor
     */
    virtual ~SX1276_LoRaRadio();

    /**
     * Registers radio events with the Mbed LoRaWAN stack and
     * undergoes initialization steps if any
     *
     *  @param events Structure containing the driver callback functions
     */
    virtual void init_radio(radio_events_t *events);

    /**
     * Resets the radio module
     */
    virtual void radio_reset();

    /**
     *  Put the RF module in sleep mode
     */
    virtual void sleep(void);

    /**
     *  Sets the radio in standby mode
     */
    virtual void standby(void);

    /**
     *  Sets the reception parameters
     *
     *  @param modem         Radio modem to be used [0: FSK, 1: LoRa]
     *  @param bandwidth     Sets the bandwidth
     *                          FSK : >= 2600 and <= 250000 Hz
     *                          LoRa: [0: 125 kHz, 1: 250 kHz,
     *                                 2: 500 kHz, 3: Reserved]
     *  @param datarate      Sets the Datarate
     *                          FSK : 600..300000 bits/s
     *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
     *                                10: 1024, 11: 2048, 12: 4096  chips]
     *  @param coderate      Sets the coding rate ( LoRa only )
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
     *  @param bandwidth_afc Sets the AFC Bandwidth ( FSK only )
     *                          FSK : >= 2600 and <= 250000 Hz
     *                          LoRa: N/A ( set to 0 )
     *  @param preamble_len  Sets the Preamble length ( LoRa only )
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: Length in symbols ( the hardware adds 4 more symbols )
     *  @param symb_timeout  Sets the RxSingle timeout value
     *                          FSK : timeout number of bytes
     *                          LoRa: timeout in symbols
     *  @param fixLen        Fixed length packets [0: variable, 1: fixed]
     *  @param payload_len   Sets payload length when fixed lenght is used
     *  @param crc_on        Enables/Disables the CRC [0: OFF, 1: ON]
     *  @param freq_hop_on   Enables disables the intra-packet frequency hopping  [0: OFF, 1: ON] (LoRa only)
     *  @param hop_period    Number of symbols bewteen each hop (LoRa only)
     *  @param iq_inverted   Inverts IQ signals ( LoRa only )
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [0: not inverted, 1: inverted]
     *  @param rx_continuous Sets the reception in continuous mode
     *                          [false: single mode, true: continuous mode]
     */
    virtual void set_rx_config (radio_modems_t modem, uint32_t bandwidth,
                               uint32_t datarate, uint8_t coderate,
                               uint32_t bandwidth_afc, uint16_t preamble_len,
                               uint16_t symb_timeout, bool fix_len,
                               uint8_t payload_len,
                               bool crc_on, bool freq_hop_on, uint8_t hop_period,
                               bool iq_inverted, bool rx_continuous);

    /**
     *  Sets the transmission parameters
     *
     *  @param modem         Radio modem to be used [0: FSK, 1: LoRa]
     *  @param power         Sets the output power [dBm]
     *  @param fdev          Sets the frequency deviation ( FSK only )
     *                          FSK : [Hz]
     *                          LoRa: 0
     *  @param bandwidth     Sets the bandwidth ( LoRa only )
     *                          FSK : 0
     *                          LoRa: [0: 125 kHz, 1: 250 kHz,
     *                                 2: 500 kHz, 3: Reserved]
     *  @param datarate      Sets the Datarate
     *                          FSK : 600..300000 bits/s
     *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
     *                                10: 1024, 11: 2048, 12: 4096  chips]
     *  @param coderate      Sets the coding rate ( LoRa only )
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
     *  @param preamble_len  Sets the preamble length
     *  @param fix_len       Fixed length packets [0: variable, 1: fixed]
     *  @param crc_on        Enables disables the CRC [0: OFF, 1: ON]
     *  @param freq_hop_on   Enables disables the intra-packet frequency hopping  [0: OFF, 1: ON] (LoRa only)
     *  @param hop_period    Number of symbols bewteen each hop (LoRa only)
     *  @param iq_inverted   Inverts IQ signals ( LoRa only )
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [0: not inverted, 1: inverted]
     *  @param timeout       Transmission timeout [us]
     */
    virtual void set_tx_config(radio_modems_t modem, int8_t power, uint32_t fdev,
                              uint32_t bandwidth, uint32_t datarate,
                              uint8_t coderate, uint16_t preamble_len,
                              bool fix_len, bool crc_on, bool freq_hop_on,
                              uint8_t hop_period, bool iq_inverted, uint32_t timeout);

    /**
     *  Sends the buffer of size
     *
     *  Prepares the packet to be sent and sets the radio in transmission
     *
     *  @param buffer        Buffer pointer
     *  @param size          Buffer size
     */
    virtual void send(uint8_t *buffer, uint8_t size);

    /**
     *  Sets the radio in reception mode for the given time
     *
     *  It should be noted that if the timeout is set to 0, it essentially
     *  puts the receiver in continuous mode and hence from thereon it should
     *  be treated as if in continuous mode. However, an appropriate way of
     *  setting the receiver in continuous mode is by using set_rx_config()
     *  API.
     *
     *  @param timeout       Reception timeout [ms]
     *
     */
    virtual void receive(uint32_t timeout);

    /**
     *  Sets the carrier frequency
     *
     *  @param freq          Channel RF frequency
     */
    virtual void set_channel(uint32_t freq);

    /**
     *  Generates a 32 bits random value based on the RSSI readings
     *
     *  Remark this function sets the radio in LoRa modem mode and disables
     *         all interrupts.
     *         After calling this function either Radio.SetRxConfig or
     *         Radio.SetTxConfig functions must be called.
     *
     *  @return             32 bits random value
     */
    virtual uint32_t random(void);

    /**
     *  Get radio status
     *
     *  @param status        Radio status [RF_IDLE, RF_RX_RUNNING, RF_TX_RUNNING]
     *  @return              Return current radio status
     */
    virtual uint8_t get_status(void);

    /**
     *  Sets the maximum payload length
     *
     *  @param modem         Radio modem to be used [0: FSK, 1: LoRa]
     *  @param max           Maximum payload length in bytes
     */
    virtual void set_max_payload_length(radio_modems_t modem, uint8_t max);

    /**
     *  Sets the network to public or private
     *
     *  Updates the sync byte. Applies to LoRa modem only
     *
     *  @param enable        if true, it enables a public network
     */
    virtual void set_public_network(bool enable);

    /**
     *  Computes the packet time on air for the given payload
     *
     *  Remark can only be called once SetRxConfig or SetTxConfig have been called
     *
     *  @param modem         Radio modem to be used [0: FSK, 1: LoRa]
     *  @param pkt_len       Packet payload length
     *  @return              Computed airTime for the given packet payload length
     */
    virtual uint32_t time_on_air(radio_modems_t modem, uint8_t pkt_len);

    /**
     * Perform carrier sensing
     *
     * Checks for a certain time if the RSSI is above a given threshold.
     * This threshold determines if there is already a transmission going on
     * in the channel or not.
     *
     * @param modem                     Type of the radio modem
     * @param freq                      Carrier frequency
     * @param rssi_threshold            Threshold value of RSSI
     * @param max_carrier_sense_time    time to sense the channel
     *
     * @return                          true if there is no active transmission
     *                                  in the channel, false otherwise
     */
    virtual bool perform_carrier_sense(radio_modems_t modem,
                                       uint32_t freq,
                                       int16_t rssi_threshold,
                                       uint32_t max_carrier_sense_time);

    /**
     *  Sets the radio in CAD mode
     *
     */
    virtual void start_cad(void);

    /**
     *  Check if the given RF is in range
     *
     *  @param frequency       frequency needed to be checked
     */
    virtual bool check_rf_frequency(uint32_t frequency);

    /** Sets the radio in continuous wave transmission mode
     *
     *  @param freq          Channel RF frequency
     *  @param power         Sets the output power [dBm]
     *  @param time          Transmission mode timeout [s]
     */
    virtual void set_tx_continuous_wave(uint32_t freq, int8_t power, uint16_t time);

    /**
     * Acquire exclusive access
     */
    virtual void lock(void);

    /**
     * Release exclusive access
     */
    virtual void unlock(void);

    /**
     * Process an RX frame out of band (emscripten)
     */
    void rx_frame(uint8_t* buffer, uint32_t size, uint32_t freq, uint8_t bw, uint8_t dr);

private:

    // SPI and chip select control
    mbed::DigitalOut _chip_select;

    // module rest control
    mbed::DigitalInOut _reset_ctl;

    // Interrupt controls
    mbed::InterruptIn _dio0_ctl;
    mbed::InterruptIn _dio1_ctl;
    mbed::InterruptIn _dio2_ctl;
    mbed::InterruptIn _dio3_ctl;
    mbed::InterruptIn _dio4_ctl;
    mbed::InterruptIn _dio5_ctl;

    // Radio specific controls
    mbed::DigitalOut _rf_switch_ctl1;
    mbed::DigitalOut _rf_switch_ctl2;
    mbed::DigitalOut _txctl;
    mbed::DigitalOut _rxctl;
    mbed::DigitalInOut _ant_switch;
    mbed::DigitalOut _pwr_amp_ctl;
    mbed::DigitalOut _tcxo;

    // Contains all RF control pin names
    // This storage is needed even after assigning the
    // pins to corresponding object, as the driver needs to know
    // which control pins are connected and which are not. This
    // variation is inherent to driver because of target configuration.
    rf_ctrls _rf_ctrls;

    // We need these PinNames as not all modules have those connected
    PinName _dio4_pin;
    PinName _dio5_pin;

    // Structure containing all user and network specified settings
    // for radio module
    radio_settings_t _rf_settings;

    // Structure containing function pointers to the stack callbacks
    radio_events_t *_radio_events;

    // Data buffer used for both TX and RX
    // Size of this buffer is configurable via Mbed config system
    // Default is 256 bytes
    uint8_t _data_buffer[MAX_DATA_BUFFER_SIZE_SX1276];

    // TX/RX Timers - all use milisecond units
    mbed::Timeout tx_timeout_timer;
    mbed::Timeout rx_timeout_timer;
    mbed::Timeout rx_timeout_sync_word;
    mbed::Timeout tx_done_timer;

#ifdef MBED_CONF_RTOS_PRESENT
    // Thread to handle interrupts
    rtos::Thread irq_thread;
#endif

    // Access protection
    PlatformMutex mutex;

    uint8_t radio_variant;

    // helper functions
    void setup_registers();
    void default_antenna_switch_ctrls();
    void set_antenna_switch(uint8_t operation_mode);
    void gpio_init();
    void gpio_deinit();
    void setup_interrupts();
    void set_operation_mode(uint8_t operation_mode);
    void set_low_power_mode();
    void set_sx1276_variant_type();
    uint8_t get_pa_conf_reg(uint32_t channel);
    void set_rf_tx_power(int8_t power);
    int16_t get_rssi(radio_modems_t modem);
    uint8_t get_fsk_bw_reg_val(uint32_t bandwidth);
    void write_fifo(uint8_t *buffer, uint8_t size);
    void read_fifo(uint8_t *buffer, uint8_t size);
    void transmit(uint32_t timeout);
    void rf_irq_task(void);
    void set_modem(uint8_t modem);
    void rx_chain_calibration(void);
    void tx_done_irq();
    void rx_done_irq();

    // ISRs
    void  dio0_irq_isr();
    void  dio1_irq_isr();
    void  dio2_irq_isr();
    void  dio3_irq_isr();
    void  dio4_irq_isr();
    void  dio5_irq_isr();
    void  timeout_irq_isr();

    // Handlers called by thread in response to signal
    void handle_dio0_irq();
    void handle_dio1_irq();
    void handle_dio2_irq();
    void handle_dio3_irq();
    void handle_dio4_irq();
    void handle_dio5_irq();
    void handle_timeout_irq();
};

#endif // SX1276_LORARADIO_H_
