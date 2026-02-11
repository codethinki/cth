# Copilot Instructions

## General Guidelines
- Follow best practices for code readability and maintainability.
- Use concise and actionable language in comments and documentation.

## Code Style
- Adhere to the C++ code style found in the `cth_coro` directory:
  - No namespace indentation.
  - Attached braces for `else` and `while` statements.
  - Use compact short blocks and loops.
  - Set `ColumnLimit` to 110 characters in `.clang-format` instead of 120.
  - For function declarations, "chop" arguments (one per line) if they wrap, with the closing parenthesis on a new line (`AlignAfterOpenBracket: BlockIndent`). Avoid parameter alignment.
  - Set `AllowShortBlocksOnASingleLine` to `Empty`, allowing empty braces in macros and blocks to be fused (e.g., `MACRO() {}`), while strictly splitting multiline/non-empty blocks.

## Project-Specific Rules
- Ensure that all code adheres to the specified formatting rules and conventions.