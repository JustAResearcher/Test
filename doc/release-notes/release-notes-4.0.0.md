Meowcoin Core version 4.0.0 is now available.

This release updates Meowcoin to the Bitcoin Core v28 codebase, includes the
multi-algorithm LWMA difficulty work, and schedules SegWit activation for a
smooth transition.

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes in some cases), then install or
replace the binaries. Upgrading directly from older versions is supported, but
it might take some time if the data directory needs to be migrated.

Consensus Changes
=================

SegWit Activation
-----------------

SegWit is scheduled to activate at height 2072166 on all networks.
This corresponds to Aug 7, 2026 00:00:00 UTC assuming 60-second blocks
from genesis. This is a softfork (not a hardfork). Nodes should upgrade
before this height to fully enforce SegWit rules.

Mining and Difficulty
=====================

- Multi-algorithm LWMA difficulty adjustment is enabled.
- MeowPoW remains the primary PoW algorithm.
- Scrypt AuxPoW support is included where enabled by network parameters.

Compatibility
=============

Meowcoin Core is supported and tested on Windows 10+, macOS 11+, and Linux
with kernel 3.17+. It should also work on most other UNIX-like systems but is
not as frequently tested.

Notes
=====

- This release focuses on a smooth transition by scheduling SegWit activation
  ahead of time. No hardfork is required.
- Balances carry over as normal. This update does not reset or replace the
  chain; it only schedules a future softfork activation.
- Please report bugs or regressions in the project issue tracker.

FAQ
===

Will balances carry over?
--------------------------

Yes. This release does not change the mainnet genesis or reset the chain. All
existing UTXOs remain valid.

Is this a hardfork?
-------------------

No. SegWit is a softfork. Older nodes will continue to follow the chain but
will not enforce SegWit rules after activation.

When should operators upgrade?
------------------------------

Before height 2072166 so their nodes enforce SegWit rules at activation.
