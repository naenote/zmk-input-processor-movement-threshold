# zmk-input-processor-movement-threshold

[English](#english) | [日本語](#日本語)

---

## 日本語

### 概要

トラックボールやトラックパッドを搭載したキーボードで、打鍵時の振動による微細な動作を無視するための ZMK Input Processor モジュールです。

キー入力の衝撃でトラックボールが微小に動いた場合、Auto Mouse Layer (AML) が誤ってアクティブになることがあります。このモジュールは X/Y 軸の移動量が閾値以下であれば値を 0 に丸めることで、AML の誤爆を防ぎます。

### インストール

`zmk-config` の `config/west.yml` にこのモジュールを追加します。

```yaml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    - name: naenote
      url-base: https://github.com/naenote
  projects:
    - name: zmk
      remote: zmkfirmware
      revision: v0.1.0
      import: app/west.yml
    - name: zmk-input-processor-movement-threshold
      remote: naenote
      revision: v0.1.0
  self:
    path: config
```

### 使い方

#### 1. キーボードの `.conf` ファイル

```conf
CONFIG_ZMK_POINTING=y
```

> `CONFIG_ZMK_INPUT_PROCESSOR_MOVEMENT_THRESHOLD` は devicetree にノードが存在すれば自動的に有効になります。

#### 2. キーボードの `.overlay` / `.dtsi` ファイル

プロセッサノードを定義します。

```dts
/ {
    zip_movement_threshold: zip_movement_threshold {
        compatible = "zmk,input-processor-movement-threshold";
        #input-processor-cells = <1>;
    };
};
```

input listener の `input-processors` に追加します。閾値の数値（例: `8`）は環境に合わせて調整してください。

```dts
/ {
    trackball_listener: trackball_listener {
        compatible = "zmk,input-listener";
        device = <&trackball>;
        input-processors = <&zip_movement_threshold 8>;
    };
};
```

### パラメータ

| パラメータ | 説明 |
|-----------|------|
| `threshold` | 無視する移動量の閾値。X または Y の絶対値がこの値**以下**の場合、移動量を 0 に丸めます。 |

値を大きくするほど多くの微細な動作を無視しますが、意図した微小移動も失われます。実際の打鍵環境でテストしながら調整してください。

---

## English

### Overview

A ZMK Input Processor module that suppresses spurious trackball/trackpad movement caused by keyboard vibration during keystrokes.

When typing, physical vibrations can cause the trackball to register tiny movements, unintentionally activating the Auto Mouse Layer (AML). This module zeroes out X/Y movement events whose absolute value is at or below a configurable threshold, preventing false AML triggers.

### Installation

Add this module to your `zmk-config`'s `config/west.yml`:

```yaml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    - name: naenote
      url-base: https://github.com/naenote
  projects:
    - name: zmk
      remote: zmkfirmware
      revision: v0.1.0
      import: app/west.yml
    - name: zmk-input-processor-movement-threshold
      remote: naenote
      revision: v0.1.0
  self:
    path: config
```

### Usage

#### 1. Keyboard `.conf` file

```conf
CONFIG_ZMK_POINTING=y
```

> `CONFIG_ZMK_INPUT_PROCESSOR_MOVEMENT_THRESHOLD` is enabled automatically when a compatible node is present in the devicetree.

#### 2. Keyboard `.overlay` / `.dtsi` file

Define the processor node:

```dts
/ {
    zip_movement_threshold: zip_movement_threshold {
        compatible = "zmk,input-processor-movement-threshold";
        #input-processor-cells = <1>;
    };
};
```

Add it to your input listener's `input-processors`. Adjust the threshold value (e.g. `8`) to suit your hardware:

```dts
/ {
    trackball_listener: trackball_listener {
        compatible = "zmk,input-listener";
        device = <&trackball>;
        input-processors = <&zip_movement_threshold 8>;
    };
};
```

### Parameters

| Parameter | Description |
|-----------|-------------|
| `threshold` | Movement threshold. If the absolute value of an X or Y event is **less than or equal to** this value, the movement is zeroed out. |

A larger value suppresses more noise but may also discard intentional small movements. Tune it while testing with your actual typing environment.
