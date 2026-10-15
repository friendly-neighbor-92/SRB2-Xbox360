# Source and license notes

This repository includes SRB2 2.1.6 source code adapted for an Xbox 360 XDK target, along with platform integration and helper code.

## SRB2

The included `SRB2_LICENSE.txt` is the GNU General Public License version 2 text. SRB2 source files retain their own copyright and licensing headers; keep those notices intact. The port's modifications should be identified in the relevant files with dates where practical, as required by GPLv2 §2(a).

## Third-party code

The source tree contains third-party components (including Lua-related files) with their own notices. Refer to the headers and license notices in those directories. This note is not a complete software bill of materials and does not replace each component's license.

## Xbox 360 SDK

The Xbox 360 XDK is proprietary software and is not included. Build using a legally obtained SDK installation. Do not commit SDK headers, libraries, tools, credentials, or other proprietary SDK files to this repository.

## Runtime assets

This repository does not establish redistribution rights for external game data, WADs, or other runtime assets. Review each asset's terms before distributing it.

## Compliance caveat

This file is an organizational guide, not a legal opinion or a certification that every file in the archive has been audited. Before making a binary release, verify that the exact corresponding source, project/build scripts, copyright notices, and license texts are included, and check the terms for all third-party components and assets.
