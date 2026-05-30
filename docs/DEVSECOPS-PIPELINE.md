# План добавления DevSecOps pipeline

Цель: добавить в Pirks воспроизводимый DevSecOps pipeline, который проверяет
сборку, тесты, форматирование, статический анализ, секреты, зависимости,
лицензии и security-анализ C++ кода до merge в основную ветку.

Базовое предположение: основной CI будет GitHub Actions. Если проект переедет
в GitLab CI, Jenkins или Azure Pipelines, сначала стоит оставить те же локальные
скрипты и команды, а затем перенести только слой orchestration.

## 1. Зафиксировать текущую карту проекта

Описание: перед добавлением CI нужно явно описать, что именно pipeline обязан
собирать и проверять. В этом репозитории уже есть CMake-проект C++23, GoogleTest
через CTest, `BUILD_TESTS`, `TEST_MICROPHONE`, submodules в `third-party`,
`scripts/clang-format-all.sh` и `scripts/cppcheck-all.sh`.

Команды:

```powershell
git status --short
git submodule status --recursive
cmake --version
ctest --version
```

Ожидаемый результат:

- рабочее дерево чистое или известны все незакоммиченные изменения;
- submodules инициализированы;
- понятна минимальная версия CMake на CI runner;
- известно, какие тесты можно запускать без доступа к микрофону.

Важно: `microphone-test` зависит от прав ОС и доступного аудиоустройства, поэтому
обычный PR pipeline должен собирать проект с `-DTEST_MICROPHONE=OFF`. Отдельный
manual/nightly job можно оставить для self-hosted runner с настроенным audio
device.

Snapshot текущей карты проекта сохранен в
`docs/DEVSECOPS-PROJECT-MAP.md`. На момент snapshot CTest показывает тесты
из `build/test`, но не из корня `build`; перед первым CI gate нужно либо
перенести `enable_testing()` в корневой `CMakeLists.txt`, либо запускать CTest
из `build/<ci-dir>/test`.

## 2. Ввести единые локальные CI-команды

Описание: pipeline не должен содержать уникальную магию только в YAML. Сначала
нужно добавить локальные команды или скрипты, которые разработчик может запустить
до push.

Предлагаемые файлы:

```text
scripts/ci/configure.ps1
scripts/ci/build.ps1
scripts/ci/test.ps1
scripts/ci/format-check.ps1
scripts/ci/static-analysis.ps1
scripts/ci/security-scan.ps1
scripts/ci/configure.sh
scripts/ci/build.sh
scripts/ci/test.sh
scripts/ci/format-check.sh
scripts/ci/static-analysis.sh
scripts/ci/security-scan.sh
```

Команда создания директорий:

```powershell
New-Item -ItemType Directory -Force scripts\ci
New-Item -ItemType Directory -Force build\reports
```

Минимальный baseline для Windows PowerShell:

```powershell
cmake -S . -B build\ci-windows -DBUILD_TESTS=ON -DTEST_MICROPHONE=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build\ci-windows --config Debug --parallel
ctest --test-dir build\ci-windows -C Debug --output-on-failure --parallel 4
```

Минимальный baseline для Linux/macOS shell:

```bash
cmake -S . -B build/ci -G Ninja -DBUILD_TESTS=ON -DTEST_MICROPHONE=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build/ci --parallel
ctest --test-dir build/ci --output-on-failure --parallel 4
```

Definition of done:

- команды проходят локально хотя бы на Windows и WSL/Linux;
- `TEST_MICROPHONE=OFF` исключает аппаратно-зависимые тесты из обычного CI;
- отчеты складываются в `build/reports`;
- временные директории не попадают в git.

## 3. Добавить build-and-test matrix

Описание: первый CI gate должен ловить регрессии сборки и unit-тестов на основных
платформах проекта: Windows, Linux и macOS.

Создать workflow:

```powershell
New-Item -ItemType Directory -Force .github\workflows
New-Item -ItemType File -Force .github\workflows\ci.yml
```

Рекомендуемые job'ы:

- `build-test-windows`: MSVC, Debug, `BUILD_TESTS=ON`, `TEST_MICROPHONE=OFF`;
- `build-test-linux`: GCC или Clang, Ninja, Debug, `BUILD_TESTS=ON`,
  `TEST_MICROPHONE=OFF`;
