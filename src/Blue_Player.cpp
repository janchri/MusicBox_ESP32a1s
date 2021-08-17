#include "Blue_Player.hpp"

Blue_Player::Blue_Player(Board *board) : board(board)
{
    i2s_pin_config_t esp32a1s_i2sconfig = {
        .bck_io_num = GPIO_NUM_27,
        .ws_io_num = GPIO_NUM_26,
        .data_out_num = GPIO_NUM_25,
        .data_in_num = GPIO_NUM_35};
    //a2dp_sink = new BluetoothA2DPSink();
    a2dp_sink.set_pin_config(esp32a1s_i2sconfig);
    a2dp_sink.set_mono_downmix(true);

    static const i2s_config_t i2s_config = {
        .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX ),
        .sample_rate = 44100, // corrected by info from bluetooth
        .bits_per_sample = (i2s_bits_per_sample_t) 16, // the DAC module will only take the 8bits from MSB
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // default interrupt priority
        .dma_buf_count = 8,
        .dma_buf_len = 512,
        .use_apll = 1 //APLL_ENABLE
    };
    a2dp_sink.set_i2s_config(i2s_config);
    a2dp_sink.start("WirbelWind.Box");
}

void Blue_Player::listen()
{
    if (a2dp_sink.isConnected()){
        digitalWrite(GPIO_PA_EN, HIGH);
    }
    delay(71);
}

bool Blue_Player::isConnected(){
    return a2dp_sink.isConnected();
}