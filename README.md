# CryptoWear

![issues](https://img.shields.io/github/issues/milan338/CryptoWear?style=flat-square)
![Release](https://img.shields.io/github/v/release/milan338/CryptoWear?include_prereleases&style=flat-square)
![Downloads](https://img.shields.io/github/downloads/milan338/CryptoWear/total?style=flat-square)
![size](https://img.shields.io/github/repo-size/milan338/CryptoWear?style=flat-square)

### ***Your crypto portfolio from the comfort of your wrist***

## Features

- Portfolio data direct from your exchanges
- Quickly view coin prices, movement, volume, and more
- Live price charts powered by [TradingView](https://www.tradingview.com/)
- Summary pie-chart of your top assets
- Browse the latest news for your coins

## Supported Exchanges

- Coinbase
- CoinSpot

Feel free to [submit a feature request](https://github.com/milan338/CryptoWear/issues/new/choose) if your exchange isn't yet supported.

## Usage

*Before using this software, ensure your mobile device supports the Samsung Accessory Protocol* (required for syncing exchange API keys to your wearable).

To connect to your exchange, first create a new **Read-Only** API key. Some exchanges like Coinbase use a permissions system instead. For this, make sure you give the API key access to all accounts you wish to track, and **only** allow the **wallet : accounts : read** permission.

Add your API key and its Secret Key to the companion app, then in your wearable in settings, select `sync keys`. Optionally, you can save these keys to your mobile device using the `save keys` button. Note, when syncing keys, any empty keys in the companion app will act to remove the associated keys from your wearable.

## Privacy

CryptoWear **does not** collect any data from users, and it **does not** employ any user tracking or analytics solutions.

CryptoWear needs internet access to get your portfolio data, to collect coin price data, to download coin information, to fetch news data, and to display live price charts. Your private exchange API keys are only ever sent directly to your exchanges, and your secret keys are only ever used to sign API requests, never leaving your device.

Your API keys are privately stored in [Tizen's secure keystore](https://developer.tizen.org/development/guides/native-application/security/secure-key-management) directly on your wearable, and are not stored anywhere else. You can also store these keys on your mobile device should you choose; even though these keys are stored in the [application's preferences storage](https://developer.android.com/training/data-storage/shared-preferences), if your device is rooted and you're worried about malicious access to these keys, you can always choose not to store them on your device. Keep in mind, without saving keys to your mobile device, updating one key will require re-inputting all other keys you wish to still use, since syncing empty keys will cause those keys to be removed from the wearable.

## Building it Yourself

### Prerequisites

- Tizen Studio
- Tizen 5.5 Wearable SDK
- Samsung Wearable Extension SDK
- Samsung Certificate Manager
- Android Studio

### Downloading

```
git clone https://github.com/milan338/CryptoWear.git
```

### Building for Android

Communication between the Android provider and Tizen client requires matching certificates on both ends. You can find how to create a new keystore [here](https://developer.android.com/studio/publish/app-signing).

To build the CryptoWear Companion app, Android Studio provides a tool to generate a signed APK under `Build > Generate Signed Bundle / APK`.

### Building for Tizen

Under `Properties > C/C++ Build > Tizen Settings`, in the *platform* section, ensure *Architecture* is set to `arm`, then under *Framework*, ensure `Native_API` and `samsung-sap` are enabled. If the latter framework does not appear, make sure *Samsung Wearable Extension* is installed in the Tizen Package Manager. The SAP library is necessary for facilitating a connection between an Android provider and the Tizen client to sync API keys.

The Tizen application must be signed using a Samsung mobile / wearable certificate. In Tizen's Certificate Manager, create a new Samsung certificate, select `Mobile / Wearable` as the device type, create a new certificate profile, then select `Create a new author certificate`. Under *Advanced options*, select `Use an existing author certificate` and set the certificate path to your Android keystore. Using this same keystore will allow both applications to securely authenticate.

Under the project settings, go to `Build Configurations > Set Active > Release`. Then simply run `Build Signed Project`.

## Media

![1](/media/img_1.jpg)

![2](/media/img_2.jpg) ![3](/media/img_3.jpg) ![4](/media/img_4.jpg)