- `build-test-macos`: AppleClang, Debug, `BUILD_TESTS=ON`,
  `TEST_MICROPHONE=OFF`.

Команды для Linux runner:

```bash
sudo apt-get update
sudo apt-get install -y cmake ninja-build build-essential pkg-config libpulse-dev
cmake -S . -B build/ci -G Ninja -DBUILD_TESTS=ON -DTEST_MICROPHONE=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build/ci --parallel
ctest --test-dir build/ci --output-on-failure --parallel 4
```

Команды для Windows runner:

```powershell
cmake -S . -B build\ci -DBUILD_TESTS=ON -DTEST_MICROPHONE=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build\ci --config Debug --parallel
ctest --test-dir build\ci -C Debug --output-on-failure --parallel 4
```

Команды для macOS runner:

```bash
brew install ninja pkg-config
cmake -S . -B build/ci -G Ninja -DBUILD_TESTS=ON -DTEST_MICROPHONE=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build/ci --parallel
ctest --test-dir build/ci --output-on-failure --parallel 4
```

YAML-заметки:

- использовать `actions/checkout` с `submodules: recursive`;
- выставить минимальные `permissions`, например `contents: read`;
- добавить `concurrency`, чтобы отменять старые runs для того же PR;
- после первого green run закрепить actions по commit SHA, а не только по tag;
- кешировать build можно позже, после стабилизации pipeline.

Definition of done:

- PR получает обязательные checks для всех трех платформ;
- build logs достаточно подробные для диагностики;
- падение любого unit-теста блокирует merge.

## 4. Добавить проверку форматирования

Описание: в репозитории уже есть `.clang-format` и `scripts/clang-format-all.sh`.
Этот скрипт форматирует файлы in-place, поэтому в CI нужно запускать его и затем
проверять отсутствие diff.

Команды:

```bash
./scripts/clang-format-all.sh
git diff --exit-code -- src test
```

PowerShell-вариант:

```powershell
bash ./scripts/clang-format-all.sh
git diff --exit-code -- src test
```

Если хочется избежать изменения файлов в CI, добавить отдельный check-only
скрипт:

```bash
find src test -name '*.h' -o -name '*.cpp' -o -name '*.mm' -o -name '*.m' | \
  xargs clang-format --dry-run -Werror
```

Definition of done:

- форматирование является отдельным быстрым required check;
- PR с неформатированным C++ кодом падает до дорогих security job'ов;
- third-party исходники не форматируются.

## 5. Добавить статический анализ C++

Описание: для быстрого PR gate использовать умеренный cppcheck/clang-tidy
профиль по `src` и `test`. Полный exhaustive scan можно запускать nightly, потому
что существующий `scripts/cppcheck-all.sh` дорогой и сейчас настроен на глубокую
проверку.

Быстрый cppcheck для PR:

```bash
cmake -S . -B build/analysis -G Ninja -DBUILD_TESTS=ON -DTEST_MICROPHONE=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cppcheck \
  --enable=warning,style,performance,portability \
  --std=c++23 \
  --project=build/analysis/compile_commands.json \
  --suppressions-list=scripts/suppressions.txt \
  --inline-suppr \
  --error-exitcode=1 \
  -ithird-party \
  -ibuild \
  --template=gcc
```

Полный cppcheck для nightly:

```bash
./scripts/cppcheck-all.sh > build/reports/cppcheck.log
```

clang-tidy для PR или scheduled job:

```bash
cmake -S . -B build/tidy -G Ninja -DBUILD_TESTS=ON -DTEST_MICROPHONE=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
run-clang-tidy -p build/tidy src test
```

Definition of done:

- cppcheck падает на новых предупреждениях в `src` и `test`;
- существующие suppressions документированы в `scripts/suppressions.txt`;
- exhaustive scan сохраняет лог как artifact;
- clang-tidy начинается с небольшого набора checks, затем расширяется.

## 6. Добавить CodeQL для C/C++

Описание: CodeQL дает SAST-анализ и интеграцию с GitHub Security tab. Для C++
лучше использовать manual build, чтобы CodeQL видел реальную CMake-сборку.

Команды manual build внутри CodeQL job:

```bash
cmake -S . -B build/codeql -G Ninja -DBUILD_TESTS=ON -DTEST_MICROPHONE=OFF
cmake --build build/codeql --parallel
```

Рекомендуемые настройки workflow:

