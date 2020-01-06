# #{projectName}

## Getting Started

Install the dependencies for this project:

```sh
yarn install
```<% if (isHTTPS) { %>

## HTTPS

This project relies on [`mkcert`](https://github.com/FiloSottile/mkcert) to install self-signed development certificates.
Make sure you have it installed for your system before generating certificates.

Generate certificates for local HTTPS:

```sh
yarn gen-certs
```<% } %>

## Usage

To build a bundled version of the `knot` app:

```sh
yarn build
```

To run a local dev server at `http<% if (isHTTPS) { %>s<% } %>://localhost:1337`:

```sh
yarn start
```
