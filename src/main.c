/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <sys/printk.h>
#include <sys/util.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

/* Example number index 
  EXAMPLE_NUMBER 1: Unmodified/equivalent to scan_adv sample from Zephyr
  EXAMPLE_NUMBER 2: Static address, name, Company ID
  EXAMPLE_NUMBER 3: Advertising interval
  EXAMPLE_NUMBER 4: Scan response, send more data (128-bit UUID)
  EXAMPLE_NUMBER 5: Connectable
*/
#define EXAMPLE_NUMBER 1


#if EXAMPLE_NUMBER >= 2
/*
  defining BT_LE_ADV_NCONN_ID instead of using the predefined ones like BT_LE_ADV_NCONN \
  found in bluetooth.h
  Add this configuration to your project's proj.conf file and choose a name: 
  CONFIG_BT_DEVICE_NAME="Lutra lutra"
  Remember to reload after making changes to proj.conf if you \ 
  are using Segger Embedded Studio (Project -> Reload scan_adv ).
*/
#define BT_LE_ADV_NCONN_ID BT_LE_ADV_PARAM(BT_LE_ADV_OPT_USE_IDENTITY | BT_LE_ADV_OPT_USE_NAME, \
                         BT_GAP_ADV_FAST_INT_MIN_2, \
                         BT_GAP_ADV_FAST_INT_MAX_2, NULL)
#endif

#if EXAMPLE_NUMBER == 3
#define BT_LE_ADV_NCONN_SLOW BT_LE_ADV_PARAM(0, BT_GAP_ADV_SLOW_INT_MIN, \
					BT_GAP_ADV_SLOW_INT_MAX, NULL)
#endif

#if EXAMPLE_NUMBER == 1 
static u8_t mfg_data[] = { 0xff, 0xff, 0x00 };

#else 
static u8_t mfg_data[] = { 0x59, 0x00, 0x00 }; // 0x0059, the Company ID for Nordic Semiconductor ASA
#endif

#if EXAMPLE_NUMBER >= 4
static u8_t uuid_hh[] = {0x32, 0x8d, 0xdd, 0x96, 0xe8, 0xa9, 0x94, 0x87,\
                         0xe2, 0x47, 0x8b, 0x85, 0x22, 0xef, 0x42, 0x9a}; // Random 128 bit UUID
#endif

#if EXAMPLE_NUMBER >= 4
/*Scan response data*/
static const struct bt_data sr[] = {
	BT_DATA(BT_DATA_UUID128_ALL, uuid_hh, 16),
};
#endif

/*Advertising data, limit this to 31 bytes*/ 
static const struct bt_data ad[] = {
	BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, 3),
};

static void scan_cb(const bt_addr_le_t *addr, s8_t rssi, u8_t adv_type,
		    struct net_buf_simple *buf)
{
	mfg_data[2]++;
}

void main(void)
{
	struct bt_le_scan_param scan_param = {
		.type       = BT_HCI_LE_SCAN_PASSIVE,
		.options    = BT_LE_SCAN_OPT_NONE,
		.interval   = 0x0010,
		.window     = 0x0010,
	};
	int err;

	printk("Starting Scanner/Advertiser Demo\n");


	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	err = bt_le_scan_start(&scan_param, scan_cb);
	if (err) {
		printk("Starting scanning failed (err %d)\n", err);
		return;
	}

	do {
		k_sleep(K_MSEC(400));

		/* Start advertising */
                #if EXAMPLE_NUMBER == 1  /* Unmodified scan_adv sample, non connectable */
                err = bt_le_adv_start(BT_LE_ADV_NCONN, ad, ARRAY_SIZE(ad),
				      NULL, 0);
                
                #elif EXAMPLE_NUMBER == 2 /* Non connectable, static address, name */
                err = bt_le_adv_start(BT_LE_ADV_NCONN_ID, ad, ARRAY_SIZE(ad),
				      NULL, 0);
                
                #elif EXAMPLE_NUMBER == 3 /* Advertising interval */
                err = bt_le_adv_start(BT_LE_ADV_NCONN_SLOW, ad, ARRAY_SIZE(ad),
				      NULL, 0);

                #elif EXAMPLE_NUMBER == 4 /* Scan response */
                err = bt_le_adv_start(BT_LE_ADV_NCONN_ID, ad, ARRAY_SIZE(ad),
				      sr, ARRAY_SIZE(sr));   

                #elif EXAMPLE_NUMBER == 5 /* Connectable, static address, name */	                
                err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad),
				      NULL, 0);                                                                   
                #endif
                                     
		if (err) {
			printk("Advertising failed to start (err %d)\n", err);
			return;
		}

		k_sleep(K_MSEC(400));

		err = bt_le_adv_stop();
		if (err) {
			printk("Advertising failed to stop (err %d)\n", err);
			return;
		}
	} while (1);
}