- `permissions: security-events: write, contents: read`;
- запуск на `pull_request`, `push` в основную ветку и `schedule`;
- `languages: cpp`;
- `build-mode: manual`;
- upload результата через CodeQL analyze step;
- actions закрепить по SHA после стабилизации.

Definition of done:

- CodeQL results появляются в Security tab;
- high/critical findings требуют triage перед merge;
- false positive помечается только с коротким обоснованием.

## 7. Добавить secret scanning

Описание: секреты должны ловиться до merge. Для локального и CI-сканирования
подойдет Gitleaks, а на GitHub дополнительно стоит включить встроенный secret
scanning в настройках репозитория.

Команды:

```bash
gitleaks detect --source . --redact --report-format sarif --report-path build/reports/gitleaks.sarif
```

Локальная проверка перед push:

```bash
gitleaks protect --staged --redact
```

Definition of done:

- secret scan запускается на PR;
- SARIF загружается в Security tab или сохраняется как artifact;
- в документации описан порядок ротации секрета при срабатывании;
- секреты для CI хранятся только в GitHub Actions secrets/environments.

## 8. Добавить dependency и supply-chain checks

Описание: проект хранит зависимости как git submodules и vendored source в
`third-party`, поэтому нужно проверять не только package manifests, но и сами
submodule SHAs, лицензии и SBOM.

Инвентаризация:

```bash
git submodule status --recursive > build/reports/submodules.txt
git ls-tree HEAD third-party > build/reports/third-party-tree.txt
```

SBOM:

```bash
syft dir:. -o cyclonedx-json=build/reports/sbom.cdx.json
syft dir:. -o spdx-json=build/reports/sbom.spdx.json
```

Vulnerability scan по SBOM:

```bash
grype sbom:build/reports/sbom.cdx.json --fail-on high
```

OSV scan:

```bash
osv-scanner --recursive --format sarif --output build/reports/osv.sarif .
```

GitHub Dependency Review для PR:

```text
Добавить отдельный job dependency-review, который запускается только на
pull_request и блокирует known vulnerable dependency updates.
```

Definition of done:

- SBOM публикуется как artifact для каждого release build;
- PR, обновляющий submodule, показывает old/new SHA и ссылку на upstream release;
- high/critical vulnerabilities блокируют merge, если нет принятого waiver;
- `docs/THIRD-PARTY.md` обновляется вместе с изменениями в `third-party`.

CI guard для этого правила:

```bash
changed_files="$(git diff --name-only origin/main...HEAD)"
if echo "$changed_files" | grep -Eq '^(\.gitmodules|third-party/)'; then
  echo "$changed_files" | grep -q '^docs/THIRD-PARTY.md$' || {
    echo "third-party changes require docs/THIRD-PARTY.md update"
    exit 1
  }
fi
```

## 9. Добавить license compliance gate

Описание: Pirks распространяется под AGPL-3.0-or-later, а `third-party` содержит
исходники с собственными лицензиями. Нужно не только хранить список библиотек,
но и проверять, что новые зависимости не нарушают лицензионную политику.

Команды для первичного аудита:

```bash
syft dir:. -o table > build/reports/dependencies.txt
scancode --license --summary --json-pp build/reports/scancode-licenses.json third-party
```

Минимальная политика:

- разрешены совместимые open-source лицензии, явно перечисленные в
  `docs/THIRD-PARTY.md`;
- новая vendored dependency требует обновления `docs/THIRD-PARTY.md`;
- для GPL/AGPL/LGPL/MPL зависимостей нужен отдельный review;
- неизвестная лицензия блокирует merge.

Definition of done:

- license report сохраняется как artifact;
- изменение `third-party` без изменения `docs/THIRD-PARTY.md` падает в CI;
- waiver для лицензии хранится в репозитории, а не только в комментарии PR.

## 10. Добавить sanitizer jobs

Описание: sanitizers хорошо ловят memory safety и undefined behavior проблемы.
Их лучше запускать на Linux Clang в PR или хотя бы nightly. Windows/macOS можно
добавить позже, когда Linux job стабилен.

AddressSanitizer + UndefinedBehaviorSanitizer:

```bash
cmake -S . -B build/asan-ubsan -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTS=ON \
  -DTEST_MICROPHONE=OFF \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build build/asan-ubsan --parallel
ctest --test-dir build/asan-ubsan --output-on-failure --parallel 4
```

