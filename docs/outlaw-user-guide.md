# Outlaw GPS Tracker

---

## Contents

1. [Before You Power On](#before-you-power-on)
2. [Indicators](#indicators)
3. [Getting a GPS Fix](#getting-a-gps-fix)
4. [Frequency Variants](#frequency-variants)
5. [Charging](#charging)
6. [Pre-Flight Checklist](#pre-flight-checklist)
7. [Troubleshooting](#troubleshooting)
8. [Safety](#safety)
9. [Transmissions Without a GPS Fix](#transmissions-without-a-gps-fix)
10. [What the Tracker Sends](#what-the-tracker-sends)
11. [Configuring with the UART Shell](#configuring-with-the-uart-shell)

---

## Before You Power On

**Always attach the antenna before applying power.** The SMA antenna connector is on the edge of the board. Hand-tighten it until snug. Running the tracker without a proper antenna connected can permanently damage the radio.

---

## Indicators

The Outlaw has three indicators:

| Indicator | Colour | Meaning                                         |
|---|---|-------------------------------------------------|
| Power LED | Red | Board is powered on (with PWR LED pins bridged) |
| Charging LED | White | LiPo battery is currently charging              |
| Debug LED | Red | Internal use only — ignore this                 |

> **Note:** The white charging LED will turn off once the battery is fully charged. This is normal.

> **Note**: To increase battery life, the power LED is only active when the PWR LED jumper is bridged. If you have not bridged this jumper, the red power LED will not light up.

---

## Getting a GPS Fix

Place the Outlaw outdoors with a clear view of the sky. GPS acquisition on a cold start can take a few minutes. Once the tracker has a fix, it will begin transmitting your position automatically.

---

## Frequency Variants

Outlaws are pre-configured for a specific frequency band. There are two variants:

| Variant | Frequency   | Who can use it |
|---|-------------|---|
| Standard | 903.000 MHz | Anyone — no licence required |
| Licensed | 435.000 MHz | Licensed amateur radio operators only |

Your Deputy receiver must be on the same variant.

> **Licensed tracker users:** The tracker will not transmit until a valid amateur radio callsign has been programmed into the device.

---

## Charging

Connect a LiPo battery to the battery connector on the board. Then connect USB-C to charge. The white LED indicates charging is in progress. Disconnect USB-C once the white LED goes off.

Use a standard 5 V USB-C charger. Do not leave unattended while charging.

---

## Pre-Flight Checklist

Before mounting the Outlaw to your flight vehicle, confirm the following:

1. **Antenna attached** — SMA connector, hand-tight
2. **Battery charged** — White LED goes off when full
3. **Power LED on** — Red LED confirms the board is live. You may unbridge the PWR LED jumper if you prefer to run without the power LED active.
4. **GPS fix acquired** — Allow several minutes outdoors before flight
5. **Dispatch receiving** — Confirm packets are being received on a Hunter receiver either through UART or the Dispatch application

---

## Troubleshooting

**Packets not being received**
- Confirm the antenna is attached to the Outlaw.
- Confirm the Hunter receiver antenna is also attached.
- Ensure both the Outlaw and Hunter are the same frequency variant (both Standard or both Licensed).

**Red power LED is off**
- Check battery connection and charge level. Connect USB-C to power the board directly while troubleshooting.
- Confirm the PWR LED jumper is bridged if you want the power LED active. The board will function without the power LED, but it won't light up.

---

## Transmissions Without a GPS Fix

The Outlaw does not wait silently if it cannot acquire a GPS fix — it keeps transmitting so the ground station knows the tracker is alive and on-air. When there is no fix, it sends a **NOFIX packet** instead of position data.

In Dispatch, a tracker sending NOFIX packets will appear in the tracker list with its fix status shown as **NOFIX** and no latitude/longitude values. This tells you the hardware is working and communicating — it just hasn't locked onto satellites yet.

Once a fix is acquired, the tracker automatically switches to sending full position packets. No action is needed on your part.

Common reasons for a prolonged NOFIX state:

- The tracker is indoors or has no clear view of the sky
- This is a cold start — first fix can take several minutes
- The GPS antenna is not connected (if using an external antenna)

---

## What the Tracker Sends

Every packet the Outlaw transmits over LoRa contains the following information:

| Field | Description |
|---|---|
| **Node ID** | A number (0–9) identifying which tracker this packet is from |
| **Latitude** | Current GPS latitude in decimal degrees |
| **Longitude** | Current GPS longitude in decimal degrees |
| **Fix Status** | Whether the GPS has a valid position lock (see below) |
| **Satellites** | Number of satellites the GPS module is currently tracking |
| **Callsign** | *(Licensed builds only)* Your amateur radio callsign, prepended to every packet |

**Fix status values you may see in Dispatch:**

| Status | Meaning |
|---|---|
| FIX | Standard GPS position lock |
| DIFF | Differential GPS fix — more accurate than standard |
| EST | Estimated position — GPS is computing a position but confidence is lower |
| NOFIX | No position lock yet |

The tracker sends packets continuously throughout flight. Dispatch logs every one, so you have a complete position history for the entire flight available to export after recovery.

---

## Configuring with the UART Shell

If you prefer not to use Dispatch, or need to configure the tracker in the field without a computer running Dispatch, the Outlaw has a text-based shell accessible over USB.

**What you need:**

- A computer with a terminal application (e.g. PuTTY on Windows, Screen or minicom on macOS/Linux)
- A USB-C cable connected to the Outlaw

**Connecting:**

Open your terminal application and connect to the Outlaw's serial port at **9600 baud**. On connection you will see a prompt:

```
uart:~$
```

If you see no prompt, press Enter once to wake the shell.

**Available commands:**

Set the node ID (0–9). Each tracker on a flight should have a unique ID:

```
uart:~$ config node_id 2
```

Set the frequency *(Standard/unlicensed builds only, 902–928 MHz)*:

```
uart:~$ config freq 903.125000
```

Set your callsign *(Licensed builds only, 4–6 uppercase characters)*:

```
uart:~$ config callsign W1ABC
```

**All settings are saved to the device automatically** and will persist through power cycles. Changes take effect after a reboot:

```
uart:~$ kernel reboot cold
```

> **Note:** If you enter a callsign shorter than 4 characters, the device will suspend all transmissions after reboot until a valid callsign is set. This is a regulatory safeguard.