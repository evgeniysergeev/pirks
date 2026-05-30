# Third party libraries

## Change policy

Any change under `third-party/` or `.gitmodules` must be reviewed as a
dependency change, not as ordinary source churn.

For every submodule SHA update, the pull request must include:

- old and new submodule SHA;
- upstream repository link;
- upstream release, tag, or commit notes when available;
- reason for the update;
- security impact, including known CVEs or advisories if applicable;
- license impact.

For every new, removed, or updated third-party dependency, update this file in
the same pull request. The entry should include the dependency purpose, source,
version or SHA, and license.

CI should fail when `third-party/` or `.gitmodules` changes without a matching
change to `docs/THIRD-PARTY.md`.

## clang-format-all

- Purpose: helper script for formatting project sources.
- Source: https://github.com/eklitzke/clang-format-all.git
- Version/SHA: `4d6ee56f1bdfd12d1172ed489d7ebc3760e33f9f`
  (`heads/master`).
- License: GPL-3.0-only, see `third-party/clang-format-all/LICENSE.txt`.

## CLI11

- Purpose: command-line argument and option parser.
- Source: https://github.com/CLIUtils/CLI11.git
- Version/SHA: `a4e5560c5d05bef5dac0c948fb305195c3b8c254`
  (`v1.7.1-533-ga4e5560`).
- License: BSD-3-Clause, see `third-party/CLI11/LICENSE`.

## deferral

- Purpose: scope-exit/deferred action helper.
- Source: https://github.com/justusc/deferral.git
- Version/SHA: `a07c5af66e3a3d41b8cafb65d5826886f36f1537`
  (`v1.0.0-1-ga07c5af`).
- License: MIT, see `third-party/deferral/LICENSE`.

## spdlog

- Purpose: logging library.
- Source: https://github.com/gabime/spdlog.git
- Version/SHA: `79524ddd08a4ec981b7fea76afd08ee05f83755d`
  (`v1.2.1-2548-g79524ddd`).
- License: MIT, see `third-party/spdlog/LICENSE`.
- Notes: bundled fmt dependency is also MIT-licensed according to the spdlog
  license file.

## TPCircularBuffer

- Purpose: circular audio buffer used by the macOS microphone capture path.
- Source: https://github.com/evgeniysergeev/TPCircularBuffer
- Version/SHA: `5beeeb81d9c328db99cde3631a81ed6cd92606a5`
  (`heads/master`).
- License: zlib-style permissive license in
  `third-party/TPCircularBuffer/README.markdown`; podspec declares MIT.
