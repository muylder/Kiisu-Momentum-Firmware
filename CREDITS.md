# Credits and attribution

Kiisu Momentum Firmware integrates work from many independent authors. This fork claims only its own changes. Names and links below acknowledge contributions and do not imply endorsement.

## Firmware and Kiisu foundations

| Author / project | Contribution and source |
| --- | --- |
| Flipper Devices and Flipper Zero firmware contributors | Original firmware platform, services, tooling and libraries: [flipperzero-firmware](https://github.com/flipperdevices/flipperzero-firmware). |
| Next-Flip / Momentum team and contributors | Momentum firmware foundation: [Momentum-Firmware](https://github.com/Next-Flip/Momentum-Firmware), including its Xtreme lineage. |
| HiennNek / hiennek and contributors | [Kiisu-MNTM](https://github.com/HiennNek/kiisu-mntm), Kiisu integration and [external-app collection](https://github.com/HiennNek/Momentum-Apps-Kiisu-MNTM). |
| DarkFlippers / Unleashed team and contributors | Inherited firmware features and documentation: [unleashed-firmware](https://github.com/DarkFlippers/unleashed-firmware). |
| kiisu-io and contributors | Kiisu platform and companion foundations: [kiisu-firmware](https://github.com/kiisu-io/kiisu-firmware), [kiisu4-companion-fw](https://github.com/kiisu-io/kiisu4-companion-fw). |
| twoel / twoelw / 2elw | Kiisu apps and enhanced companion firmware, detailed below. |

[CONTRIBUTORS.md](CONTRIBUTORS.md) lists the names recorded in reachable firmware and external-app Git history, including Willy-JL/WillyJL, MX, RogueMaster, Clara K, and many others. Historical names are retained as recorded; this is not a claim that every contribution survives in the current build.

## twoelw / 2elw

The checked-in manifests explicitly credit **2elw** for all three apps:

| Component | Original project | Local attribution / license |
| --- | --- | --- |
| Kiisu Manager | [twoelw/kiisu-manager](https://github.com/twoelw/kiisu-manager) | [Manifest](applications/system/kiisu-manager/application.fam), [Apache-2.0 license](applications/system/kiisu-manager/LICENSE) |
| Kiisu Sensor Hub | [twoelw/kiisu-sensor-hub](https://github.com/twoelw/kiisu-sensor-hub) | [Manifest](applications/system/kiisu-sensor-hub/application.fam), [Apache-2.0 license](applications/system/kiisu-sensor-hub/LICENSE) |
| Kiisu Companion Bridge | [twoelw/kiisu-companion-bridge](https://github.com/twoelw/kiisu-companion-bridge) | [Manifest](applications/system/kiisu-companion-bridge/application.fam), [License](applications/system/kiisu-companion-bridge/LICENSE) |
| Enhanced Kiisu companion firmware | [twoelw/enhanced-kiisu4-fw](https://github.com/twoelw/enhanced-kiisu4-fw), derived from kiisu-io's companion firmware | Referenced by the local Manager and Bridge documentation. The exact provenance of the bundled `aux-fw-22-05-26.bin` still needs verification. |

## External applications

Credit belongs to each app's original authors and subsequent contributors, not solely to the firmware packager. The [application inventory](documentation/third_party_attributions.md#application-authors-and-sources) records author fields, manifests and available source URLs for 554 app declarations, including plugins and examples.

The [external-app README](https://github.com/HiennNek/Momentum-Apps-Kiisu-MNTM/blob/e1d6d02ad0d4980e31af4ceb3814106912111ef5/README.md) documents the Momentum collection and its use of original repositories and [xMasterX's all-the-plugins](https://github.com/xMasterX/all-the-plugins). The individual subtree records retain those source references.

## Artwork, animations and fonts

The [original asset-pack credits](assets/packs/ReadMe.md) name **Kuronons**, **WrenchAtHome**, **David Libeau**, **Gissio**, **Ife Mena**, and **WillyJL** for their respective animations, portraits, backgrounds, fonts and other assets. Consult that file for the exact contribution and original links. Kiisu-MNTM's inherited artwork and its contributors are also acknowledged; this fork does not claim authorship of imported artwork.

Credit alone does not establish permission to redistribute third-party artwork or fonts. Preserve their original notices and check their own terms.

## Libraries, drivers and tooling

The [dependency inventory](documentation/third_party_attributions.md#pinned-submodules) records upstream URLs and pinned revisions for M*LIB, nanopb, libusb_stm32, FreeRTOS, microtar, Mbed TLS, heatshrink, STM32WB CMSIS/HAL/coprocessor components, uzlib, protobuf definitions and doxygen-awesome-css, as well as the external-app collection.

Additional vendored components include **Cesanta's mJS** ([local documentation](documentation/js/js_about.md)), **olikraus's U8g2** ([source notice](lib/u8g2/u8g2.h)), **ChaN's FatFs** ([source notice](lib/fatfs/ff.c)), and **Arm's CMSIS Core** ([source notice](lib/cmsis_core/core_cm4.h)). Nested libraries retain their own authors and notices; see the [notice-file index](documentation/third_party_attributions.md#license-copyright-author-and-notice-files).

## Licenses and remaining provenance work

The repository contains the [GNU GPL v3 license](LICENSE). Individual dependencies, apps, assets and companion components may carry different licenses. Their original copyright statements, license texts, attribution requirements and NOTICE files must remain intact. This document does not replace those notices, relicense third-party work, or certify license compliance.

For binary distribution, attribution is only one part of compliance: applicable licenses may also require corresponding source, license copies, notices and identification of modifications. See the [GNU license FAQ](https://www.gnu.org/licenses/gpl-faq.en.html) and each component's actual license.

The [binary inventory](documentation/third_party_attributions.md#bundled-binaries-provenance-still-to-confirm) identifies **18 additional FAPs and one companion firmware binary** whose exact source revisions and license mapping have not yet been verified. They are already present in this branch; a credit list alone does not resolve that gap. Original download URLs and matching source/license records are still needed.

To correct attribution, [open an issue](https://github.com/muylder/Kiisu-Momentum-Firmware/issues) with the component path, author name and original source. These indexes are evidence-based records, not an assertion that every copyright holder has been identified.
