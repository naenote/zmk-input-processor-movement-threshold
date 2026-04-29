# zmk-input-processor-movement-threshold

[English](#english) | [日本語](#日本語)

---

## 日本語

### 概要

トラックボールやトラックパッドを搭載したキーボードで、打鍵時の振動による微細な動作を無視するための ZMK Input Processor モジュールです。

キー入力の衝撃でトラックボールが微小に動いた場合、Auto Mouse Layer (AML) が誤ってアクティブになることがあります。このモジュールは X/Y 軸の移動量が閾値以下であれば、後続のプロセッサへの伝搬を停止することで AML の誤爆を防ぎます。

> **注意:** `ZMK_INPUT_PROC_STOP` は後続プロセッサだけでなく HID 更新（ポインタ移動）も止めます。閾値以下のときにポインタを動かしつつ AML を発動させないためには、後述の **2 リスナー構成**を使用してください。

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
      revision: main
  self:
    path: config
```

### プロセッサの種類

#### `zmk,input-processor-movement-threshold`

X または Y の絶対値が閾値**以下**の場合、後続のプロセッサへの伝搬を停止します（HID 更新も止まります）。

| パラメータ | 説明 |
|-----------|------|
| `threshold` | 移動量の閾値。X または Y の絶対値がこの値以下で停止。 |

#### `zmk,input-processor-movement-drop`

X/Y 移動イベントを常に停止します。AML 専用リスナーの**最後尾**に置き、HID 二重更新を防ぐために使用します（後述の 2 リスナー構成を参照）。

パラメータなし（`#input-processor-cells = <0>`）。

---

### 使い方

#### パターン 1: AML を完全にブロック（閾値以下はポインタも止まる）

閾値以下の動きはポインタ移動ごと捨てて構わない場合の、シンプルな構成です。

```conf
# .conf
CONFIG_ZMK_POINTING=y
```

```dts
/ {
    zip_movement_threshold: zip_movement_threshold {
        compatible = "zmk,input-processor-movement-threshold";
        #input-processor-cells = <1>;
    };

    trackball_listener: trackball_listener {
        compatible = "zmk,input-listener";
        device = <&trackball>;
        input-processors =
            <&zip_movement_threshold 8>,
            <&zip_temp_layer MOUSE 300>;
    };
};
```

#### パターン 2: ポインタは常に動かしつつ、大きな動きのみ AML を発動（2 リスナー構成）

閾値以下でもポインタを動かしたい（AML だけを抑制したい）場合は、**同一デバイスに 2 つの input-listener を定義**します。

- **Listener A（HID 専用）**: プロセッサなし。全イベントがポインタ移動に使われる。
- **Listener B（AML 専用）**: 閾値プロセッサ → AML プロセッサ → `zip_movement_drop`。最後の `zip_movement_drop` が HID 更新を防いで二重移動を避ける。

```conf
# .conf
CONFIG_ZMK_POINTING=y
```

```dts
/ {
    /* Listener A: HID 専用（プロセッサなし、AML なし） */
    trackball_listener: trackball_listener {
        compatible = "zmk,input-listener";
        device = <&trackball>;
        /* input-processors は省略 — 全移動がポインタに伝わる */
    };

    zip_movement_threshold: zip_movement_threshold {
        compatible = "zmk,input-processor-movement-threshold";
        #input-processor-cells = <1>;
    };

    zip_movement_drop: zip_movement_drop {
        compatible = "zmk,input-processor-movement-drop";
        #input-processor-cells = <0>;
    };

    /* Listener B: AML 専用（HID 更新なし） */
    trackball_aml_listener: trackball_aml_listener {
        compatible = "zmk,input-listener";
        device = <&trackball>;
        input-processors =
            <&zip_movement_threshold 8>,   /* 閾値以下は STOP → AML 発動しない */
            <&zip_temp_layer MOUSE 300>,   /* 閾値超えのみここに届く → AML 発動 */
            <&zip_movement_drop>;          /* HID 更新をブロック（二重移動防止） */
    };
};
```

**動作フロー:**

| 移動量 | Listener A（ポインタ） | Listener B（AML） |
|--------|----------------------|-----------------|
| 閾値以下 | ポインタ移動 ✓ | threshold が STOP → AML 発動しない ✓ |
| 閾値超え | ポインタ移動 ✓ | threshold CONTINUE → AML 発動 ✓ → drop が HID をブロック ✓ |

---

## English

### Overview

A ZMK Input Processor module that suppresses spurious trackball/trackpad movement caused by keyboard vibration during keystrokes.

When typing, physical vibrations can cause the trackball to register tiny movements, unintentionally activating the Auto Mouse Layer (AML). This module stops propagation to subsequent processors when an X/Y movement event's absolute value is at or below a configurable threshold, preventing false AML triggers.

> **Note:** `ZMK_INPUT_PROC_STOP` halts both subsequent processors **and** the HID update (pointer movement). To keep the pointer moving for sub-threshold events while still suppressing AML, use the **dual-listener pattern** described below.

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
      revision: main
  self:
    path: config
```

### Processor Types

#### `zmk,input-processor-movement-threshold`

Stops the processor chain (including HID update) when the absolute value of an X or Y event is **at or below** the threshold.

| Parameter | Description |
|-----------|-------------|
| `threshold` | Movement threshold. Events with abs(value) ≤ threshold are stopped. |

#### `zmk,input-processor-movement-drop`

Unconditionally stops X/Y movement events. Place it **last** in a dedicated AML-only listener to prevent duplicate HID updates (see dual-listener pattern below).

No parameters (`#input-processor-cells = <0>`).

---

### Usage

#### Pattern 1: Block AML entirely (pointer also stops for sub-threshold moves)

Simple single-listener setup when you are comfortable with the pointer not moving for sub-threshold events.

```conf
# .conf
CONFIG_ZMK_POINTING=y
```

```dts
/ {
    zip_movement_threshold: zip_movement_threshold {
        compatible = "zmk,input-processor-movement-threshold";
        #input-processor-cells = <1>;
    };

    trackball_listener: trackball_listener {
        compatible = "zmk,input-listener";
        device = <&trackball>;
        input-processors =
            <&zip_movement_threshold 8>,
            <&zip_temp_layer MOUSE 300>;
    };
};
```

#### Pattern 2: Pointer always moves, AML only activates for large movements (dual-listener)

When you want the pointer to move even for small movements and only suppress AML, define **two input-listeners for the same device**.

- **Listener A (HID only)**: no processors — all events reach the HID pointer.
- **Listener B (AML only)**: threshold → AML processor → `zip_movement_drop`. The final `zip_movement_drop` blocks the HID update in this listener, preventing double pointer movement.

```conf
# .conf
CONFIG_ZMK_POINTING=y
```

```dts
/ {
    /* Listener A: HID only (no processors, no AML) */
    trackball_listener: trackball_listener {
        compatible = "zmk,input-listener";
        device = <&trackball>;
        /* no input-processors — all movement reaches the pointer */
    };

    zip_movement_threshold: zip_movement_threshold {
        compatible = "zmk,input-processor-movement-threshold";
        #input-processor-cells = <1>;
    };

    zip_movement_drop: zip_movement_drop {
        compatible = "zmk,input-processor-movement-drop";
        #input-processor-cells = <0>;
    };

    /* Listener B: AML only (no HID update) */
    trackball_aml_listener: trackball_aml_listener {
        compatible = "zmk,input-listener";
        device = <&trackball>;
        input-processors =
            <&zip_movement_threshold 8>,   /* sub-threshold → STOP, AML not triggered */
            <&zip_temp_layer MOUSE 300>,   /* only reached for above-threshold moves → AML activates */
            <&zip_movement_drop>;          /* blocks HID update to prevent double movement */
    };
};
```

**Behaviour matrix:**

| Movement | Listener A (pointer) | Listener B (AML) |
|----------|---------------------|-----------------|
| ≤ threshold | pointer moves ✓ | threshold STOPs → AML not triggered ✓ |
| > threshold | pointer moves ✓ | threshold passes → AML activates ✓ → drop blocks HID ✓ |
