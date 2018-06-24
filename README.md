# Omega2 DHT11 LoRa Demo

### Description

A simple program which just invokes the [DHT11 reader program](https://github.com/h0l0gram/omega2-checkHumidity), encodes the data using a [CBOR library](https://github.com/naphaso/cbor-cpp), and sends it [over LoRa to TTN](https://github.com/maxgerhardt/omega2-lmic-lorawan) / AllThingsTalk.

Refer to the original post at https://community.onion.io/topic/2982/omega2-as-a-lorawan-node/2

![graph](https://community.onion.io/assets/uploads/files/1529839853743-allthingstalk_data.png)

![board](https://raw.githubusercontent.com/maxgerhardt/omega2-dht11-lora-demo/master/schematics/omega_lora_dht11_Steckplatine.png)

Note: RESET of the I2C bridge must be held high.

### License

The CBOR-CPP library is taken from https://github.com/naphaso/cbor-cpp. License is Apache 2.0. [Copyright 2014-2015 Stanislav Ovsyannikov]
