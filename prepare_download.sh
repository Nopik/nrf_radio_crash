#!/usr/bin/env bash

set -e

ROOT_DIR=`pwd`
THIRD_PARTY_DIR="${ROOT_DIR}/third_party"

mkdir -p ${THIRD_PARTY_DIR}

pushd ${THIRD_PARTY_DIR}

echo "Downloading nRF5 SDK for Thread"
NRF_SDK_VERSION=1.0.0
NRF_SDK_FILE=nRF5_SDK_for_Thread_and_Zigbee_v${NRF_SDK_VERSION}.zip
NRF_SDK_DIR_NAME=nRF5_SDK_for_Thread_and_Zigbee

wget http://www.nordicsemi.com/eng/nordic/download_resource/67604/1/94281150/143298 -O ${NRF_SDK_FILE}

mkdir -p ${NRF_SDK_DIR_NAME}
unzip -q -o ${NRF_SDK_FILE} -d ${NRF_SDK_DIR_NAME}

echo "Downloading nRF 802.15.4 radio driver"
NRF_RADIO_DRIVER_DIR=nRF802.15.4-radio-driver
NRF_RADIO_DRIVER_COMMIT_SHA=40c2e6fcfdc1a2d427b26b38ffa13023665bf0b9
NRF_RADIO_DRIVER_FILE=nRF802.15.4-radio-driver-${NRF_RADIO_DRIVER_COMMIT_SHA}.zip

wget https://github.com/NordicSemiconductor/nRF-IEEE-802.15.4-radio-driver/archive/${NRF_RADIO_DRIVER_COMMIT_SHA}.zip -O ${NRF_RADIO_DRIVER_FILE}

mkdir -p ${NRF_RADIO_DRIVER_DIR}
unzip -q ${NRF_RADIO_DRIVER_FILE} -d ${NRF_RADIO_DRIVER_DIR}
mv ${NRF_RADIO_DRIVER_DIR}/nRF-IEEE-802.15.4-radio-driver-${NRF_RADIO_DRIVER_COMMIT_SHA}/src ${NRF_RADIO_DRIVER_DIR}/src
rm -r ${NRF_RADIO_DRIVER_DIR}/nRF-IEEE-802.15.4-radio-driver-${NRF_RADIO_DRIVER_COMMIT_SHA}

popd # ${THIRD_PARTY_DIR}
