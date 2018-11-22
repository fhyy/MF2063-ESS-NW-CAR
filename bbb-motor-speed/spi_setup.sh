#!/usr/bin/env sh

#Shell script for setting up SPI communication automatically on a BBB


BB-SPI1-01-00A0="
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

SOURCE_FILE=BB-SPI1-01-00A0.dts
DEST_FILE=BB-SPI1-01-00A0.dtbo

# Remove if file already exists
if [ -f "$SOURCE_FILE" ]
then
    rm -f "$SOURCE_FILE"
fi

echo "$BB-SPI1-01-00A0" > "$DEST_FILE"

#Complie the file
dtc -O dtb -o "$DEST_FILE" -b 0 -@ "$SOURCE_FILE"

#Copy output file into right folder
cp "$DEST_FILE" /lib/firmware/

echo "BB-SPI1-01" > /sys/devices/bone_capemgr.*/slots

#Setup GPIO pins to be used as SS
cd /sys/class/gpio || exit
echo "60" > "export"
echo "48" > "export"
cd gpio48 || exit
cat out > direction 
echo "1" > value
cd ../gpio60 || exit
cat out > direction
echo "1" > value

#Some manual intervention is still required, we print instructions (printf required for escape sequences)
printf "Go to My Computer>BeagleBone Getting Started> and open uEnv.txt Copy and paste this command into the .txt file. Make sure to save your changes. (Ctrl+s)\n\n"
printf "optargs=quiet drm.debug=7 capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN
capemgr.enable_partno=BB-SPI1-01\n\n"
printf "after you save the changes, reboot your beaglebone black
Make sure it is enabled You should now have two spidev-files in the folder /dev/\n\n
ls -al /dev/spidev1.*\n\n
You should also be able to see the pingroups:\n\n
cat /sys/kernel/debug/pinctrl/44e10800.pinmux/pingroups"

