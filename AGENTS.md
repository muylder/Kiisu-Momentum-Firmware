# AI Agent Policy

This document defines how AI may be used in this repository. It applies to anyone using an AI agent to contribute, and to the AI agents themselves.

## 1. General Policy

AI is allowed in this repository, but must follow the rules below.

## 2. Code and Assets

- AI-generated code is allowed.
- AI-generated assets are **not** allowed. This includes images, artwork, and icons. All assets must be created or sourced by a human.

## 3. Committing

Before committing any change, an AI agent must show the user the generated code and ask whether they have reviewed it.

- If the user does not respond, or says "no": the AI agent must **not** commit.
- If the user says "yes": the AI agent may commit. The commit message must start with the `[AI]` tag:

  ```
  [AI] <commit message content>
  ```

## 4. Pull Requests

When an AI agent opens a pull request:

- The **title** must start with the `[AI]` tag, followed by the agent's name and a short description:

  ```
  [AI] HAL: Code refactor
  ```

- The **end of the description** must disclose the AI's identity, in this format:

  ```
  <AI name> | <Company> | <Model> | <Client>
  ```

  Example:

  ```
  Claude | Anthropic | Claude Sonnet 4.6 | Claude Code
  ```

## 5. Maintainers

Maintainers must not use AI to:

- Review pull requests.
- Review or commit their own commits.

All maintainer reviews and maintainer commits must be done by a human, without AI assistance.
