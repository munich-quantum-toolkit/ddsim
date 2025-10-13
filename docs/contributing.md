<!--- This file has been generated from an external template. Please do not modify it directly. -->
<!--- Changes should be contributed to https://github.com/munich-quantum-toolkit/templates. -->

# Contributing

Thank you for your interest in contributing to MQT DDSIM!
This document outlines how to contribute and the development guidelines.

We use GitHub to [host code](https://github.com/munich-quantum-toolkit/ddsim), to [track issues and feature requests][issues], as well as accept [pull requests](https://github.com/munich-quantum-toolkit/ddsim/pulls).
See <https://docs.github.com/en/get-started/quickstart> for a general introduction to working with GitHub and contributing to projects.

## Types of Contributions

Pick the path that fits your time and interests:

- 🐛 Report bugs:

  Use the _🐛 Bug report_ template at <https://github.com/munich-quantum-toolkit/ddsim/issues>.
  Include steps to reproduce, expected vs. actual behavior, environment, and a minimal example.

- 🛠️ Fix bugs:

  Browse [issues][issues], especially those labeled "bug", "help wanted", or "good first issue".
  Open a draft PR early to get feedback.

- 💡 Propose features:

  Use the _✨ Feature request_ template at <https://github.com/munich-quantum-toolkit/ddsim/issues>.
  Describe the motivation, alternatives considered, and (optionally) a small API sketch.

- ✨ Implement features:

  Pick items labeled "feature" or "enhancement".
  Coordinate in the issue first if the change is substantial; start with a draft PR.

- 📝 Improve documentation:

  Add or refine docstrings, tutorials, and examples; fix typos; clarify explanations.
  Small documentation-only PRs are very welcome.

- ⚡️ Performance and reliability:

  Profile hot paths, add benchmarks, reduce allocations, deflake tests, and improve error messages.

- 📦 Packaging and tooling:

  Improve build configuration, type hints/stubs, CI workflows, and platform wheels.
  Incremental tooling fixes have a big impact.

- 🙌 Community support:

  Triage issues, reproduce reports, and answer questions in Discussions:
  <https://github.com/munich-quantum-toolkit/ddsim/discussions>.

## Guidelines

Please adhere to the following guidelines to help the project grow sustainably.

### Core Guidelines

- ["Commit early and push often"](https://www.worklytics.co/blog/commit-early-push-often).
- Write meaningful commit messages, preferably using [gitmoji](https://gitmoji.dev) for additional context.
- Focus on a single feature or bug at a time and only touch relevant files.
  Split multiple features into separate contributions.
- Add tests for new features to ensure they work as intended.
- Document new features.
  For user-facing changes, add a changelog entry; for breaking changes, update the upgrade guide.
  For details, see {ref}`maintaining-changelog-upgrade-guide`.
- Add tests for bug fixes to demonstrate the fix.
- Document your code thoroughly and ensure it is readable.
- Keep your code clean by removing debug statements, leftover comments, and unrelated code.
- Check your code for style and linting errors before committing.
- Follow the project's coding standards and conventions.
- Be open to feedback and willing to make necessary changes based on code reviews.

### Pull Request Workflow

- Create PRs early.
  Work-in-progress PRs are welcome; mark them as drafts on GitHub.
- Use a clear title, reference related issues by number, and describe the changes.
  Follow the PR template; only omit the issue reference if not applicable.
- CI runs on all supported platforms and Python versions to build, test, format, and lint.
  All checks must pass before merging.
- When ready, convert the draft to a regular PR and request a review from a maintainer.
  If unsure, ask in PR comments.
  If you are a first-time contributor, mention a maintainer in a comment to request a review.
- If your PR gets a "Changes requested" review, address the feedback and push updates to the same branch.
  Do not close and reopen a new PR.
  Respond to comments to signal that you have addressed the feedback.
  Do not resolve review comments yourself; the reviewer will do so once satisfied.
- Re-request a review after pushing changes that address feedback.
- Do not squash commits locally; maintainers typically squash on merge.
  Avoid rebasing or force-pushing before reviews; you may rebase after addressing feedback if desired.

## Get Started 🎉

Ready to contribute?
We value contributions from people with all levels of experience.
In particular, if this is your first PR, not everything has to be perfect.
We will guide you through the process.

## Installation

Check out our {ref}`installation guide for developers <development-setup>` for instructions on how to set up your development environment.

## Working on the C++ Library

Building the project requires a C++20-capable [C++ compiler](https://en.wikipedia.org/wiki/List_of_compilers#C++_compilers) and [CMake](https://cmake.org/) 3.24 or newer.
As of August 2025, our CI pipeline on GitHub continuously tests the library across the following matrix of systems and compilers:

- {code}`ubuntu-24.04`: {code}`Release` and {code}`Debug` builds using {code}`gcc`
- {code}`ubuntu-24.04-arm`: {code}`Release` build using {code}`gcc`
- {code}`macos-15-intel`: {code}`Release` build using {code}`AppleClang`
- {code}`macos-14`: {code}`Release` and {code}`Debug` builds using {code}`AppleClang`
- {code}`windows-2022`: {code}`Release` and {code}`Debug` builds using {code}`msvc`
- {code}`windows-11-arm`: {code}`Release` build using {code}`msvc`

To access the latest build logs, visit the [GitHub Actions page](https://github.com/munich-quantum-toolkit/ddsim/actions/workflows/ci.yml).

Additionally, we regularly run extensive tests with an even wider matrix of compilers and operating systems.
We are not aware of any issues with other compilers or operating systems.
If you encounter any problems, please [open an issue][issues] and let us know.

### Configure and Build

:::{tip}
We recommend using an IDE like [CLion][clion] or [Visual Studio Code][vscode] for development.
Both IDEs have excellent support for CMake projects and provide a convenient way to run CMake and build the project.
If you prefer to work on the command line, the following instructions will guide you through the process.
:::

Our projects use CMake as the main build configuration tool.
Building a project using CMake is a two-stage process.
First, CMake needs to be _configured_ by calling:

```console
$ cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

This tells CMake to

- search the current directory {code}`.` (passed via {code}`-S`) for a {code}`CMakeLists.txt` file,
- process it into a directory {code}`build` (passed via {code}`-B`), and
- configure a {code}`Release` build (passed via {code}`-DCMAKE_BUILD_TYPE`) as opposed to, e.g., a {code}`Debug` build.

After configuring CMake, the project can be _built_ by calling:

```console
$ cmake --build build --config Release
```

This builds the project in the {code}`build` directory (passed via {code}`--build`).
Some operating systems and development environments explicitly require a configuration to be set, which is why the {code}`--config` flag is also passed to the build command.
The flag {code}`--parallel <NUMBER_OF_THREADS>` may be added to trigger a parallel build.

Building the project this way generates

- the main project libraries in the {code}`build/src` directory and
- some test executables in the {code}`build/test` directory.

:::{note}

This project uses CMake's [{code}`FetchContent`](https://cmake.org/cmake/help/latest/module/FetchContent.html) module to download and build its dependencies.
Because of this, the first time you configure the project, you will need an active internet connection to fetch the required libraries.

However, there are several ways to bypass these downloads:

- **Use system-installed dependencies**:
  If the dependencies are already installed on your system and Find-modules exist for them, {code}`FetchContent` will use those versions instead of downloading them.
- **Provide a local copy**:
  If you have local copies of the dependencies (from a previous build or another project), you can point {code}`FetchContent` to them by passing the [{code}`-DFETCHCONTENT_SOURCE_DIR_<uppercaseName>`](https://cmake.org/cmake/help/latest/module/FetchContent.html#variable:FETCHCONTENT_SOURCE_DIR_%3CuppercaseName%3E) flag to your CMake configure step.
  The {code}`<uppercaseName>` should be replaced with the name of the dependency as specified in the project's CMake files.
- **Use project-specific options**:
  Some projects provide specific CMake options to use a system-wide dependency instead of downloading it.
  Check the project's documentation or CMake files for these types of flags.

:::

### Running the C++ Tests and Code Coverage

We use the [GoogleTest](https://google.github.io/googletest/primer.html) framework for unit testing of the C++ library.
All tests are contained in the {code}`test` directory, which is further divided into subdirectories for different parts of the library.
You are expected to write tests for any new features you implement and ensure that all tests pass.
Our CI pipeline on GitHub will also run the tests and check for any failures.
It will also collect code coverage information and upload it to [Codecov](https://codecov.io/gh/munich-quantum-toolkit/ddsim).
Our goal is to have new contributions at least maintain the current code coverage level, while striving for covering as much of the code as possible.
Try to write meaningful tests that actually test the correctness of the code and not just exercise the code paths.

Most IDEs like [CLion][clion] or [Visual Studio Code][vscode] provide a convenient way to run the tests directly from the IDE.
If you prefer to run the tests from the command line, you can use CMake's test runner [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html).
To run the tests, run the following command from the main project directory after building the project as described above:

```console
$ ctest -C Release --test-dir build
```

:::{tip}
If you want to disable configuring and building the C++ tests, you can pass {code}`-DBUILD_MQT_DDSIM_TESTS=OFF` to the CMake configure step.
:::

### C++ Code Formatting and Linting

This project mostly follows the [LLVM Coding Standard](https://llvm.org/docs/CodingStandards.html), which is a set of guidelines for writing C++ code.
To ensure the quality of the code and that it conforms to these guidelines, we use:

- [`clang-tidy`](https://clang.llvm.org/extra/clang-tidy/), a static analysis tool that checks for common mistakes in C++ code, and
- [`clang-format`](https://clang.llvm.org/docs/ClangFormat.html), a tool that automatically formats C++ code according to a given style guide.

Common IDEs like [CLion][clion] or [Visual Studio Code][vscode] have plugins that can automatically run {code}`clang-tidy` on the code and automatically format it with {code}`clang-format`.

- If you are using CLion, you can configure the project to use the {code}`.clang-tidy` and {code}`.clang-format` files in the project root directory.
- If you are using Visual Studio Code, you can install the [clangd extension](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd).

They will automatically execute {code}`clang-tidy` on your code and highlight any issues.
In many cases, they also provide quick-fixes for these issues.
Furthermore, they provide a command to automatically format your code according to the given style.

:::{note}

After configuring CMake, you can run {code}`clang-tidy` on a file by calling the following command:

```console
$ clang-tidy <FILE> -- -I <PATH_TO_INCLUDE_DIRECTORY>
```

Here, {code}`<FILE>` is the file you want to analyze and {code}`<PATH_TO_INCLUDE_DIRECTORY>` is the path to the {code}`include` directory of the project.

:::

Our {code}`pre-commit` configuration also includes {code}`clang-format`.
If you have installed {code}`pre-commit`, it will automatically run {code}`clang-format` on your code before each commit.
If you do not have {code}`pre-commit` set up, the [pre-commit.ci](https://pre-commit.ci) bot will run {code}`clang-format` on your code and automatically format it according to the style guide.

:::{tip}
Remember to pull the changes back into your local repository after the bot has formatted your code to avoid merge conflicts.
:::

Our CI pipeline will also run {code}`clang-tidy` over the changes in your PR and report any issues it finds.
Due to technical limitations, the workflow can only post PR comments if the changes are not coming from a fork.
If you are working on a fork, you can still see the {code}`clang-tidy` results either in the GitHub Actions logs, on the workflow summary page, or in the "Files changed" tab of the PR.

### C++ Documentation

Historically, the C++ part of the code base has not been sufficiently documented.
Given the substantial size of the code base, we have set ourselves the goal to improve the documentation over time.
We expect any new additions to the code base to be documented using [Doxygen](https://www.doxygen.nl/index.html) comments.
When touching existing code, we encourage you to add Doxygen comments to the code you touch or refactor.

For some tips on how to write good Doxygen comments, see the [Doxygen Manual](https://www.doxygen.nl/manual/docblocks.html).

The C++ API documentation is integrated into the overall documentation that we host on ReadTheDocs using the [breathe](https://breathe.readthedocs.io/en/latest/) extension for Sphinx.
See {ref}`working-on-documentation` for more information on how to build the documentation.

## Working on the Python Package

We use [{code}`pybind11`](https://pybind11.readthedocs.io/en/stable) to expose large parts of the C++ core library to Python.
This allows us to keep the performance-critical parts of the code in C++ while providing a convenient interface for Python users.
All code related to C++-Python bindings is contained in the {code}`bindings` directory.

:::{tip}

To build only the Python bindings, pass {code}`-DBUILD_MQT_DDSIM_BINDINGS=ON` to the CMake configure step.
CMake will then try to find Python and the necessary dependencies ({code}`pybind11`) on your system and configure the respective targets.
In [CLion][clion], you can enable an option to pass the current Python interpreter to CMake.
Go to {code}`Preferences` -> {code}`Build, Execution, Deployment` -> {code}`CMake` -> {code}`Python Integration` and check the box {code}`Pass Python Interpreter to CMake`.
Alternatively, you can pass {code}`-DPython_ROOT_DIR=<PATH_TO_PYTHON>` to the configure step to point CMake to a specific Python installation.

:::

The Python package itself lives in the {code}`python/mqt/ddsim` directory.

The package lives in the {code}`src/mqt/ddsim` directory.

We recommend using [{code}`nox`][nox] for development.
{code}`nox` is a Python automation tool that allows you to define tasks in a {code}`noxfile.py` file and then run them with a single command.
If you have not installed it yet, see our {ref}`installation guide for developers <development-setup>`.

We define four convenient {code}`nox` sessions in our {code}`noxfile.py`:

- {code}`tests` to run the Python tests
- {code}`minimums` to run the Python tests with the minimum dependencies
- {code}`lint` to run the Python code formatting and linting
- {code}`docs` to build the documentation

These are explained in more detail in the following sections.

## Running the Python Tests

The Python code is tested by unit tests using the [{code}`pytest`](https://docs.pytest.org/en/latest/) framework.
The corresponding test files can be found in the {code}`test/python` directory.
A {code}`nox` session is provided to conveniently run the Python tests.

```console
$ nox -s tests
```

This command automatically builds the project and runs the tests on all supported Python versions.
For each Python version, it will create a virtual environment (in the {code}`.nox` directory) and install the project into it.
We take extra care to install the project without build isolation so that rebuilds are typically very fast.

If you only want to run the tests on a specific Python version, you can pass the desired Python version to the {code}`nox` command.

```console
$ nox -s tests-3.12
```

:::{note}

If you do not want to use {code}`nox`, you can also run the tests directly using {code}`pytest`.
This requires that you have the project and its test dependencies installed in your virtual environment (e.g., by running {code}`uv sync`).

```console
(.venv) $ pytest
```

:::

We provide an additional nox session {code}`minimums` that makes use of {code}`uv`'s {code}`--resolution=lowest-direct` flag to install the lowest possible versions of the direct dependencies.
This ensures that the project can still be built and the tests pass with the minimum required versions of the dependencies.

```console
$ nox -s minimums
```

## Python Code Formatting and Linting

The Python code is formatted and linted using a collection of [{code}`pre-commit`][pre-commit] hooks.
This collection includes

- [ruff][ruff], an extremely fast Python linter and formatter written in Rust, and
- [mypy][mypy], a static type checker for Python code.

The hooks can be installed by running the following command in the root directory:

```console
$ pre-commit install
```

This will install the hooks in the {code}`.git/hooks` directory of the repository.
The hooks will be executed whenever you commit changes.

You can also run the {code}`nox` session {code}`lint` to run the hooks manually.

```console
$ nox -s lint
```

:::{note}

If you do not want to use {code}`nox`, you can also run the hooks manually by using {code}`pre-commit`.

```console
$ pre-commit run --all-files
```

:::

## Python Documentation

The Python code is documented using [Google-style docstrings](https://google.github.io/styleguide/pyguide.html#s3.8-comments-and-docstrings).
Every public function, class, and module should have a docstring that explains what it does and how to use it.
{code}`ruff` will check for missing docstrings and will explicitly warn you if you forget to add one.

We heavily rely on [type hints](https://docs.python.org/3/library/typing.html) to document the expected types of function arguments and return values.
For the compiled parts of the code base, we provide type hints in the form of stub files in the {code}`python/mqt/ddsim` directory.

The Python API documentation is integrated into the overall documentation that we host on ReadTheDocs using the
[{code}`sphinx-autoapi`](https://sphinx-autoapi.readthedocs.io/en/latest/) extension for Sphinx.

(working-on-documentation)=

## Working on the Documentation

The documentation is written in [MyST](https://myst-parser.readthedocs.io/en/latest/index.html) (a flavor of Markdown) and built using [Sphinx](https://www.sphinx-doc.org/en/master/).
The documentation source files can be found in the {code}`docs/` directory.

On top of the API documentation, we provide a set of tutorials and examples that demonstrate how to use the library.
These are written in Markdown using [myst-nb](https://myst-nb.readthedocs.io/en/latest/), which allows executing Python code blocks in the documentation.
The code blocks are executed during the documentation build process, and the output is included in the documentation.
This allows us to provide up-to-date examples and tutorials that are guaranteed to work with the latest version of the library.

You can build the documentation using the {code}`nox` session {code}`docs`.

```console
$ nox -s docs
```

This will install all dependencies for building the documentation in an isolated environment, build the Python package, and then build the documentation.
It will then host the documentation on a local web server for you to view.

:::{note}

If you do not want to use {code}`nox`, you can also build the documentation directly using {code}`sphinx-build`.
This requires that you have the project and its documentation dependencies installed in your virtual environment (e.g., by running {code}`uv sync`).

```console
(.venv) $ sphinx-build -b html docs/ docs/_build
```

The docs can then be found in the {code}`docs/_build` directory.

:::

## Tips for Development

If something goes wrong, the CI pipeline will notify you.
Here are some tips for finding the cause of certain failures:

- If any of the {code}`CI / 🇨‌ Test` checks fail, this indicates build errors or test failures in the C++ part of the code base.
  Look through the respective logs on GitHub for any error or failure messages.
- If any of the {code}`CI / 🐍 Test` checks fail, this indicates build errors or test failures in the Python part of the code base.
  Look through the respective logs on GitHub for any error or failure messages.

- If any of the {code}`codecov/\*` checks fail, this means that your changes are not appropriately covered by tests or that the overall project coverage decreased too much.
  Ensure that you include tests for all your changes in the PR.

- If {code}`cpp-linter` comments on your PR with a list of warnings, these have been raised by {code}`clang-tidy` when checking the C++ part of your changes for warnings or style guideline violations.
  The individual messages frequently provide helpful suggestions on how to fix the warnings.
  If you don't see any messages, but the {code}`🇨‌ Lint / 🚨 Lint` check is red, click on the {code}`Details` link to see the full log of the check and a step summary.

- If the {code}`pre-commit.ci` check fails, some of the {code}`pre-commit` checks failed and could not be fixed automatically by the _pre-commit.ci_ bot.
  The individual log messages frequently provide helpful suggestions on how to fix the warnings.
- If the {code}`docs/readthedocs.org:\*` check fails, the documentation could not be built properly.
  Inspect the corresponding log file for any errors.

(maintaining-changelog-upgrade-guide)=

## Maintaining the Changelog and Upgrade Guide

MQT DDSIM adheres to [Semantic Versioning], with the exception that minor releases may include breaking changes.
To inform users about changes to the project, we maintain a {doc}`changelog <CHANGELOG>` and an {doc}`upgrade guide <UPGRADING>`.

If your PR includes noteworthy changes, please update the changelog.
The format is based on a mixture of [Keep a Changelog] and [Common Changelog].
There are the following categories:

- {code}`Added` for new features.
- {code}`Changed` for changes in existing functionality.
- {code}`Deprecated` for soon-to-be removed features.
- {code}`Removed` for now removed features.
- {code}`Fixed` for any bug fixes.
- {code}`Security` in case of vulnerabilities.

When updating the changelog, follow these guidelines:

- Add a changelog entry for every user-facing change in your PR.
- Write entries in the imperative mood (e.g., "Add support for X" or "Fix bug in Y").
- A single PR may result in multiple changelog entries.
- Entries in each category are sorted by merge time, with the latest PR appearing first.
- Each entry links to the PR and all contributing authors.
  The links are defined at the bottom of the file.
  If this is your first contribution to this project, do not forget to add a link to your GitHub profile.

If your PR introduces major or breaking changes, or if you think additional context would help users, please also add a section to the upgrade guide.
The upgrade guide is intended to provide a general overview of significant changes in a more descriptive and prose-oriented form than the changelog.
Use it to explain how users may need to adapt their usage of MQT DDSIM, highlight new workflows, or clarify the impact of important updates.
Feel free to write in a style that is helpful and accessible for users seeking to understand the broader implications of recent changes.

## Releasing a New Version

When it is time to release a new version of MQT DDSIM, create a PR that prepares the release.
This PR should:

- add new version titles in both the changelog and the upgrade guide,
- add the release date to the changelog entry for the new version,
- update the version links at the bottom of both files,
- review and streamline all changelog and upgrade guide entries for clarity and consistency,
- ensure that all links (to PRs, authors, etc.) are defined and correct,
- double-check that the changelog comprehensively covers all changes since the last release and that nothing is missing,
- review the upgrade guide to ensure it covers all major or breaking changes and provides helpful context, and
- if the upgrade guide contains a section relevant to the release, add a reference to it in the changelog.

Before merging the PR preparing the release, check the GitHub release draft generated by the Release Drafter for unlabelled PRs.
Unlabelled PRs would appear at the top of the release draft below the main heading.
If you missed updating labels before merging, you can still update them and re-run the Release Drafter afterward.
Furthermore, check whether the version number in the release draft is correct.
The version number in the release draft is dictated by the presence of certain labels on the PRs involved in a release.
By default, a patch release will be created.
If any PR has the {code}`minor` or {code}`major` label, a minor or major release will be created, respectively.

:::{note}

Sometimes, Dependabot or Renovate will tag a PR updating a dependency with a {code}`minor` or {code}`major` label because the dependency update itself is a minor or major release.
This does not mean that the dependency update itself is a breaking change for MQT DDSIM.
If you are sure that the dependency update does not introduce any breaking changes for MQT DDSIM, you can remove the {code}`minor` or {code}`major` label from the PR.
This will ensure that the respective PR does not influence the type of an upcoming release.

:::

Once everything is in order, you can merge the PR preparing the release.
Afterward, navigate to the [Releases page](https://github.com/munich-quantum-toolkit/ddsim/releases) on GitHub, edit the release draft if necessary, and publish the release.

<!--- Links --->

[clion]: https://www.jetbrains.com/clion/
[mypy]: https://mypy-lang.org/
[nox]: https://nox.thea.codes/en/stable/
[issues]: https://github.com/munich-quantum-toolkit/ddsim/issues
[pipx]: https://pypa.github.io/pipx/
[pre-commit]: https://pre-commit.com/
[ruff]: https://docs.astral.sh/ruff/
[uv]: https://docs.astral.sh/uv/
[vscode]: https://code.visualstudio.com/
[Keep a Changelog]: https://keepachangelog.com/en/1.1.0/
[Common Changelog]: https://common-changelog.org
[Semantic Versioning]: https://semver.org/spec/v2.0.0.html
