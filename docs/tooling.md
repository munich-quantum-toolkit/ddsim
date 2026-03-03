<!--- This file has been generated from an external template. Please do not modify it directly. -->
<!--- Changes should be contributed to https://github.com/munich-quantum-toolkit/templates. -->

# Tooling

This page summarizes the main tools, software, and standards used in MQT DDSIM.
It serves as a quick reference for new contributors and users who want to understand the project's ecosystem.

## C++

| Tool             | Description                          | Links / Notes                                                                                                 |
| ---------------- | ------------------------------------ | ------------------------------------------------------------------------------------------------------------- |
| **CMake**        | Build system.                        | [Documentation](https://cmake.org/).                                                                          |
| **clang-format** | Code formatter (LLVM style).         | [Documentation](https://clang.llvm.org/docs/ClangFormat.html). Config: {code}`.clang-format` in project root. |
| **clang-tidy**   | Static analysis and linting for C++. | [Documentation](https://clang.llvm.org/extra/clang-tidy/). Config: {code}`.clang-tidy` in project root.       |
| **Doxygen**      | C++ API documentation (comments).    | [Documentation](https://www.doxygen.nl/). Rendered in Sphinx via [breathe](https://breathe.readthedocs.io/).  |
| **GoogleTest**   | C++ unit testing.                    | [Primer](https://google.github.io/googletest/primer.html). Tests in {code}`test/`; run via CTest.             |

## C++/Python Bindings and Packaging

| Tool                  | Description                                                    | Links / Notes                                                                                                                             |
| --------------------- | -------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------- |
| **nanobind**          | C++/Python bindings (exposes C++ library to Python).           | [Documentation](https://nanobind.readthedocs.io/). Bindings live in {code}`bindings`; stubs are auto-generated (see {doc}`contributing`). |
| **scikit-build-core** | Build backend for Python package.                              | [Documentation](https://scikit-build-core.readthedocs.io/en/latest/).                                                                     |
| **cibuildwheel**      | Builds wheels for all supported platforms and Python versions. | [Documentation](https://cibuildwheel.pypa.io/en/stable/). Configured in {code}`pyproject.toml`.                                           |

By using nanobind, we can take advantage of the [Stable ABI](https://docs.python.org/3/c-api/stable.html) for Python 3.12+.
This means that, starting from Python 3.12, we only need to build one wheel per platform, which can be used across all Python 3.12+ versions.
We still build separate wheels for older supported Python versions.

Additionally, we support the free-threading version of Python that is no longer marked experimental as of Python 3.14.
The corresponding wheels are built separately since there is no stable ABI for free-threading Python yet.

## Python

| Tool       | Description                                                                              | Links / Notes                                                                                                                                          |
| ---------- | ---------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------ |
| **uv**     | Fast Python package and project manager (install, venv, dependencies).                   | [Documentation](https://docs.astral.sh/uv/). Recommended over {code}`pip` for installs and development.                                                |
| **nox**    | Task automation for tests, lint, docs, and other sessions defined in {code}`noxfile.py`. | [Documentation](https://nox.thea.codes/en/stable/). Run sessions with {code}`nox -s <session>`.                                                        |
| **prek**   | Runs hooks (formatting, linting) before each commit.                                     | [Documentation](https://prek.j178.dev). Install and run via {code}`prek install`, {code}`prek run` (staged files), or {code}`prek run -a` (all files). |
| **ruff**   | Linter and formatter for Python, written in Rust.                                        | [Documentation](https://docs.astral.sh/ruff/). Used in prek and CI.                                                                                    |
| **ty**     | Fast Python type checker and language server.                                            | [Documentation](https://docs.astral.sh/ty/).                                                                                                           |
| **pytest** | Testing framework for Python.                                                            | [Documentation](https://docs.pytest.org/). Run via {code}`nox -s tests` or {code}`uv run pytest`.                                                      |

The project adheres to modern standards and practices. For the Python ecosystem, we make use of the following standards:

| Standard    | Description                                                     | Links / Notes                                       |
| ----------- | --------------------------------------------------------------- | --------------------------------------------------- |
| **PEP 8**   | Style guide for Python code.                                    | [Documentation](https://peps.python.org/pep-0008/). |
| **PEP 518** | Specifying build system requirements in {code}`pyproject.toml`. | [Documentation](https://peps.python.org/pep-0518/). |
| **PEP 621** | Storing project metadata in {code}`pyproject.toml`.             | [Documentation](https://peps.python.org/pep-0621/). |
| **PEP 639** | Standardized license metadata in {code}`pyproject.toml`.        | [Documentation](https://peps.python.org/pep-0639/). |
| **PEP 723** | Inline script metadata for efficient script execution.          | [Documentation](https://peps.python.org/pep-0723/). |
| **PEP 735** | Dependency groups in {code}`pyproject.toml`.                    | [Documentation](https://peps.python.org/pep-0735/). |

## Documentation

| Tool       | Description                                | Links / Notes                                                                               |
| ---------- | ------------------------------------------ | ------------------------------------------------------------------------------------------- |
| **Sphinx** | Documentation generator.                   | [Documentation](https://www.sphinx-doc.org/). Docs source in {code}`docs/`.                 |
| **MyST**   | Markdown flavor for Sphinx (used in docs). | [Documentation](https://myst-parser.readthedocs.io/). Enables rich Markdown in doc sources. |

## CI and Quality

| Tool               | Description                                 | Links / Notes                                                          |
| ------------------ | ------------------------------------------- | ---------------------------------------------------------------------- |
| **GitHub Actions** | CI workflows (build, test, lint, coverage). | [Reusable MQT Workflows] in {code}`.github/workflows/`; see [Actions]. |
| **Codecov**        | Code coverage reporting.                    | [Codecov] for this repo.                                               |
| **CodeRabbit**     | Automated code review on PRs.               | [CodeRabbit](https://www.coderabbit.ai/). See {doc}`contributing`.     |
| **pre-commit.ci**  | Runs pre-commit in CI and can auto-fix.     | [pre-commit.ci](https://pre-commit.ci).                                |

[Actions]: https://github.com/munich-quantum-toolkit/ddsim/actions
[Codecov]: https://codecov.io/gh/munich-quantum-toolkit/ddsim
[Reusable MQT Workflows]: https://github.com/munich-quantum-toolkit/workflows