ThreadSanitizer, если появятся многопоточные тесты:

```bash
cmake -S . -B build/tsan -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTS=ON \
  -DTEST_MICROPHONE=OFF \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_CXX_FLAGS="-fsanitize=thread -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=thread"
cmake --build build/tsan --parallel
ctest --test-dir build/tsan --output-on-failure --parallel 4
```

Definition of done:

- ASan/UBSan job проходит на Linux;
- sanitizer failures блокируют merge для кода в `src` и `test`;
- flaky или hardware-dependent тесты не входят в sanitizer gate.

## 11. Добавить coverage reporting

Описание: coverage не является security-гарантией, но помогает видеть, где
security-critical код не покрыт тестами.

Команды:

```bash
cmake -S . -B build/coverage -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTS=ON \
  -DTEST_MICROPHONE=OFF \
  -DCMAKE_CXX_FLAGS="--coverage -O0 -g" \
  -DCMAKE_EXE_LINKER_FLAGS="--coverage"
cmake --build build/coverage --parallel
ctest --test-dir build/coverage --output-on-failure --parallel 4
gcovr -r . --exclude 'third-party/.*' --exclude 'build/.*' --xml-pretty -o build/reports/coverage.xml
gcovr -r . --exclude 'third-party/.*' --exclude 'build/.*' --html-details build/reports/coverage.html
```

Definition of done:

- coverage XML/HTML сохраняются как artifacts;
- threshold вводится постепенно, например сначала informational, затем required;
- падение coverage threshold не блокирует срочные security fixes без waiver.

## 12. Добавить fuzzing для парсеров и сетевых сообщений

Описание: в проекте есть сетевые и control-plane сообщения. Для них полезны
libFuzzer targets, особенно для `ControlPlaneMessage` и будущих packet parsers.

Предлагаемые файлы:

```text
test/fuzz/CMakeLists.txt
test/fuzz/ControlPlaneMessageFuzz.cpp
corpus/control-plane-message/
```

Новая CMake option:

```cmake
option(BUILD_FUZZERS "Build fuzz targets" OFF)
```

Команды:

```bash
cmake -S . -B build/fuzz -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTS=OFF \
  -DBUILD_FUZZERS=ON \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_CXX_FLAGS="-fsanitize=fuzzer,address,undefined"
cmake --build build/fuzz --parallel
./build/fuzz/control-plane-message-fuzz -max_total_time=60 corpus/control-plane-message
```

Definition of done:

- хотя бы один fuzz target проверяет parsing/serialization path;
- PR job запускает короткий smoke fuzzing;
- nightly job запускает более длинный fuzzing;
- crash artifacts сохраняются для воспроизведения.

## 13. Добавить hardening для release builds

Описание: security pipeline должен проверять, что release-сборки используют
базовые hardening flags. Эти флаги лучше добавить через CMake interface target,
а не размазывать по workflow.

Linux hardening candidates:

```text
-fstack-protector-strong
-D_FORTIFY_SOURCE=3
-fPIE
-Wl,-z,relro
-Wl,-z,now
-Wl,-z,noexecstack
```

MSVC hardening candidates:

```text
/guard:cf
/Qspectre
/DYNAMICBASE
/NXCOMPAT
```

Проверочные команды:

```bash
cmake -S . -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DTEST_MICROPHONE=OFF
cmake --build build/release --parallel
ctest --test-dir build/release --output-on-failure --parallel 4
```

Дополнительная Linux-проверка binary hardening:

```bash
checksec --file=build/release/src/server/pirks-server
```

Definition of done:

- release build проходит tests;
- hardening flags документированы;
- проверка не применяется к third-party targets, если это ломает сборку.

## 14. Добавить artifact, SBOM и provenance для release

Описание: когда появится release packaging, pipeline должен публиковать binaries,
checksums, SBOM и provenance/attestation.

Команды:

```bash
cmake -S . -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
cmake --build build/release --parallel
syft dir:. -o cyclonedx-json=build/reports/sbom.cdx.json
sha256sum build/release/src/server/pirks-server > build/reports/SHA256SUMS
```

Если используется Sigstore/cosign:

```bash
cosign sign-blob --yes --output-signature build/reports/pirks-server.sig build/release/src/server/pirks-server
cosign verify-blob --signature build/reports/pirks-server.sig build/release/src/server/pirks-server
```

