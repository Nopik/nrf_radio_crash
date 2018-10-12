# Introduction

This repository shows nRF52840 crashing (probably in radio driver) and was created for debugging purposes.

Application contained here is very simple: it contains transmitter and receiver code, to be run on 2 separate devices.

Transmitter is just sending identical simple packets every 20ms or so. This part seems to be working fine.

Receiver only receives those packets - and does nothing with them. Yet, after receiving enough of them, code crashes.

The way it crashes is interesting, however: it usually crashes within first few seconds of starting. If it survives initial few seconds (lets say 10), it usually runs for long time - sometimes it crashes after 40 minutes, sometimes few hours (and maybe sometimes is running fine forever).
Therefore, in order to speed up problem reproduction, the code contains self-reset every 500 packets (roughly 10 seconds) received.

Both transmitter and receiver are blinking leds, so it is easy to tell whether one of them is not working.

When transmitter is running, it usually takes few seconds up to minute for receiver to crash.

If transmitter is not running, and receiver is started, it can run for very long time without problems. And once transmitter starts sending packets, receiver usually crashes within seconds.

# Requirements

Two PCA10056 boards from Nordic.

# Preparation

After checking out the repository, run the `./prepare_download.sh` script to download nRF52 SDK as well as radio driver.

For convenience, check out the repository twice, to different folders (and run the prepare script twice, too). In one folder, set `CFLAGS += -DMASTER_NODE` to be present at the top of `src/platform/nrf52840/crasher/Makefile`, on the other repo comment out that line.
Build both repos, flash them, run & wait until crash. 

# Background story

While developing custom OpenThread application and custom nRF52840-based board, we've noticed strange instabilities. Sometimes it was just resets (by watchdog), sometimes devices seemed to slow down. Sometimes after running 20-40 minutes they started to slow down x2, even up to x16. During troubleshooting and debugging this problem, I managed to reduce the application to simple tx/rx stuff and still encounter crashes. So, I created this repository, being really stripped down version of our app, with tons and tons of stuff removed - just to pinpoint the problem.

## Slowdowns

On PCA10056, however, I could hardly reproduce the slowdowns. They do happen, but it happens maybe 1 times out of 500 runs, or less. On my application & board, when they happen, they seem to be caused by increased number of IRQ received by CPU. I'm not sure which IRQ handler was causing that (if I traced properly, it wasn't the radio one). Generally the application had simple `while( true ) { ...; sleep( 10ms ); }` kind of loop. Sleep was taking *exactly* 10ms each time for 20-40 minutes, and then suddenly were taking 15-18ms - just like CPU would be busy, so FreeRTOS couldn't wake task on time. This repository contains some logging set up to catch that - if you will see the `SLEEP ...` (with few numbers) logs, it means, that you've encountered this problem.