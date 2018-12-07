#!/usr/bin/env bash

#Shell script for setting up SPI communication automatically on a BBB

#Make sure run as sudo
if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

BB_SPI1_01_00A0="
/dts-v1/;
/plugin/;

/* SPI1 */
/* D1 Output and D0 Input */

/ {
    compatible = \"ti,beaglebone\", \"ti,beaglebone-black\";

    /* identification */
    part-number = \"spi1mux\";

    fragment@0 {
        target = <&am33xx_pinmux>;
        __overlay__ {
            spi1_pins_s0: spi1_pins_s0 {
                pinctrl-single,pins = <
                    0x190 0x33      /* mcasp0_aclkx.spi1_sclk, INPUT_PULLUP | MODE3 */
                    0x194 0x33      /* mcasp0_fsx.spi1_d0, INPUT_PULLUP | MODE3 */
                    0x198 0x13      /* mcasp0_axr0.spi1_d1, OUTPUT_PULLUP | MODE3 */
                    0x19c 0x13      /* mcasp0_ahclkr.spi1_cs0, OUTPUT_PULLUP | MODE3 */
                >;
            };
            pinctrl_test: DM_GPIO_Test_Pins {
                pinctrl-single,pins = <
                    0x078 0x17     /* P9 12 gpio60 PULLup + enabled (SS needs to be high) | MODE7 | OUTPUT */
                    0x040 0x17     /* P9 15 gpio48 PULLup + enabled (SS needs to be high) | MODE7 | OUTPUT */
                >;
            };
        };
    };

    fragment@1 {
        target = <&spi1>;
        __overlay__ {

             #address-cells = <1>;
             #size-cells = <0>;
             status = \"okay\";
             pinctrl-names = \"default\";
             pinctrl-0 = <&spi1_pins_s0>;

             spidev@1 {
                 spi-max-frequency = <24000000>;
                 reg = <0>;
                 compatible = \"linux,spidev\";
            };
        };
    };
};
"

SOURCE_NAME=BB-SPI1-01-00A0.dts
DEST_NAME=BB-SPI1-01-00A0.dtbo

# Remove if file already exists
if [ -f "$SOURCE_NAME" ]
then
    rm -f "$SOURCE_NAME"
fi

echo "$BB_SPI1_01_00A0" > "$SOURCE_NAME"

#Complie the file
dtc -O dtb -o "$DEST_NAME" -b 0 -@ "$SOURCE_NAME"

#Copy output file into right folder
cp "$DEST_NAME" /lib/firmware/

rm "$SOURCE_NAME" "$DEST_NAME"

dd if=/dev/zero of=/dev/mmcblk1 bs=1M count=10

printf "\nSetup finished! Please restart the BBB for changes to take effect. You should then have two spidev-files in the folder /dev/ using the command:\n\n"
printf "ls -al /dev/spidev1.*\n\n\n"
printf "You should also be able to see the pingroups using the command:\n\n"
printf "cat /sys/kernel/debug/pinctrl/44e10800.pinmux/pingroups\n\n"