Definition of done:

- release artifacts имеют checksums;
- SBOM привязан к конкретному release commit;
- signing/provenance job имеет отдельные permissions и environment protection.

## 15. Настроить branch protection и required checks

Описание: pipeline приносит пользу только если его нельзя обойти случайным merge.
После стабилизации checks нужно включить branch protection.

Команды через GitHub CLI примерные, перед запуском заменить owner/repo/branch:

```bash
gh repo view --json nameWithOwner
gh api repos/OWNER/REPO/branches/main/protection \
  --method PUT \
  --field required_status_checks='{"strict":true,"contexts":["build-test-linux","build-test-windows","build-test-macos","format","static-analysis","secrets","dependency-review"]}' \
  --field enforce_admins=true \
  --field required_pull_request_reviews='{"required_approving_review_count":1,"dismiss_stale_reviews":true}' \
  --field restrictions=null
```

Рекомендуемые required checks на старте:

- `format`;
- `build-test-linux`;
- `build-test-windows`;
- `build-test-macos`;
- `static-analysis`;
- `secrets`;
- `dependency-review`.

Рекомендуемые non-required checks на старте:

- `coverage`;
- `asan-ubsan`;
- `codeql`;
- `license-audit`;
- `sbom`;
- `fuzz-smoke`.

Definition of done:

- merge в `main` невозможен при красном required check;
- required checks не включают заведомо flaky jobs;
- emergency bypass описан и требует явного review.

## 16. Ввести правила triage и waiver

Описание: security pipeline неизбежно будет находить false positives и legacy
issues. Нужен короткий процесс, чтобы не превращать CI в шум.

Предлагаемый файл:

```text
docs/SECURITY-TRIAGE.md
```

Минимальные правила:

- critical/high findings блокируют merge;
- medium findings требуют issue или fix перед release;
- false positive фиксируется в suppressions/waiver с причиной и сроком пересмотра;
- suppressions без ссылки на issue не принимаются;
- повторное появление закрытого finding считается regression.

Шаблон waiver:

```markdown
## Finding

- Tool:
- Rule:
- File:
- Severity:
- Reason:
- Expiration:
- Tracking issue:
```

Definition of done:

- suppressions прозрачны для reviewer;
- каждое исключение имеет владельца и срок;
- security debt виден в issues или отдельном TODO.

## 17. Рекомендуемая очередность внедрения

Этап 1: базовый CI.

```text
scripts/ci/*
.github/workflows/ci.yml
```

Проверки: configure, build, ctest, format.

Этап 2: быстрый security gate.

```text
.github/workflows/security.yml
scripts/ci/static-analysis.*
scripts/ci/security-scan.*
```

Проверки: cppcheck, gitleaks, dependency review, SBOM generation.

Этап 3: глубокий анализ.

```text
.github/workflows/codeql.yml
.github/workflows/nightly.yml
```

Проверки: CodeQL, exhaustive cppcheck, license audit, ASan/UBSan, coverage.

Этап 4: supply-chain hardening.

```text
docs/SECURITY-TRIAGE.md
docs/THIRD-PARTY.md updates
release artifact jobs
```

Проверки: signed artifacts, release SBOM, provenance, branch protection.

Этап 5: fuzzing.

```text
test/fuzz/*
corpus/*
BUILD_FUZZERS CMake option
```

Проверки: short PR fuzz smoke и nightly fuzz job.

## 18. Итоговый acceptance checklist

- PR pipeline запускается на `pull_request`.
- Push pipeline запускается на основной ветке.
- Scheduled pipeline запускает дорогие проверки хотя бы раз в сутки или неделю.
- Сборка и CTest проходят на Windows, Linux и macOS.
- Microphone tests исключены из обычного CI и вынесены в manual/self-hosted job.
- Format check использует существующую `.clang-format`.
- cppcheck/clang-tidy проверяют `src` и `test`, но не шумят на `third-party`.
- CodeQL публикует результаты в Security tab.
- Gitleaks или аналог блокирует секреты.
- SBOM создается для release artifacts.
- Vulnerability scan падает на high/critical findings.
- License audit покрывает `third-party`.
- ASan/UBSan job есть хотя бы на Linux.
- Coverage report публикуется как artifact.
- Branch protection требует зеленые ключевые checks.
- Waiver/triage процесс задокументирован.
